// (c) Oleg N. Peregudov
//	12/12/2010	simple tool for ude-network performance measurement
//	12/17/2010	new packet structure (size field first)
//	12/19/2010	new name for base ude classes
//	01/03/2011	integer types
//
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#endif

#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif

#include <crs/ude/pingtool.h>
#include <crs/math/unimath.h>

namespace ude {

pingTool::pingTool ( device_manager * bdm )
	: device( bdm )
	, _timer( )
	, _mutex( )
	, _notify( )
	, _flag( false )
	, _packet( 0, 2 * sizeof( double ), cPacketHeader::domainStat, 0 )
{
	_packet.header().recepient = 0x0001;
	_packet.value( 0 ) = _timer.getStamp();
	if( _device_manager )
		_device_manager->register_device( cPacketHeader::domainStat, this );
}

pingTool::~pingTool ()
{
	if( _device_manager )
		_device_manager->unregister_device( cPacketHeader::domainStat, this );
}

int pingTool::take ( const ude::cTalkPacket & inPacket )
{
	CrossClass::_LockIt _lock ( _mutex );
	if( _flag )
	{
		if(	( inPacket.header().domain == _packet.header().domain )
			&& ( inPacket.header().recepient == _packet.header().recepient )
			&& UniMath::isZero( inPacket.value( 0 ) - _packet.value( 0 ) ) )
		{
			_flag = false;
			_packet.value( 1 ) = _timer.getStamp() - inPacket.value( 1 );
			_notify.notify_one();
			return 2;
		}
	}
	return 0;	// not my packet
}

double pingTool::ping ()
{
	CrossClass::_LockIt _lock ( _mutex );
	if( _flag )
		return -1.0;
	else
	{
		if( UniMath::isZero( _packet.value( 1 ) ) )
		{
			_packet.value( 1 ) = _timer.getStamp();
			postPacket( _packet );
			_flag = true;
			return -1.0;
		}
		else
		{
			double delta = _packet.value( 1 );
			_packet.value( 1 ) = 0.0;
			return delta;
		}
	}
}

bool pingTool::poolControlled () const
{
	return false;
}

void pingTool::reset ()
{
	CrossClass::_LockIt _lock ( _mutex );
	_packet.value( 0 ) = _timer.getStamp();
	_flag = false;
}

double pingTool::ping ( const unsigned long msTimeOut )
{
	CrossClass::_LockIt _lock ( _mutex );
	if( _flag )
		return -1.0;
	else
	{
		_packet.value( 1 ) = _timer.getStamp();
		postPacket( _packet );
		_flag = true;
		if( msTimeOut == static_cast<const unsigned long>( -1 ) )
		{
			_notify.wait( _lock );
			double delta = _packet.value( 1 );
			_packet.value( 1 ) = 0.0;
			_flag = false;
			return delta;
		}
		else if( _notify.wait_for( _lock, msTimeOut, waitpred( &_flag ) ) )
		{
			double delta = _packet.value( 1 );
			_packet.value( 1 ) = 0.0;
			_flag = false;
			return delta;
		}
		else
			return std::numeric_limits<double>::infinity();
	}
}

} // namespace ude

