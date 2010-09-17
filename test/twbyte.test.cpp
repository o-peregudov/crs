#include <crs/sc/twelvebyte.h>
#include <crs/thread.h>
#include <crs/timer.h>
#include <iostream>

class asyncReadThread : public CrossClass::cThread
{
protected:
	sc::twByte * port;
	
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
		catch( sc::twByte::syncTimeOut & e )
		{
			std::cerr << "syncTimeOut: " << e.what() << std::endl;
		}
		catch( std::runtime_error & e )
		{
			std::cerr << "port read runtime_error: " << e.what() << std::endl;
		}
		catch( ... )
		{
			std::cerr << "Step: unexpected port read error" << std::endl;
		}
		return false;
	}
	
public:
	asyncReadThread ( sc::twByte * p ) : CrossClass::cThread( ), port( p )	{ }
	~asyncReadThread ( )									{ kill(); }
};

int main ( int argc, const char ** argv )
{
	std::cout.precision( 4 );
	std::cout.setf( std::ios_base::fixed );
	std::cerr.setf( std::ios_base::fixed );
	sc::twByte::comPacket packet, inPacket;
	double startTime = 0, diffTime = 0;
	CrossClass::cTimer timer;
	
	int a;
	
	try
	{
		sc::twByte port;
		
#if defined( USE_POSIX_API )
		std::cout << "Trying to open '/dev/ttyS1' ... ";
		port.open( "/dev/ttyS1", 115200 );
#elif defined( USE_WIN32_API )
		std::cout << "Trying to open 'COM2' ... ";
		port.open( "COM2", 115200 );
#endif
		std::cout << "success" << std::endl;
		asyncReadThread readThread ( &port );
		readThread.Resume();
		
		std::cout << "Pause: ";
		std::cin >> a;
		
		bool sw = true;
		
		for( long i = 0; i < 5000000L; ++i )
		{
			packet.setZero();
			packet.byteArray[ 0 ] = 0x01;
			if( sw )
				packet.data.cmd = 0x3A;	// read collector current
			else
				packet.data.cmd = 0x3E;	// read accelerating voltage
			sw = !sw;
			packet.buildCRC( );
			startTime = timer();
			try
			{
				if( !port.comSection( packet, inPacket ) )
					std::cerr << "timeout: " << packet.byteString() << std::endl;
			}
			catch( sc::twByte::syncTimeOut & e )
			{
				std::cerr << "timeout: " << e.what() << std::endl;
			}
			catch( sc::twByte::crcError & e )
			{
				std::cerr << "CRC error: " << e.what() << std::endl;
			}
			catch( std::runtime_error & e )
			{
				std::cerr << "ERROR: " << e.what() << std::endl;
			}
			catch( ... )
			{
				throw;
			}
			diffTime = timer() - startTime;
			std::cerr
				<< i
				<< '\t' << diffTime
//				<< '\t' << *reinterpret_cast<float *>( inPacket.byteArray + 3 )
//				<< "\t\t" << *reinterpret_cast<float *>( inPacket.byteArray + 7 )
				<< '\r' << std::flush;
		}
		port.postTerminate();
	}
	catch( int & e )
	{
		std::cerr << "int?" << std::endl;
	}
	catch( std::runtime_error & e )
	{
		std::cerr << "ERROR: " << e.what() << std::endl;
	}
	return 0;
}

