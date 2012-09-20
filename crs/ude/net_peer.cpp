/*
 *  crs/ude/net_peer.h
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
#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif

#include <crs/ude/net_peer.h>
#include <crs/math/unimath.h>
#include <cstdio>
#include <cerrno>

#define EMSGLENGTH	256

template <class error_class>
bool	handle_error ()
{
#if USE_WIN32_API
	if (WSAGetLastError () == WSAEWOULDBLOCK)
		return true;
	else
	{
		char msgText [ EMSGLENGTH ];
		sprintf (msgText, "code: %d", WSAGetLastError ());
		throw error_class (msgText);
	}
#else
	if (errno == EAGAIN)
		return true;
	else if (errno != EINTR)
	{
		char msgText [ EMSGLENGTH ];
		sprintf (msgText, "%d: %s", errno, strerror (errno));
		throw error_class (msgText);
	}
#endif
	return false;
}

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
// members of class net_peer
//
net_peer::net_peer ( CrossClass::event_loop * ev_loop, const unsigned long inBufSize )
	: CrossClass::tcp_peer (ev_loop)
	/* receive traces		*/
	, _inBuf (new char [ inBufSize ])
	, _inBufSize (inBufSize)
	, _inVirtualBytes (0)
	, _inPacketQueueMutex ()
	, _inPacketQueue ()
	, _inCallBackMutex ()
	, _inCallBack ()
	, _inCallBackData ()
	, _inPacket ()
	, _inBytesRest (0)
	, _inNextPacketByte (0)
	/* transmit traces	*/
	, _outBuf ()
	, _outNextByte (0)
	, _outVirtualBytes (0)
	, _outBytesTotal (0)
	, _outNotify ()
	, _outMutex ()
	, _outFlag (false)
{
}

net_peer::net_peer ( CrossClass::event_loop * ev_loop, CrossClass::cSocket & clientSocket, const unsigned long inBufSize )
	: CrossClass::tcp_peer (ev_loop, clientSocket)
	/* receive traces		*/
	, _inBuf (new char [ inBufSize ])
	, _inBufSize (inBufSize)
	, _inVirtualBytes (0)
	, _inPacketQueueMutex ()
	, _inPacketQueue ()
	, _inCallBackMutex ()
	, _inCallBack ()
	, _inCallBackData ()
	, _inPacket ()
	, _inBytesRest (0)
	, _inNextPacketByte (0)
	/* transmit traces	*/
	, _outBuf ()
	, _outNextByte (0)
	, _outVirtualBytes (0)
	, _outBytesTotal (0)
	, _outNotify ()
	, _outMutex ()
	, _outFlag (false)
{
}

net_peer::~net_peer ()
{
	delete [] _inBuf;
	_inBuf = 0;
}

CrossClass::tcp_peer * net_peer::handle_new_connection ( CrossClass::cSocket & s, const CrossClass::cSockAddr & sa )
{
	return new net_peer (_event_loop, s, _inBufSize);
}

