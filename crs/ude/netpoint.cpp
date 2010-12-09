// (c) Jan 26, 2009 Oleg N. Peregudov
// 11/30/2010 - condition variables for thread blocking
// 12/03/2010 - blocking and non-blocking send/receive packet members
//              call-back function for incoming packets
// 12/06/2010 - extended error info
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#endif
#include <crs/ude/netpoint.h>
#include <crs/CRCStuff.h>
#include <cstdio>
#include <cerrno>

#define EMSGLENGTH	256

namespace ude {

struct inversePredicate
{
	bool * _flag;
	
	inversePredicate ( bool * flag ) : _flag( flag ) { }
	
	bool operator () ()
	{
		return !(*_flag);
	}
};

//
// members of class netPoint
//
netPoint::netPoint ()
	: CrossClass::netPoint( )
	// receive traces
	, _inBuf( )
	, _inStage( 0 )
	, _inVirtualBytes( 0 )
	, _inBytesTotal( 0 )
	, _inNextByte( 0 )
	, _inBufHeader( )
	
	, _inPacketQueueMutex( )
	, _inPacketQueue( )
	
	, _inCallBackMutex( )
	, _inCallBack( )
	, _inCallBackData( )
	
	// transmit traces
	, _outBuf( )
	, _outStage( 0 )
	, _outVirtualBytes( 0 )
	, _outBytesTotal( 0 )
	, _outNextByte( 0 )
	, _outBufHeader( )
	
	, _outNotify( )
	, _outMutex( )
	, _outFlag( false )
{
	initMembers();
}

netPoint::netPoint ( CrossClass::cSocket & clientSocket )
	: CrossClass::netPoint( clientSocket )
	// receive traces
	, _inBuf( )
	, _inStage( 0 )
	, _inVirtualBytes( 0 )
	, _inBytesTotal( 0 )
	, _inNextByte( 0 )
	, _inBufHeader( )
	
	, _inPacketQueueMutex( )
	, _inPacketQueue( )
	
	, _inCallBackMutex( )
	, _inCallBack( )
	, _inCallBackData( )
	
	// transmit traces
	, _outBuf( )
	, _outStage( 0 )
	, _outVirtualBytes( 0 )
	, _outBytesTotal( 0 )
	, _outNextByte( 0 )
	, _outBufHeader( )
	
