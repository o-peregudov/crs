/*
 *  General mass spectrum presentation database
 *
 *  crs/msdc/ring_pool.cpp
 *  Copyright (c) 2007-2012 Oleg N. Peregudov <o.peregudov@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif
#include <crs/msdc/ring_pool.h>
#include <iostream>
#include <iomanip>

namespace msdc {

//
// members of class ring_pool::bit_mask_calculator
//
ring_pool::bit_mask_calculator::bit_mask_calculator ( const size_t desiredPoolSize, const size_t desiredRootSize )
	: nPoolSize( 0 )
	, poolMask( 0 )
	, nRootSize( 1 )
	, rootMask( 0 )
	, usedBits( 0 )
	, rootMaskShift( 0 )
{
	size_t bit;
	for( int bitNo = std::numeric_limits<size_t>::digits; bitNo >= std::numeric_limits<unsigned char>::digits; --bitNo )
	{
		bit = ( static_cast<size_t>( 0x01 ) << ( bitNo - 1 ) );
		if( ( desiredPoolSize & bit ) == bit )
		{
			poolMask = ( bit | ( bit - 1 ) );
			rootMaskShift = bitNo;
			break;
		}
	}
	
	if( rootMaskShift == 0 )
	{
		bit = ( static_cast<size_t>( 0x01 ) << ( std::numeric_limits<unsigned char>::digits - 1 ) );
		poolMask = ( bit | ( bit - 1 ) );
		rootMaskShift = std::numeric_limits<unsigned char>::digits;
	}
	nPoolSize = poolMask + 1;
	
	size_t rootDigits ( std::numeric_limits<size_t>::digits - rootMaskShift - 1 );
	if( std::numeric_limits<size_t>::digits == rootMaskShift )
		rootDigits = 0;
	else
	{
		bit = ( static_cast<size_t>( 0x01 ) << rootDigits );
		nRootSize = ( bit | ( bit - 1 ) );
		if( desiredRootSize < nRootSize )
		{
			for( int bitNo = rootDigits; bitNo; --bitNo )
			{
				bit = ( static_cast<size_t>( 0x01 ) << ( bitNo - 1 ) );
				if( ( desiredRootSize & bit ) == bit )
				{
					nRootSize = ( bit | ( bit - 1 ) );
					break;
				}
			}
		}
		rootMask = nRootSize;
		++nRootSize;
	}
	
	usedBits = poolMask + ( rootMask << rootMaskShift );
}

//
// members of class ring_pool::ring_pool_swaper
//

ring_pool::ring_pool_swaper::ring_pool_swaper ( ring_pool * srp )
	: CrossClass::cThread()
	, rp( srp )
{ }

ring_pool::ring_pool_swaper::~ring_pool_swaper ()
{
	kill();
}

bool ring_pool::ring_pool_swaper::Step ()
{
	Stop();		// post deactivate condition
	rp->_swap();	// swap memory pool
	return false;	// always continue
}

//
// members of class ring_pool
//

ring_pool::ring_pool ( const size_t nData, const size_t nMemPools, const size_t nPoolSize, const size_t nRootSize )
	: _nData( nData )
	, _metrix( nPoolSize, nRootSize )
	, _rootMutex( )
	, _iterMutex( )
	, _root( )
	, _currentPool( 0 )
	, _nPool2Swap( 0 )
	, _nPool2Swap2( _metrix.nRootSize < nMemPools ? _metrix.nRootSize : nMemPools )
	, _beginPos( 0 )
	, _endPos( 0 )
	, _callBackMutex( )
	, _callBack( 0 )
	, _callBackData( 0 )
	, _thread( )
	, _semaphore( )
{
	if( _nPool2Swap2 < 2 )
		_nPool2Swap2 = 2;
	try
	{
		_root.reserve( _metrix.nRootSize );
		for( size_t i = 0; i < _metrix.nRootSize; ++i )
		{
			if( i < _nPool2Swap2 )
				_root.push_back( new mem_pool ( _nData, _metrix.nPoolSize ) );
			else
				_root.push_back( new file_pool ( _nData, _metrix.nPoolSize ) );
		}
	}
	catch( ... )
	{
		std::for_each( _root.begin(), _root.end(), poolReleaser() );
		_root.clear();
		throw;
	}
	_thread = new ring_pool_swaper ( this );
	_semaphore = new CrossClass::cSemaphore (_nPool2Swap2 - 1);
}

ring_pool::~ring_pool ()
{
	_thread = 0;	/* this will cause deallocation */
	_semaphore = 0;	/* this will also cause deallocation */
	std::for_each( _root.begin(), _root.end(), poolReleaser() );
	_root.clear();
}

