//////////////////////////////////////////////////////////////////////
//
// posix.rs232port.h: interface for the rs232port class (POSIX API).
// (c) Sep 7, 2010 Oleg N. Peregudov
//	2010/09/09	baudrate constant selector
//	2010/09/16	some non standard baudrate constant fixed
//	2010/09/20	non-blocking postTerminate
//	2010/09/22	using 'poll' instead of 'select'
//	2011/01/03	integer types
//	2011/04/14	reset incoming buffers on open
//	2011/12/03	advanced port open
//			extended exception info
//	2012/05/10	port polling is now moved to a separate class
//	2012/07/10	callback function for eventually port disconnection
//			or other port failure
//
//////////////////////////////////////////////////////////////////////

#if defined (HAVE_CONFIG_H)
#	include "config.h"
#endif

#include <crs/sc/posix.rs232port.h>
#include <cerrno>
#include <cstdio>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

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

static const char portConf [] = 
	"[SerialPortConfig]\n"					\
	"port=/dev/ttyS0\n"					\
	"baud=115200\n"						\
	"data=8\n"							\
	"stop=0\n"							\
	"parity=none; (odd | even | none | space)\n"	\
	"flow=none; (xon | xoff)\n"				\
	;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

posixRS232port::posixRS232port ( const size_t inBufSize )
	: basicRS232port (inBufSize)
	, CrossClass::event_descriptor ()
	, m_hCommPort (-1)
	, bufSize (inBufSize)
	, outBuf (new char [ inBufSize ])
	, outBufPtr (outBuf)
	, outBufMutex ()
{
	m_Baud = B115200;
	m_DataBits = CS8;
	m_StopBits = 0;
	m_Parity = NOPARITY;
}

posixRS232port::~posixRS232port ()
{
	close ();
	delete [] outBuf;
}

