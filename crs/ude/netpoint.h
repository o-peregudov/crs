#ifndef CROSS_UDE_NETPOINT_H_INCLUDED
#define CROSS_UDE_NETPOINT_H_INCLUDED 1
// (c) Jan 22, 2009 Oleg N. Peregudov
//	11/30/2010	condition variables for thread blocking
//	12/03/2010	blocking and non-blocking send/receive packet members
//			call-back function for incoming packets
//	12/09/2010	observer for transmission flag
//	12/10/2010	using unsigned long instead of size_t for packet size
//			sequential send/recv operations relying for non-blocking sockets
//	12/19/2010	new packet structure (size field first)
//
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

} // namespace ude
#endif // CROSS_UDE_NETPOINT_H_INCLUDED

