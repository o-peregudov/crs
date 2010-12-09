#ifndef CROSS_MSDC_RING_POOL_H
#define CROSS_MSDC_RING_POOL_H 1
//
// ring_pool.h: general mass spectrum representation data base
// (c) Jan 8, 2008 Oleg N. Peregudov
//	09/07/2010	conforming libcrs v1.0.x
//	09/11/2010	thread safety revision
//	09/14/2010	check for empty iterator in iterator::operator =
//			iterator observers bug fixed
//	09/20/2010	swap thread
//			notify validate iterators callback
//	12/07/2010	included into libcrs
//

#include <crs/msdc/pools.h>
#include <crs/thread.h>
#include <crs/callback.h>
#include <crs/semaphore.hpp>
#include <algorithm>
#include <vector>
#include <limits>
#include <memory>

namespace msdc {

//
// class ring_pool (concrete)
//
// indexing is based on a simple shift of binary digits
//
//	IMPORTANT NOTE:
//		before any iterator validation call ring_pool::begin()
//
class CROSS_EXPORT ring_pool
{
protected:
	struct CROSS_EXPORT bit_mask_calculator
	{
		size_t	nPoolSize,
				poolMask,
				nRootSize,
				rootMask,
				usedBits;
		int		rootMaskShift;
		
		bit_mask_calculator ( const size_t desiredPoolSize, const size_t desiredRootSize );
		
		size_t root_idx ( const size_t idx ) const
		{
			return ( ( idx >> rootMaskShift ) & rootMask );
		}
		
		size_t pool_idx ( const size_t idx ) const
		{
			return ( idx & poolMask );
		}
		
		size_t full_idx ( const size_t idx ) const
		{
			return ( idx & usedBits );
		}
	};
	
	struct poolReleaser
	{
		void operator () ( basic_pool * bp )
		{
			delete bp;
		}
	};
	
	typedef std::vector<basic_pool*> root_type;
	
	const size_t			_nData;
	const bit_mask_calculator	_metrix;
	mutable CrossClass::LockType	_rootMutex,
						_iterMutex;
	
	root_type	_root;
	size_t	_currentPool,
			_nPool2Swap,
			_nPool2Swap2,
			_beginPos,
			_endPos;
	
	CrossClass::LockType	_callBackMutex;
	callBackFunction		_callBack;
	void *			_callBackData;
	
	class CROSS_EXPORT ring_pool_swaper : public CrossClass::cThread
	{
	protected:
		ring_pool * rp;
		virtual bool Step ();
		
	public:
		ring_pool_swaper ( ring_pool * mrp );
		virtual ~ring_pool_swaper ();
	};
	
	std::auto_ptr<ring_pool_swaper>		_thread;
	std::auto_ptr<CrossClass::cSemaphore>	_semaphore;
	
protected:
	cIntensityPoint * allocateRecord () const
	{
		return new cIntensityPoint( _nData );
	}
	
	void	idx ( const size_t i, cIntensityPoint & r ) const
	{
		CrossClass::_LockIt lockRootMutex ( _rootMutex );
		_root[ _metrix.root_idx( i ) ]->get( _metrix.pool_idx( i ), r );
	}
	
	basic_pool * get_pool ( const size_t nPool )
	{
		CrossClass::_LockIt lockRootMutex ( _rootMutex );
		return _root[ nPool & _metrix.rootMask ];
	}
	
	bool	increment_begin_iter ()
	{
		CrossClass::_LockIt lockIterMutex ( _iterMutex );
		size_t nStartPool = _metrix.root_idx( _beginPos );
		if( _nPool2Swap2 == nStartPool )
		{
			_beginPos = ( ( ( _nPool2Swap2 + 1 ) & _metrix.rootMask ) << _metrix.rootMaskShift );
			return true;
		}
		else
			return false;
	}
	
	void	notifyValidateIterators ()
	{
		CrossClass::_LockIt lockCallBackMutex ( _callBackMutex );
		if( _callBack )
			_callBack( _callBackData );
	}
	
	virtual void _release ( const cIntensityPoint & );
	virtual void _fixLink ( const cIntensityPoint &, const size_t newPos );
	virtual void _swap ();
	
public:
	// forward definition
	class iterator;
	friend class iterator;
	
	class iterator : public std::iterator<std::random_access_iterator_tag, const cIntensityPoint>
	{
	private:
		friend class ring_pool;
		
		const ring_pool * pool;
		cIntensityPoint * data;
		size_t		index;
		bool			invalid;
		
		void	setPos ( const size_t newPos )
		{
			index = newPos;
			invalid = true;
		}
		
		bool	mustRead () const
		{
			return invalid;
		}
		
		size_t getSafeIdx () const
		{
			return index;
		}
		
		iterator ( const ring_pool * container, const size_t position )
			: pool( container )
			, data( pool->allocateRecord() )
			, index( position )
			, invalid( true )
		{ }
		
	public:
		iterator ()
			: pool( 0 )
			, data( 0 )
			, index( 0 )
			, invalid( true )
		{ }
		
		iterator ( const iterator & o )
			: pool( o.pool )
			, data( pool->allocateRecord() )
			, index( o.index )
			, invalid( true )
		{ }
		
