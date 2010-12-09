#ifndef CROSS_UDE_CLIENT_H_INCLUDED
#define CROSS_UDE_CLIENT_H_INCLUDED 1
// (c) O. Peregudov
// 12/04/2010 - improved workflow
// 12/06/2010 - compartibility vs VS 2005

#include <crs/ude/netpoint.h>
#include <crs/ude/basics.h>

namespace ude {

class CROSS_EXPORT client_device_manager	: public netPoint
							, public basic_device_manager
{
protected:
	// hide parent members
	netPoint::bindSocket;
	netPoint::startServer;
	netPoint::serverSendRecv;
	
	virtual bool _send ( const cTalkPacket & );
	virtual bool _receive ( cTalkPacket & );
	
public:
	client_device_manager ();
	virtual ~client_device_manager ();
};

} // namespace ude
#endif // CROSS_UDE_CLIENT_H_INCLUDED