bool	net_peer::handle_read ()
{
	long	nNewPackets = 0,
		_inBytesProcessed = 0,
		_inBytesTransferred = 0;
	char*	_inBufPtr = _inBuf;
	size_t nBytes2Copy = 0;
	
	for (bool exhausted = true; exhausted; )
	{
		_inBytesTransferred = recv (_socket, (_inBuf + _inVirtualBytes), _inBufSize - _inVirtualBytes, 0);
		if (_inBytesTransferred == -1)
		{
			if (handle_error<read_error> ())
				break;	/* no more data in the input buffer	*/
			else
				continue;	/* recv was interrupted by a signal	*/
		}
		else if (_inBytesTransferred == 0)
		{
			char msgText [ EMSGLENGTH ];
			sprintf (msgText, "%s (%d)", strerror (errno), errno);
			throw end_of_file (msgText);
		}
		else
			_inVirtualBytes += _inBytesTransferred;
		
		/*
		 * check if we have more data to receive
		 */
		exhausted = ( _inBytesTransferred == (_inBufSize - _inVirtualBytes) );
		
		/*
		 * fill in incomplete packet
		 */
		if (_inBytesRest)
		{
			nBytes2Copy = UniMath::min (_inVirtualBytes, _inBytesRest);
			memcpy (_inNextPacketByte, _inBufPtr, nBytes2Copy);
			
			_inBytesProcessed += nBytes2Copy;
			_inVirtualBytes -= nBytes2Copy;
			_inBytesRest -= nBytes2Copy;
			_inBufPtr += nBytes2Copy;
			
			if (_inBytesRest == 0)
			{
				CrossClass::_LockIt _inPacketQueueLock ( _inPacketQueueMutex );
				_inPacketQueue.push_back (_inPacket);
				++nNewPackets;
			}
			else
				_inNextPacketByte += nBytes2Copy;
		}
		
		/*
		 * take all the packets from the incoming buffer
		 */
		while (_inVirtualBytes >= sizeof (cPacketHeader))
		{
			_inPacket = cTalkPacket (*reinterpret_cast<cPacketHeader *> (_inBufPtr));
			_inNextPacketByte = static_cast<char *> (static_cast<void *> (_inPacket));
			
			nBytes2Copy = UniMath::min (_inVirtualBytes, static_cast<unsigned long> (_inPacket.rawSize ()));
			memcpy (_inNextPacketByte, _inBufPtr, nBytes2Copy);
			
			_inBytesProcessed += nBytes2Copy;
			_inVirtualBytes -= nBytes2Copy;
			_inBufPtr += nBytes2Copy;
			
			if (nBytes2Copy == _inPacket.rawSize ())
			{
				CrossClass::_LockIt _inPacketQueueLock ( _inPacketQueueMutex );
				_inPacketQueue.push_back (_inPacket);
				++nNewPackets;
			}
			else
			{
				_inNextPacketByte += nBytes2Copy;
				_inBytesRest = _inPacket.rawSize () - nBytes2Copy;
				break;
			}
		}
		
		/*
		 * move unprocessed bytes to the front of the incoming buffer
		 */
		memmove (_inBuf, _inBufPtr, _inVirtualBytes);
		
		/*
		 * notify about new packets
		 */
		if (nNewPackets)
		{
			CrossClass::_LockIt _inCallBackLock ( _inCallBackMutex );
			if (_inCallBack)
				_inCallBack (_inCallBackData);
		}
	}
	
	return (nNewPackets != 0);
}

bool	net_peer::recv_packet ( cTalkPacket & packet )
{
	CrossClass::_LockIt inPacketQueueLock ( _inPacketQueueMutex );
	if (_inPacketQueue.empty ())
		return false;
	else
	{
		packet = _inPacketQueue.front ();
		_inPacketQueue.pop_front ();
		return true;
	}
}

bool	net_peer::handle_write ()
{
	CrossClass::_LockIt lockTransmission ( _outMutex );
	for (long nBytesProcessed; _outFlag; )
	{
		nBytesProcessed = send (_socket, _outNextByte, _outBytesTotal - _outVirtualBytes, 0);
		if (nBytesProcessed == -1)
		{
			if (handle_error<write_error> ())
				break;	/* output buffer is full		*/
			else
				continue;	/* send was interrupted by a signal	*/
		}
		_outVirtualBytes += nBytesProcessed;
		_outNextByte += nBytesProcessed;
		if (_outVirtualBytes == _outBytesTotal)
		{
			_outFlag = false;
			_outNotify.notify_one ();
		}
	}
	return _outFlag;
}

bool	net_peer::want2write ()
{
	CrossClass::_LockIt lockTransmission ( _outMutex );
	return _outFlag;
}

bool	net_peer::send_packet ( const cTalkPacket & packet )
{
	CrossClass::_LockIt lockTransmission ( _outMutex );
	if (_outFlag)
		return false;
	else
	{
		_outBuf = packet;
		_outNextByte = static_cast<const char *> (static_cast<const void *> (_outBuf));
		_outVirtualBytes = 0;
		_outBytesTotal = _outBuf.rawSize ();
		_outFlag = true;
		return true;
	}
}

bool	net_peer::send_packet ( const cTalkPacket & packet, const unsigned long msTimeOut )
{
	if (send_packet (packet))
	{
		CrossClass::_LockIt lockTransmission ( _outMutex );
		if (msTimeOut == static_cast<unsigned long> (-1))
		{
			_outNotify.wait (lockTransmission);
			_outFlag = false;
			return true;		/* the packet was sent!				*/
		}
		else if (_outNotify.wait_for (lockTransmission, msTimeOut, inversePredicate (&_outFlag)))
		{
			_outFlag = false;
			return true;		/* the packet was sent!				*/
		}
	}
	return false;				/* send was timed out or would block	*/
}

}	/* namespace ude	*/
