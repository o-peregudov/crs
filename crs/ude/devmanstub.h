#ifndef CROSS_UDE_DEVMANSTUB_H_INCLUDED
#define CROSS_UDE_DEVMANSTUB_H_INCLUDED 1

#include <crs/ude/netpoint.h>
#include <crs/ude/basics.h>

namespace ude {

class CROSS_EXPORT device_manager_stub
	: public basic_device
	, public netPoint
{
protected:
	std::list<cTalkPacket>	_queue;
	CrossClass::LockType	_queLock;
	cTalkPacket			_inPacket;
	
	// hide parent members
	netPoint::startServer;
	netPoint::clientConnect;
	netPoint::clientSendRecv;
	
public:
	device_manager_stub ( basic_device_manager * dm, CrossClass::cSocket & );
	virtual ~device_manager_stub ();
	
	void dutyCycle ();
	
	virtual int take ( const cTalkPacket & );
	
	virtual void reset ();
	virtual bool invariant ();
	
	virtual bool poolControlled () const { return false; }
	// return true for devices allocated in a heap memory
	// (dynamic allocation). this will indicate that device
	// should be destroyed automatically within pool destruction procedure
};

} // namespace ude
#endif // CROSS_UDE_DEVMANSTUB_H_INCLUDED
