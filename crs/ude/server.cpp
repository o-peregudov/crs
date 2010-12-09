// (c) O. Peregudov
// 12/04/2010 - improved workflow
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#endif
#include <crs/ude/server.h>

namespace ude {

//
// members of class client_device_manager_stub
//

client_device_manager_stub::client_device_manager_stub ( basic_device_manager * dm, CrossClass::cSocket & sckt )
	: netPoint( sckt )
	, basic_device( dm )
	, _queue( )
	, _queueMutex( )
	, _inPacket( )
{
	_device_manager->register_device( cPacketHeader::domainRDevMan, this );
}

client_device_manager_stub::~client_device_manager_stub ()
{
	_device_manager->unregister_device( cPacketHeader::domainRDevMan, this );
}

void client_device_manager_stub::transmit ()
{
	CrossClass::_LockIt _queueLock ( _queueMutex );
	if( !_queue.empty() )
	{
		if( sendPacket( _queue.front() ) )
			_queue.pop_front();
	}
	_queueLock.unlock();
	netPoint::transmit();
}

void client_device_manager_stub::receive ()
{
	netPoint::receive();
	if( recvPacket( _inPacket ) )
		_device_manager->register_request( this, _inPacket );
}

int client_device_manager_stub::take ( const cTalkPacket & packet )
{
	CrossClass::_LockIt _queueLock ( _queueMutex );
	_queue.push_back( packet );
	return 2;
}

void client_device_manager_stub::reset ()
{
}

bool client_device_manager_stub::invariant ()
{
	return true;
}

//
// members of class server_device_manager
//

server_device_manager::server_device_manager ()
	: netPoint( )
	, basic_device_manager( )
{
}

server_device_manager::~server_device_manager ()
{
}

CrossClass::basicNetPoint *
server_device_manager::handleNewConnection ( CrossClass::cSocket & sckt, const CrossClass::cSockAddr & scktAddr )
{
	return new client_device_manager_stub ( this, sckt );
}

bool server_device_manager::_send ( const cTalkPacket & packet )
{
	return true;
}

bool server_device_manager::_receive ( cTalkPacket & packet )
{
	return false;
}

} // namespace ude

