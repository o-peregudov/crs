#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif

#include <crs/sc/i41serial.h>
#include <crs/oio/iniFile.h>
#include <crs/timer.h>

#include <cstdlib>
#include <csignal>
#include <iostream>
#include <fstream>
#include <sstream>

typedef sc::i41serial	portClass;

#if defined( USE_WIN32_API )
static const char * defaultConfigString =
	"[local connection]; RS232 port configuration\n"	\
	"port=COM1; port name (string)\n"				\
	"baud=115200; baud rate (115200, 57600, etc)\n"		\
	"data bits=8; data bits (5 | 6 | 7 | 8)\n"		\
	"stop bits=1; stop bits (1 | 2)\n"				\
	"parity=no; parity (odd | even | no | mark | space)\n"\
	"\n";
#elif defined( USE_POSIX_API )
static const char * defaultConfigString =
	"[local connection]; RS232 port configuration\n"	\
	"port=/dev/ttyS0; port name (string)\n"			\
	"baud=115200; baud rate (115200, 57600, etc)\n"		\
	"data bits=8; data bits (5 | 6 | 7 | 8)\n"		\
	"stop bits=1; stop bits (1 | 2)\n"				\
	"parity=no; parity (odd | even | no | mark | space)\n"\
	"\n";
#endif

static const char * CONF_NAME = "loop.conf";

CrossClass::event_loop * evpoll4signal = 0;
portClass * port2signal = 0;

void	terminate ( int signal )
{
	std::cout << "\rPress Ctrl+C to terminate ... ";
	if (port2signal)
	{
		evpoll4signal->remove (*port2signal);
		port2signal = 0;
	}
	else
		evpoll4signal->terminate (false);
	std::cout << "pressed" << std::endl;
}

void	processor ( void * pData )
{
	portClass::comPacket packet;
	portClass * port = reinterpret_cast<portClass *>( pData );
	if( port->peekPacket( packet ) )
	{
		port->pop();
		if( packet.data.id != 0x01 )
			packet.data.cmd = 0xAD;		// wrong address (byte[0])
		else
		{
			switch( packet.data.cmd )
			{
			case	0x10:				// set state
				packet.data.id = 0x00;
				packet.data.cmd = 0xFF;
				packet.data.ext = 0x01;
				packet.buildCRC();
				port->write( reinterpret_cast<const char *>( packet.byteArray ), sizeof( portClass::hostPacketType ) );
				
				packet.data.id = 0x00;
				packet.data.cmd = 0xFF;
				packet.data.ext = 0x02;
				packet.buildCRC();
				port->write( reinterpret_cast<const char *>( packet.byteArray ), sizeof( portClass::hostPacketType ) );
				break;
			
			default:
				break;
			}
		}
		packet.data.id = 0x10;
		packet.buildCRC();
		port->write( reinterpret_cast<const char *>( packet.byteArray ), sizeof( portClass::hostPacketType ) );
	}
}

int	main ( int argc, const char ** argv )
{
	bool runLoop = true;
	if( argc > 1 )
	{
		if( strcmp( argv[ 1 ], "--write-conf" ) == 0 )
			runLoop = false;
	}
	
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
	
	if( runLoop )
	{
		int a = 0;
		std::cout << "Ready? ";
		std::cin >> a;
		
		//
		// open serial port
		//
		portClass port;
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
		port2signal = &port;
		signal( SIGINT, terminate );
		
		//
		// set packet processor as a callback function
		//
		port.setAsyncDataCallback( processor, &port );
		
		CrossClass::event_loop eveloop (1);
		evpoll4signal = &eveloop;
		eveloop.add (port);
		
		while (eveloop.runOnce (-1));
	}
	
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
