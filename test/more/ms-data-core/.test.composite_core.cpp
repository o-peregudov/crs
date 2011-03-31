#include <iostream>
#include "rrtg.2/ms-data-core/composite_core.h"

#define NCHANNELS	11
#define NMASSRATIO 10

int main ()
{
	MSDataCore::cIntensityPoint ip ( NCHANNELS, 1 );
	for( size_t i = 0; i < ip.size(); ++i )
		ip[ i ] = i * i;
	
	MSDataCore::cDAQCore cc ( NCHANNELS, 0x2, 0xFFFF, 0x3F );
	MSDataCore::cDAQCore::iterator p = cc.begin();
	
	std::cout	<< "Before insertion:" << std::endl
			<< "\tSize: " << cc.size() << std::endl
			<< "\tUsage: " << cc.usage() << std::endl
			<< "\tCapacity: " << cc.capacity() << " (" << (void *)cc.capacity() << ")" << std::endl
			<< "\tBegin iterator position: " << (const void *)p << std::endl
			<< "\tBegin iterator time: " << (*p)->time << std::endl;
	
	MSDataCore::cFieldState fs;
	for( size_t i = 0, j = NMASSRATIO; i < 0x42000F; ++i )
	{
		if( ++j > (NMASSRATIO-1) )
		{
			cc.field( fs );
			fs.mass += NMASSRATIO;
			j = 0;
		}
		cc.acquire( ip );
		ip->time += 1;
	}
	
	std::cout	<< std::endl
			<< "After insertion:" << std::endl
			<< "\tSize: " << cc.size() << std::endl
			<< "\tUsage: " << cc.usage() << std::endl
			<< "\tCapacity: " << cc.capacity() << " (" << (void *)cc.capacity() << ")" << std::endl
			<< "\tField indexes: " << cc.nFieldIndexes() << std::endl;
	
	std::cout	<< std::endl
			<< "Begin iterator position: " << (const void *)p << std::endl
			<< "End iterator position: " << (const void *)cc.end() << std::endl;
	cc.validate( p );
	std::cout	<< std::endl
			<< "After validation: " << (const void *)p << std::endl;
	
	std::cout	<< "Testing ..." << std::flush;
	std::cout.precision( 10 );
	
	MSDataCore::cBuildPoint point;
	bool sync = false;
	for( size_t n = 0; ( p != cc.end() ); ++n, ++p )
	{
		cc.expand( p, point );
		if( sync )
		{
			if( n > (NMASSRATIO-1) )
			{
				n = 0;
				if( point.mass != point->time )
					std::cout	<< std::endl
							<< "pos = " << (const void *)p
							<< " m = " << point.mass << " t = " << point->time
							<< std::flush;
				/*
				if( (*cc.convert( p ))->mass != (*p)->time )
					std::cout	<< std::endl
							<< "pos = " << (const void *)p
							<< " m = " << (*cc.convert( p ))->mass << " t = " << (*p)->time
							<< std::flush;
				*/
			}
		}
		else if( point.mass == point->time )//( (*cc.convert( p ))->mass == (*p)->time )
		{
			sync = true;
			n = 0;
		}
		else
			std::cout	<< std::endl
					<< "pos = " << (const void *)p
					<< " m = " << point.mass << " t = " << point->time
					<< std::flush;
			/*
			std::cout	<< std::endl
					<< "pos = " << (const void *)p
					<< " m = " << (*cc.convert( p ))->mass << " t = " << (*p)->time
					<< std::flush;
			*/
	}
	std::cout << "done!" << std::endl;
	
	int a;
	std::cin >> a;
	
	return 0;
}
