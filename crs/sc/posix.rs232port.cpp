//////////////////////////////////////////////////////////////////////
//
// posix.rs232port.h: interface for the rs232port class (POSIX API).
// (c) Sep 3, 2010 Oleg N. Peregudov
//
//////////////////////////////////////////////////////////////////////

#include <crs/sc/posix.rs232port.h>
#include <cstdio>
#include <cerrno>

static const char * termString = "^X";

namespace sc {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

posixRS232port::posixRS232port( const size_t inBufSize )
	: basicRS232port( inBufSize )
	, m_hCommPort( -1 )
	, ipcPipeEnd( )
	, pipeInBuf( )
	, pipeInBufPtr( 0 )
	, mutexTerminate( )
	, condTerminate( )
	, flagTerminate( false )
{
	m_DataBits = CS8;
	m_StopBits = 0;
	m_Parity = NOPARITY;
}

posixRS232port::~posixRS232port()
{
	close();
}

void posixRS232port::open ( const std::string & portName, const size_t baudRate )
{
	//
	// Close previous session
	//
	close();
	
	//
	// Open communication port handle
	//
	m_Baud = baudRate;
	m_cComPortName = portName;
	m_hCommPort = ::open( m_cComPortName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY );
	if( m_hCommPort == -1 )
	{
		char msgText [ 64 ];
		sprintf( msgText, "open (%d)", errno );
		throw basicRS232port::errOpen( msgText );
	}
	
	//
	// Set overall connect flag
	//
	m_bConnected = true;
	
	//
	// Set port state
	//
	UpdateConnection();
	
	//
	// create an interprocess communication pipe
	//
	if( pipe( ipcPipeEnd ) == -1 )
	{
		char msgText [ 64 ];
		sprintf( msgText, "pipe (%d)", errno );
		throw basicRS232port::errOpen( msgText );
	}
	else
		pipeInBufPtr = pipeInBuf;
}

void posixRS232port::UpdateConnection()
{
	termios options;
	
	//
	// get port options
	//
	if( tcgetattr( m_hCommPort, &options ) == -1 )
	{
		char msgText [ 64 ];
		sprintf( msgText, "tcgetattr (%d)", errno );
		throw basicRS232port::errStatus( msgText );
	}
	
	//
	// preserve original port options
	//
	memcpy( &m_portOptions, &options, sizeof( termios ) );
	
	//
	// enable the receiver and set local mode
	//
	options.c_cflag |= ( CLOCAL | CREAD );
	
	//
	// update baud rate
	//
	cfsetispeed( &options, m_Baud );
	cfsetospeed( &options, m_Baud );
	
	//
	// update parity and stopbits
	//
	switch( m_Parity )
	{
	case	ODDPARITY:
		// odd parity (7O1)
		options.c_cflag |= PARENB;	// parity enable bit
		options.c_cflag |= PARODD;	// odd parity, else even
		options.c_cflag &= ~CSTOPB;	// send two stop bits, else one
		options.c_cflag &= ~CSIZE;	// character size
		options.c_cflag |= CS7;
		break;
	
	case	EVENPARITY:
		// even parity (7E1)
		options.c_cflag |= PARENB;
		options.c_cflag &= ~PARODD;
		options.c_cflag &= ~CSTOPB;
		options.c_cflag &= ~CSIZE;
		options.c_cflag |= CS7;
		break;
	
	default:
		// no parity (8N1)
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		options.c_cflag &= ~CSIZE;
		options.c_cflag |= CS8;
	}
	
	//
	// choosing raw input & output
	//
	options.c_lflag &= ~( ICANON | ECHO | ECHOE | ISIG );
	options.c_oflag &= ~OPOST;
	
	//
	// read timeouts
	//
	options.c_cc[ VMIN ] = 0;	// the minimum number of characters to read
	options.c_cc[ VTIME ] = 15;	// the time to wait for the first character read
	
	//
	// setting software flow control
	//
	options.c_iflag &= ~( IXON | IXOFF | IXANY );	// disable software flow control
	
	//
	// set new port options
	//
	if( tcsetattr( m_hCommPort, TCSANOW, &options ) == -1 )
	{
		char msgText [ 64 ];
		sprintf( msgText, "tcsetattr (%d)", errno );
		throw basicRS232port::errStatus( msgText );
	}
}

void posixRS232port::close()
{
	if( m_bConnected )
	{
		//
		// close termination pipe
		//
		if( ::close( ipcPipeEnd[ 1 ] ) == -1 )
		{
			char msgText [ 64 ];
			sprintf( msgText, "pipe#1 (%d)", errno );
			throw basicRS232port::errClose( msgText );
		}
		if( ::close( ipcPipeEnd[ 0 ] ) == -1 )
		{
			char msgText [ 64 ];
			sprintf( msgText, "pipe#0 (%d)", errno );
			throw basicRS232port::errClose( msgText );
		}
		
		//
		// Reset overall connect flag
		//
		m_bConnected = false;
		
		//
		// restore original port options
		//
		if( tcsetattr( m_hCommPort, TCSANOW, &m_portOptions ) == -1 )
		{
			char msgText [ 64 ];
			sprintf( msgText, "on close: tcsetattr (%d)", errno );
			throw basicRS232port::errStatus( msgText );
		}
		
		//
		// do close port
		//
		if( ::close( m_hCommPort ) == -1 )
		{
			char msgText [ 64 ];
			sprintf( msgText, "close (%d)", errno );
			throw basicRS232port::errClose( msgText );
		}
	}
}

bool posixRS232port::wait4write ()
{
	fd_set readset;
	fd_set writeset;
	
	int highdesc = m_hCommPort;
	if( highdesc < ipcPipeEnd[ 0 ] )
		highdesc = ipcPipeEnd[ 0 ];
	
	while( true )
	{
		FD_ZERO( &readset );
		FD_ZERO( &writeset );
		FD_SET( m_hCommPort, &writeset );
		FD_SET( ipcPipeEnd[ 0 ], &readset );
		
		switch( ::select( highdesc + 1, &readset, &writeset, NULL, NULL ) )
		{
		case	-1:	// select failed
			{
				char msgText [ 64 ];
				sprintf( msgText, "select (%d)", errno );
				throw basicRS232port::errWrite( msgText );
			}
		
		case	0:	// timeout
			return false;
		
		default:
			if( FD_ISSET( m_hCommPort, &writeset ) )
				return true;	// we can write
			if( FD_ISSET( ipcPipeEnd[ 0 ], &readset ) && checkTerminate() )
				return false;	// we have to stop
		}
	}
}

bool	posixRS232port::checkTerminate ()
{
	long	nBytesTotal = sizeof( pipeInBuf ) / sizeof( char ) - 1,
		nBytesRest = nBytesTotal - ( pipeInBufPtr - pipeInBuf );
	if( nBytesRest )
	{
		long nBytesRead = ::read( ipcPipeEnd[ 0 ], pipeInBufPtr, nBytesRest );
		if( nBytesRead == -1 )
		{
			char msgText [ 64 ];
			sprintf( msgText, "pipe read (%d)", errno );
			throw basicRS232port::errRead( msgText );
		}
		pipeInBufPtr += nBytesRead;
		*pipeInBufPtr = 0;
		if( strcmp( pipeInBuf, termString ) == 0 )
			return true;
	}
	else
		pipeInBufPtr = pipeInBuf;
	return false;
}

void posixRS232port::write ( const char * lpBuf, const size_t dwToWrite )
{
	long nBytesWritten;
	const char * pWriteFrom = lpBuf;
	for( size_t nBytesRest = dwToWrite; nBytesRest; )
		if( wait4write() )
		{
			nBytesWritten = ::write( m_hCommPort, pWriteFrom, nBytesRest );
			if( nBytesWritten == -1 )
			{
				char msgText [ 64 ];
				sprintf( msgText, "write (%d)", errno );
				throw basicRS232port::errWrite( msgText );
			}
			pWriteFrom += nBytesWritten;
			nBytesRest -= nBytesWritten;
		}
		else
			break;
}

bool posixRS232port::receive ()
{
	fd_set readset;
	
	int highdesc = m_hCommPort;
	if( highdesc < ipcPipeEnd[ 0 ] )
		highdesc = ipcPipeEnd[ 0 ];
	
	FD_ZERO( &readset );
	FD_SET( m_hCommPort, &readset );
	FD_SET( ipcPipeEnd[ 0 ], &readset );
	
	switch( ::select( highdesc + 1, &readset, NULL, NULL, NULL ) )
	{
	case	-1:	// select failed
		if( errno != EINTR )
		{
			char msgText [ 64 ];
			sprintf( msgText, "select (%d)", errno );
			throw basicRS232port::errRead( msgText );
		}
		break;
	
	case	0:
		break;// timeout
	
	default:	// the data could be read
		if( FD_ISSET( ipcPipeEnd[ 0 ], &readset ) && checkTerminate() )
		{
			CrossClass::_LockIt lockTerminate ( mutexTerminate );
			flagTerminate = true;
			condTerminate.notify_one();
			return false;	// we have to stop
		}
		if( FD_ISSET( m_hCommPort, &readset ) )
		{
			long dwRead = ::read( m_hCommPort, _inBufPtr, _inBufSize - ( _inBufPtr - _inBuf ) );
			if( dwRead == -1 )
			{
				switch( errno )
				{
				case	EINTR:
				case	EAGAIN:
					break;
				
				default:
					{
						char msgText [ 64 ];
						sprintf( msgText, "read (%d)", errno );
						throw basicRS232port::errRead( msgText );
					}
					break;
				}
			}
			else if( dwRead )
				processIncomingBytes( dwRead );
		}
	}
	return true;
}

void posixRS232port::postTerminate ()
{
	long	nBytesWritten = 0;
	size_t nBytes2Write = strlen( termString ),
		 nBytesRest = nBytes2Write;
	for( const char * p = termString; nBytesRest; )
	{
		nBytesWritten = ::write( ipcPipeEnd[ 1 ], p, nBytesRest );
		if( nBytesWritten == -1 )
		{
			char msgText [ 64 ];
			sprintf( msgText, "write pipe (%d)", errno );
			throw basicRS232port::errWrite( msgText );
		}
		else
		{
			nBytesRest -= nBytesWritten;
			p += nBytesWritten;
		}
	}
	
	CrossClass::_LockIt lockTerminate ( mutexTerminate );
	while( !flagTerminate )
		condTerminate.wait( lockTerminate );
	flagTerminate = false;
}

} // namespace sc
