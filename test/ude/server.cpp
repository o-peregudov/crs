#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#endif

#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif

#include <crs/ude/server.h>
#include <csignal>
#include <iostream>
#include <sstream>

class sdm : public ude::device_manager
{
protected:
	virtual bool checkTerminate ()
	{
		bool result = ude::device_manager::checkTerminate();
		if( result )
			std::cout << "\rGot terminate signal!" << std::endl;
		return result;
	}
	
	virtual CrossClass::basicNetPoint * handleNewConnection ( CrossClass::cSocket & s, const CrossClass::cSockAddr & sa )
	{
		std::cout << "Connection from " << sa.DottedDecimal() << ':' << sa.Port() << std::endl;
		CrossClass::basicNetPoint * newPeer = ude::device_manager::handleNewConnection( s, sa );
		ude::device * newDevice = dynamic_cast<ude::device *>( newPeer );
		
		const char * welcomText = "You are Welcome!";
		
		ude::cTalkPacket welcom ( 0, static_cast<ude::ushort>( strlen( welcomText ) + 1 ) );
		welcom.header().domain = ude::cPacketHeader::domainRDevMan;
		welcom.header().recepient = 0xFFFF;
		strcpy( reinterpret_cast<char *>( &(welcom.byte( 0 )) ), welcomText );
		newDevice->take( welcom );
		
		return newPeer;
	}
	
	virtual bool handleDisconnect ( CrossClass::basicNetPoint * peer )
	{
		std::cout << "client disconnected" << std::endl;
		return ude::device_manager::handleDisconnect( peer );
	}
	
public:
	sdm () : ude::device_manager( ) { }
	virtual ~sdm () { }
};

sdm * netServ;

void	terminate ( int signal )
{
	netServ->postTerminate();
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
	
	sdm server;
	CrossClass::cSockAddr sa ( "127.0.0.1", 19802 );
	server.bindSocket( sa );
	
	//
	// set Ctrl+C signal handler
	//
	std::cout << "Press Ctrl+C to terminate ... " << std::endl;
	netServ = &server;
	signal( SIGINT, terminate );
	
	while( server.serverSendRecv() )
	{
		while( server.proceed_response() );
		while( server.proceed_request() );
	}
	
#if defined( USE_WIN32_API )
	WSACleanup();
#endif
	return 0;
}

