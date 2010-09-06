// (c) Jan 31, 2009 Oleg N. Peregudov
// Apr 23, 2009 - Win/Posix defines
// Aug 26, 2010 - new server termination algorithm based on pipes
#include <crs/bits/posix.netpoint.h>
#include <cstring>
#include <sstream>

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
{
}

posixNetPoint::posixNetPoint ( cSocket & clientSocket )
	: basicNetPoint( clientSocket )
	, ipcPipeEnd( )
	, pipeInBuf( )
	, pipeInBufPtr( 0 )
{
	setNonBlock();
}

posixNetPoint::~posixNetPoint ()
{
	shutdown( _socket, SHUT_RDWR );
}

void posixNetPoint::setNonBlock ()
{
	// set socket to non-blocking
	int opts = fcntl( _socket, F_GETFL, 0 );
	if( opts == -1 )
	{
		std::basic_ostringstream<char> errMsg;
		errMsg << "F_GETFL (" << errno << ')';
		throw basicNetPoint::socket_options_error( errMsg.str() );
	}
	else if( fcntl( _socket, F_SETFL, ( opts | O_NONBLOCK ) ) == -1 )
	{
		std::basic_ostringstream<char> errMsg;
		errMsg << "F_SETFL (" << errno << ')';
		throw basicNetPoint::socket_options_error( errMsg.str() );
	}
}

void posixNetPoint::buildSelectList ()
{
	basicNetPoint::buildSelectList();
	if( highsock < ipcPipeEnd[ 0 ] )
		highsock = ipcPipeEnd[ 0 ];
	FD_SET( ipcPipeEnd[ 0 ], &readset );
	FD_SET( ipcPipeEnd[ 0 ], &exceptset );
}

void posixNetPoint::bindSocket ( const cSockAddr & sa )
{
	// so we can re-bind to it without TIME_WAIT problems
	int reuse_addr = 1;
	if( setsockopt( _socket, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof( reuse_addr ) ) == -1 )
	{
		std::basic_ostringstream<char> errMsg;
		errMsg << "setsockopt (" << errno << ')';
		throw basicNetPoint::socket_options_error( errMsg.str() );
	}
	
	basicNetPoint::bindSocket( sa );
}

timeval * posixNetPoint::onStartServer ()
{
	// create an intereprocess channel (pipe)
	if( pipe( ipcPipeEnd ) == -1 )
	{
		std::basic_ostringstream<char> errMsg;
		errMsg << "failed to create a pipe (" << errno << ')';
		throw basicNetPoint::server_startup_error( errMsg.str() );
	}
	pipeInBufPtr = pipeInBuf;
	return 0;
}

bool posixNetPoint::postCheckTerminate ()
{
	if( FD_ISSET( ipcPipeEnd[ 0 ], &readset ) )	// termination event ?
	{
		long	nBytesTotal = sizeof( pipeInBuf ) / sizeof( char ) - 1,
			nBytesRest = nBytesTotal - ( pipeInBufPtr - pipeInBuf );
		if( nBytesRest )
		{
			long nBytesRead = read( ipcPipeEnd[ 0 ], pipeInBufPtr, nBytesRest );
			if( nBytesRead == -1 )
			{
				std::basic_ostringstream<char> errMsg;
				errMsg << "pipe read error (" << errno << ')';
				throw basicNetPoint::read_error( errMsg.str() );
			}
			pipeInBufPtr += nBytesRead;
			*pipeInBufPtr = 0;
			if( strcmp( pipeInBuf, termString ) == 0 )
			{
				close( ipcPipeEnd[ 1 ] );
				close( ipcPipeEnd[ 0 ] );
				return true;
			}
		}
		else
			pipeInBufPtr = pipeInBuf;
	}
	return false;
}

void posixNetPoint::logGracefulDisconnect ()
{
	syslog( LOG_DEBUG, "netPoint::graceful disconnect" );
}

void posixNetPoint::logRuntimeError ( const std::string & msg )
{
	syslog( LOG_ERR, msg.c_str() );
}

void posixNetPoint::logUnhandledError ()
{
	syslog( LOG_ERR, "unhandled exception in 'netPoint::client_handler::operator ()'" );
}

void posixNetPoint::terminateServer ()
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
			std::basic_ostringstream<char> errMsg;
			errMsg << "pipe write error (" << errno << ')';
			throw basicNetPoint::write_error( errMsg.str() );
		}
		else
		{
			nBytesRest -= nBytesWritten;
			p += nBytesWritten;
		}
	}
}

cHandle<basicNetPoint> posixNetPoint::handleNewConnection ( cSocket & s, const cSockAddr & sa )
{
	return new posixNetPoint( s );
}

} // namespace CrossClass

