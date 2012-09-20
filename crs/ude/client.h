#ifndef CROSS_UDE_CLIENT_H_INCLUDED
#define CROSS_UDE_CLIENT_H_INCLUDED 1
// (c) O. Peregudov
//	12/04/2010	improved workflow
//	12/06/2010	compartibility vs VS 2005
//	12/12/2010	imediate packet delivery
//			initiate transmission restart on process_request
//	12/19/2010	new names: _post/_send/_recv

#include <crs/ude/server.h>

namespace ude {

class CROSS_EXPORT client_device_manager	: public device_manager
{
protected:
	// hide parent members
	using netPoint::bindSocket;
	using netPoint::startServer;
	using netPoint::serverSendRecv;
	
	virtual bool _send ( const cTalkPacket & );
	virtual bool _recv ( cTalkPacket & );
	
	virtual void receive ();
public:
	client_device_manager ();
	virtual ~client_device_manager ();
};

} // namespace ude
#endif // CROSS_UDE_CLIENT_H_INCLUDED

