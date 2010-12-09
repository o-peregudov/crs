// (c) O. Peregudov
// 12/06/2010 - just for compartibility vs VS 2005
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#endif
#include <crs/ude/client.h>

namespace ude {

bool client_device_manager::_send ( const cTalkPacket & packet )
{
	return sendPacket( packet );
}

bool client_device_manager::_receive ( cTalkPacket & packet )
{
	return recvPacket( packet );
}

client_device_manager::client_device_manager ()
	: netPoint( )
	, basic_device_manager( )
{
}

client_device_manager::~client_device_manager ()
{
}

} // namespace ude
