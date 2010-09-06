#include <crs/timer.h>
#include <crs/sc/rs232port.h>
#include <iostream>

//#define READTHREAD	1

int main ( int argc, const char ** argv )
{
	std::cout.precision( 4 );
	std::cout.setf( std::ios_base::fixed );
	
	try
	{
		CrossClass::cTimer timer;
		sc::rs232port port;
		double startTime;
		
#if defined( READTHREAD )
		const bool doStartReadThread = true;
#else
		const bool doStartReadThread = false;
#endif
#if defined( USE_POSIX_API )
		port.open( "/dev/ttyS0", B57600, doStartReadThread );
#elif defined( USE_WIN32_API )
		port.open( "COM0", CBR_57600, doStartReadThread );
#endif
		port.write( std::string( ":SYST:ZCH:STAT OFF\r" ) );
		CrossClass::sleep( 1000 );
		
		port.write( std::string( ":ARM:COUNT 1\r" ) );
		CrossClass::sleep( 1000 );
		
		port.write( std::string( ":ARM:TIM 0.01\r" ) );
		CrossClass::sleep( 1000 );
		
		port.write( std::string( ":TRIG:COUNT 1\r" ) );
		CrossClass::sleep( 1000 );
		
		port.write( std::string( "*IDN?\r" ) );
#if !defined( READTHREAD )
		while( !port.evntNewData().WaitEvent( 1 ) )
			port.receive();
#else
		if( port.evntNewData().WaitEvent( 1000 ) )
#endif
			std::cout << port.getString() << std::endl;
		
		size_t nPoints2Read = 25;
		if( argc > 1 )
			nPoints2Read = atoi( argv[ 1 ] );
		
		timer.reset();
		for( size_t i = 0; i < nPoints2Read; ++i )
		{
			try
			{
				startTime = timer();
				port.write( std::string( "READ?\r" ) );
#if !defined( READTHREAD )
				while( !port.evntNewData().WaitEvent( 1 ) )
					port.receive();
#else
				if( port.evntNewData().WaitEvent( 2500 ) )
#endif
					std::cout << ( i + 1 ) << '\t' << timer() << '\t' << port.getString() << std::endl;
			}
			catch( sc::rs232port::errTimeout )
			{
				std::cout << "timeout" << std::endl;
			}
			catch( ... )
			{
				std::cerr << "some exception" << std::endl;
				throw;
			}
		}
		port.write( std::string( "SYST:LOC\r" ) );
	}
	catch( std::runtime_error & e )
	{
		std::cerr << "ERROR: " << e.what() << std::endl;
	}
	return 0;
}