void ring_pool::_release ( const cIntensityPoint & )
{
	// reserved for future use
}

void ring_pool::_fixLink ( const cIntensityPoint &, const size_t newPos )
{
	// reserved for future use
}

void ring_pool::_swap ()
{
	if( increment_begin_iter() )
	{
		size_t nBeginPos = _metrix.full_idx( _nPool2Swap2 << _metrix.rootMaskShift ),
			 nEndPos = _metrix.full_idx( ( _nPool2Swap2 + 1 ) << _metrix.rootMaskShift );
		iterator i ( this, nBeginPos ),
			   e ( this, nEndPos );
		for( basic_ipoint::link_type lnk = (*i)->link; i != e; ++i )
		{
			if( (*i)->link != lnk )
			{
				lnk = (*i)->link;
				_release( *i );
			}
		}
		_fixLink( *e, nEndPos );
		notifyValidateIterators();
	}
	mem_pool * mp = dynamic_cast<mem_pool *>( get_pool( _nPool2Swap ) );
	file_pool * fp = dynamic_cast<file_pool *>( get_pool( _nPool2Swap2 ) );
	fp->assign( *mp );
	{
		CrossClass::_LockIt lockRootMutex ( _rootMutex );
		std::swap( _root[ _nPool2Swap ], _root[ _nPool2Swap2 ] );
		_root[ _nPool2Swap2 ]->clear();
		(++_nPool2Swap2) &= _metrix.rootMask;
		(++_nPool2Swap) &= _metrix.rootMask;
	}
	_semaphore->unlock();
}

void ring_pool::push ( const cIntensityPoint & point )
{
	basic_pool * pool = get_pool( _currentPool );
	if( pool->push( point ) )
	{
		(++_currentPool) &= _metrix.rootMask;
		_semaphore->lock();	// wait until swap job is done
		_thread->Resume();	// rise up swap thread
	}
	CrossClass::_LockIt lockIterMutex ( _iterMutex );
	(++_endPos) &= _metrix.usedBits;
}

bool ring_pool::validate ( iterator & it ) const
{
	if( empty() )
		return false;
	else
	{
		CrossClass::_LockIt lockIterMutex ( _iterMutex );
		size_t bpos = _metrix.full_idx( _beginPos - _endPos ),
			 ipos = _metrix.full_idx( it.index - _endPos );
		if( ( ipos > 0 ) && ( ipos < bpos ) )
		{
			it = iterator( this, _beginPos );
			return true;
		}
		else
			return false;
	}
}

int ring_pool::size () const
{
	CrossClass::_LockIt lockIterMutex ( _iterMutex );
	return _metrix.full_idx( _endPos - _beginPos );
}

bool ring_pool::empty () const
{
	CrossClass::_LockIt lockIterMutex ( _iterMutex );
	return ( _beginPos == _endPos );
}

ring_pool::iterator ring_pool::begin () const
{
	CrossClass::_LockIt lockIterMutex ( _iterMutex );
	return iterator( this, _beginPos );
}

ring_pool::iterator ring_pool::recent () const
{
	if( empty() )
	{
		CrossClass::_LockIt lockIterMutex ( _iterMutex );
		return iterator( this, _endPos );
	}
	else
	{
		CrossClass::_LockIt lockIterMutex ( _iterMutex );
		return iterator( this, _endPos - 1 );
	}
}

ring_pool::iterator ring_pool::end () const
{
	CrossClass::_LockIt lockIterMutex ( _iterMutex );
	return iterator( this, _endPos );
}

size_t ring_pool::_getSafeBeginPos () const
{
	CrossClass::_LockIt lockIterMutex ( _iterMutex );
	return _beginPos;
}

size_t ring_pool::_getSafeEndPos () const
{
	CrossClass::_LockIt lockIterMutex ( _iterMutex );
	return _endPos;
}

double ring_pool::getIntensityByTime ( const size_t nChannel, const basic_ipoint::time_type & time ) const
{
	iterator ub = upper_bound( time ),
		   lb = lower_bound( time );
	if( ( time > (*lb)->time ) && ( lb != begin() ) )
		--lb;
	return ( (*ub)->intensity[ nChannel ] + (*lb)->intensity[ nChannel ] ) / 2.0;
}

} /* namespace msdc	*/
