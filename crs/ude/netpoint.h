#ifndef CROSS_UDE_NETPOINT_H_INCLUDED
#define CROSS_UDE_NETPOINT_H_INCLUDED 1
// (c) Jan 22, 2009 Oleg N. Peregudov
// 11/30/2010 - condition variables for thread blocking
// 12/03/2010 - blocking and non-blocking send/receive packet members
//              call-back function for incoming packets

#include <crs/netpoint.h>
#include <crs/callback.h>
#include <crs/ude/packets.h>
#include <crs/condition_variable.hpp>
#include <list>

namespace ude {

class CROSS_EXPORT netPoint : public CrossClass::netPoint
{
protected:
#pragma pack(push, 1)
	struct headerAndSize
	{
		ude::cPacketHeader header;
		size_t		size;
		char			contents [ 32 * sizeof( double ) ];
	};
#pragma pack(pop)
	
	//
	// receive traces
	//
	ude::cTalkPacket		_inBuf;
	size_t			_inStage,
					_inVirtualBytes,
					_inBytesTotal;
	char *			_inNextByte;
	headerAndSize		_inBufHeader;
	
	CrossClass::LockType	_inPacketQueueMutex;
	std::list<ude::cTalkPacket> _inPacketQueue;
	
	CrossClass::LockType	_inCallBackMutex;
	callBackFunction		_inCallBack;
	void *			_inCallBackData;
	
	//
	// transmit traces
	//
	ude::cTalkPacket		_outBuf;
	size_t			_outStage,
					_outVirtualBytes,
					_outBytesTotal;
	const char *		_outNextByte;
	headerAndSize		_outBufHeader;
	
	CrossClass::cConditionVariable _outNotify;
	CrossClass::LockType	_outMutex;
	bool				_outFlag;
	
protected:
	void initMembers ();
	
	void pushPacket ()
	{
		CrossClass::_LockIt inPacketQueueLock ( _inPacketQueueMutex );
		_inPacketQueue.push_back( _inBuf );
	}
	
	void inCallBack ()
	{
		CrossClass::_LockIt _inCallBackLock ( _inCallBackMutex );
		if( _inCallBack )
			_inCallBack( _inCallBackData );
	}
	
	virtual void transmit ();
	virtual void receive ();
	virtual CrossClass::basicNetPoint * handleNewConnection ( CrossClass::cSocket &, const CrossClass::cSockAddr & );
	
	netPoint ( CrossClass::cSocket & );
	
public:
	netPoint ();
	virtual ~netPoint ();
	
	bool sendPacket ( const cTalkPacket & packet );	// non-blocking version
	bool sendPacket ( const cTalkPacket & packet, const unsigned long msTimeOut );
	bool recvPacket ( cTalkPacket & packet );
	
	void setInCallBack ( callBackFunction func, void * data )
	{
		CrossClass::_LockIt _inCallBackLock ( _inCallBackMutex );
		_inCallBack = func;
		_inCallBackData = data;
	}
};

} // namespace ude
#endif // CROSS_UDE_NETPOINT_H_INCLUDED

