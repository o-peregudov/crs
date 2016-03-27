#include <iostream>
#include <crs/msdc/ring_pool.h>
#include <crs/timer.h>

#define NCHANNELS	16

std::ostream & operator << (std::ostream & os, const msdc::ring_pool & rp)
{
	os	<< "[size = " << rp.size()
		<< ", usage = " << rp.usage()
		<< ", capacity = " << rp.capacity()
		<< " (" << (void *)rp.capacity() << ")]";
	return os;
}

int main ( )
{
	CrossClass::cTimer timer;
	
	msdc::cIntensityPoint ip ( NCHANNELS, 0.1 );
	for (size_t i = 0; i < ip.size(); ++i)
		ip[ i ] = i * i;
	
	msdc::ring_pool rp ( NCHANNELS, 4, 0xFFFF, 3 );
	
	std::cout	<< "Initial state: " << rp << std::endl;
	std::cout	<< "Fill in the pool .... " << std::flush;
	size_t nTotalPoints = rp.capacity() * 3;
	msdc::ring_pool::iterator p = rp.begin();
	double tmStart = timer.getStamp();
	for (size_t i = 0; i < nTotalPoints; ++i)
	{
		ip->time = timer.getStamp();
		rp.push (ip);
	}
	double elapsedTime = timer.getStamp () - tmStart;
	std::cout	<< "done in " << elapsedTime << " sec (" << elapsedTime / nTotalPoints * 1e3 << " ms/point)" << std::endl;
	std::cout	<< "State after insertion: " << rp << std::endl;
	
	msdc::ring_pool::iterator recent = rp.recent();
	std::cout	<< "Recent point: "
			<< ( static_cast<bool>( *recent ) ? "local" : "remote" ) << ", time = " << (*recent)->time << std::endl;
	
	std::cout	<< std::endl
			<< "Begin iterator position: " << (const void *)p << std::endl
			<< "Recent iterator position: " << (const void *)recent << std::endl
			<< "End iterator position: " << (const void *)rp.end() << std::endl;
	rp.validate( p );
	std::cout	<< std::endl
			<< "After validation: " << (const void *)p << std::endl;
	
	std::cout.precision( 8 );
	for( size_t i = 0; ( i < 10 ) && ( p != rp.end() ); ++i, ++p )
		std::cout << ( static_cast<bool>( *p ) ? "local" : "remote" ) << ", time = " << (*p)->time << std::endl;
	
	return 0;
}
