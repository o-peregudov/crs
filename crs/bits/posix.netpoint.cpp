/*
 *  crs/bits/posix.netpoint.cpp
 *  Copyright (c) 2009-2012 Oleg N. Peregudov <o.peregudov@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#if defined (HAVE_CONFIG_H)
#	include "config.h"
#endif

#include <crs/bits/posix.netpoint.h>
#include <cstring>
#include <cstdio>
#include <cerrno>

#define EMSGLENGTH	256

static const char * termString = "^X";
static const char * restartString = "^R";

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
	, postRestartMutex( )
	, postRestartFlag( false )
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
	, postRestartMutex( )
	, postRestartFlag( false )
{
	ipcPipeEnd[ 0 ] = ipcPipeEnd[ 1 ] = -1;
	setNonBlock();
}

posixNetPoint::~posixNetPoint ()
{
	//
	// release pollfds' memory
	//
	delete [] fds;
	fds = 0;
	
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
	for( size_t i = 0; i < _clientList.size(); ++i )
	{
		while( ( i < _clientList.size() ) && ( _clientList[ i ] == 0 ) )
		{
			_clientList[ i ] = _clientList.back();
			_clientList.pop_back();
		}
		if( i < _clientList.size() )
		{
			fds[ nfdsUsed ].fd = _clientList[ i ]->getSocket();
			fds[ nfdsUsed ].events = POLLIN;
			if( _clientList[ i ]->want2transmit() )
				fds[ nfdsUsed ].events |= POLLOUT;
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
		else if( strcmp( pipeInBuf, restartString ) == 0 )
		{
			CrossClass::_LockIt postRestartLock ( postRestartMutex );
			postRestartFlag = false;
			pipeInBufPtr = pipeInBuf;
			return false;
		}
	}
	else
		pipeInBufPtr = pipeInBuf;
	return false;
}

void posixNetPoint::postRestart ()
{
	CrossClass::_LockIt postRestartLock ( postRestartMutex );
	if( postRestartFlag )
		return;
	else
		postRestartFlag = true;
	postRestartLock.unlock();
	long	nBytesWritten = 0;
	size_t nBytes2Write = strlen( restartString ),
		 nBytesRest = nBytes2Write;
	const char * p = restartString;
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
	lfds[ 1 ].events = POLLIN;
	if( want2transmit() )
		lfds[ 1 ].events |= POLLOUT;
	
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
		
		if( lfds[ 1 ].revents & POLLHUP )
			return false;		// connection is broken or closed
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
				if( fds[ i ].revents & POLLOUT )
					clientTransmit( i - 2 );
				if( fds[ i ].revents & POLLIN )
					clientReceive( i - 2 );
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
