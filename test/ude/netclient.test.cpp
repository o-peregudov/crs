#include "cross/ude/netpoint.h"
#include <iostream>

int main ()
{
	ude::netPoint np;
	CrossClass::cSockAddr sa ( "127.0.0.1", 1980 );
	std::cout << "Connecting to server ... ";
	np.clientConnect( sa );
	std::cout << "done" << std::endl;
	
	ude::cTalkPacket	outPacket,
				inPacket;
	for( size_t i = 0; i < 1500; np.clientSendRecv() )
	{
		if( np.recvPacket( inPacket ) )
		{
			++i;
			std::cout
				<< "packet from server: " << std::endl
				<< "\tdomain = " << (void*)( inPacket.domain ) << std::endl
				<< "\trecepient = " << (void*)( inPacket.recepient ) << std::endl
				<< "\tsender = " << (void*)( inPacket.sender ) << std::endl
				<< "\tcrc = " << (void*)( inPacket.crc ) << std::endl
				<< "\tcrc2 = " << (void*)( inPacket.crc2 ) << std::endl
				<< "\tcontents = '" << (const char *)( inPacket.byte() ) << '\'' << std::endl;
		}
	}
	std::cout << "close" << std::endl;
	
	return 0;
}
