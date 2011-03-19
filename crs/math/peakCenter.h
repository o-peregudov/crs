#ifndef PEAKCENTER_H_INCLUDED
#define PEAKCENTER_H_INCLUDED 1
// (c) Mar 11, 2010 Oleg N. Peregudov
//	2011-Mar-19	new algorithms for peak measurement

#include <crs/libexport.h>
#include <vector>

struct CROSS_EXPORT intPair
{
	int first, second;
	
	intPair ( const int f = 0, const int s = 0 )
		: first( f )
		, second( s )
	{ }
};

struct CROSS_EXPORT intDoublePair
{
	int		first;
	double	second;
	
	intDoublePair ()
		: first( 0 )
		, second( 0 )
	{}
	
	explicit intDoublePair( const int i )
		: first( i )
		, second( 0 )
	{}
	
	explicit intDoublePair( const int i, const double & d )
		: first( i )
		, second( d )
	{}
	
	intDoublePair( const intDoublePair & p )
		: first( p.first )
		, second( p.second )
	{}
	
	intDoublePair & operator = ( const intDoublePair & p )
	{
		if( &p != this )
		{
			first = p.first;
			second = p.second;
		}
		return *this;
	}
	
	operator const int & () const
	{
		return first;
	}
};

struct CROSS_EXPORT peakWidthData
{
	double		height,
				width,
				center,
				centroid;
	intPair		idxSlope;
	
	double resolution () const
	{
		return ( center + 0.5 ) / width;
	}
};

struct CROSS_EXPORT peakCenterData
{
	intDoublePair	peakMaximum;
	peakWidthData	height50;
	peakWidthData	height10;
	peakWidthData	height98;
};

extern void CROSS_EXPORT fltMovingAverage ( const int nData, const double * x, const double * y, double * yo, const int nFilterSize );
extern void	CROSS_EXPORT fltSavitzkyGolay ( const int nData, const double * x, const double * y, double * yo, const int nFilterSize );

extern intDoublePair CROSS_EXPORT findMaximum ( const int nData, const double * y );

extern int CROSS_EXPORT findLevelLeft ( const int nData, const double * y, const intDoublePair & maxPosition, const double & height );
extern int CROSS_EXPORT findLevelRight ( const int nData, const double * y, const intDoublePair & maxPosition, const double & height );

inline
intPair measPeak ( const int nData, const double * y, const intDoublePair & maxPosition, const double & height )
{
	return intPair( findLevelLeft( nData, y, maxPosition, height )
			  , findLevelRight( nData, y, maxPosition, height ) );
}

// estimate center of gravity
extern double CROSS_EXPORT centroid ( const int nData, const double * x, const double * y, const double & absoluteLevel );

// do several peak measurements and calculate resolution
extern void	CROSS_EXPORT peakCenter ( const int nData, const double * x, const double * y, peakCenterData & pcd );

// estimate background level by evaluating most probable spectrum intensity
extern double CROSS_EXPORT background ( const int nData, const double * y );

// full preprocessing
extern void CROSS_EXPORT estimateGuess ( const int nData, const double * x, const double * y, double * yo, peakCenterData & pcd, double & bgLevel, std::vector<intDoublePair> & hist, const int nFilterSize = 5 );
#endif
