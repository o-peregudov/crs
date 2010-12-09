#include <crs/sc/twelvebyte.h>
#include <crs/oio/iniFile.h>
#include <crs/timer.h>

#include <cstdlib>
#include <csignal>
#include <iostream>
#include <fstream>
#include <sstream>

#if defined( USE_WIN32_API )
static const char * defaultConfigString =
	"[local connection]; RS232 port configuration\n"	\
	"port=COM1; port name (string)\n"				\
	"baud=115200; baud rate (115200, 57600, etc)\n"		\
	"data bits=8; data bits (5 | 6 | 7 | 8)\n"		\
	"stop bits=1; stop bits (1 | 2)\n"				\
	"parity=noparity; parity (odd | even | noparity)\n"	\
	"\n";
#elif defined( USE_POSIX_API )
static const char * defaultConfigString =
	"[local connection]; RS232 port configuration\n"	\
	"port=/dev/ttyS0; port name (string)\n"			\
	"baud=115200; baud rate (115200, 57600, etc)\n"		\
	"data bits=8; data bits (5 | 6 | 7 | 8)\n"		\
	"stop bits=1; stop bits (1 | 2)\n"				\
	"parity=noparity; parity (odd | even | noparity)\n"	\
	"\n";
#endif

static const char * CONF_NAME = "loop.conf";

sc::twByte * port4signal;

void	terminate ( int signal )
{
	std::cout << "\rPress Ctrl+C to terminate ... ";
	port4signal->postTerminate( false );
	std::cout << "pressed" << std::endl;
}

unsigned char transmuteStateByte ( const unsigned char stateByte )
{
	unsigned char result = 0;
	if( ( stateByte & 0x01 ) == 0x01 )
		result |= 0x04;	// HV pump unit
	if( ( stateByte & 0x02 ) == 0x02 )
		result |= 0x08;	// IS unit
	if( ( stateByte & 0x04 ) == 0x04 )
		result |= 0x10;	// HV unit
	return result;
}

void	processor ( void * pData )
{
	static unsigned char state = 0;
	
	sc::twByte::comPacket packet;
	sc::twByte * port = reinterpret_cast<sc::twByte *>( pData );
	if( port->peekPacket( packet ) )
	{
		port->pop();
		if( packet.data.id != 0x01 )
			packet.data.cmd = 0xAD;		// неверный адрес (байт [0])
		else
		{
			switch( packet.data.cmd )
			{
			case	0x2A:
				state = state & ~packet.byteArray[ 4 ] | packet.byteArray[ 5 ];
				packet.byteArray[ 2 ] = transmuteStateByte( state );
				break;
			
			case	0x3A:				// ток коллектора
				*( reinterpret_cast<float*>( packet.byteArray + 3 ) ) = static_cast<float>( rand() ) / RAND_MAX + 1.0;
				packet.byteArray[ 2 ] = transmuteStateByte( state );
				break;
			
			case	0x3B:				// ток эмиссии
				*( reinterpret_cast<float*>( packet.byteArray + 3 ) ) = static_cast<float>( rand() ) / RAND_MAX - 1.0;
				packet.byteArray[ 2 ] = transmuteStateByte( state );
				break;
			
			case	0x3E:				// ускоряющее напряжение
				*( reinterpret_cast<float*>( packet.byteArray + 3 ) ) = static_cast<float>( rand() ) / RAND_MAX + 2.0;
				packet.byteArray[ 2 ] = transmuteStateByte( state );
				break;
			
			default:
				packet.data.cmd = 0xCD;	// неизвестная команда
			}
		}
		packet.data.id = 0x10;
		packet.buildCRC();
		port->write( reinterpret_cast<const char *>( packet.byteArray ), sizeof( sc::twByte::comPacket ) );
	}
}

int	main ( int argc, const char ** argv )
{
	//
	// read configuration file
	//
	ObjectIO::cINIFile conf;
	std::fstream confFile ( CONF_NAME, std::ios_base::in|std::ios_base::binary );
	if( confFile.good() )
	{
		std::cout << "Reading configuration file ... ";
		conf.read( confFile );
		std::cout << "done" << std::endl;
	}
	else
	{
		std::cout << "Using default configuration ... ";
		std::istringstream confString ( defaultConfigString );
		conf.read( confString );
		std::cout << "done" << std::endl;
	}
	confFile.clear();
	confFile.close();
	
	//
	// open serial port
	//
	sc::twByte port;
	std::string portName;
	unsigned int baudRate;
	
	ObjectIO::cHubHandle hubLocalConnection = conf.get( "local connection" );
	conf.readvar( hubLocalConnection, "port", portName );
	conf.readvar( hubLocalConnection, "baud", baudRate );
	std::cout << "Opening '" << portName << "' at " << baudRate << " baud ... ";
	port.open( portName, baudRate );
	std::cout << "done" << std::endl;
	
	//
	// set Ctrl+C signal handler
	//
	std::cout << "Press Ctrl+C to terminate ... " << std::flush;
	port4signal = &port;
	signal( SIGINT, terminate );
	
	//
	// set packet processor as a callback function
	//
	port.setAsyncDataCallback( processor, &port );
	while( port.receive() );
	
	//
	// write configuration file
	//
	std::cout << "Writting configuration file ... ";
	confFile.open ( CONF_NAME, std::ios_base::out|std::ios_base::trunc|std::ios_base::binary );
	if( confFile.good() )
		conf.write( confFile );
	std::cout << "done" << std::endl;
	
	return 0;
}
