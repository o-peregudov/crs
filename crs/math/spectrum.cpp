#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif
#include <crs/math/spectrum.h>
#include <crs/math/unimath.h>

#include <cstring>
#include <sstream>
#include <limits>

massSpectrum::massSpectrum ()
	: nPoints( 0 )
	, nPointsReserved( 1000 )
	, mass( 0 )
	, time( 0 )
	, intensity( 0 )
	, sigma( 0 )
	, bgLevel( 0 )
	, smoothIntensity( 0 )
	, pcd( )
	, hist( )
{
	mass = new double [ nPointsReserved ];
	time = new double [ nPointsReserved ];
	intensity = new double [ nPointsReserved ];
	sigma = new double [ nPointsReserved ];
}

massSpectrum::~massSpectrum ()
{
	delete [] smoothIntensity;
	delete [] sigma;
	delete [] intensity;
	delete [] time;
	delete [] mass;
}

massSpectrum::massSpectrum ( const massSpectrum & s )
	: nPoints( s.nPoints )
	, nPointsReserved( s.nPointsReserved )
	, mass( 0 )
	, time( 0 )
	, intensity( 0 )
	, sigma( 0 )
	, bgLevel( s.bgLevel )
	, smoothIntensity( 0 )
	, pcd( s.pcd )
	, hist( s.hist )
{
	mass = new double [ nPointsReserved ];
	time = new double [ nPointsReserved ];
	intensity = new double [ nPointsReserved ];
	sigma = new double [ nPointsReserved ];
	smoothIntensity = new double [ nPoints ];
	
	memcpy( mass, s.mass, nPoints * sizeof( double ) );
	memcpy( time, s.time, nPoints * sizeof( double ) );
	memcpy( intensity, s.intensity, nPoints * sizeof( double ) );
	memcpy( sigma, s.sigma, nPoints * sizeof( double ) );
	memcpy( smoothIntensity, s.smoothIntensity, nPoints * sizeof( double ) );
}

massSpectrum & massSpectrum::operator = ( const massSpectrum & s )
{
	if( &s != this )
	{
		if( nPointsReserved < s.nPointsReserved )
		{
			delete [] sigma;
			delete [] intensity;
			delete [] time;
			delete [] mass;
			
			nPointsReserved = s.nPointsReserved;
			
			mass = new double [ nPointsReserved ];
			time = new double [ nPointsReserved ];
			intensity = new double [ nPointsReserved ];
			sigma = new double [ nPointsReserved ];
		}
		
		if( nPoints < s.nPoints )
		{
			delete [] smoothIntensity;
			
			nPoints = s.nPoints;
			
			smoothIntensity = new double [ nPoints ];
		}
		
		bgLevel = s.bgLevel;
		pcd = s.pcd;
		hist = s.hist;
		
		memcpy( mass, s.mass, nPoints * sizeof( double ) );
		memcpy( time, s.time, nPoints * sizeof( double ) );
		memcpy( intensity, s.intensity, nPoints * sizeof( double ) );
		memcpy( sigma, s.sigma, nPoints * sizeof( double ) );
		memcpy( smoothIntensity, s.smoothIntensity, nPoints * sizeof( double ) );
	}
	return *this;
}

void	massSpectrum::read ( std::istream & is, const std::string & format )
{
	std::string lineBuffer;
	std::istringstream line;
	
	double m( 1 ), t( 0 ), i( 0 ), s( 1 );
	try
	{
		for( double tmp; is.good(); )
		{
			std::getline( is, lineBuffer );
			
			line.str( lineBuffer );
			line.clear();
			
			for( size_t p = 0; ( p < format.size() ) && line.good(); ++p )
			{
				tmp = std::numeric_limits<double>::infinity();
				line >> tmp;
				if( tmp == std::numeric_limits<double>::infinity() )
					throw int ();	// end of reading
				
				s = 1.0;
				switch( format[ p ] )
				{
				case	'm':
				case	'M':
					m = tmp;
					break;
				
				case	't':
				case	'T':
					t = tmp;
					break;
				
				case	'i':
				case	'I':
					i = tmp;
					break;
				
				case	's':
				case	'S':
					s = tmp;
					break;
				
				default:
					break;
				}
			}
			
			if( ( m != std::numeric_limits<double>::infinity() ) &&
			    ( i != std::numeric_limits<double>::infinity() ) )
			{
				addPoint( m, i, s, t );
			}
			else
				break;
		}
	}
	catch( int )
	{
	}
	catch( ... )
	{
		throw;
	}
}

void	massSpectrum::addPoint ( const double & mm, const double & ii, const double & ss, const double & tt )
{
	mass[ nPoints ] = mm;
	intensity[ nPoints ] = ii;
	sigma[ nPoints ] = ss;
	time[ nPoints ] = tt;
	
	if( ++nPoints == nPointsReserved )
	{
		nPointsReserved += 1000;
		
		double * nm = new double [ nPointsReserved ],
			 * nt = new double [ nPointsReserved ],
			 * ni = new double [ nPointsReserved ],
			 * ns = new double [ nPointsReserved ];
		
		memcpy( nm, mass, nPoints * sizeof( double ) );
		memcpy( nt, time, nPoints * sizeof( double ) );
		memcpy( ni, intensity, nPoints * sizeof( double ) );
		memcpy( ns, sigma, nPoints * sizeof( double ) );
		
		delete [] sigma;
		delete [] intensity;
		delete [] time;
		delete [] mass;
		
		mass = nm;
		time = nt;
		intensity = ni;
		sigma = ns;
	}
}

void	massSpectrum::doSmoothing ()
{
	delete [] smoothIntensity;
	smoothIntensity = new double [ nPoints ];
	fltMovingAverage( nPoints, mass, intensity, smoothIntensity, 5 );
}

void	massSpectrum::doCompensateBackground ()
{
	bgLevel = background ( nPoints, intensity );
	if( !UniMath::isZero( bgLevel ) )
	{
		#pragma omp parallel for if( nPoints > 10000 )
		for( int i = 0; i < ( nPoints / 2 ); ++i )
		{
			intensity[ 2 * i ] -= bgLevel;
			intensity[ 2 * i + 1 ] -= bgLevel;
		}
		if( ( nPoints & 0x01 ) == 0x01 )
			intensity[ nPoints - 1 ] -= bgLevel;
	}
}

void	massSpectrum::clear ()
{
	nPoints = 0;
	bgLevel = 0;
}

void	massSpectrum::doPeakCenter ()
{
	peakCenter( nPoints, mass, smoothIntensity, pcd );
}

