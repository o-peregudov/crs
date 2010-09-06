// (c) Jan 18, 2009 Oleg N. Peregudov
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#endif
#include <crs/ude/devman.h>

namespace ude {

bool device_manager::cDeliveryThread::Step ()
{
	if( devman->proceed_delivery( batchSize ) == basic_device_manager::jcAllDone )
		CrossClass::sleep( 1 );
	return false;
}

bool device_manager::_send ( const cTalkPacket & packet )
{
	return sendPacket( packet );
}

bool device_manager::_receive ( cTalkPacket & packet )
{
	return recvPacket( packet );
}

void device_manager::_resume_transmission ()
{
}

bool device_manager::Step ()
{
	try
	{
		proceed_request( 1 );
		clientSendRecv();
		proceed_response( 0 );
	}
	catch( std::range_error & )
	{
		// broken packet
		throw;
	}
	catch( ... )
	{
		// socket exceptions
		Stop();
	}
	return false;
}

void device_manager::_resume_delivery ()
{
	_deliveryThread->Resume();
}

device_manager::device_manager ( const size_t nDeliveryBatchSize )
	: basic_device_manager()
	, netPoint( )
	, CrossClass::cThread( false )
	, _deliveryThread()
{
	_deliveryThread = std::auto_ptr<cDeliveryThread>( new cDeliveryThread ( this, nDeliveryBatchSize ) );
}

device_manager::~device_manager ()
{
	kill();
	shut_down();
}

void device_manager::shut_down ( const int sdWhat )
{
	if( ( sdWhat & basic_device_manager::sdReceive ) == basic_device_manager::sdReceive )
		kill();
	
	if( ( sdWhat & basic_device_manager::sdTransmit ) == basic_device_manager::sdTransmit )
		kill();
	
	if( ( sdWhat & basic_device_manager::sdDelivery ) == basic_device_manager::sdDelivery )
		_deliveryThread->kill();
}

void device_manager::connect ( const CrossClass::cSockAddr & sa )
{
	clientConnect( sa );
	Resume();
}

} // namespace ude
