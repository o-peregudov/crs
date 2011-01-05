#include <windows.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <stdexcept>
#include <sstream>

#include "cross/events.h"
#include "cross/security.h"

#define PURGE_FLAGS           (PURGE_TXABORT|PURGE_TXCLEAR|PURGE_RXABORT|PURGE_RXCLEAR)
#define MAX_WRITE_BUFFER      512

	std::string		m_cComPortName;
	HANDLE		m_hCommPort;
	COMMTIMEOUTS	m_TimeoutsOrig;
	COMMTIMEOUTS	m_TimeoutsNew;
	CrossClass::cEvent
				m_evntRead ( false, true ),
				m_evntWrite ( false, true ),
				evntCancelWrite ( false, true );
	OVERLAPPED		osRead,
				osWrite;
	size_t		m_Baud;
      unsigned char	m_DataBits,
				m_Parity,
				m_StopBits;
	bool			m_bConnected;
	size_t		m_dwTimeOut;	// milliseconds
	
	//
	// incoming buffer
	//
	CrossClass::cEvent evntIncomingData;
	CrossClass::cLock strInBufLock;
	const size_t _inBufSize ( 2048 );
	char	* _inBuf,
		* _inBufPtr;
	std::string _strInBuf;
      
size_t compileResponse ( const char * lpBuf, const size_t dwAvailable )
{
	static const char * delim = "\n\r";
	const char * p = strpbrk( lpBuf, delim );
	if( p )
	{
		size_t nBytes2Save = p - lpBuf;
		if( nBytes2Save < dwAvailable )
		{
			CrossClass::_LockIt exclusive_access ( strInBufLock, true );
			_strInBuf.append( lpBuf, nBytes2Save );
			exclusive_access.Unlock();
			evntIncomingData.SetEvent();
			
			while( ( nBytes2Save < dwAvailable ) && ( ( *p == '\n' ) || ( *p == '\r' ) ) )
				++p, ++nBytes2Save;
			return nBytes2Save;
		}
	}
	return 0;
}

void	open ()
{
	//
	//	CONSTRUCT
	//
	_inBufPtr = _inBuf = new char [ _inBufSize ];
	
	osWrite.hEvent = m_evntWrite;
	osRead.hEvent = m_evntRead;
	
	m_TimeoutsNew.ReadIntervalTimeout = 1;			// 1 ms between two serial bytes
      m_TimeoutsNew.ReadTotalTimeoutMultiplier = 0;		// 1 ms per byte
      m_TimeoutsNew.ReadTotalTimeoutConstant = 1500;
      m_TimeoutsNew.WriteTotalTimeoutConstant = 0;          // no timeouts
      m_TimeoutsNew.WriteTotalTimeoutMultiplier = 1;
	
	m_bConnected = false;
	m_DataBits = 8;
	m_Parity = NOPARITY;
	m_StopBits = ONESTOPBIT;
	
	//
	//	OPEN
	//
	m_cComPortName = "COM2";
	m_Baud = CBR_57600;
	
	//
      // open communication port handle
      //
      m_hCommPort = CreateFile( m_cComPortName.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            0,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
            0 );
      
      if( m_hCommPort == INVALID_HANDLE_VALUE )
            throw std::runtime_error( "CreateFile() returns INVALID_HANDLE_VALUE!" );
	else
	{
	      //
	      // set overall connect flag
	      //
		m_bConnected = TRUE;
		
		evntCancelWrite.ResetEvent();
	}
	
      //
      // Save original comm timeouts and set new ones
      //
      if( !GetCommTimeouts( m_hCommPort, &m_TimeoutsOrig ) )
            throw std::runtime_error( "GetCommTimeouts" );
      
      //
      // Set port state
      //
      //UpdateConnection();***
      DCB dcb = {0};
      
      dcb.DCBlength = sizeof(dcb);
      
      //
      // get current DCB settings
      //
      if( !GetCommState( m_hCommPort, &dcb ) )
            std::runtime_error( "GetCommState" );
      
      //
      // update DCB rate, byte size, parity, and stop bits size
      //
      dcb.BaudRate = m_Baud;
      dcb.ByteSize = m_DataBits;
      dcb.Parity   = m_Parity;
      dcb.StopBits = m_StopBits;
      
      //
      // DCB settings not in the user's control
      //
      dcb.fParity = TRUE;
      
      //
      // set new state
      //
      if( !SetCommState( m_hCommPort, &dcb ) )
            std::runtime_error( "SetCommState" );
      
      //
      // set new timeouts
      //
      if( !SetCommTimeouts( m_hCommPort, &m_TimeoutsNew ) )
            std::runtime_error( "SetCommTimeouts" );
      //UpdateConnection();***
	
	//
	// set comm buffer sizes
	//
	SetupComm( m_hCommPort, ( _inBufSize < 128 ) ? _inBufSize * 10 : _inBufSize /*MAX_READ_BUFFER*/, MAX_WRITE_BUFFER );
	
	//
	// purge all buffers
	//
	if( !PurgeComm( m_hCommPort, PURGE_RXCLEAR|PURGE_TXCLEAR|PURGE_RXABORT|PURGE_TXABORT ) )
		throw std::runtime_error( "PurgeComm" );
}

