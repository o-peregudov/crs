#include <crs/sc/i41serial.h>
#include <crs/thread.h>
#include <crs/timer.h>
#include <iostream>

void	asyncPacketHandler ( void * pData )
{
	sc::i41serial::comPacket inPacket;
	sc::i41serial * port = reinterpret_cast<sc::i41serial*>( pData );
	while( port->peekPacket( inPacket ) )
	{
		port->pop();
		std::cout	<< "A-Reply:\t\t" << inPacket.byteString() << std::endl;
	}
}

class asyncReadThread : public CrossClass::cThread
{
protected:
	sc::i41serial * port;
	
	virtual bool Step ()
	{
		try
		{
			if( !port->receive() )
				Stop();
		}
		catch( sc::basicRS232port::errRead & e )
		{
			std::cerr << "port read error: " << e.what() << std::endl;
		}
		catch( ... )
		{
			std::cerr << "unexpected port read error" << std::endl;
		}
		return false;
	}
	
public:
	asyncReadThread ( sc::i41serial * p )
		: CrossClass::cThread()
		, port( p )
	{
	}
	
	~asyncReadThread( )
	{
		kill();
	}
};

int main ( int argc, const char ** argv )
{
	std::cout.precision( 4 );
	std::cout.setf( std::ios_base::fixed );
	sc::i41serial::comPacket packet, inPacket;
	double startTime = 0, diffTime = 0;
	CrossClass::cTimer timer;
	
	int a;
	std::cout << "pause?";
	std::cin >> a;
	
	try
	{
		sc::i41serial port;
		
#if defined( USE_POSIX_API )
		std::cout << "Trying to open '/dev/ttyS1' ... ";
		port.open( "/dev/ttyS1", B115200 );
#elif defined( USE_WIN32_API )
		std::cout << "Trying to open 'COM1' ... ";
		port.open( "COM1", CBR_115200 );
#endif
		std::cout << "success" << std::endl;
		asyncReadThread readThread ( &port );
		readThread.Resume();
		
		if( port.synchronize() )
			std::cout << "SYNC - ok!" << std::endl;
		else
			std::cout << "SYNC - failed!" << std::endl;
		
		port.setAsyncDataCallback( asyncPacketHandler, &port );
		for( int i = 0; i < 5; ++i )
		{
			packet.byteArray[ 0 ] = 0x01;
			packet.byteArray[ 1 ] = 0x05;
			packet.byteArray[ 2 ] = 0x01;
			packet.byteArray[ 3 ] = 0x00;
			packet.byteArray[ 4 ] = 0x03;
			packet.byteArray[ 5 ] = 0xFF;
			packet.byteArray[ 6 ] = 0x00;
			packet.buildCRC( );
			std::cout	<< "Request:\t" << packet.byteString() << std::endl;
			startTime = timer();			//std::chrono::high_resolution_clock::now();
			port.comSection( packet, inPacket );
			diffTime = timer() - startTime;	//std::chrono::high_resolution_clock::now().time_since_epoch() - startTime.time_since_epoch();
			std::cout	<< "Reply:\t\t" << inPacket.byteString() << ", "
//					<< static_cast<double>( diffTime.count() ) / 1e3
					<< diffTime * 1e3
					<< " ms" << std::endl;
			CrossClass::sleep( 250 );
			packet.byteArray[ 1 ] = 0x05;
			packet.byteArray[ 2 ] = 0x02;
			packet.byteArray[ 3 ] = 0x00;
			packet.byteArray[ 4 ] = 0x03;
			packet.byteArray[ 5 ] = 0xFF;
			packet.byteArray[ 6 ] = 0x00;
			packet.buildCRC( );
			std::cout	<< "Request:\t" << packet.byteString() << std::endl;
			startTime = timer();			//std::chrono::high_resolution_clock::now();
			port.comSection( packet, inPacket );
			diffTime = timer() - startTime;	//std::chrono::high_resolution_clock::now().time_since_epoch() - startTime.time_since_epoch();
			std::cout	<< "Reply:\t\t" << inPacket.byteString() << ", "
//					<< static_cast<double>( diffTime.count() ) / 1e3
					<< diffTime * 1e3
					<< " ms" << std::endl;
			CrossClass::sleep( 250 );
		}
		for( int i = 0; i < 2; ++i )
		{
			packet.byteArray[ 0 ] = 0x01;
			packet.byteArray[ 1 ] = 0x55;
			packet.byteArray[ 2 ] = 0x00;
			packet.byteArray[ 3 ] = 0x00;
			packet.byteArray[ 4 ] = 0x00;
			packet.byteArray[ 5 ] = 0x00;
			packet.byteArray[ 6 ] = 0x00;
			packet.buildCRC( );
			std::cout	<< "Request:\t" << packet.byteString() << std::endl;
			startTime = timer();			//std::chrono::high_resolution_clock::now();
			port.comSection( packet, inPacket );
			diffTime = timer() - startTime;	//std::chrono::high_resolution_clock::now().time_since_epoch() - startTime.time_since_epoch();
			std::cout	<< "Reply:\t\t" << inPacket.byteString() << ", "
//					<< static_cast<double>( diffTime.count() ) / 1e3
					<< diffTime * 1e3
					<< " ms" << std::endl;
		}
		port.postTerminate();
	}
	catch( sc::i41serial::syncTimeOut & e )
	{
		std::cerr << "synchronous exchange timed out" << std::endl;
	}
	catch( std::runtime_error & e )
	{
		std::cerr << "ERROR: " << e.what() << std::endl;
	}
	return 0;
}

