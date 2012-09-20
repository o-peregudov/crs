/*
 * (c) Jan 26, 2009 Oleg N. Peregudov
 *	2010/11/30	condition variables for thread blocking
 *	2010/12/03	blocking and non-blocking send/receive packet members
 *			call-back function for incoming packets
 *	2010/12/06	extended error info
 *	2010/12/09	observer for transmission flag
 *	2010/12/12	using unsigned long instead of size_t for packet size
 *			sequential send/recv operations relying for non-blocking sockets
 *	2010/12/19	new packet structure (size field first)
 *	2011/01/03	integer types
 *	2012/08/15	new platform specific defines
 */
#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif

#include <crs/ude/netpoint.h>
#include <crs/math/unimath.h>
#include <cstdio>
#include <cerrno>

#define EMSGLENGTH	256

#if USE_WIN32_API
#define HANDLE_ERROR(error_class)								\
	switch (WSAGetLastError ())								\
	{												\
	case	WSAEWOULDBLOCK:									\
		return;										\
	default:											\
		{											\
			char msgText [ EMSGLENGTH ];						\
			sprintf (msgText, "code: %d", WSAGetLastError ());		\
			throw error_class (msgText);						\
		}											\
	}
#else
#define HANDLE_ERROR(error_class)								\
	{												\
		if (errno == EINTR)								\
			continue;									\
		else if (errno == EAGAIN)							\
			return;									\
		else											\
		{											\
			char msgText [ EMSGLENGTH ];						\
			sprintf (msgText, "%d: %s", errno, strerror (errno));		\
			throw error_class (msgText);						\
		}											\
	}
#endif

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
netPoint::netPoint ( const unsigned long inBufSize )
	: CrossClass::netPoint( )
	// receive traces
	, _inBuf( new char [ inBufSize ] )
	, _inBufSize( inBufSize )
	, _inVirtualBytes( 0 )
	, _inPacketQueueMutex( )
	, _inPacketQueue( )
	, _inCallBackMutex( )
	, _inCallBack( )
	, _inCallBackData( )
	, _inPacket( )
	, _inBytesRest( 0 )
	, _inNextPacketByte( 0 )
	// transmit traces
	, _outBuf( )
	, _outNextByte( 0 )
	, _outVirtualBytes( 0 )
	, _outBytesTotal( 0 )
	, _outNotify( )
	, _outMutex( )
	, _outFlag( false )
{
}

netPoint::netPoint ( CrossClass::cSocket & clientSocket, const unsigned long inBufSize )
	: CrossClass::netPoint( clientSocket )
	// receive traces
	, _inBuf( new char [ inBufSize ] )
	, _inBufSize( inBufSize )
	, _inVirtualBytes( 0 )
	, _inPacketQueueMutex( )
	, _inPacketQueue( )
	, _inCallBackMutex( )
	, _inCallBack( )
	, _inCallBackData( )
	, _inPacket( )
	, _inBytesRest( 0 )
	, _inNextPacketByte( 0 )
	// transmit traces
	, _outBuf( )
	, _outNextByte( 0 )
	, _outVirtualBytes( 0 )
	, _outBytesTotal( 0 )
	, _outNotify( )
	, _outMutex( )
	, _outFlag( false )
{
}

netPoint::~netPoint ()
{
	delete [] _inBuf;
	_inBuf = 0;
}

CrossClass::basicNetPoint * netPoint::handleNewConnection ( CrossClass::cSocket & s, const CrossClass::cSockAddr & sa )
{
	return new netPoint( s, _inBufSize );
}

