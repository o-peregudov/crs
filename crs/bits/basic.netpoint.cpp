// (c) Jan 31, 2009 Oleg N. Peregudov
// Apr 23, 2009 - Win/Posix defines
// Aug 24, 2010 - new server termination algorithm
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#endif
#include <crs/bits/basic.netpoint.h>
#include <algorithm>
#include <sstream>

namespace CrossClass {

//
// members of class netPoint
//
basicNetPoint::basicNetPoint ()
	: readset( )
	, writeset( )
	, exceptset( )
	, highsock( 0 )
	, clientList( )
	, selectTimeOut( )
	, _socket( AF_INET, SOCK_STREAM, 0 )
{
	if( _socket == -1 )
		throw socket_allocation_error( "basicNetPoint::basicNetPoint" );
}

basicNetPoint::basicNetPoint ( cSocket & clientSocket )
	: readset( )
	, writeset( )
	, exceptset( )
	, highsock( 0 )
	, clientList( )
	, selectTimeOut( )
	, _socket( clientSocket )
{
}

basicNetPoint::~basicNetPoint ()
{
}

void basicNetPoint::buildSelectList ()
{
	FD_ZERO( &readset );
	FD_ZERO( &writeset );
	FD_ZERO( &exceptset );
	
	highsock = _socket;
	FD_SET( _socket, &readset );
	FD_SET( _socket, &exceptset );
	
	size_t nClients = clientList.size();
	for( size_t i = 0; i < nClients; ++i )
	{
		while( nClients && ( clientList[ i ].get() == 0 ) )
		{
			clientList[ i ] = clientList[ nClients - 1 ];
			clientList[ nClients - 1 ].bind( 0 );
			--nClients;
		}
		
		if( nClients == 0 )
			break;
		
		if( clientList[ i ]->getSocket() > 0 )
		{
			FD_SET( clientList[ i ]->getSocket(), &readset );
			FD_SET( clientList[ i ]->getSocket(), &writeset );
			FD_SET( clientList[ i ]->getSocket(), &exceptset );
			if( highsock < clientList[ i ]->getSocket() )
				highsock = clientList[ i ]->getSocket();
		}
	}
	if( nClients < clientList.size() )
		clientList.resize( nClients );
}

bool basicNetPoint::doServerJob ( const bool idleState )
{
	return false;
}

void basicNetPoint::bindSocket ( const cSockAddr & sa )
{
	setNonBlock();
	
	if( bind( _socket, sa, sizeof( sa ) ) == -1 )
	{
		std::basic_ostringstream<char> errMsg;
		errMsg << "bind (" << errno << ')';
		throw socket_bind_error( errMsg.str() );
	}
	
	if( listen( _socket, 16 ) == -1 )
	{
		std::basic_ostringstream<char> errMsg;
		errMsg << "listen (" << errno << ')';
		throw socket_listen_error( errMsg.str() );
	}
}

void basicNetPoint::client_handler::operator () ( cHandle<basicNetPoint> & client )
{
	bool clientChanged = false;
	try
	{
		if( FD_ISSET( client->getSocket(), &(base->readset) ) )
		{
			clientChanged = true;
			client->receive();
		}
		
		if( FD_ISSET( client->getSocket(), &(base->writeset) ) )
		{
			clientChanged = true;
			client->transmit();
		}
		
		if( FD_ISSET( client->getSocket(), &(base->exceptset) ) )
		{
			clientChanged = true;
			client->except();
		}
		
		if( clientChanged && base->doHandleClient( *(client.get()) ) )
			if( base->handleDisconnect( *(client.get()) ) )
				client.bind( 0 );
		
		return;
	}
	catch( basicNetPoint::end_of_file )
	{
		client->logGracefulDisconnect();
	}
	catch( std::runtime_error & e )
	{
		client->logRuntimeError( e.what() );
	}
	catch( ... )
	{
		client->logUnhandledError();
	}
	
	if( base->handleDisconnect( *(client.get()) ) )
		client.bind( 0 );
}

void basicNetPoint::startServer ( const cSockAddr & sa )
{
	bindSocket( sa );
	timeval * pSelectTimeOut = onStartServer();
	bool	clientChanged = false,
		idleState = false;
	while( true )
	{
		if( preCheckTerminate() )
			return;
		idleState = false;
		buildSelectList();
		switch( select( highsock+1, &readset, &writeset, &exceptset, pSelectTimeOut ) )
		{
		case	-1:	// failed to select
			{
				int selectError = errno;
				if( selectError == EBADF )
					continue;
				
				std::basic_ostringstream<char> errMsg;
				errMsg << "select (" << selectError << ')';
				throw socket_select_error( errMsg.str() );
			}
		
		case	0:	// timeout
			idleState = true;
			break;
		
		default:	// something was selected
			if( FD_ISSET( _socket, &readset ) )			// new connection ?
			{
				cSocket newPeer;
				cSockAddr newPeerAddr;
				if( _socket.accept( newPeer, newPeerAddr ) )
					clientList.push_back( handleNewConnection( newPeer, newPeerAddr ) );
			}
			if( postCheckTerminate() )
				return;
			std::for_each( clientList.begin(), clientList.end(), client_handler( this ) );
			/*{
				client_handler handler ( this );
				
				#pragma omp parallel for
				for( int i = 0; i < clientList.size(); ++i )
					handler( clientList[ i ] );
			}*/
		}
		
		if( doServerJob( idleState ) )
			break;
	}
}

void basicNetPoint::startServer ( const unsigned short int portNo )
{
	cSockAddr addrAny ( portNo );
	startServer( addrAny );
}

void basicNetPoint::clientConnect ( const cSockAddr & sa )
{
	if( connect( _socket, sa, sizeof( sa ) ) == -1 )
	{
		std::basic_ostringstream<char> errMsg;
		errMsg << "connect (" << errno << ')';
		throw socket_connect_error( errMsg.str() );
	}
	else
		setNonBlock();
}

void basicNetPoint::setNonBlock ()
{
}

cHandle<basicNetPoint> basicNetPoint::handleNewConnection ( cSocket & s, const cSockAddr & sa )
{
	return 0;	// new basicNetPoint( s );
}

cSocket & basicNetPoint::getSocket ()
{
	return _socket;
}
	
bool basicNetPoint::doHandleClient ( basicNetPoint & )
{
	return false;
}

void basicNetPoint::clientSendRecv ()
{
	FD_ZERO( &readset );
	FD_ZERO( &writeset );
	FD_ZERO( &exceptset );
	
	highsock = _socket;
	FD_SET( _socket, &readset );
	FD_SET( _socket, &writeset );
	FD_SET( _socket, &exceptset );
	
	selectTimeOut.milliseconds( 5 );
	if( select( highsock+1, &readset, &writeset, &exceptset, &selectTimeOut ) == -1 )
	{
		std::basic_ostringstream<char> errMsg;
		errMsg << "select (" << errno << ')';
		throw socket_select_error( errMsg.str() );
	}
	
	if( FD_ISSET( _socket, &readset ) )
		receive();
	
	if( FD_ISSET( _socket, &writeset ) )
		transmit();
	
	if( FD_ISSET( _socket, &exceptset ) )
		except();
}

bool basicNetPoint::handleDisconnect ( basicNetPoint & )
{
	return true;
}

void basicNetPoint::transmit ()
{
}

void basicNetPoint::receive ()
{
}

void basicNetPoint::except ()
{
}

timeval * basicNetPoint::onStartServer ()
{
	return 0;
}

bool basicNetPoint::preCheckTerminate ()
{
	return false;
}

bool basicNetPoint::postCheckTerminate ()
{
	return false;
}

void basicNetPoint::logGracefulDisconnect ()
{
}

void basicNetPoint::logRuntimeError ( const std::string & msg )
{
}

void basicNetPoint::logUnhandledError ()
{
}

void basicNetPoint::terminateServer ()
{
}

} // namespace CrossClass