void posixRS232port::open ( const std::string & port, const size_t baud )
{
	/*
	 * Close previous session
	 */
	close ();
	
	/*
	 * Select appropriate baud rate constant
	 */
	size_t i = 0;
	for (; i < sizeof (baudRate) / sizeof (baudRate[ 0 ]); ++i)
		if (baud == baudRate[ i ])
		{
			m_Baud = baudRateConstant[ i ];
			break;
		}
	if (i == sizeof (baudRate) / sizeof (baudRate[ 0 ]))
		throw basicRS232port::errOpen ("Unsupported baud rate");
	
	/*
	 * Open communication port handle
	 */
	m_cComPortName = port;
	m_hCommPort = ::open (m_cComPortName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
	if (m_hCommPort == -1)
		throw basicRS232port::errOpen (strerror (errno));
	
	/*
	 * Set overall connect flag
	 */
	m_bConnected = true;
	
	try
	{
		/*
		 * Set port state
		 */
		UpdateConnection ();
	}
	catch (...)
	{
		/*
		 * do close port
		 */
		::close (m_hCommPort);
		
		/*
		 * Reset overall connect flag
		 */
		m_bConnected = false;
		
		/*
		 * re-throw the exception
		 */
		throw;
	}
	
	/*
	 * reset incoming buffers
	 */
	basicRS232port::synchronize (0);
}

void posixRS232port::open ( std::istream & connStream )
{
	std::string token;
	while (connStream.good ())
	{
		std::getline (connStream, token, '=');
		if (token == "port")
		{
			std::getline (connStream, token, ' ');
			m_cComPortName = token;
		}
		else if (token == "baud")
			connStream >> m_Baud;
		else if (token == "dataBits")
			connStream >> m_DataBits;
		else if (token == "stopBits")
			connStream >> m_StopBits;
		else if (token == "parity")
		{
			std::getline (connStream, token, ' ');
			if (token == "odd")
				m_Parity = ODDPARITY;
			else if (token == "even")
				m_Parity = EVENPARITY;
			else if (token == "mark")
				m_Parity = MARKPARITY;
			else if (token == "space")
				m_Parity = SPACEPARITY;
			else
				m_Parity = NOPARITY;
		}
		int ch = connStream.get ();
		if (ch != ' ')
			connStream.putback (ch);
	}
	
	open (m_cComPortName, m_Baud);
}

void posixRS232port::UpdateConnection ()
{
	termios options;
	
	/*
	 * get port options
	 */
	if (tcgetattr (m_hCommPort, &options) == -1)
		throw basicRS232port::errReadStatus (strerror (errno));
	
	/*
	 * preserve original port options
	 */
	memcpy (&m_portOptions, &options, sizeof (termios));
	
	/*
	 * enable the receiver and set local mode
	 */
	options.c_cflag |= ( CLOCAL | CREAD );
	
	/*
	 * update baud rate
	 */
	cfsetispeed (&options, m_Baud);
	cfsetospeed (&options, m_Baud);
	
	/*
	 * update parity and stopbits
	 */
	switch (m_Parity)
	{
	case	ODDPARITY:
		/* odd parity (7O1) */
		options.c_cflag |= PARENB;	/* parity enable bit			*/
		options.c_cflag |= PARODD;	/* odd parity, else even		*/
		options.c_cflag &= ~CSTOPB;	/* send two stop bits, else one	*/
		options.c_cflag &= ~CSIZE;	/* character size				*/
		options.c_cflag |= CS7;
		break;
	
	case	EVENPARITY:
		/* even parity (7E1) */
		options.c_cflag |= PARENB;
		options.c_cflag &= ~PARODD;
		options.c_cflag &= ~CSTOPB;
		options.c_cflag &= ~CSIZE;
		options.c_cflag |= CS7;
		break;
	
	default:
		/* no parity (8N1) */
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		options.c_cflag &= ~CSIZE;
		options.c_cflag |= CS8;
	}
	
	/*
	 * choosing raw input & output
	 */
	options.c_lflag &= ~( ICANON | ECHO | ISIG | IEXTEN );
	options.c_oflag &= ~OPOST;
	
	/*
	 * read timeouts
	 */
	options.c_cc[ VMIN ] = 0;	/* the minimum number of characters to read		*/
	options.c_cc[ VTIME ] = 10;	/* the time to wait for the first character read	*/
	
	/*
	 * setting software flow control
	 */
	options.c_iflag &= ~( IXON | IXOFF | IXANY | ICRNL | INLCR );
	
	/*
	 * set new port options
	 */
	if( tcsetattr( m_hCommPort, TCSANOW, &options ) == -1 )
		throw basicRS232port::errSetStatus( strerror( errno ) );
}

void posixRS232port::close ()
{
	if( m_bConnected )
	{
		/*
		 * restore original port options
		 */
		if (tcsetattr (m_hCommPort, TCSANOW, &m_portOptions) == -1)
			throw basicRS232port::errSetStatus (strerror (errno));
		
		/*
		 * Reset overall connect flag
		 */
		m_bConnected = false;
		
		/*
		 * do close port
		 */
		if (::close (m_hCommPort) == -1)
			throw basicRS232port::errClose (strerror (errno));
	}
}

bool posixRS232port::post ( const char * lpBuf, const size_t dwToWrite )
{
	CrossClass::_LockIt outBufLock ( outBufMutex );
	if ((bufSize - (outBufPtr - outBuf)) < dwToWrite)
		return false;
	else
	{
		memcpy (outBufPtr, lpBuf, dwToWrite);
		outBufPtr += dwToWrite;
		return true;
	}
}

void posixRS232port::write ( const char * lpBuf, const size_t dwToWrite )
{
	ssize_t nBytesWritten = 0;
	const char * pWriteFrom = lpBuf;
	for( size_t nBytesRest = dwToWrite; nBytesRest; )
	{
		nBytesWritten = ::write (m_hCommPort, pWriteFrom, nBytesRest);
		if (nBytesWritten == -1)
		{
			if (errno == EINTR)
				continue;
			else if (errno == EAGAIN)
			{
				if (post (pWriteFrom, nBytesRest))
					return;
				else
					throw basicRS232port::errWrite ("write buffer is full");
			}
			else
			{
				char msgText [ 256 ];
				sprintf (msgText, "%d: %s in 'posixRS232port::write'", errno, strerror (errno));
				throw basicRS232port::errWrite (msgText);
			}
		}
		else
		{
			pWriteFrom += nBytesWritten;
			nBytesRest -= nBytesWritten;
		}
	}
}

bool posixRS232port::handle_read ()
/* return true if a message was received	*/
{
	for (ssize_t nBytesRead = 0;;)
	{
		nBytesRead = ::read (m_hCommPort, _inBufPtr, _inBufSize - ( _inBufPtr - _inBuf ));
		if (nBytesRead == -1)
		{
			if (errno == EINTR)
				continue;
			else if (errno == EAGAIN)
				break;
			else
				throw basicRS232port::errRead (strerror (errno));
		}
		else if (nBytesRead)
		{
			processIncomingBytes (nBytesRead);
			break;
		}
	}
	return false;
}

bool posixRS232port::handle_write ()
/* return true if a send should be pending */
{
	CrossClass::_LockIt outBufLock ( outBufMutex );
	char * endPtr = outBufPtr;
	char * runPtr = outBuf;
	outBufLock.unlock ();
	
	ssize_t nBytesWritten = 0;
	for (size_t nBytesRest = endPtr - runPtr; nBytesRest; )
	{
		nBytesWritten = ::write (m_hCommPort, runPtr, nBytesRest);
		if (nBytesWritten == -1)
		{
			if (errno == EINTR)
				continue;
			else if (errno == EAGAIN)
				break;
			else
				throw basicRS232port::errWrite (strerror (errno));
		}
		else
		{
			nBytesRest -= nBytesWritten;
			runPtr += nBytesWritten;
		}
	}
	
	outBufLock.lock ();
	size_t nBytesRest = outBufPtr - endPtr;
	if (nBytesRest)
	{
		memmove (outBuf, endPtr, nBytesRest);
		outBufPtr = outBuf + nBytesRest;
		return true;
	}
	else
	{
		outBufPtr = outBuf;
		return false;
	}
}

bool posixRS232port::want2write ()
/* transmission is pending */
{
	CrossClass::_LockIt outBufLock ( outBufMutex );
	return (outBuf != outBufPtr);
}

bool posixRS232port::needs_prepare ()
/* needs preprocessing before polling */
{
	return true;
}

int posixRS232port::get_descriptor ()
{
	return m_hCommPort;
}

bool posixRS232port::auto_destroy ()
/* destroy object by 'event_poll' class */
{
	return false;
}

} // namespace sc
