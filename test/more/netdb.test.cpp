#include <crs/socket.h>
#include <iostream>

int main ( int argc, const char ** argv )
{
	if( argc )
	{
		CrossClass::cSockAddr sa;
		sa.createFromName( argv[ 1 ], 121 );
		std::cout << sa.DottedDecimal() << " : " << sa.Port() << std::endl;
	}
	return 0;
}
