#ifndef CROSS_MSDC_COMPOSITE_CORE_H
#define CROSS_MSDC_COMPOSITE_CORE_H 1
//
// composite_core.h: general mass spectrum representation data base
// (c) Dec 18, 2007 Oleg N. Peregudov
//	09/07/2010	conforming libcrs v1.0.x
//	12/07/2010	included into libcrs
//

#include <crs/msdc/ring_pool.h>
#include <set>

namespace msdc {

//
// class cBuildPoint
//
struct cBuildPoint : cFieldState, cIntensityPoint
{
	cBuildPoint ()
		: cFieldState()
		, cIntensityPoint()
	{ }
	
	cBuildPoint ( const cFieldState & o )
		: cFieldState( o )
		, cIntensityPoint()
	{ }
	
	cBuildPoint & operator = ( const cFieldState & o )
	{
		cFieldState::operator = ( o );
		return *this;
	}
	
	cBuildPoint & operator = ( const cIntensityPoint & o )
	{
		cIntensityPoint::operator = ( o );
		return *this;
	}
};

//
// class cFieldStateRec
//
struct cFieldStateRec : cFieldState
{
	typedef size_t link_type;
	
	link_type link;
	
	cFieldStateRec ()
		: cFieldState()
		, link( 0 )
	{ }
	
	cFieldStateRec ( const pilot_type & p, const mass_type & m, const link_type & l = 0 )
		: cFieldState( p, m )
		, link( l )
	{ }
	
	cFieldStateRec ( const cFieldState & fs, const link_type & l = 0 )
		: cFieldState( fs )
		, link( l )
	{ }
	
	bool	operator < ( const cFieldStateRec & o ) const
	{
		return cFieldState::operator < ( o );
	}
};

//
// class cDAQCore (concrete)
//
class CROSS_EXPORT cDAQCore : public ring_pool
{
protected:
	typedef cFieldStateRec * pFieldStateRec;
	
	struct fsComparator : std::binary_function<pFieldStateRec, pFieldStateRec, bool>
	{
		bool operator () ( pFieldStateRec a, pFieldStateRec b ) const
		{
			return ( *a < *b );
		}
	};
	
	struct fsReleaser
	{
		void operator () ( pFieldStateRec fsr ) const
		{
			delete fsr;
		}
	};
	
	typedef std::multiset<pFieldStateRec, fsComparator> cFieldStateIDX;
	typedef std::pair<cFieldStateIDX::iterator, cFieldStateIDX::iterator> iPair;
	typedef std::pair<cFieldStateIDX::const_iterator, cFieldStateIDX::const_iterator> ciPair;
	
	virtual void _release ( const cIntensityPoint & );
	virtual void _fixLink ( const cIntensityPoint &, const size_t newPos );
	
	cFieldStateRec *	_field;
	cFieldStateIDX	_fieldIndex;
	bool			_newFieldState;
	
	mutable CrossClass::LockType _fieldIndexMutex;
	
public:
	struct notFound { };
	
	typedef ring_pool::iterator time_iter;
	typedef cFieldStateIDX::const_iterator mass_iter;
	
	cDAQCore (	const size_t nData,
			const size_t nMemPools = 2,
			const size_t nPoolSize = 0x00FFFF,
			const size_t nRootSize = 0x3F );
	~cDAQCore ();
	
	void field ( const cFieldState & );
	void acquire ( const cIntensityPoint & );
	
	size_t nFieldIndexes () const;
	mass_iter convert ( time_iter & ti ) const;
	time_iter convert ( const mass_iter & mi ) const;
	
	const cFieldState & field ( time_iter & ti ) const {
		return *(reinterpret_cast<const cFieldStateRec *>( (*ti)->link ) );
	}
	
	void expand ( time_iter & ti, cBuildPoint & p ) const {
		( p = field( ti ) ) = *ti;
	}
	
	double getIntensityByMass ( const size_t nChannel, const cFieldState::mass_type & mass ) const;
};

} // namespace msdc
#endif // CROSS_MSDC_COMPOSITE_CORE_H
