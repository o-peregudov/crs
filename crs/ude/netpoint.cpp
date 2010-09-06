// (c) Jan 26, 2009 Oleg N. Peregudov
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#endif
#include <crs/ude/netpoint.h>
#include <crs/CRCStuff.h>
#include <sstream>

namespace ude {

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
	// transmit traces
	, _outBuf( )
	, _outStage( 0 )
	, _outVirtualBytes( 0 )
	, _outBytesTotal( 0 )
	, _outNextByte( 0 )
	, _outBufHeader( )
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
	// transmit traces
	, _outBuf( )
	, _outStage( 0 )
	, _outVirtualBytes( 0 )
	, _outBytesTotal( 0 )
	, _outNextByte( 0 )
	, _outBufHeader( )
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

CrossClass::cHandle<CrossClass::netPoint> netPoint::handleNewConnection ( CrossClass::cSocket & s, const CrossClass::cSockAddr & sa )
{
	return new netPoint( s );
}

void netPoint::receive ()
{
	if( _inEventReady.WaitEvent( 0 ) )
		return;
	
	long nBytesProcessed = recv( _socket, _inNextByte, _inBytesTotal - _inVirtualBytes, 0 );
	if( nBytesProcessed == -1 )
	{
		std::basic_ostringstream<char> errMsg;
		errMsg << "receive (" << errno << ')';
		throw read_error( errMsg.str() );
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
			_inNextByte = reinterpret_cast<char *>( &_inBufHeader );
			_inBytesTotal = sizeof( _inBufHeader.header ) + sizeof( _inBufHeader.size );
			_inStage = 0;
			_inEventReady.SetEvent();
			break;
		}
	}
}

bool netPoint::recvPacket ( cTalkPacket & packet )
{
	if( _inEventReady.WaitEvent( 0 ) )
	{
		packet = _inBuf;
		_inEventReady.ResetEvent();
		return true;
	}
	else
		return false;
}

void netPoint::transmit ()
{
	if( _outEventReady.WaitEvent( 0 ) )
		return;
	
	long nBytesProcessed = send( _socket, _outNextByte, _outBytesTotal - _outVirtualBytes, 0 );
	if( nBytesProcessed == -1 )
	{
		std::basic_ostringstream<char> errMsg;
		errMsg << "transmit (" << errno << ')';
		throw write_error( errMsg.str() );
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
			_outEventReady.SetEvent();
			break;
		}
	}
}

bool netPoint::sendPacket ( const cTalkPacket & packet )
{
	if( _outEventReady.WaitEvent( 0 ) )
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
		_outEventReady.ResetEvent();
		return true;
	}
	else
		return false;
}

} // namespace ude
