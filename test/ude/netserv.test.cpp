#include "cross/ude/netpoint.h"
#include <iostream>
#include <sstream>

class netserv : public ude::netPoint
{
protected:
	virtual CrossClass::cHandle<CrossClass::netPoint> handleNewConnection ( CrossClass::cSocket &, const CrossClass::cSockAddr & );
	virtual bool doHandleClient ( CrossClass::netPoint & );
	
	ude::cTalkPacket outPacket;
	ude::cTalkPacket inPacket;
	size_t counter;
	
public:
	netserv ()
		: ude::netPoint( )
		, outPacket( 0x0000, 32, 0xF1F2, 0xF3F4 )
		, inPacket( )
	{
		strcpy( reinterpret_cast<char * const>( outPacket.byte() ), "p: 0" );
	}
};

int main ()
{
	openlog( "cross.netserv.demo", LOG_PID|LOG_CONS, LOG_USER );
	
	netserv server;
	server.startServer( 1980 );
	
	closelog();
	return 0;
}

CrossClass::cHandle<CrossClass::netPoint>
netserv::handleNewConnection ( CrossClass::cSocket & sckt, const CrossClass::cSockAddr & sa )
{
	CrossClass::cHandle<CrossClass::netPoint> peer = ude::netPoint::handleNewConnection( sckt, sa );
	std::cout << "new connection " << peer->getSocket() << " from " << sa.DottedDecimal() << ':' << sa.Port() << std::endl;
	return peer;
}

bool netserv::doHandleClient ( CrossClass::netPoint & peer )
{
	ude::netPoint & np = dynamic_cast<ude::netPoint &>( peer );
	if( np.recvPacket( inPacket ) )
	{
		std::cout
			<< "packet from client: " << std::endl
			<< "\tdomain = " << (void*)( inPacket.domain ) << std::endl
			<< "\trecepient = " << (void*)( inPacket.recepient ) << std::endl
			<< "\tcontents = '" << (const char *)( inPacket.byte() ) << '\'' << std::endl;
	}
	if( np.sendPacket( outPacket ) )
	{
		std::basic_ostringstream<char> strm;
		strm << "p: " << ++counter;
		strcpy( reinterpret_cast<char * const>( outPacket.byte() ), strm.str().c_str() );
	}
	return false;
}
