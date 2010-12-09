// (c) Jun 12, 2007 Oleg N. Peregudov
// 11/21/2007 - new place for cross-compiling routines
// 01/30/2008 - placed in cross library
// 12/03/2010 - improved workflow
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#endif

#include <crs/ude/basics.h>

using namespace CrossClass;

namespace ude {

int	basic_device::cachedTake ( const cTalkPacket & packet )
{
	try
	{
		// do perform standard actions for predefined packets
		if( ( packet.id != cTalkPacket::idLoopBack ) && ( packet.word( 0 ) == 0xFFFF ) )
		{
			invariant();
			return 0x02;
		}
		else
			return take( packet );
	}
	catch( std::range_error )
	{
		return -1;		// broken packet
	}
	catch( ... )
	{
		return -2;		// strange exception
	}
}

basic_device_manager::basic_device_manager ()
	: _dns()
	, _outbox()
	, _dnsMutex()
	, _outboxMutex()
{
}

basic_device_manager::~basic_device_manager ()
{
}

bool	basic_device_manager::register_device ( const ushort domain, basic_device * device )
{
	if( device )
	{
		_LockIt _dnsLock ( _dnsMutex );
		_dns[ domain ].push_back( device );
		return true;
	}
	return false;
}

int	basic_device_manager::unregister_device ( const ushort domain, basic_device * device )
{
	_LockIt _dnsLock ( _dnsMutex );
	device_pool & dp = _dns[ domain ];
	device_pool::iterator ptr = std::find( dp.begin(), dp.end(), device );
	if( ptr != dp.end() )
	{
		dp.remove( ptr );
		return udSuccess;	// device was successfully dettached
	}
	else
		return udNotFound;// device was not found in a specified domain
}

void	basic_device_manager::_post ( const cTalkPacket & packet )
{
	_LockIt _outboxLock ( _outboxMutex );
	_outbox.push_back( packet );
}

void	basic_device_manager::proceed_response ( const size_t nPacketSize )
{
	cTalkPacket packet ( 0, nPacketSize );
	if( _receive( packet ) )
		proceed_response( packet );
}

void	basic_device_manager::proceed_response ( const cTalkPacket & packet )
// domain level delivery routine
{
	_LockIt _dnsLock ( _dnsMutex );
	if( packet.domain == cTalkPacket::domainCommon )
		for( dnsiter dp = _dns.begin(); dp != _dns.end(); _delivery_within_domain( packet, (dp++)->second ) );
	else
	{
		_delivery_within_domain( packet, _dns[ packet.domain ] );
		_delivery_within_domain( packet, _dns[ cTalkPacket::domainRDevMan ] );
	}
}

void	basic_device_manager::_delivery_within_domain ( const cTalkPacket & packet, device_pool & dp )
// low level delivery routine
{
	for( device_pool::iterator ptr = dp.begin(); ptr != dp.end(); )
		if( (*ptr)->cachedTake( packet ) == 1 )
			dp.remove( ptr++ );
		else
			++ptr;
}

} // namespace ude

