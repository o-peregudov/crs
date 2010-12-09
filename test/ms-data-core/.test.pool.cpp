#include <iostream>
#include "rrtg.2/ms-data-core/pools.h"

int main( )
{
	MSDataCore::cIntensityPoint ip ( 16, 0.1 );
	for( size_t i = 0; i < ip.size(); ++i )
		ip[ i ] = i * i;
	
	MSDataCore::mem_pool mp ( 16, 25 );
	do
	{
		ip->time += 1;
	} while ( !mp.push( ip ) );
	
	MSDataCore::file_pool fp ( mp );
	for( size_t j = 0; j < 25; ++j )
	{
		if( ( j & 0x01 ) == 0x01 )
			ip.linkWith( fp.idx( j ) );
		else
			ip.linkWith( mp.idx( j ) );
		std::cout << ( ip ? "local" : "remote" ) << '\t' << ip->time << std::endl;
	}
	
	int a;
	std::cin >> a;
	
	return 0;
}
