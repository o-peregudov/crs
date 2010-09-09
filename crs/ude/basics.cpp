// (c) Jun 12, 2007 Oleg N. Peregudov
// (c) Nov 21, 2007 - new place for cross-compiling routines
// (c) Jan 30, 2008 - placed in cross library
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
	, _inbox()
	, _outbox()
	, _dnsLock()
	, _inboxLock()
	, _outboxLock()
{
}

basic_device_manager::~basic_device_manager ()
{
}

bool	basic_device_manager::_delivery_failed ( const cTalkPacket & )
{
	return true; // by default - remove packet from queue
}

void	basic_device_manager::_do_post_packet ( const cTalkPacket & packet )
{
	_LockIt ea ( _inboxLock );
	_outbox.push_back( packet );
}

bool	basic_device_manager::register_device ( const ushort domain, basic_device * device )
{
	if( device )
	{
		_LockIt _dnsLocker ( _dnsLock );
		_dns[ domain ].push_back( device );
		return true;
	}
	return false;
}

void	basic_device_manager::register_request ( basic_device * device, const cTalkPacket & packet )
{
	_post_packet( packet );
	_resume_transmission();
}

void	basic_device_manager::proceed_response ( const size_t nPacketSize )
{
	cTalkPacket packet ( 0, nPacketSize );
	if( _receive( packet ) )
		proceed_response( packet );
}

void	basic_device_manager::proceed_response ( const cTalkPacket & packet )
{
	_LockIt _inboxLocker ( _inboxLock );
	_inbox.push_back( packet );
	_inboxLocker.unlock();
	_resume_delivery();
}

bool	basic_device_manager::_delivery_within_domain ( const cTalkPacket & packet, device_pool & dp )
// low level delivery routine
{
	bool accepted ( false );
	device_pool::iterator ptr = dp.begin(),
				    qtr = dp.end();
	while( ptr != dp.end() )
	{
		if( *ptr == 0 )
			qtr = ptr;
		else
			switch( (*ptr)->cachedTake( packet ) )
			{
			case	1:	// this is my packet and i'm satisfied
				qtr = ptr;
				accepted |= true;
				break;
			
			case	2:	// this is my packet but i'll wait another one
				accepted |= true;
				break;
			
			case	-1:	// broken packet - stop delivering
				return accepted;
			
			case	-2:	// strange exception on this packet - do nothing
				break;
			}
		++ptr;
		if( qtr != dp.end() )
		{
			dp.remove( qtr );
			qtr = dp.end();
		}
	}
	return accepted;
}

bool	basic_device_manager::_delivery_core ( const cTalkPacket & packet )
// domain level delivery routine
{
	bool accepted ( false );
	_LockIt _dnsLocker ( _dnsLock );
	if( packet.domain == cTalkPacket::domainCommon )
		for( dnsiter dp = _dns.begin(); dp != _dns.end(); accepted |= _delivery_within_domain( packet, (dp++)->second ) );
	else
	{
		accepted |= _delivery_within_domain( packet, _dns[ packet.domain ] );
		accepted |= _delivery_within_domain( packet, _dns[ cTalkPacket::domainRDevMan ] );
	}
	return accepted;
}

int	basic_device_manager::unregister_device ( const ushort domain, basic_device * device )
{
	_LockIt _dnsLocker ( _dnsLock );
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

} // namespace ude

