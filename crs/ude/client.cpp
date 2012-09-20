// (c) O. Peregudov
//	12/06/2010	just for compartibility vs VS 2005
//	12/12/2010	imediate packet delivery
//			initiate transmission restart on register_request
//	12/19/2010	new names: _post/_send/_recv
//	01/03/2011	integer types
//
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#endif

#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif

#include <crs/ude/client.h>

namespace ude {

bool client_device_manager::_send ( const cTalkPacket & packet )
{
	return sendPacket( packet );
}

bool client_device_manager::_recv ( cTalkPacket & packet )
{
	return recvPacket( packet );
}

void client_device_manager::receive ()
{
	netPoint::receive();
	while( proceed_response() );
}

client_device_manager::client_device_manager ()
	: device_manager( )
{
}

client_device_manager::~client_device_manager ()
{
}

} // namespace ude

