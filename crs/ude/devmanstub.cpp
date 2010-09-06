#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#endif
#include <crs/ude/devmanstub.h>

namespace ude {

device_manager_stub::device_manager_stub ( basic_device_manager * dm, CrossClass::cSocket & sckt )
	: basic_device( dm )
	, netPoint( sckt )
	, _queue( )
	, _queLock( )
	, _inPacket( )
{
	_device_manager->register_device( cPacketHeader::domainRDevMan, this );
}

device_manager_stub::~device_manager_stub ()
{
	_device_manager->unregister_device( cPacketHeader::domainRDevMan, this );
}

int device_manager_stub::take ( const cTalkPacket & packet )
{
	CrossClass::_LockIt _queLocker ( _queLock, true );
	_queue.push_back( packet );
	return 2;
}

void device_manager_stub::dutyCycle ()
{
	//
	// send
	//
	CrossClass::_LockIt _queLocker ( _queLock, true );
	if( !_queue.empty() )
		if( sendPacket( _queue.front() ) )
			_queue.pop_front();
	_queLocker.Unlock();
	
	//
	// receive
	//
	if( recvPacket( _inPacket ) )
		if( _device_manager )
			_device_manager->register_request( this, _inPacket );
}

void device_manager_stub::reset ()
{
}

bool device_manager_stub::invariant ()
{
	return true;
}

} // namespace ude
