#include <iostream>
#include "rrtg.2/ms-data-core/ring_pool.h"

#define NCHANNELS	6

int main ( )
{
	MSDataCore::cIntensityPoint ip ( NCHANNELS, 0.1 );
	for( size_t i = 0; i < ip.size(); ++i )
		ip[ i ] = i * i;
	
	MSDataCore::ring_pool rp ( NCHANNELS );
	
	std::cout	<< "Before insertion:" << std::endl
			<< "\tSize: " << rp.size() << std::endl
			<< "\tUsage: " << rp.usage() << std::endl
			<< "\tCapacity: " << rp.capacity() << " (" << (void *)rp.capacity() << ")" << std::endl;
	
	MSDataCore::ring_pool::iterator p = rp.begin();
	for( size_t i = 0; i < 0x7100F1; ++i )
	{
		ip->time += 1;
		rp.push( ip );
	}
	
	std::cout	<< std::endl
			<< "After insertion:" << std::endl
			<< "\tSize: " << rp.size() << std::endl
			<< "\tUsage: " << rp.usage() << std::endl
			<< "\tCapacity: " << rp.capacity() << " (" << (void *)rp.capacity() << ")" << std::endl;
	
	MSDataCore::ring_pool::iterator recent = rp.recent();
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
	
	int a;
	std::cin >> a;
	
	return 0;
}
