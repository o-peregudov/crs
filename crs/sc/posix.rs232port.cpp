//////////////////////////////////////////////////////////////////////
//
// posix.rs232port.h: interface for the rs232port class (POSIX API).
// (c) Sep 7, 2010 Oleg N. Peregudov
//	09/09/2010	baudrate constant selector
//	09/16/2010	some non standard baudrate constant fixed
//	09/20/2010	non-blocking postTerminate
//	09/22/2010	using 'poll' instead of 'select'
//	01/03/2011	integer types
//
//////////////////////////////////////////////////////////////////////

#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif

#include <crs/sc/posix.rs232port.h>
#include <cerrno>
#include <poll.h>

static const char * termString = "^C";

namespace sc {

static const size_t baudRateConstant [] = {
	B0, B50, B75, B110, B134, B150, B200, B300, B600,
	B1200, B1800, B2400, B4800, B9600, B19200, B38400,
	B57600, B115200, B230400, B460800, B500000, B576000,
	B921600, B1000000, B1152000, B1500000, B2000000,
	B2500000, B3000000
};

static const size_t baudRate [] = {
	0L, 50L, 75L, 110L, 134L, 150L, 200L, 300L, 600L,
	1200L, 1800L, 2400L, 4800L, 9600L, 19200L, 38400L,
	57600L, 115200L, 230400L, 460800L, 500000L, 576000L,
	921600L, 1000000L, 1152000L, 1500000L, 2000000L,
	2500000L, 3000000L
};

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
	m_Baud = B115200;
	m_DataBits = CS8;
	m_StopBits = 0;
	m_Parity = NOPARITY;
	ipcPipeEnd[ 0 ] = ipcPipeEnd[ 1 ] = -1;
}

posixRS232port::~posixRS232port()
{
	close();
}

void posixRS232port::open ( const std::string & port, const size_t baud )
{
	//
	// Close previous session
	//
	close();
	
	//
	// Select appropriate baud rate constant
	//
	size_t i = 0;
	for(; i < sizeof( baudRate ) / sizeof( baudRate[ 0 ] ); ++i )
		if( baud == baudRate[ i ] )
		{
			m_Baud = baudRateConstant[ i ];
			break;
		}
	if( i == sizeof( baudRate ) / sizeof( baudRate[ 0 ] ) )
		throw basicRS232port::errOpen( "Unsupported baud rate" );
	
	//
	// Open communication port handle
	//
	m_cComPortName = port;
	m_hCommPort = ::open( m_cComPortName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY );
	if( m_hCommPort == -1 )
		throw basicRS232port::errOpen( strerror( errno ) );
	
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
		throw basicRS232port::errOpen( strerror( errno ) );
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
		throw basicRS232port::errStatus( strerror( errno ) );
	
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
	options.c_lflag &= ~( ICANON | ECHO | ISIG | IEXTEN );
	options.c_oflag &= ~OPOST;
	
	//
	// read timeouts
	//
	options.c_cc[ VMIN ] = 0;	// the minimum number of characters to read
	options.c_cc[ VTIME ] = 10;	// the time to wait for the first character read
	
	//
	// setting software flow control
	//
	options.c_iflag &= ~( IXON | IXOFF | IXANY | ICRNL | INLCR );
	
	//
	// set new port options
	//
	if( tcsetattr( m_hCommPort, TCSANOW, &options ) == -1 )
		throw basicRS232port::errStatus( strerror( errno ) );
}

void posixRS232port::close()
{
	if( m_bConnected )
	{
		//
		// close termination pipe
		//
		if( ::close( ipcPipeEnd[ 1 ] ) == -1 )
			throw basicRS232port::errClose( strerror( errno ) );
		if( ::close( ipcPipeEnd[ 0 ] ) == -1 )
			throw basicRS232port::errClose( strerror( errno ) );
		
		//
		// Reset overall connect flag
		//
		m_bConnected = false;
		
		//
		// restore original port options
		//
		if( tcsetattr( m_hCommPort, TCSANOW, &m_portOptions ) == -1 )
			throw basicRS232port::errStatus( strerror( errno ) );
		
		//
		// do close port
		//
		if( ::close( m_hCommPort ) == -1 )
			throw basicRS232port::errClose( strerror( errno ) );
	}
}

