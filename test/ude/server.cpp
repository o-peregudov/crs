#include <crs/ude/server.h>
#include <csignal>
#include <iostream>
#include <sstream>

class sdm : public ude::server_device_manager
{
protected:
	virtual bool checkTerminate ()
	{
		bool result = ude::server_device_manager::checkTerminate();
		if( result )
			std::cout << "\rGot terminate signal!" << std::endl;
		return result;
	}
	
	virtual CrossClass::basicNetPoint * handleNewConnection ( CrossClass::cSocket & s, const CrossClass::cSockAddr & sa )
	{
		std::cout << "Connection from " << sa.DottedDecimal() << ':' << sa.Port() << std::endl;
		CrossClass::basicNetPoint * newPeer = ude::server_device_manager::handleNewConnection( s, sa );
		ude::basic_device * newDevice = dynamic_cast<ude::basic_device *>( newPeer );
		
		const char * welcomText = "Welcome!";
		
		ude::cTalkPacket welcom ( 0, strlen( welcomText ) + 1 );
		welcom.domain = ude::cTalkPacket::domainRDevMan;
		welcom.recepient = 0xFFFF;
		strcpy( reinterpret_cast<char *>( &(welcom.byte( 0 )) ), welcomText );
		newDevice->take( welcom );
		
		return newPeer;
	}
	
	virtual bool handleDisconnect ( CrossClass::basicNetPoint * peer )
	{
		std::cout << "client disconnected" << std::endl;
		return ude::server_device_manager::handleDisconnect( peer );
	}
	
	virtual bool _send ( const ude::cTalkPacket & );
	
public:
	sdm ()
		: ude::server_device_manager( )
	{
	}
	
	virtual ~sdm ()
	{
	}
};

bool sdm::_send ( const ude::cTalkPacket & packet )
{
	switch( packet.domain )
	{
	case	ude::cTalkPacket::domainPMeasure:
		proceed_response( packet );
		break;
	
	default:
		break;
	}
	return true;
}

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
		server.proceed_response( );
		server.proceed_request( 1 );
	}
	
#if defined( USE_WIN32_API )
	WSACleanup();
#endif
	return 0;
}

