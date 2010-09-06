#ifndef PEAKCENTER_H_INCLUDED
#define PEAKCENTER_H_INCLUDED 1
// (c) Mar 11, 2010 Oleg N. Peregudov

#include <crs/libexport.h>

struct CROSS_EXPORT peakCenterData
{
      int	 idxPeakCenter,
             leftSlope,
             rightSlope;
	
	double centroid,
      	 pcMaximum,
             desiredHeight,
             peakWidth,
             fwhm;
};

struct CROSS_EXPORT intPair
{
	int first, second;
	
	intPair ( const int f = 0, const int s = 0 )
		: first( f )
		, second( s )
	{ }
};

extern intPair CROSS_EXPORT measPeak ( const int nData, const double * x, const double * y, const int maxPosition, const double & height = 0.5 );
extern int	CROSS_EXPORT peakCenter ( const int nData, const double * x, const double * y, peakCenterData & pcd, const double & height = 0.5 );
extern int	CROSS_EXPORT cutBackground ( const int nData, double * x, double * y, double & c0, double & c1, double & sumsq );
extern void	CROSS_EXPORT estimateGuess ( const int nData, const double * x, const double * y, peakCenterData & pcd );
extern void	CROSS_EXPORT medSmooth ( const int nData, double * x, double * y );
extern void	CROSS_EXPORT SavitzkyGolaySmooth ( const int nData, const double * x, double * y, const int nPoints );
extern void	CROSS_EXPORT SavitzkyGolaySmooth ( const int nData, const double * x, const double * y, double * yo, const int nPoints );

#endif
