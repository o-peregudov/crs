// (c) Jan 31, 2009 Oleg N. Peregudov
// 04/23/2009 - Win/Posix defines
// 08/26/2010 - new server termination algorithm based on pipes
// 11/30/2010 - usage of the poll system call
// 12/04/2010 - checkTerminate bug fixed (pipe close)
// 12/05/2010 - buildClientList bug fixed
//              extended error info
#include <crs/bits/posix.netpoint.h>
#include <cstring>
#include <cstdio>
#include <cerrno>

#define EMSGLENGTH	256

static const char * termString = "^X";

namespace CrossClass {

//
// members of class posixNetPoint
//
posixNetPoint::posixNetPoint ()
	: basicNetPoint( )
	, ipcPipeEnd( )
	, pipeInBuf( )
	, pipeInBufPtr( 0 )
	, fds( 0 )
	, nfdsAllocated( 0 )
	, nfdsUsed( 0 )
{
	ipcPipeEnd[ 0 ] = ipcPipeEnd[ 1 ] = -1;
}

posixNetPoint::posixNetPoint ( cSocket & clientSocket )
	: basicNetPoint( clientSocket )
	, ipcPipeEnd( )
	, pipeInBuf( )
	, pipeInBufPtr( 0 )
	, fds( 0 )
	, nfdsAllocated( 0 )
	, nfdsUsed( 0 )
{
	ipcPipeEnd[ 0 ] = ipcPipeEnd[ 1 ] = -1;
	setNonBlock();
}

posixNetPoint::~posixNetPoint ()
{
	//
	// disconnect all clients
	//
	disconnectAll();
	
	//
	// shutdown socket
	//
	shutdown( _socket, SHUT_RDWR );
	
	//
	// close termination pipe
	//
	closePipe();
}

void posixNetPoint::createPipe ()
{
	// create an intereprocess channel (pipe)
	if( pipe( ipcPipeEnd ) == -1 )
	{
		char msgText [ EMSGLENGTH ];
		sprintf( msgText, "%s (%d)", strerror( errno ), errno );
		throw basicNetPoint::pipe_open_error( msgText );
	}
	pipeInBufPtr = pipeInBuf;
}

void posixNetPoint::closePipe ()
{
	if( ( ipcPipeEnd[ 0 ] != -1 ) && ( ipcPipeEnd[ 1 ] != -1 ) )
	{
		// close an intereprocess channel (pipe)
		if( ::close( ipcPipeEnd[ 1 ] ) == -1 )
		{
			char msgText [ EMSGLENGTH ];
			sprintf( msgText, "%s (%d)", strerror( errno ), errno );
			throw basicNetPoint::pipe_close_error( msgText );
		}
		if( ::close( ipcPipeEnd[ 0 ] ) == -1 )
		{
			char msgText [ EMSGLENGTH ];
			sprintf( msgText, "%s (%d)", strerror( errno ), errno );
			throw basicNetPoint::pipe_close_error( msgText );
		}
		ipcPipeEnd[ 0 ] = ipcPipeEnd[ 1 ] = -1;
	}
}

void posixNetPoint::setNonBlock ()
{
	// set socket to non-blocking
	int opts = fcntl( _socket, F_GETFL, 0 );
	if( opts == -1 )
	{
		char msgText [ EMSGLENGTH ];
		sprintf( msgText, "%s (%d)", strerror( errno ), errno );
		throw basicNetPoint::socket_options_error( msgText );
	}
	else if( fcntl( _socket, F_SETFL, ( opts | O_NONBLOCK ) ) == -1 )
	{
		char msgText [ EMSGLENGTH ];
		sprintf( msgText, "%s (%d)", strerror( errno ), errno );
		throw basicNetPoint::socket_options_error( msgText );
	}
}

basicNetPoint * posixNetPoint::handleNewConnection ( cSocket & s, const cSockAddr & sa )
{
	return new posixNetPoint( s );
}

size_t posixNetPoint::enumerateDescriptors ()
{
	return basicNetPoint::enumerateDescriptors() + 2;
}

void posixNetPoint::buildSelectList ()
{
	// allocate memory for pollfd structures
	size_t nfds = enumerateDescriptors();
	if( nfdsAllocated < nfds )
	{
		delete [] fds;
		fds = new pollfd [ ( nfdsAllocated = nfds ) ];
	}
	
	// fill in basic handles - self socket and termination pipe
	fds[ 0 ].fd = ipcPipeEnd[ 0 ];
	fds[ 0 ].events = POLLIN;
	fds[ 1 ].fd = _socket;
	fds[ 1 ].events = POLLIN;
	nfdsUsed = 2;
	
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
			fds[ nfdsUsed ].fd = _clientList[ i ]->getSocket();
			fds[ nfdsUsed ].events = POLLIN|POLLOUT;
			++nfdsUsed;
		}
	}
}