	, _outNotify( )
	, _outMutex( )
	, _outFlag( false )
{
	initMembers();
}

netPoint::~netPoint ()
{
}

void netPoint::initMembers ()
{
	// read buffers
	_inNextByte = reinterpret_cast<char *>( &_inBufHeader );
	_inBytesTotal = sizeof( _inBufHeader.header ) + sizeof( _inBufHeader.size );
	
	// write buffers
	_outNextByte = reinterpret_cast<char *>( &_outBufHeader );
	_outBytesTotal = sizeof( _outBufHeader.header ) + sizeof( _outBufHeader.size );
}

CrossClass::basicNetPoint * netPoint::handleNewConnection ( CrossClass::cSocket & s, const CrossClass::cSockAddr & sa )
{
	return new netPoint( s );
}

void netPoint::receive ()
{
	long nBytesProcessed = recv( _socket, _inNextByte, _inBytesTotal - _inVirtualBytes, 0 );
	if( nBytesProcessed == -1 )
	{
		char msgText [ EMSGLENGTH ];
#if defined( USE_WIN32_API )
		sprintf( msgText, "code: %d", WSAGetLastError() );
#else
		sprintf( msgText, "%s (%d)", strerror( errno ), errno );
#endif
		throw read_error( msgText );
	}
	else if( nBytesProcessed == 0 )
		throw end_of_file( "receive" );
	
	_inVirtualBytes += nBytesProcessed;
	_inNextByte += nBytesProcessed;
	
	if( _inVirtualBytes == _inBytesTotal )
	{
		// jump to the next stage
		_inVirtualBytes = 0;
		switch( _inStage++ )
		{
		case	0:	// header was received
			#if defined( UDE_SENDER_IDENTIFICATION )
			{
				ude::ushort crcRecv = _inBufHeader.header.crc;
				ude::ushort crcInit = _inBufHeader.header.id^_inBufHeader.header.domain^_inBufHeader.header.recepient;
				_inBufHeader.header.crc = 0;
				if( get_crc_ccitt( crcInit, reinterpret_cast<const char *>( &_inBufHeader.header ), sizeof( _inBufHeader.header ) ) != crcRecv )
					throw read_crc_error( "receive" );
			}
			#endif
			_inBuf = cTalkPacket( _inBufHeader.header, _inBufHeader.size );
			_inNextByte = reinterpret_cast<char *>( _inBuf.byte() );
			_inBytesTotal = _inBufHeader.size;
			break;
		
		case	1:	// contents was received
			pushPacket();
			inCallBack();
			_inNextByte = reinterpret_cast<char *>( &_inBufHeader );
			_inBytesTotal = sizeof( _inBufHeader.header ) + sizeof( _inBufHeader.size );
			_inStage = 0;
			break;
		}
	}
}

bool netPoint::recvPacket ( cTalkPacket & packet )
{
	CrossClass::_LockIt inPacketQueueLock ( _inPacketQueueMutex );
	if( _inPacketQueue.empty() )
		return false;
	else
	{
		packet = _inPacketQueue.front();
		_inPacketQueue.pop_front();
		return true;
	}
}

void netPoint::transmit ()
{
	CrossClass::_LockIt lockTransmission ( _outMutex );
	if( _outFlag )
	{
		long nBytesProcessed = send( _socket, _outNextByte, _outBytesTotal - _outVirtualBytes, 0 );
		if( nBytesProcessed == -1 )
		{
			char msgText [ EMSGLENGTH ];
#if defined( USE_WIN32_API )
			sprintf( msgText, "code: %d", WSAGetLastError() );
#else
			sprintf( msgText, "%s (%d)", strerror( errno ), errno );
#endif
			throw write_error( msgText );
		}
		
		_outVirtualBytes += nBytesProcessed;
		_outNextByte += nBytesProcessed;
		
		if( _outVirtualBytes == _outBytesTotal )
		{
			// jump to the next stage
			_outVirtualBytes = 0;
			switch( _outStage++ )
			{
			case	0:	// sending contents
				_outNextByte = reinterpret_cast<const char *>( _outBuf.byte() );
				_outBytesTotal = _outBuf.byteSize();
				break;
			
			case	1:	// transmission complete
				_outStage = 0;
				_outNextByte = reinterpret_cast<char *>( &_outBufHeader );
				_outBytesTotal = sizeof( _outBufHeader.header ) + sizeof( _outBufHeader.size );
				_outFlag = false;
				_outNotify.notify_one();
				break;
			}
		}
	}
}

bool netPoint::sendPacket ( const cTalkPacket & packet )
{
	CrossClass::_LockIt lockTransmission ( _outMutex );
	if( _outFlag )
		return false;
	else
	{
		_outBufHeader.header = _outBuf = packet;
		_outBufHeader.size = packet.byteSize();
		#if defined( UDE_SENDER_IDENTIFICATION )
		{
			ude::ushort crcInit = _outBufHeader.header.id^_outBufHeader.header.domain^_outBufHeader.header.recepient;
			_outBufHeader.header.crc = 0;
			_outBufHeader.header.crc = get_crc_ccitt( crcInit, reinterpret_cast<const char *>( &_outBufHeader.header ), sizeof( _outBufHeader.header ) );
		}
		#endif
		_outFlag = true;
		return true;
	}
}

bool netPoint::sendPacket ( const cTalkPacket & packet, const unsigned long msTimeOut )
{
	if( sendPacket( packet ) )
	{
		CrossClass::_LockIt lockTransmission ( _outMutex );
		if( msTimeOut == static_cast<unsigned long>( -1 ) )
		{
			_outNotify.wait( lockTransmission );
			_outFlag = false;
			return true;		// the packet was sent!
		}
		else if( _outNotify.wait_for( lockTransmission, msTimeOut, inversePredicate( &_outFlag ) ) )
		{
			_outFlag = false;
			return true;		// the packet was sent!
		}
	}
	return false;				// send timeout or would block
}

} // namespace ude

