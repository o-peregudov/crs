// (c) Jan 31, 2009 Oleg N. Peregudov
// 04/23/2009 - Win/Posix defines
// 08/24/2010 - new server termination algorithm based on events
// 12/01/2010 - new name for termination method
//              new implementation
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#endif
#include <crs/bits/win32.netpoint.h>
#include <cstdio>

namespace CrossClass {

//
// members of class win32NetPoint
//
win32NetPoint::win32NetPoint ()
	: basicNetPoint( )
	, readset( )
	, writeset( )
	, exceptset( )
	, highsock( )
	, evntTerminate( CreateEvent( NULL, FALSE, FALSE, NULL ) )
	, selectTimeOut( )
{
}

win32NetPoint::win32NetPoint ( cSocket & clientSocket )
	: basicNetPoint( clientSocket )
	, readset( )
	, writeset( )
	, exceptset( )
	, highsock( )
	, evntTerminate( CreateEvent( NULL, FALSE, FALSE, NULL ) )
	, selectTimeOut( )
{
	setNonBlock();
}

win32NetPoint::~win32NetPoint ()
{
	//
	// disconnect all clients
	//
	disconnectAll();
	
	//
	// shutdown socket
	//
	shutdown( _socket, SD_BOTH );
	
	//
	// release system handle
	//
	CloseHandle( evntTerminate );
}

basicNetPoint * win32NetPoint::handleNewConnection ( cSocket & s, const cSockAddr & sa )
{
	return new win32NetPoint( s );
}

void win32NetPoint::buildSelectList ()
{
	FD_ZERO( &readset );
	FD_ZERO( &writeset );
	FD_ZERO( &exceptset );
	
	highsock = _socket;
	FD_SET( _socket, &readset );
	FD_SET( _socket, &exceptset );
	
	// fill in clients
	for( size_t i = 0; i < _nClients; ++i )
	{
		while( _nClients && ( _clientList[ i ] == 0 ) )
		{
			if( i < --_nClients )
			{
				_clientList[ i ] = _clientList[ _nClients ];
				_clientList[ _nClients ] = 0;
			}
			else
				break;
		}
		if( _clientList[ i ] )
		{
			FD_SET( _clientList[ i ]->getSocket(), &readset );
			FD_SET( _clientList[ i ]->getSocket(), &writeset );
			FD_SET( _clientList[ i ]->getSocket(), &exceptset );
			if( highsock < _clientList[ i ]->getSocket() )
				highsock = _clientList[ i ]->getSocket();
		}
	}
}

bool win32NetPoint::checkTerminate ()
{
	return ( WaitForSingleObject( evntTerminate, 0 ) == WAIT_OBJECT_0 );
}

void win32NetPoint::postTerminate ()
{
	SetEvent( evntTerminate );
}

bool win32NetPoint::clientSendRecv ()
{
	if( checkTerminate() )
		return false;
	else
	{
		FD_ZERO( &readset );
		FD_ZERO( &writeset );
		FD_ZERO( &exceptset );
		
		highsock = _socket;
		FD_SET( _socket, &readset );
		FD_SET( _socket, &writeset );
		FD_SET( _socket, &exceptset );
		
		selectTimeOut.milliseconds( 10 );
		switch( select( highsock+1, &readset, &writeset, &exceptset, &selectTimeOut ) )
		{
		case	SOCKET_ERROR:
			{
				char msgText [ 64 ];
				sprintf( msgText, "code: %d", WSAGetLastError() );
				throw socket_select_error( msgText );
			}
		
		case	0:	// timeout
			break;
		
		default:
			if( FD_ISSET( _socket, &readset ) )
				receive();
			
			if( FD_ISSET( _socket, &writeset ) )
				transmit();
		}
		
		return true;
	}
}

bool win32NetPoint::serverSendRecv ()
{
	if( checkTerminate() )
		return false;
	else
	{
		buildSelectList();
		selectTimeOut.milliseconds( 10 );
		switch( select( highsock+1, &readset, &writeset, &exceptset, &selectTimeOut ) )
		{
		case	SOCKET_ERROR:
			{
				char msgText [ 64 ];
				sprintf( msgText, "code: %d", WSAGetLastError() );
				throw socket_select_error( msgText );
			}
		
		case	0:	// timeout
			break;
		
		default:	// something was selected
			if( FD_ISSET( _socket, &readset ) )			// do we have a new connection ?
			{
				cSocket newPeer;
				cSockAddr newPeerAddr;
				if( _socket.accept( newPeer, newPeerAddr ) )
					addClient( handleNewConnection( newPeer, newPeerAddr ) );
			}
			
			#pragma omp parallel for if ( _nClients > 100 )
			for( int i = 0; i < _nClients; ++i )
			{
				try
				{
					if( FD_ISSET( _clientList[ i ]->getSocket(), &readset ) )
						clientReceive( i );
					
					if( FD_ISSET( _clientList[ i ]->getSocket(), &writeset ) )
						clientTransmit( i );
					
					if( FD_ISSET( _clientList[ i ]->getSocket(), &exceptset ) )
						throw basicNetPoint::end_of_file( "exceptset" );
				}
				catch( ... )
				{
					removeClient( i );
				}
			}
		}
		
		return true;
	}
}

} // namespace CrossClass

