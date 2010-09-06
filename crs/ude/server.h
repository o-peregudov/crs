#ifndef CROSS_UDE_SERVER_H_INCLUDED
#define CROSS_UDE_SERVER_H_INCLUDED 1

#include <crs/ude/devman.h>
#include <crs/ude/devmanstub.h>

namespace ude {

class CROSS_EXPORT server : public device_manager
{
protected:
	device_manager::connect;
	
	virtual CrossClass::cHandle<CrossClass::netPoint> handleNewConnection ( CrossClass::cSocket &, const CrossClass::cSockAddr & );
	virtual bool doHandleClient ( CrossClass::netPoint & );
	
	virtual bool _send ( const cTalkPacket & );
	virtual bool _receive ( cTalkPacket & );
	
public:
	server ( const size_t nDeliveryBatchSize = 16 );
      virtual ~server ();
};

} // namespace ude
#endif // CROSS_UDE_SERVER_H_INCLUDED