void	close ()
{
      if( m_bConnected )
      {
            m_bConnected = FALSE;
		evntCancelWrite.SetEvent();
            
            //
            // restore original comm timeouts
            //
            if( !SetCommTimeouts( m_hCommPort, &m_TimeoutsOrig ) )
                  throw std::runtime_error( "SetCommTimeouts (Restoration to original)" );
            
            //
            // Purge reads/writes, input buffer and output buffer
            //
            if( !PurgeComm( m_hCommPort, PURGE_FLAGS ) )
                  throw std::runtime_error( "PurgeComm" );
            
            CloseHandle( m_hCommPort );
		
		delete [] _inBuf;
      }
}

void	write ( const char * lpBuf, const unsigned long dwToWrite )
{
	const char * writePtr = lpBuf;
	unsigned long nBytesWritten = 0, errCode = ERROR_SUCCESS;
	while( nBytesWritten < dwToWrite )
	{
		//
		// issue write
		//
		if( WriteFile( m_hCommPort, writePtr, dwToWrite - ( writePtr - lpBuf ), &nBytesWritten, &osWrite ) )
		{
			writePtr += nBytesWritten;
			continue;
		}
		
		errCode = GetLastError();
		if( errCode != ERROR_IO_PENDING )
		{
			//
                  // writefile failed, but it isn't delayed
                  //
			std::basic_ostringstream<char> msgText;
			msgText.setf( std::ios_base::uppercase );
			msgText << "WriteFile failed with code 0x" << std::hex << errCode << '!';
			throw std::runtime_error( msgText.str() );
		}
		
		//
		// write is delayed
		//
		if( m_evntWrite.WaitEventOrCancel( evntCancelWrite ) )
		{
			if( GetOverlappedResult( m_hCommPort, &osWrite, &nBytesWritten, FALSE ) )
			{
				writePtr += nBytesWritten;
				continue;
			}
			
			//
			// writefile failed, but it isn't delayed
			//
			errCode = GetLastError();
			std::basic_ostringstream<char> msgText;
			msgText.setf( std::ios_base::uppercase );
			msgText << "WriteFile:GetOverlappedResult failed with code 0x" << std::hex << errCode << '!';
			throw std::runtime_error( msgText.str() );
		}
	}
}

void	write ( const std::string & text )
{
	write( text.c_str(), text.size() );
}

bool	read ()
{
	unsigned long dwRead = 0, errCode = ERROR_SUCCESS;
	
	try
	{
		//
		// issue read
		//
		if( ReadFile( m_hCommPort, _inBufPtr, _inBufSize - ( _inBufPtr - _inBuf ), &dwRead, &osRead ) )
			throw bool ( true );
		
		errCode = GetLastError();
		if( errCode != ERROR_IO_PENDING )
		{
			//
			// readfile failed, but it isn't delayed
			//
			std::basic_ostringstream<char> msgText;
			msgText.setf( std::ios_base::uppercase );
			msgText << "ReadFile failed with code 0x" << std::hex << errCode << '!';
			throw std::runtime_error( msgText.str() );
		}
		
		//
		// read is delayed
		//
		if( m_evntRead.WaitEventOrCancel( evntCancelWrite ) )
		{
			if( GetOverlappedResult( m_hCommPort, &osRead, &dwRead, FALSE ) )
				throw bool ( true );
			
			errCode = GetLastError();
			if( errCode != ERROR_IO_PENDING )
			{
				//
				// readfile failed, but it isn't delayed
				//
				std::basic_ostringstream<char> msgText;
				msgText.setf( std::ios_base::uppercase );
				msgText << "ReadFile:GetOverlappedResult failed with code 0x" << std::hex << errCode << '!';
				throw std::runtime_error( msgText.str() );
			}
		}
	}
	catch( bool readComplete )
	{
		if( dwRead )
		{
			_inBufPtr += dwRead;
			size_t nBytesAvailable = _inBufPtr - _inBuf,
				nBytesProcessed = compileResponse( _inBuf, nBytesAvailable );
			if( nBytesProcessed )
			{
				size_t nBytesRest = nBytesAvailable - nBytesProcessed;
				if( nBytesRest )
					memmove( _inBuf, _inBuf + nBytesProcessed, nBytesRest );
				_inBufPtr = _inBuf;
			}
			return false;
		}
		else
			Sleep( 1 );
	}
	catch( ... )
	{
		throw;
	}
	return true;
}

std::string getString ()
{
	CrossClass::_LockIt exclusive_access ( strInBufLock, true );
	std::string result = _strInBuf;
	_strInBuf = "";
	return result;
}

void	body ()
{
	write( std::string( "READ?\r" ) );
	while( !evntIncomingData.WaitEvent( 1 ) )
		read();
	std::cout << getString() << std::endl;
}

int main ( int argc, const char ** argv )
{
	try
	{
		open ();
		
		write( std::string( ":SYST:ZCH:STAT OFF\r" ) );
		Sleep( 1000 );
		
		write( std::string( ":ARM:COUNT 1\r" ) );
		Sleep( 1000 );
		
		write( std::string( ":ARM:TIM 0.01\r" ) );
		Sleep( 1000 );
		
		write( std::string( ":TRIG:COUNT 1\r" ) );
		Sleep( 1000 );
		
		write( std::string( "*IDN?\r" ) );
		while( !evntIncomingData.WaitEvent( 1 ) )
			read();
		std::cout << getString() << std::endl;
		
		size_t nPoints2Read = 25;
		if( argc > 1 )
			nPoints2Read = atoi( argv[ 1 ] );
		
		for( size_t i = 0; i < nPoints2Read; ++i )
			body ();
		
		write( std::string( "SYST:LOC\r" ) );
		
		close ();
	}
	catch( std::runtime_error & e )
	{
		std::cout << "ERROR: " << e.what() << std::endl;
	}
	return 0;
}
