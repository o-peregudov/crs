/*
 *  crs/ude/server.cpp - General Hardware-to-PC interface ( base classes )
 *  Copyright (c) 2003-2012 Oleg N. Peregudov <o.peregudov@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif

#include <crs/ude/server.h>
#include <cstdio>

namespace ude {

using namespace CrossClass;

/*
 * members of class device
 */

void	device::postPacket ( const cTalkPacket & packet )
{
	_device_manager->register_request (this, packet);
}

bool	device::attach ( const ushort domain )
{
	return _device_manager->register_device (domain, this);
}

bool	device::detach ( const ushort domain )
{
	return _device_manager->unregister_device (domain, this);
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
	/*
	 * return true for devices allocated in a heap memory
	 * (dynamic allocation). this will indicate that device
	 * should be destroyed automatically during pool destruction procedure
	 */
	return true;
}

/*
 * members of class client_stub
 */

client_stub::client_stub ( event_loop * ev_loop, device_manager * dm, cSocket & sckt )
	: net_peer (ev_loop, sckt)
	, device (dm)
	, _queue ( )
	, _queueMutex ( )
	, _inPacket ( )
{
}

client_stub::~client_stub ()
{
}

bool	client_stub::handle_read ()
{
	if (net_peer::handle_read ())
	{
		while (recv_packet (_inPacket))
			postPacket (_inPacket);
		return true;
	}
	return false;
}

bool	client_stub::handle_write ()
{
	_LockIt _queueLock ( _queueMutex );
	for( ;; )
	{
		if (net_peer::handle_write () || _queue.empty ())
			break;	/* output buffer is full or we don't have any packets for transmission	*/
		else if (send_packet (_queue.front ()))
			_queue.pop_front ();
		else
			break;	/* because we have something to transmit but operation is pending		*/
	}
}

int	client_stub::take ( const cTalkPacket & packet )
{
	if (!send_packet (packet))
	{
		_LockIt _queueLock ( _queueMutex );
		_queue.push_back (packet);
	}
	return 2;	/* always take all packets	*/
}

/*
 * members of class device_manager
 */

device_manager::device_manager ( CrossClass::event_loop * ev_loop )
	: net_peer (ev_loop)
	, _container ()
	, _outbox ()
	, _dnsMutex ()
	, _outboxMutex ()
	, _outboxNotify ()
	, _requestThreadMutex ()
	, _requestThreadNotify ()
	, _requestThreadFlag (true)
{
}

device_manager::~device_manager ()
{
	/*
	 * force terminate of the request thread
	 */
	cTalkPacket terminatePacket ( cPacketHeader::idLocal|cPacketHeader::idFast, sizeof( double ), cPacketHeader::domainSDevMan, cPacketHeader::rcpThread );
	terminatePacket.word( 0 ) = 0x0001;
	_post (terminatePacket);
	
	/*
	 * waiting for actual termination of the request thread
	 */
	_LockIt _requestThreadLock ( _requestThreadMutex );
	while (!_requestThreadFlag)
		_requestThreadNotify.wait (_requestThreadLock);
}

bool	device_manager::register_device ( const ushort domain, device * dev )
{
	if (dev)
	{
		_LockIt _dnsLock ( _dnsMutex );
		_container.add (domain, dev);
		return true;
	}
	return false;
}

bool	device_manager::unregister_device ( const ushort domain, device * dev )
{
	if (dev)
	{
		_LockIt _dnsLock ( _dnsMutex );
		_container.remove (domain, dev);
		return true;
	}
	return false;
}

void	device_manager::_post ( const cTalkPacket & packet )
{
	_LockIt _outboxLock ( _outboxMutex );
	if (packet.header ().id_check_bits (cPacketHeader::idFast))
		_outbox.push_front (packet);
	else
		_outbox.push_back (packet);
	_outboxNotify.notify_one ();
}

bool	device_manager::_send ( const cTalkPacket & packet )
{
	switch (packet.header ().domain)
	{
	case	cPacketHeader::domainStat:
		switch (packet.header ().recepient)
		{
		case	0x0001:	/* packet from ping tool	*/
			deliver_response (packet);
			break;
		
		default:
			break;
		}
		break;
	
	case	cPacketHeader::domainSDevMan:
		switch (packet.header ().recepient)
		{
		case	cPacketHeader::rcpThread:
			if (packet.header ().id_check_bits (cPacketHeader::idLocal) && (packet.word (0) == 0x0001))
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

tcp_peer * device_manager::handle_new_connection ( cSocket & sckt, const cSockAddr & scktAddr )
{
	return new client_stub (_event_loop, this, sckt);
}

void	device_manager::register_request ( device * dev, const cTalkPacket & packet )
{
	if (packet.header ().id_check_bits (cPacketHeader::idLocal|cPacketHeader::idDTD))
		deliver_response (packet);	/* device-to-device interaction	*/
							/* don't send it to hardware!		*/
	else
		_post (packet);			/* remote packet - send it!		*/
}

bool	device_manager::proceed_request ()
{
	_LockIt _outboxLock ( _outboxMutex );
	if (_outbox.empty ())
		return false;
	else
	{
		cTalkPacket packet2send ( _outbox.front () );
		_outbox.pop_front ();
		_outboxLock.unlock ();
		if (_send (packet2send))
			;
		_outboxLock.lock ();
		return !_outbox.empty ();
	}
}

bool	device_manager::proceed_response ()
{
	cTalkPacket packet;
	if (_recv (packet))
	{
		deliver_response (packet);
		return true;
	}
	else
		return false;
}

void	device_manager::deliver_response ( const cTalkPacket & packet )
/* domain level delivery routine	*/
{
	packet_delivery_action action (packet);
	
	_LockIt _dnsLock ( _dnsMutex );
	if (packet.header ().domain == cPacketHeader::domainCommon)
		_container.for_each (action);
	else
		_container.for_each (packet.header ().domain, action);
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
	_requestThreadNotify.notify_one ();
}

void	device_manager::wait_request_thread_activation ()
{
	_LockIt _outboxLock ( _outboxMutex );
	while (_outbox.empty ())
		_outboxNotify.wait (_outboxLock);
}

void	device_manager::proceed_request_job ()
{
	mark_request_thread_start ();
	for( ;; )
	{
		try
		{
			if (!proceed_request ())
				wait_request_thread_activation ();
		}
		catch (terminate_request_thread)
		{
			notify_request_thread_termination ();
			break;
		}
		catch (...)
		{
			throw;
		}
	}
}

}	/* namespace ude	*/
