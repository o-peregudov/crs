#ifndef CROSS_UDE_SERVER_H_INCLUDED
#define CROSS_UDE_SERVER_H_INCLUDED 1
// (c) O. Peregudov
// 12/04/2010 - improved workflow

#include <crs/ude/netpoint.h>
#include <crs/ude/basics.h>

namespace ude {

class CROSS_EXPORT client_device_manager_stub	: public netPoint
								, public basic_device
{
protected:
	// hide parent members
	netPoint::startServer;
	netPoint::clientConnect;
	netPoint::clientSendRecv;
	
	std::list<cTalkPacket>	_queue;
	CrossClass::LockType	_queueMutex;
	cTalkPacket			_inPacket;
	
	virtual void transmit ();
	virtual void receive ();
	
public:
	client_device_manager_stub ( basic_device_manager * dm, CrossClass::cSocket & );
	virtual ~client_device_manager_stub ();
	
	virtual int take ( const cTalkPacket & );
	
	virtual void reset ();
	virtual bool invariant ();
	
	virtual bool poolControlled () const { return false; }
	// this devices will be deleted by basicNetPoint::removeClient
};

class CROSS_EXPORT server_device_manager	: public netPoint
							, public basic_device_manager
{
protected:
	virtual CrossClass::basicNetPoint * handleNewConnection ( CrossClass::cSocket &, const CrossClass::cSockAddr & );
	virtual bool _send ( const cTalkPacket & );
	virtual bool _receive ( cTalkPacket & );
	
public:
	server_device_manager ();
      virtual ~server_device_manager ();
};

} // namespace ude
#endif // CROSS_UDE_SERVER_H_INCLUDED

