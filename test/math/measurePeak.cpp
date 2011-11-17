#include <crs/math/spectrum.h>
#include <crs/timer.h>
#include <fstream>
#include <iomanip>

int main ( int argc, const char ** argv )
{
	std::string defaultFormat = "mi";
	if( argc > 1 )
		defaultFormat = argv[ 1 ];
	
	massSpectrum data;
	data.read( std::cin, defaultFormat );
	data.estimateGuess();
	
	std::cout.precision( 7 );
	for( int i = 0; i < data.nPoints; ++i )
	{
		std::cout	<< data.mass[ i ] << '\t'
				<< data.intensity[ i ] << '\t'
				<< data.smoothIntensity[ i ] << '\t';
		if( ( i < data.pcd.height10.idxSlope.first ) ||
		    ( data.pcd.height10.idxSlope.second < i ) )
			std::cout << 0.0;
		else
			std::cout << data.pcd.height10.height;
		std::cout << '\t';
		if( ( i < data.pcd.height50.idxSlope.first ) ||
		    ( data.pcd.height50.idxSlope.second < i ) )
			std::cout << 0.0;
		else
			std::cout << data.pcd.height50.height;
		std::cout << '\t';
		if( ( i < data.pcd.height98.idxSlope.first ) ||
		    ( data.pcd.height98.idxSlope.second < i ) )
			std::cout << 0.0;
		else
			std::cout << data.pcd.height98.height;
		std::cout << std::endl;
	}
	
	std::cerr.precision( 6 );
	std::cerr.setf( std::ios_base::fixed );
	std::cerr	<< "\tnPoints:\t"	<< data.nPoints << std::endl
			<< "\tbgLevel:\t"	<< data.bgLevel << std::endl
			<< "\tmaximum:\t"	<< (data.pcd.peakMaximum.second+data.bgLevel) << " @ " << data.pcd.peakMaximum.first << std::endl
			<< "\tcenter:\t\t"<< data.pcd.height50.center	<< std::endl;
	std::cerr	<< "\tFW10%:\t\t"	<< data.pcd.height10.width	<< " [" << data.mass[ data.pcd.height10.idxSlope.first ]
											<< "; " << data.mass[ data.pcd.height10.idxSlope.second ]
											<< ']' << std::endl;
	std::cerr	<< "\tFW50%:\t\t"	<< data.pcd.height50.width	<< " [" << data.mass[ data.pcd.height50.idxSlope.first ]
											<< "; " << data.mass[ data.pcd.height50.idxSlope.second ]
											<< ']' << std::endl;
	std::cerr	<< "\tFW98%:\t\t"	<< data.pcd.height98.width	<< " [" << data.mass[ data.pcd.height98.idxSlope.first ]
											<< "; " << data.mass[ data.pcd.height98.idxSlope.second ]
											<< ']' << std::endl;
	std::cerr.precision( 0 );
	std::cerr	<< "\tR10%:\t\t"	<< data.pcd.height10.resolution() << std::endl
			<< "\tR50%:\t\t"	<< data.pcd.height50.resolution() << std::endl
			<< "\tR98%:\t\t"	<< data.pcd.height98.resolution() << std::endl;
	
	std::fstream hout ( "hout", std::ios_base::trunc|std::ios_base::out|std::ios_base::binary );
	if( hout.good() )
	{
		hout.precision( 6 );
		hout.setf( std::ios_base::fixed );
		for( std::map<double, size_t>::const_iterator i = data.hist.begin(); ( i != data.hist.end() ) && hout.good(); ++i )
			hout	<< (*i).first << '\t'
				<< ( static_cast<double>( (*i).second ) / data.nPoints ) << std::endl;
	}
	
	return 0;
}