bool posixRS232port::wait4write ()
{
	pollfd fds [ 2 ];
	fds[ 0 ].fd = ipcPipeEnd[ 0 ];
	fds[ 0 ].events = POLLIN;
	fds[ 1 ].fd = m_hCommPort;
	fds[ 1 ].events = POLLOUT;
	for(;;)
	{
		switch( poll( fds, sizeof( fds ) / sizeof( pollfd ), -1 ) )
		{
		case	-1:		// poll failed
			if( errno != EINTR )	// any error except signal interrupt
				throw basicRS232port::errPoll( strerror( errno ) );
			break;
		
		case	0:		// poll timeout ?
			break;
		
		default:
			if( fds[ 0 ].revents & POLLIN )
			{
				if( checkTerminate() )
					return false;	// we have to stop
			}
			
			if( fds[ 1 ].revents & POLLOUT )
				return true;
		}
	}
	return false;
}

bool	posixRS232port::checkTerminate ()
{
	long	nBytesTotal = sizeof( pipeInBuf ) / sizeof( char ) - 1,
		nBytesRest = nBytesTotal - ( pipeInBufPtr - pipeInBuf );
	if( nBytesRest )
	{
		long nBytesRead = ::read( ipcPipeEnd[ 0 ], pipeInBufPtr, nBytesRest );
		if( nBytesRead == -1 )
			throw basicRS232port::errRead( strerror( errno ) );
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
				throw basicRS232port::errWrite( strerror( errno ) );
			pWriteFrom += nBytesWritten;
			nBytesRest -= nBytesWritten;
		}
		else
			break;
}

bool posixRS232port::receive ()
{
	pollfd fds [ 2 ];
	fds[ 0 ].fd = ipcPipeEnd[ 0 ];
	fds[ 0 ].events = POLLIN;
	fds[ 1 ].fd = m_hCommPort;
	fds[ 1 ].events = POLLIN;
	switch( poll( fds, sizeof( fds ) / sizeof( pollfd ), -1 ) )
	{
	case	-1:		// poll failed
		if( errno != EINTR )	// any error except signal interrupt
			throw basicRS232port::errPoll( strerror( errno ) );
		break;
	
	case	0:		// poll timeout ?
		break;
	
	default:		// something was selected
		if( fds[ 0 ].revents & POLLIN )
		{
			if( checkTerminate() )
			{
				CrossClass::_LockIt lockTerminate ( mutexTerminate );
				flagTerminate = true;
				condTerminate.notify_one();
				return false;	// we have to stop
			}
		}
		
		if( fds[ 1 ].revents & POLLIN )
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
					throw basicRS232port::errRead( strerror( errno ) );
				}
			}
			else if( dwRead )
				processIncomingBytes( dwRead );
		}
	}
	return true;	// we have to continue the loop
}

void posixRS232port::postTerminate ( const bool doWaitTerminate )
{
	long	nBytesWritten = 0;
	size_t nBytes2Write = strlen( termString ),
		 nBytesRest = nBytes2Write;
	for( const char * p = termString; nBytesRest; )
	{
		nBytesWritten = ::write( ipcPipeEnd[ 1 ], p, nBytesRest );
		if( nBytesWritten == -1 )
			throw basicRS232port::errWrite( strerror( errno ) );
		else
		{
			nBytesRest -= nBytesWritten;
			p += nBytesWritten;
		}
	}
	
	if( doWaitTerminate )
	{
		CrossClass::_LockIt lockTerminate ( mutexTerminate );
		while( !flagTerminate )
			condTerminate.wait( lockTerminate );
		flagTerminate = false;
	}
}

} // namespace sc
