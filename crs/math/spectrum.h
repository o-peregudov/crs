#ifndef CRS_MATH_SPECTRUM_H_INCLUDED
#define CRS_MATH_SPECTRUM_H_INCLUDED 1
// (c) 2011-03-19 O. Peregudov

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
	std::vector<intDoublePair>	hist;
	
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
};

#endif // CRS_MATH_SPECTRUM_H_INCLUDED
