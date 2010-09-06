#ifndef CROSS_UDE_NETPOINT_H_INCLUDED
#define CROSS_UDE_NETPOINT_H_INCLUDED 1
// (c) Jan 22, 2009 Oleg N. Peregudov

#include <crs/netpoint.h>
#include <crs/ude/packets.h>

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
	
	//
	// transmit traces
	//
	ude::cTalkPacket		_outBuf;
	size_t			_outStage,
					_outVirtualBytes,
					_outBytesTotal;
	const char *		_outNextByte;
	headerAndSize		_outBufHeader;
	
protected:
	void initMembers ();
	
	virtual void transmit ();
	virtual void receive ();
	virtual CrossClass::cHandle<CrossClass::basicNetPoint> handleNewConnection ( CrossClass::cSocket &, const CrossClass::cSockAddr & );
	
	netPoint ( CrossClass::cSocket & );
	
public:
	netPoint ();
	virtual ~netPoint ();
	
	bool sendPacket ( const cTalkPacket & packet );
	bool recvPacket ( cTalkPacket & packet );
};

} // namespace ude
#endif // CROSS_UDE_NETPOINT_H_INCLUDED
