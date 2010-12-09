// (c) Jan 31, 2009 Oleg N. Peregudov
// 04/23/2009 - Win/Posix defines
// 08/24/2010 - new server termination algorithm
// 11/30/2010 - new name for termination method
//              new implementation
// 12/05/2010 - stored socket address
//              extended error info
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#endif

#include <crs/bits/basic.netpoint.h>
#include <algorithm>
#include <cstdio>
#include <cerrno>

#define EMSGLENGTH	256

namespace CrossClass {

//
// members of class netPoint
//
basicNetPoint::basicNetPoint ()
	: _socket( AF_INET, SOCK_STREAM, 0 )
	, _sockAddress( )
	, _clientList( 0 )
	, _nClients( 0 )
	, _nClientsAllocated( 0 )
{
	if( _socket == -1 )
		throw socket_allocation_error( "basicNetPoint::basicNetPoint" );
}

basicNetPoint::basicNetPoint ( cSocket & clientSocket )
	: _socket( clientSocket )
	, _sockAddress( )
	, _clientList( 0 )
	, _nClients( 0 )
	, _nClientsAllocated( 0 )
{
}

basicNetPoint::~basicNetPoint ()
{
	disconnectAll();
}

void basicNetPoint::addClient ( basicNetPoint * newClient )
{
	++_nClients;
	if( _nClientsAllocated < _nClients )
	{
		basicNetPoint* * newClients = new basicNetPoint* [ _nClients ];
		memcpy( newClients, _clientList, _nClientsAllocated * sizeof( basicNetPoint* ) );
		std::swap( _clientList, newClients );
		_nClientsAllocated = _nClients;
		delete [] newClients;
	}
	_clientList[ _nClients - 1 ] = newClient;
}

void basicNetPoint::removeClient ( const size_t nClient )
{
	if( handleDisconnect( _clientList[ nClient ] ) )
	{
		delete _clientList[ nClient ];
		_clientList[ nClient ] = 0;
	}
}

void basicNetPoint::clientReceive ( const size_t nClient )
{
	_clientList[ nClient ]->receive();
}

void basicNetPoint::clientTransmit ( const size_t nClient )
{
	_clientList[ nClient ]->transmit();
}

void basicNetPoint::disconnectAll ()
{
	if( _clientList )
	{
		for( int i = 0; i < _nClients; ++i )
			delete _clientList[ i ];
		
		delete [] _clientList;
		
		_nClients = _nClientsAllocated = 0;
		_clientList = 0;
	}
}

void basicNetPoint::setNonBlock ()
{
}

void basicNetPoint::bindSocket ( const cSockAddr & sa )
{
	setNonBlock();
	
	_sockAddress = sa;
	
	if( bind( _socket, sa, sizeof( sa ) ) == -1 )
	{
		char msgText [ EMSGLENGTH ];
		sprintf( msgText, "%s (%d)", strerror( errno ), errno );
		throw socket_bind_error( msgText );
	}
	
	if( listen( _socket, 16 ) == -1 )
	{
		char msgText [ EMSGLENGTH ];
		sprintf( msgText, "%s (%d)", strerror( errno ), errno );
		throw socket_listen_error( msgText );
	}
}

void basicNetPoint::transmit ()
{
}

void basicNetPoint::receive ()
{
}

basicNetPoint * basicNetPoint::handleNewConnection ( cSocket & s, const cSockAddr & sa )
{
	return 0;		// new basicNetPoint( s );
}

size_t basicNetPoint::enumerateDescriptors ()
{
	return _nClients;
}

void basicNetPoint::buildSelectList ()
{
}

bool basicNetPoint::handleDisconnect ( basicNetPoint * )
{
	return true;
}

bool basicNetPoint::checkTerminate ()
{
	return true;	// yes, we have to stop!
}

void basicNetPoint::startServer ( const unsigned short int portNo )
{
	cSockAddr addrAny ( portNo );
	bindSocket( addrAny );
}

void basicNetPoint::clientConnect ( const cSockAddr & sa )
{
	if( connect( _socket, sa, sizeof( sa ) ) == -1 )
	{
		char msgText [ EMSGLENGTH ];
		sprintf( msgText, "%s (%d)", strerror( errno ), errno );
		throw socket_connect_error( msgText );
	}
	else
		setNonBlock();
}

void basicNetPoint::postTerminate ()
{
}
	
bool basicNetPoint::clientSendRecv ()
{
	return true;
}

bool basicNetPoint::serverSendRecv ()
{
	return true;
}

cSocket & basicNetPoint::getSocket ()
{
	return _socket;
}

} // namespace CrossClass

