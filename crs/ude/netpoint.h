#ifndef CROSS_UDE_NETPOINT_H_INCLUDED
#define CROSS_UDE_NETPOINT_H_INCLUDED 1
/*
 *  crs/ude/netpoint.h
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

/*
 *	2009/01/22
 *	2010/11/30	condition variables for thread blocking
 *	2010/12/03	blocking and non-blocking send/receive packet members
 *			call-back function for incoming packets
 *	2010/12/09	observer for transmission flag
 *	2010/12/10	using unsigned long instead of size_t for packet size
 *			sequential send/recv operations relying for non-blocking sockets
 *	2010/12/19	new packet structure (size field first)
 *	2012/08/16	new platform specific defines
 */

#include <crs/netpoint.h>
#include <crs/callback.h>
#include <crs/ude/packets.h>
#include <crs/condition_variable.hpp>
#include <list>

namespace ude {

class CROSS_EXPORT netPoint : public CrossClass::netPoint
{
protected:
	typedef std::list<ude::cTalkPacket> 	queueType;
	typedef CrossClass::cMutex			mutexType;
	typedef CrossClass::cConditionVariable	condType;
	
	//
	// receive traces
	//
	char *		_inBuf;
	const long		_inBufSize;
	unsigned long	_inVirtualBytes;
	mutexType		_inPacketQueueMutex;
	queueType		_inPacketQueue;
	mutexType		_inCallBackMutex;
	callBackFunction	_inCallBack;
	void *		_inCallBackData;
	ude::cTalkPacket	_inPacket;
	unsigned long	_inBytesRest;
	char *		_inNextPacketByte;
	
	//
	// transmit traces
	//
	ude::cTalkPacket	_outBuf;
	const char *	_outNextByte;
	size_t		_outVirtualBytes,
				_outBytesTotal;
	condType		_outNotify;
	mutexType		_outMutex;
	bool			_outFlag;
	
protected:
	virtual void transmit ();
	virtual void receive ();
	
	virtual CrossClass::basicNetPoint * handleNewConnection ( CrossClass::cSocket &, const CrossClass::cSockAddr & );
	
	netPoint ( CrossClass::cSocket &, const unsigned long inBufSize = 1024 );
	
public:
	netPoint ( const unsigned long inBufSize = 1024 );
	virtual ~netPoint ();
	
	virtual bool want2transmit ();
	
	bool	sendPacket ( const cTalkPacket & packet );	// non-blocking version
	bool	sendPacket ( const cTalkPacket & packet, const unsigned long msTimeOut );
	bool	recvPacket ( cTalkPacket & packet );
	
	void	setInCallBack ( callBackFunction func, void * data )
	{
		CrossClass::_LockIt _inCallBackLock ( _inCallBackMutex );
		_inCallBack = func;
		_inCallBackData = data;
	}
};

}	/* namespace ude				*/
#endif/* CROSS_UDE_NETPOINT_H_INCLUDED	*/

