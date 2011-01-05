// (c) Jun 12, 2007 Oleg N. Peregudov
//	11/21/2007	new place for cross-compiling routines
//	01/30/2008	placed in cross library
//	12/03/2010	improved workflow
//	12/17/2010	default action on basic_device::reset & basic_device::invariant
//	12/18/2010	new names: _post/_send/_recv
//	01/03/2011	integer types
//			notification about posted packet & proceed_request_job
//
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#endif

#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif

#include <crs/ude/server.h>
#include <cstdio>

namespace ude {

using namespace CrossClass;

//
// members of class device
//

void	device::postPacket ( const cTalkPacket & packet )
{
	_device_manager->register_request( this, packet );
}

void	device::reset ()
{
}

bool	device::invariant ()
{
	return true;
}

bool	device::poolControlled () const
{
	// return true for devices allocated in a heap memory
	// (dynamic allocation). this will indicate that device
	// should be destroyed automatically during pool destruction procedure
	return true;
}

//
// members of class client_stub
//

client_stub::client_stub ( device_manager * dm, CrossClass::cSocket & sckt )
	: netPoint( sckt )
	, device( dm )
	, _queue( )
	, _queueMutex( )
	, _inPacket( )
{
}

client_stub::~client_stub ()
{
}

void	client_stub::transmit ()
{
	_LockIt _queueLock ( _queueMutex );
	for( ;; )
	{
		netPoint::transmit();
		if( _queue.empty() )
			break;	// there is nothing to transmit
		else if( sendPacket( _queue.front() ) )
			_queue.pop_front();
		else
			break;	// because we have something to transmit and
					// operation is pending
	}
}

void	client_stub::receive ()
{
	netPoint::receive();
	while( recvPacket( _inPacket ) )
		postPacket( _inPacket );
}

int	client_stub::take ( const cTalkPacket & packet )
{
	if( !sendPacket( packet ) )
	{
		CrossClass::_LockIt _queueLock ( _queueMutex );
		_queue.push_back( packet );
	}
	return 2;
}

//
// members of class device_manager
//

device_manager::device_manager ()
	: _dns()
	, _outbox()
	, _dnsMutex()
	, _outboxMutex()
	, _outboxNotify()
	, _requestThreadMutex()
	, _requestThreadNotify()
	, _requestThreadFlag( true )
{
}

device_manager::~device_manager ()
{
	//
	// force terminate of the request thread
	//
	cTalkPacket terminatePacket ( cPacketHeader::idLocal|cPacketHeader::idFast, sizeof( double ), cPacketHeader::domainSDevMan, cPacketHeader::rcpThread );
	terminatePacket.word( 0 ) = 0x0001;
	_post( terminatePacket );
	
	//
	// waiting for actual termination of the request thread
	//
	CrossClass::_LockIt _requestThreadLock ( _requestThreadMutex );
	while( !_requestThreadFlag )
		_requestThreadNotify.wait( _requestThreadLock );
}

bool	device_manager::register_device ( const ushort domain, device * dev )
{
	if( dev )
	{
		_LockIt _dnsLock ( _dnsMutex );
		_dns[ domain ].push_back( dev );
		return true;
	}
	return false;
}

bool	device_manager::unregister_device ( const ushort domain, device * dev )
{
	_LockIt _dnsLock ( _dnsMutex );
	device_pool & dp = _dns[ domain ];
	for( device_pool::iterator p = dp.begin(); p != dp.end(); ++p )
	{
		if( *p == dev )
		{
			device_pool::iterator q = --dp.end();
			*p = *q;
			dp.pop_back();
			return true;
		}
	}
	return false;
}

void	device_manager::_post ( const cTalkPacket & packet )
{
	_LockIt _outboxLock ( _outboxMutex );
	if( ( packet.header().id & cPacketHeader::idFast ) == cPacketHeader::idFast )
		_outbox.push_front( packet );
	else
		_outbox.push_back( packet );
	_outboxNotify.notify_one();
}

bool	device_manager::_send ( const cTalkPacket & packet )
{
	switch( packet.header().domain )
	{
	case	cPacketHeader::domainStat:
		switch( packet.header().recepient )
		{
		case	0x0001:	// packet from ping tool
			deliver_response( packet );
			break;
		
		default:
			break;
		}
		break;
	
	case	cPacketHeader::domainSDevMan:
		switch( packet.header().recepient )
		{
		case	cPacketHeader::rcpThread:
			if( ( ( packet.header().id & cPacketHeader::idLocal ) == cPacketHeader::idLocal ) && ( packet.word( 0 ) == 0x0001 ) )
				throw terminate_request_thread ();
			break;
		
		default:
			break;
		}
		break;
	
	default:
		break;
	}
	return true;
}

bool	device_manager::_recv ( cTalkPacket & packet )
{
	return false;
}

basicNetPoint * device_manager::handleNewConnection ( cSocket & sckt, const cSockAddr & scktAddr )
{
	return new client_stub ( this, sckt );
}

void	device_manager::register_request ( device * dev, const cTalkPacket & packet )
{
	if( ( packet.header().id & (cPacketHeader::idLocal|cPacketHeader::idDTD) ) == (cPacketHeader::idLocal|cPacketHeader::idDTD) )
		deliver_response( packet );	// device-to-device interaction
							// don't send it to hardware!
	else
		_post( packet );			// remote packet - send it!
}

bool	device_manager::proceed_request ()
{
	_LockIt _outboxLock ( _outboxMutex );
	if( _outbox.empty() )
		return false;
	else
	{
		cTalkPacket packet2send ( _outbox.front() );
		_outbox.pop_front();
		_outboxLock.unlock();
		if( _send( packet2send ) )
			;
		_outboxLock.lock();
		return !_outbox.empty();
	}
}

bool	device_manager::proceed_response ()
{
	cTalkPacket packet;
	if( _recv( packet ) )
	{
		deliver_response( packet );
		return true;
	}
	else
		return false;
}

void	device_manager::deliver_response ( const cTalkPacket & packet )
// domain level delivery routine
{
	_LockIt _dnsLock ( _dnsMutex );
	if( packet.header().domain == cPacketHeader::domainCommon )
		for( dnsiter dp = _dns.begin(); dp != _dns.end(); _delivery_within_domain( packet, (dp++)->second ) );
	else
		_delivery_within_domain( packet, _dns[ packet.header().domain ] );
	_dnsLock.unlock();
	
	client_stub * cstub;
	for( size_t i = 0; i < _clientList.size(); ++i )
	{
		cstub = dynamic_cast<client_stub *>( _clientList[ i ] );
		if( cstub )
		{
			try
			{
				if( cstub->take( packet ) == 1 )
					removeClient( i );
			}
			catch( ... )
			{
				removeClient( i );
			}
		}
	}
}

void	device_manager::_delivery_within_domain ( const cTalkPacket & packet, device_pool & dp )
// low level delivery routine
{
	for( device_pool::iterator ptr = dp.begin(); ptr != dp.end(); )
	{
		try
		{
			if( (*ptr)->take( packet ) == 1 )
				dp.remove( ptr++ );
			else
				++ptr;
		}
		catch( ... )
		{
			dp.remove( ptr++ );
		}
	}
}

void	device_manager::mark_request_thread_start ()
{
	CrossClass::_LockIt _requestThreadLock ( _requestThreadMutex );
	_requestThreadFlag = false;
}

void	device_manager::notify_request_thread_termination ()
{
	CrossClass::_LockIt _requestThreadLock ( _requestThreadMutex );
	_requestThreadFlag = true;
	_requestThreadNotify.notify_one();
}

void	device_manager::wait_request_thread_activation ()
{
	_LockIt _outboxLock ( _outboxMutex );
	while( _outbox.empty() )
		_outboxNotify.wait( _outboxLock );
}

void	device_manager::proceed_request_job ()
{
	mark_request_thread_start ();
	for( ;; )
	{
		try
		{
			if( !proceed_request() )
				wait_request_thread_activation();
		}
		catch( terminate_request_thread )
		{
			notify_request_thread_termination();
			break;
		}
		catch( ... )
		{
			throw;
		}
	}
}

} // namespace ude