void	netPoint::receive ()
{
	long	nNewPackets = 0,
		_inBytesProcessed = 0,
		_inBytesTransferred = 0;
	char*	_inBufPtr = _inBuf;
	size_t nBytes2Copy = 0;
	
	for( bool exhausted = true; exhausted; )
	{
		_inBytesTransferred = recv( _socket, ( _inBuf + _inVirtualBytes ), _inBufSize - _inVirtualBytes, 0 );
		if( _inBytesTransferred == -1 )
			HANDLE_ERROR( read_error )
		else if( _inBytesTransferred == 0 )
		{
			char msgText [ EMSGLENGTH ];
			sprintf( msgText, "%s (%d)", strerror( errno ), errno );
			throw end_of_file( msgText );
		}
		else
			_inVirtualBytes += _inBytesTransferred;
		
		//
		// check if we have more data to receive
		//
		exhausted = ( _inBytesTransferred == ( _inBufSize - _inVirtualBytes ) );
		
		//
		// fill in incomplete packet
		//
		if( _inBytesRest )
		{
			nBytes2Copy = UniMath::min( _inVirtualBytes, _inBytesRest );
			memcpy( _inNextPacketByte, _inBufPtr, nBytes2Copy );
			
			_inBytesProcessed += nBytes2Copy;
			_inVirtualBytes -= nBytes2Copy;
			_inBytesRest -= nBytes2Copy;
			_inBufPtr += nBytes2Copy;
			
			if( _inBytesRest == 0 )
			{
				CrossClass::_LockIt _inPacketQueueLock ( _inPacketQueueMutex );
				_inPacketQueue.push_back( _inPacket );
				++nNewPackets;
			}
			else
				_inNextPacketByte += nBytes2Copy;
		}
		
		//
		// take all the packets from the incoming buffer
		//
		while( _inVirtualBytes >= sizeof( cPacketHeader ) )
		{
			_inPacket = cTalkPacket( *reinterpret_cast<cPacketHeader *>( _inBufPtr ) );
			_inNextPacketByte = static_cast<char *>( static_cast<void *>( _inPacket ) );
			
			nBytes2Copy = UniMath::min( _inVirtualBytes, static_cast<unsigned long>( _inPacket.rawSize() ) );
			memcpy( _inNextPacketByte, _inBufPtr, nBytes2Copy );
			
			_inBytesProcessed += nBytes2Copy;
			_inVirtualBytes -= nBytes2Copy;
			_inBufPtr += nBytes2Copy;
			
			if( nBytes2Copy == _inPacket.rawSize() )
			{
				CrossClass::_LockIt _inPacketQueueLock ( _inPacketQueueMutex );
				_inPacketQueue.push_back( _inPacket );
				++nNewPackets;
			}
			else
			{
				_inNextPacketByte += nBytes2Copy;
				_inBytesRest = _inPacket.rawSize() - nBytes2Copy;
				break;
			}
		}
		
		//
		// move unprocessed bytes to the front of the incoming buffer
		//
		memmove( _inBuf, _inBufPtr, _inVirtualBytes );
		
		//
		// notify about new packets
		//
		if( nNewPackets )
		{
			CrossClass::_LockIt _inCallBackLock ( _inCallBackMutex );
			if( _inCallBack )
				_inCallBack( _inCallBackData );
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
	for( long nBytesProcessed; _outFlag; )
	{
		nBytesProcessed = send( _socket, _outNextByte, _outBytesTotal - _outVirtualBytes, 0 );
		if( nBytesProcessed == -1 )
			HANDLE_ERROR( write_error )
		_outVirtualBytes += nBytesProcessed;
		_outNextByte += nBytesProcessed;
		if( _outVirtualBytes == _outBytesTotal )
		{
			_outFlag = false;
			_outNotify.notify_one();
		}
	}
}

bool netPoint::want2transmit ()
{
	CrossClass::_LockIt lockTransmission ( _outMutex );
	return _outFlag;
}

bool netPoint::sendPacket ( const cTalkPacket & packet )
{
	CrossClass::_LockIt lockTransmission ( _outMutex );
	if( _outFlag )
		return false;
	else
	{
		_outBuf = packet;
		_outNextByte = static_cast<const char *>( static_cast<const void *>( _outBuf ) );
		_outVirtualBytes = 0;
		_outBytesTotal = _outBuf.rawSize();
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