		~iterator ()
		{
			if( data )
				delete data;
		}
		
		iterator & operator = ( const iterator & o )
		{
			if( &o != this )
			{
				if( data )
					delete data;
				pool = o.pool;
				if( pool )
				{
					data = pool->allocateRecord();
					index = o.getSafeIdx();
				}
				else
				{
					data = 0;
					index = 0;
				}
				invalid = true;
			}
			return *this;
		}
		
		reference operator * ()
		{
			if( mustRead() )
			{
				pool->idx( index, *data );
				invalid = false;
			}
			return static_cast<reference>( *data );
		}
		
		pointer operator -> ()
		{
			if( mustRead() )
			{
				pool->idx( index, *data );
				invalid = false;
			}
			return data;
		}
		
		iterator & operator ++ ()
		{
			invalid = true;
			++index;
			return *this;
		}
		
		iterator & operator -- ()
		{
			invalid = true;
			--index;
			return *this;
		}
		
		iterator operator ++ ( int )
		{
			iterator tmp = *this;
			++(*this);
			return tmp;
		}
		
		iterator operator -- ( int )
		{
			iterator tmp = *this;
			--(*this);
			return tmp;
		}
		
		iterator & operator += ( const int i )
		{
			invalid = true;
			index += i;
			return *this;
		}
		
		iterator & operator -= ( const int i )
		{
			invalid = true;
			index -= i;
			return *this;
		}
		
		iterator operator + ( const int i ) const
		{
			return iterator( pool, getSafeIdx() + i );
		}
		
		iterator operator - ( const int i ) const
		{
			return iterator( pool, getSafeIdx() - i );
		}
		
		int	operator - ( const iterator & o ) const
		{
			size_t m = pool->_getSafeBeginPos(),
				 l = pool->_metrix.full_idx( getSafeIdx() - m ),
				 r = pool->_metrix.full_idx( o.getSafeIdx() - m );
			return static_cast<int>( l - r );
		}
		
		bool	operator == ( const iterator & o ) const
		{
			return pool->_metrix.full_idx( getSafeIdx() ) == pool->_metrix.full_idx( o.getSafeIdx() );
		}
		
		bool	operator != ( const iterator & o ) const
		{
			return pool->_metrix.full_idx( getSafeIdx() ) != pool->_metrix.full_idx( o.getSafeIdx() );
		}
		
		bool	operator < ( const iterator & o ) const
		{
			size_t m = pool->_getSafeBeginPos(),
				 l = pool->_metrix.full_idx( getSafeIdx() - m ),
				 r = pool->_metrix.full_idx( o.getSafeIdx() - m );
			return ( l < r );
		}
		
		bool	operator > ( const iterator & o ) const
		{
			size_t m = pool->_getSafeBeginPos(),
				 l = pool->_metrix.full_idx( getSafeIdx() - m ),
				 r = pool->_metrix.full_idx( o.getSafeIdx() - m );
			return ( l > r );
		}
		
		bool	operator <= ( const iterator & o ) const
		{
			return !operator>( o );
		}
		
		bool	operator >= ( const iterator & o ) const
		{
			return !operator<( o );
		}
		
		operator const void * () const
		{
			return reinterpret_cast<const void *>( pool->_metrix.full_idx( getSafeIdx() ) );
		}
		
		bool operator ! () const
		{
			return ( pool == 0 );
		}
		
		operator const bool () const
		{
			return ( pool != 0 );
		}
	};
	
protected:
	iterator _make_iter ( const size_t position ) const
	{
		return iterator( this, position );
	}
	
	size_t _position ( const iterator & i ) const
	{
		return _metrix.full_idx( i.index );
	}
	
	size_t _getSafeBeginPos () const;
	size_t _getSafeEndPos () const;
	
public:
	ring_pool ( const size_t nData,
			const size_t nMemPools = 2,
			const size_t nPoolSize = 0x00FFFF,
			const size_t nRootSize = 0x3F );
	~ring_pool ();
	
	void	push ( const cIntensityPoint & );
	bool	validate ( iterator & ) const;
	
	bool	empty () const;
	int	size () const;
	
	size_t capacity () const
	{
		return ( _metrix.usedBits + 1 );
	}
	
	double usage () const
	{
		return static_cast<double>( size() ) / capacity();
	}
	
	iterator lower_bound ( const basic_ipoint::time_type & time ) const {
		return std::lower_bound( begin(), end(), cIntensityPoint( 1, time ) );
	}
	
	iterator upper_bound ( const basic_ipoint::time_type & time ) const {
		return std::upper_bound( begin(), end(), cIntensityPoint( 1, time ) );
	}
	
	iterator begin () const;
	iterator recent () const;
	iterator end () const;
	
	double getIntensityByTime ( const size_t nChannel, const basic_ipoint::time_type & time ) const;
	
	void	setCallBack ( callBackFunction fnct, void * data )
	{
		CrossClass::_LockIt lockCallBackMutex ( _callBackMutex );
		_callBack = fnct;
		_callBackData = data;
	}
};

} // namespace msdc
#endif // CROSS_MSDC_RING_POOL_H