void posixNetPoint::bindSocket ( const cSockAddr & sa )
{
	// so we can re-bind to it without TIME_WAIT problems
	int reuse_addr = 1;
	if( setsockopt( _socket, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof( reuse_addr ) ) == -1 )
	{
		char msgText [ EMSGLENGTH ];
		sprintf( msgText, "%s (%d)", strerror( errno ), errno );
		throw basicNetPoint::socket_options_error( msgText );
	}
	
	basicNetPoint::bindSocket( sa );
	createPipe();
}

void posixNetPoint::clientConnect ( const cSockAddr & sa )
{
	basicNetPoint::clientConnect( sa );
	createPipe();
}

bool posixNetPoint::checkTerminate ()
{
	long	nBytesTotal = sizeof( pipeInBuf ) / sizeof( char ) - 1,
		nBytesRest = nBytesTotal - ( pipeInBufPtr - pipeInBuf );
	if( nBytesRest )
	{
		long nBytesRead = read( ipcPipeEnd[ 0 ], pipeInBufPtr, nBytesRest );
		if( nBytesRead == -1 )
		{
			char msgText [ EMSGLENGTH ];
			sprintf( msgText, "%s (%d)", strerror( errno ), errno );
			throw basicNetPoint::read_error( msgText );
		}
		pipeInBufPtr += nBytesRead;
		*pipeInBufPtr = 0;
		if( strcmp( pipeInBuf, termString ) == 0 )
		{
			pipeInBufPtr = pipeInBuf;
			return true;
		}
	}
	else
		pipeInBufPtr = pipeInBuf;
	return false;
}

void posixNetPoint::postTerminate ()
{
	long	nBytesWritten = 0;
	size_t nBytes2Write = strlen( termString ),
		 nBytesRest = nBytes2Write;
	const char * p = termString;
	while( nBytesRest )
	{
		nBytesWritten = write( ipcPipeEnd[ 1 ], p, nBytesRest );
		if( nBytesWritten == -1 )
		{
			char msgText [ EMSGLENGTH ];
			sprintf( msgText, "%s (%d)", strerror( errno ), errno );
			throw basicNetPoint::write_error( msgText );
		}
		else
		{
			nBytesRest -= nBytesWritten;
			p += nBytesWritten;
		}
	}
}

bool posixNetPoint::clientSendRecv ()
{
	pollfd lfds [ 2 ];
	
	lfds[ 0 ].fd = ipcPipeEnd[ 0 ];
	lfds[ 0 ].events = POLLIN;
	lfds[ 1 ].fd = _socket;
	lfds[ 1 ].events = POLLIN|POLLOUT;
	
	switch( poll( lfds, sizeof( lfds ) / sizeof( pollfd ), -1 ) )
	{
	case	-1:		// poll failed
		if( errno != EINTR )	// any error except signal interrupt
		{
			char msgText [ EMSGLENGTH ];
			sprintf( msgText, "%s (%d)", strerror( errno ), errno );
			throw basicNetPoint::socket_select_error( msgText );
		}
		break;
	
	case	0:		// poll timeout ?
		break;
	
	default:
		if( lfds[ 0 ].revents & POLLIN )
		{
			if( checkTerminate() )
				return false;	// we have to stop
		}
		
		if( lfds[ 1 ].revents & POLLIN )
			receive();
		
		if( lfds[ 1 ].revents & POLLOUT )
			transmit();
	}
	return true;
}

bool posixNetPoint::serverSendRecv ()
{
	buildSelectList();
	switch( poll( fds, nfdsUsed, -1 ) )
	{
	case	-1:		// poll failed
		if( errno != EINTR )	// any error except signal interrupt
		{
			char msgText [ EMSGLENGTH ];
			sprintf( msgText, "%s (%d)", strerror( errno ), errno );
			throw basicNetPoint::socket_select_error( msgText );
		}
		break;
	
	case	0:		// poll timeout ?
		break;
	
	default:
		if( fds[ 0 ].revents & POLLIN )	// do we have to stop?
		{
			if( checkTerminate() )
				return false;		// we have to stop
		}
		
		if( fds[ 1 ].revents & POLLIN )	// do we have a new connection ?
		{
			cSocket newPeer;
			cSockAddr newPeerAddr;
			if( _socket.accept( newPeer, newPeerAddr ) )
				addClient( handleNewConnection( newPeer, newPeerAddr ) );
		}
		
		#pragma omp parallel for if ( nfdsUsed > 100 )
		for( int i = 2; i < nfdsUsed; ++i )
		{
			try
			{
				if( fds[ i ].revents & POLLHUP )
					throw basicNetPoint::end_of_file( "POLLHUP" );
				if( fds[ i ].revents & POLLIN )
					clientReceive( i - 2 );
				if( fds[ i ].revents & POLLOUT )
					clientTransmit( i - 2 );
			}
			catch( ... )
			{
				removeClient( i - 2 );
			}
		}
	}
	
	return true;
}

} // namespace CrossClass

