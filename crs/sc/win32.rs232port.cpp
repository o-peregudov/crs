// win32.rs232port.cpp: interface for the rs232port class (Win32 API).
// (c) Sep 4, 2010 Oleg N. Peregudov
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#	pragma warning( disable : 4996 )
#endif
#include <crs/sc/win32.rs232port.h>
#include <cstdio>

#define PURGE_FLAGS		(PURGE_TXABORT|PURGE_TXCLEAR|PURGE_RXABORT|PURGE_RXCLEAR)
#define MAX_WRITE_BUFFER	512

namespace sc {

win32RS232port::win32RS232port ( const size_t inBufSize )
	: basicRS232port( inBufSize )
	, m_hCommPort( NULL )
	, m_TimeoutsOrig( )
	, m_TimeoutsNew( )
	, m_evntRead( CreateEvent( NULL, TRUE, FALSE, NULL ) )
	, m_evntWrite( CreateEvent( NULL, TRUE, FALSE, NULL ) )
	, m_evntTerminate( CreateEvent( NULL, TRUE, FALSE, NULL ) )
	, m_evntTerminated( CreateEvent( NULL, TRUE, FALSE, NULL ) )
{
	m_DataBits = 8;
	m_StopBits = ONESTOPBIT;
	m_Parity = NOPARITY;
}

win32RS232port::~win32RS232port ()
{
	close();
	CloseHandle( m_evntTerminated );
	CloseHandle( m_evntTerminate );
	CloseHandle( m_evntWrite );
	CloseHandle( m_evntRead );
}

void win32RS232port::open ( const std::string & portName, const size_t baudRate )
{
	//
	// close previous session
	//
	close();
	
	//
	// open communication port handle
	//
	m_Baud = baudRate;
	m_cComPortName = portName;
	m_hCommPort = CreateFile( m_cComPortName.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		0 );
	
	if( m_hCommPort == INVALID_HANDLE_VALUE )
	{
		char msgText [ 64 ];
		sprintf( msgText, "CreateFile (%d)", GetLastError() );
		throw basicRS232port::errOpen( msgText );
	}
	
	//
	// set overall connect flag
	//
	m_bConnected = true;
	
	//
	// Set port state
	//
	UpdateConnection();
}

void win32RS232port::UpdateConnection ()
{
	DCB options;
	options.DCBlength = sizeof( DCB );
	
	//
	// Update baud rate, parity and stopbits
	//
	
	//
	// Save original comm timeouts and set new ones
	//
	if( !GetCommTimeouts( m_hCommPort, &m_TimeoutsOrig ) )
	{
		char msgText [ 64 ];
		sprintf( msgText, "GetCommTimeouts (%d)", GetLastError() );
		throw basicRS232port::errStatus( msgText );
	}
	
	//
	// Get current DCB settings
	//
	if( !GetCommState( m_hCommPort, &options ) )
	{
		char msgText [ 64 ];
		sprintf( msgText, "GetCommState (%d)", GetLastError() );
		throw basicRS232port::errStatus( msgText );
	}
	
	//
	// Update DCB rate, byte size, parity, and stop bits size
	//
	options.BaudRate = m_Baud;
	options.ByteSize = m_DataBits;
	options.Parity   = m_Parity;
	options.StopBits = m_StopBits;
	
	//
	// DCB settings not in the user's control
	//
	options.fParity = TRUE;
	
	//
	// Set new state
	//
	if( !SetCommState( m_hCommPort, &options ) )
	{
		char msgText [ 64 ];
		sprintf( msgText, "SetCommState (%d)", GetLastError() );
		throw basicRS232port::errStatus( msgText );
	}
	
	//
	// Set new timeouts
	//
	m_TimeoutsNew.ReadIntervalTimeout = static_cast<unsigned long>( -1 );
	m_TimeoutsNew.ReadTotalTimeoutMultiplier = 0;
	m_TimeoutsNew.ReadTotalTimeoutConstant = 1;
	m_TimeoutsNew.WriteTotalTimeoutConstant = 0;
	m_TimeoutsNew.WriteTotalTimeoutMultiplier = 1;
	
	if( !SetCommTimeouts( m_hCommPort, &m_TimeoutsNew ) )
	{
		char msgText [ 64 ];
		sprintf( msgText, "SetCommTimeouts (%d)", GetLastError() );
		throw basicRS232port::errStatus( msgText );
	}
	
	//
	// Set comm buffer sizes
	//
	SetupComm( m_hCommPort, ( _inBufSize < 128 ) ? _inBufSize * 10 : _inBufSize, MAX_WRITE_BUFFER );
	
	//
	// Purge all buffers
	//
	if( !PurgeComm( m_hCommPort, PURGE_RXCLEAR|PURGE_TXCLEAR|PURGE_RXABORT|PURGE_TXABORT ) )
	{
		char msgText [ 64 ];
		sprintf( msgText, "PurgeComm (%d)", GetLastError() );
		throw basicRS232port::errStatus( msgText );
	}
}

void win32RS232port::close()
{
	if( m_bConnected )
	{
		//
		// Cancel pending write requiest
		//
		SetEvent( m_evntTerminate );
		
		//
		// Reset overall connect flag
		//
		m_bConnected = false;
		
		//
		// restore original comm timeouts
		//
		if( !SetCommTimeouts( m_hCommPort, &m_TimeoutsOrig ) )
		{
			char msgText [ 64 ];
			sprintf( msgText, "SetCommTimeouts (Rest) (%d)", GetLastError() );
			throw basicRS232port::errStatus( msgText );
		}
		
		//
		// Purge reads/writes, input buffer and output buffer
		//
		if( !PurgeComm( m_hCommPort, PURGE_FLAGS ) )
		{
			char msgText [ 64 ];
			sprintf( msgText, "PurgeComm (%d)", GetLastError() );
			throw basicRS232port::errStatus( msgText );
		}
		
		//
		// Do close port
		//
		if( CloseHandle( m_hCommPort ) == -1 )
		{
			char msgText [ 64 ];
			sprintf( msgText, "CloseHandle (%d)", GetLastError() );
			throw basicRS232port::errClose( msgText );
		}
	}
}

void win32RS232port::write ( const char * lpBuf, const size_t dwToWrite )
{
	OVERLAPPED osWrite = {0};
	osWrite.hEvent = m_evntWrite;
	
	const char * writePtr = lpBuf;
	unsigned long nBytesWritten = 0,
		errCode = ERROR_SUCCESS,
		nBytesRest = dwToWrite;
	
	HANDLE evnt2wait4 [] = { m_evntWrite, m_evntTerminate };
	while( nBytesRest )
	{
		//
		// issue write
		//
		if( WriteFile( m_hCommPort, writePtr, nBytesRest, &nBytesWritten, &osWrite ) )
		{
			writePtr += nBytesWritten;
			nBytesRest -= nBytesWritten;
		}
		else
		{
			errCode = GetLastError();
			if( errCode != ERROR_IO_PENDING )
			{
				//
				// writefile failed, but it isn't delayed
				//
				char msgText [ 64 ];
				sprintf( msgText, "WriteFile (%d)", errCode );
				throw basicRS232port::errWrite( msgText );
			}
			
			//
			// write is delayed
			//
			switch( WaitForMultipleObjects( sizeof( evnt2wait4 ) / sizeof( HANDLE ), evnt2wait4, FALSE, INFINITE ) )
			{
			case	WAIT_OBJECT_0:		// write operation completed
				if( GetOverlappedResult( m_hCommPort, &osWrite, &nBytesWritten, FALSE ) )
				{
					writePtr += nBytesWritten;
					nBytesRest -= nBytesWritten;
				}
				else
				{
					//
					// writefile failed, but it isn't delayed
					//
					char msgText [ 64 ];
					sprintf( msgText, "GetOverlappedResult (%d)", GetLastError() );
					throw basicRS232port::errWrite( msgText );
				}
				break;
			
			case	(WAIT_OBJECT_0+1):	// termination
				return;
			
			case	WAIT_TIMEOUT:
				break;
			
			case	WAIT_FAILED:
				{
					char msgText [ 64 ];
					sprintf( msgText, "WaitForMultipleObjects (%d)", GetLastError() );
					throw basicRS232port::errWrite( msgText );
				}
			}
		}
	}
}

bool win32RS232port::receive ()
{
	//
	// issue read
	//
	OVERLAPPED osRead = {0};
	osRead.hEvent = m_evntRead;
	
	HANDLE evnt2wait4 [] = { m_evntRead, m_evntTerminate };
	
	unsigned long dwRead = 0;
	if( ReadFile( m_hCommPort, _inBufPtr, _inBufSize - ( _inBufPtr - _inBuf ), &dwRead, &osRead ) == 0 )
	{
		unsigned long errCode = GetLastError();
		if( errCode != ERROR_IO_PENDING )
		{
			//
			// readfile failed, but it isn't delayed
			//
			char msgText [ 64 ];
			sprintf( msgText, "ReadFile (%d)", errCode );
			throw basicRS232port::errRead( msgText );
		}
		
		//
		// read is delayed
		//
		switch( WaitForMultipleObjects( sizeof( evnt2wait4 ) / sizeof( HANDLE ), evnt2wait4, FALSE, INFINITE ) )
		{
		case	WAIT_OBJECT_0:		// read operation completed
			if( GetOverlappedResult( m_hCommPort, &osRead, &dwRead, FALSE ) == 0 )
			{
				errCode = GetLastError();
				if( errCode != ERROR_IO_PENDING )
				{
					//
					// readfile failed, but it isn't delayed
					//
					char msgText [ 64 ];
					sprintf( msgText, "GetOverlappedResult (%d)", errCode );
					throw basicRS232port::errRead( msgText );
				}
			}
			break;
		
		case	(WAIT_OBJECT_0+1):	// termination
			SetEvent( m_evntTerminated );
			return false;
		
		case	WAIT_TIMEOUT:
			break;
		
		case	WAIT_FAILED:
			{
				char msgText [ 64 ];
				sprintf( msgText, "WaitForMultipleObjects (%d)", GetLastError() );
				throw basicRS232port::errRead( msgText );
			}
		}
	}
	
	if( dwRead )
		processIncomingBytes( dwRead );
	
	return true;
}

void win32RS232port::postTerminate ()
{
	SetEvent( m_evntTerminate );
	WaitForSingleObject( m_evntTerminated, INFINITE );
}

} // namespace sc
