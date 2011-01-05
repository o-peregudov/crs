//
// composite_core.cpp: general mass spectrum representation data base
// (c) Dec 20, 2007 Oleg N. Peregudov
//	09/07/2010	conforming libcrs v1.0.x
//	12/07/2010	included into libcrs
//	01/03/2011	integer types
//
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4351 )
#endif

#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif

#include <crs/msdc/composite_core.h>

namespace msdc {

void cDAQCore::_release ( const cIntensityPoint & ip )
{
	pFieldStateRec rec2Release = reinterpret_cast<pFieldStateRec>( ip->link );
	if( rec2Release )
	{
		CrossClass::_LockIt lockFieldIndex ( _fieldIndexMutex );
		for( iPair iRange = _fieldIndex.equal_range( rec2Release ); iRange.first != iRange.second; ++iRange.first )
		{
			if( *iRange.first == rec2Release )
			{
				_fieldIndex.erase( iRange.first );
				break;
			}
		}
		delete rec2Release;
	}
}

void cDAQCore::_fixLink ( const cIntensityPoint & ip, const size_t newPos )
{
	CrossClass::_LockIt lockFieldIndex ( _fieldIndexMutex );
	pFieldStateRec rec2Release = reinterpret_cast<pFieldStateRec>( ip->link );
	if( rec2Release )
		rec2Release->link = newPos;
}

cDAQCore::cDAQCore ( const size_t nData, const size_t nMemPools, const size_t nPoolSize, const size_t nRootSize )
	: ring_pool( nData, nMemPools, nPoolSize, nRootSize )
	, _field( 0 )
	, _fieldIndex()
	, _newFieldState( false )
	, _fieldIndexMutex( )
{ }

cDAQCore::~cDAQCore ()
{
	std::for_each( _fieldIndex.begin(), _fieldIndex.end(), fsReleaser() );
	_fieldIndex.clear();
	
	if( ( _field != 0 ) && _newFieldState )
		delete _field;
}

void cDAQCore::field ( const cFieldState & newState )
{
	if( ( _field == 0 ) || ( *_field != newState ) )
	{
		if( _newFieldState )
			delete _field;
		_field = new cFieldStateRec( newState );
		_newFieldState = true;
	}
}

void cDAQCore::acquire ( const cIntensityPoint & data_point )
{
	cIntensityPoint fullPoint;
	fullPoint.linkWith( data_point );
	fullPoint->link = _field;
	push( fullPoint );
	
	if( _newFieldState )
	{
		CrossClass::_LockIt lockFieldIndex ( _fieldIndexMutex );
		--(_field->link = _getSafeEndPos());
		_fieldIndex.insert( _field );
		_newFieldState = false;
	}
}

size_t cDAQCore::nFieldIndexes () const
{
	CrossClass::_LockIt lockFieldIndex ( _fieldIndexMutex );
	return _fieldIndex.size();
}

cDAQCore::mass_iter cDAQCore::convert ( time_iter & ti ) const
{
	CrossClass::_LockIt lockFieldIndex ( _fieldIndexMutex );
	pFieldStateRec rec2find = reinterpret_cast<pFieldStateRec>( (*ti)->link );
	for( ciPair iRange = _fieldIndex.equal_range( rec2find ); iRange.first != iRange.second; ++iRange.first )
	{
		if( *iRange.first == rec2find )
			return iRange.first;
	}
	return _fieldIndex.end();
}

cDAQCore::time_iter cDAQCore::convert ( const mass_iter & mi ) const
{
	CrossClass::_LockIt lockFieldIndex ( _fieldIndexMutex );
	return _make_iter( (*mi)->link );
}

double cDAQCore::getIntensityByMass ( const size_t nChannel, const cFieldState::mass_type & mass ) const
{
	time_iter mti;
	cFieldStateRec rec2find ( static_cast<cFieldState::pilot_type>( nChannel ), mass );
	
	CrossClass::_LockIt lockFieldIndex ( _fieldIndexMutex );
	ciPair iRange = _fieldIndex.equal_range( &rec2find );
	if( iRange.first == iRange.second )
	{
		if( iRange.first == _fieldIndex.end() )
			throw notFound ();
		
		// the first point from the top
		mti = _make_iter( (*iRange.first)->link );
	}
	else
	{
		time_iter ti;
		mti = _make_iter( (*iRange.first)->link );
		for( ++iRange.first; iRange.first != iRange.second; ++iRange.first )
		{
			ti = _make_iter( (*iRange.first)->link );
			if( mti < ti )
				mti = ti;
		}
	}
	return (*mti)->intensity[ nChannel ];
}

} // namespace msdc
