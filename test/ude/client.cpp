#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#endif

#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif

#include <crs/ude/client.h>
#include <crs/ude/pingtool.h>

#include <csignal>
#include <iostream>
#include <iomanip>

//#define MULTITHREADED 1
#if defined( MULTITHREADED )
#	include <omp.h>
#endif

class cdm : public ude::client_device_manager
{
protected:
	virtual bool checkTerminate ()
	{
		bool result = ude::client_device_manager::checkTerminate();
		if( result )
			std::cout << "Got terminate signal!" << std::endl;
		return result;
	}
	
public:
	cdm () : ude::client_device_manager( ) { }
	virtual ~cdm () { }
};

cdm * netClient;

void	terminate ( int signal )
{
	netClient->postTerminate();
}

int	main ()
{
#if defined( USE_WIN32_API )
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD( 2, 2 );
	int err = WSAStartup( wVersionRequested, &wsaData );
	if( err != 0 ) {
		std::cout << "Could not find a usable WinSock DLL" << std::endl;
		return 1;
	}
#endif
	
	char a;
	std::cout << "Ready ? ";
	std::cin >> a;
	
	cdm client;
	CrossClass::cSockAddr sa ( "127.0.0.1", 19802 );
	client.clientConnect( sa );
	
	ude::pingTool ptool ( &client );
	
	//
	// set Ctrl+C signal handler
	//
	std::cout << "Press Ctrl+C to terminate ... " << std::endl;
	netClient = &client;
	signal( SIGINT, terminate );
	
	double rtDelay;// = ptool.ping();
	while( client.clientSendRecv() )
	{
		rtDelay = ptool.ping();
		if( rtDelay > 0 )
		{
			std::cout << "rt " << std::setiosflags( std::ios_base::fixed ) << std::setprecision( 6 ) << rtDelay << std::endl;
			ptool.reset();
			ptool.ping();
		}
		while( client.proceed_request() );
	}
	
#if defined( USE_WIN32_API )
	WSACleanup();
#endif
	return 0;
}

