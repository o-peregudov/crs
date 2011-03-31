#ifndef CRS_MATH_SPECTRUM_H_INCLUDED
#define CRS_MATH_SPECTRUM_H_INCLUDED 1
// (c) 2011-03-19 O. Peregudov
//	2011-Mar-30	copy constructor and assignment operator
//			for struct massSpectrum
//	2011-Mar-31	optimized version of the estimateGuess

#include <crs/math/peakCenter.h>
#include <iostream>

struct CROSS_EXPORT massSpectrum
{
	size_t	nPoints,
			nPointsReserved;
	double	*mass,
			*time,
			*intensity,
			*sigma;
	double	bgLevel;
	double 	*smoothIntensity;
	
	peakCenterData			pcd;
	std::map<double, size_t>	hist;
	
	void	addPoint ( const double & mm, const double & ii, const double & ss, const double & tt );
	void	read ( std::istream & is, const std::string & format );
	
	void	doSmoothing ();
	void	doCompensateBackground ();
	void	doPeakCenter ();
	
	void	clear ();
	void	estimateGuess ( const int nFilterSize = 5 )
	{
		delete [] smoothIntensity;
		smoothIntensity = new double [ nPoints ];
		::estimateGuess( nPoints, mass, intensity, smoothIntensity, pcd, bgLevel, hist, nFilterSize );
	}
	
	massSpectrum ();
	~massSpectrum ();
	
	massSpectrum ( const massSpectrum & );
	massSpectrum & operator = ( const massSpectrum & );
};

#endif // CRS_MATH_SPECTRUM_H_INCLUDED
