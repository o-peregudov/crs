#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#endif
#include <crs/ude/server.h>

namespace ude {

server::server ( const size_t nDeliveryBatchSize )
	: device_manager( nDeliveryBatchSize )
{
}

server::~server ()
{
}

CrossClass::cHandle<CrossClass::netPoint>
server::handleNewConnection ( CrossClass::cSocket & sckt, const CrossClass::cSockAddr & scktAddr )
{
	return CrossClass::cHandle<CrossClass::netPoint> ( new device_manager_stub ( this, sckt ) );
}

bool server::doHandleClient ( CrossClass::netPoint & peer )
{
	device_manager_stub & clientStub = dynamic_cast<device_manager_stub &>( peer );
	clientStub.dutyCycle();
	return false;
}

bool server::_send ( const cTalkPacket & packet )
{
	return true;
}

bool server::_receive ( cTalkPacket & packet )
{
	return false;
}

} // namespace ude
