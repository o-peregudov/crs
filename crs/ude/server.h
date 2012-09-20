#ifndef CROSS_UDE_SERVER_H_INCLUDED
#define CROSS_UDE_SERVER_H_INCLUDED 1

/*
 *  crs/ude/server.h - General Hardware-to-PC interface ( base classes )
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

/*
 *	2003/08/05	General Hardware-to-PC interface ( base classes )
 *	2005/08/17	optimized packet handling for multidomain devices
 *	2005/09/28	VS 7.1 adaptation
 *	2006/03/09	uncached delivery method (not fully functional yet)
 *	2006/10/19	cachedTake is just an exception handler
 *	2007/05/22	uniform locks
 *	2007/11/21	new place for cross-compiling routines
 *	2008/01/30	placed in cross library
 *	2008/01/18	new send/recv interface to reduce the number of the threads
 *	2010/12/03	some corrections in comments 
 *			improved workflow
 *	2010/12/10	using unsigned long instead of size_t for packet size
 *	2010/12/17	default action on basic_device::reset & basic_device::invariant
 *	2010/12/18	new names: _post/_send/_recv
 *	2011/01/03	integer types
 *			notification about posted packet & proceed_request_job
 *	2011/01/23	device::attach, device::detach members
 *	2012/08/30	external event loop version based on tcp_peer
 *	2012/09/16	core data classes are now separated
 */

#include <crs/ude/packets.h>
#include <crs/ude/net_peer.h>
#include <crs/ude/data_core.h>
#include <crs/myexcept.h>

namespace ude {

/*
 * common exceptions
 */

class CROSS_EXPORT BaseException {};

struct CROSS_EXPORT ErrCommunication : BaseException, std::resource_error {
	ErrCommunication ( const std::string & what_arg ):
		BaseException(), std::resource_error( what_arg ) {}
};

struct CROSS_EXPORT ErrPacketLost : BaseException, std::logic_error {
	ErrPacketLost ( const std::string & what_arg ):
		BaseException(), std::logic_error( what_arg ) {}
};

struct CROSS_EXPORT CommunicationWarning : BaseException, std::logic_error {
	CommunicationWarning ( const std::string & what_arg ):
		BaseException(), std::logic_error( what_arg ) {}
};

// forward definitions
class CROSS_EXPORT device_manager;

/*
 * class device (abstract)
 * general device interface
 */
class CROSS_EXPORT device
{
	/* not allowed!	*/
	device ( const device & );
	device & operator = ( const device & );
	
protected:
	device_manager * _device_manager;
	
	void	postPacket ( const cTalkPacket & );
	bool	attach ( const ushort domain );
	bool	detach ( const ushort domain );
	
public:
	device ( device_manager * dm = 0 )
		: _device_manager( dm )
	{ }
	
	virtual ~device () {}
	
	virtual int take ( const cTalkPacket & ) = 0;
	/* take must return one of the following results:					*/
	/*    0 - it's not my packet									*/
	/*    1 - it's my packet and i was satisfied (please, remove me from pool)	*/
	/*    2 - it's my packet but i'll wait another one					*/
	
	virtual void reset ();
	/* reset device to it's initial state	*/
	
	virtual bool invariant ();
	/* device' invariant check			*/
	
	virtual bool poolControlled () const;	/* { return true; }		*/
	/* return true for devices allocated in a heap memory				*/
	/* (dynamic allocation). this will indicate that device			*/
	/* should be destroyed automatically during pool destruction procedure	*/
};

class CROSS_EXPORT client_stub	: public net_peer
						, public device
{
protected:
	/* restrict members' access	*/
	using net_peer::bind_socket;
	using net_peer::start_server;
	using net_peer::connect_to_server;
	
	std::list<cTalkPacket>	_queue;
	CrossClass::cMutex	_queueMutex;
	cTalkPacket			_inPacket;
	
	virtual bool handle_read ();		/* return true if a message was received		*/
	virtual bool handle_write ();		/* return true if a send should be pending	*/
	
public:
	client_stub ( CrossClass::event_loop * ev_loop, device_manager * dm, CrossClass::cSocket & );
	virtual ~client_stub ();
	
	virtual int take ( const cTalkPacket & );
	virtual bool poolControlled () const { return false; }
	/* this devices will be deleted by the event loop, but not by the device_manager	*/
};

/*
 * class device_manager (can be a network server)
 */
class CROSS_EXPORT device_manager	: public net_peer
{
protected:
	struct device_destroy_predicate
	{
		const bool is_pool_controlled;
		
		device_destroy_predicate ( device * d )
			: is_pool_controlled (d->poolControlled ())
		{ }
		
		operator bool () const
		{
			return is_pool_controlled;
		}
	};
	
	struct packet_delivery_action
	{
		cTalkPacket packet;
		
		packet_delivery_action ( const cTalkPacket & p )
			: packet (p)
		{ }
		
		bool operator () ( device * d ) const
		{
			if (d->take (packet) == 1)
				return true;	/* my packet and I want to be removed	*/
			else
				return false;	/* I'll wait for another packet		*/
		}
	};
	
	typedef element_map<ushort, device, device_destroy_predicate>	container_type;
	
	container_type	_container;
	
protected:
	std::list<cTalkPacket>			_outbox;	/* packets to send		*/
	CrossClass::cMutex			_dnsMutex,
							_outboxMutex;
	CrossClass::cConditionVariable	_outboxNotify;
	
	CrossClass::cMutex			_requestThreadMutex;
	CrossClass::cConditionVariable	_requestThreadNotify;
	bool						_requestThreadFlag;
	
	virtual void _post ( const cTalkPacket & );	/* place packet into the outbox		*/
	virtual bool _send ( const cTalkPacket & );	/* translate & send packet to hardware	*/
	virtual bool _recv ( cTalkPacket & );		/* compile received response			*/
	/* if returns true, than packet's queue could be updated	*/
	
	virtual CrossClass::tcp_peer * handle_new_connection ( CrossClass::cSocket &, const CrossClass::cSockAddr & );
	
public:
	struct terminate_request_thread {};
	
	device_manager ( CrossClass::event_loop * ev_loop );
	virtual ~device_manager ();
	
	/*
	 * domain name system manipulators
	 */
	bool	register_device ( const ushort domain, device * );
	bool	unregister_device ( const ushort domain, device * );
	
	/*
	 * the way to send you request to hardware or another device
	 */
	void	register_request ( device *, const cTalkPacket & );
	
	/*
	 * process requests from devices and send them to hardware
	 * returns true if there are more packets to send
	 */
	bool	proceed_request ();
	void	mark_request_thread_start ();
	void	notify_request_thread_termination ();
	void	wait_request_thread_activation ();
	
	/*
	 * NOTE: typical request thread function
	 */
	void	proceed_request_job ();
	//{
	//	mark_request_thread_start ();
	//	for( ;; )
	//	{
	//		try
	//		{
	//			if( !proceed_request() )
	//				wait_request_thread_activation();
	//		}
	//		catch( terminate_request_thread )
	//		{
	//			notify_request_thread_termination();
	//			break;
	//		}
	//		catch( ... )
	//		{
	//			throw;
	//		}
	//	}
	//}
	
	/*
	 * handle hardware response (will cause imediate delivery)
	 * returns true if something has been received and delivered
	 */
	bool	proceed_response ();
	
	/*
	 * deliver packet to child devices
	 */
	void	deliver_response ( const cTalkPacket & packet );
};

}	/* namespace ude				*/
#endif/* CROSS_UDE_SERVER_H_INCLUDED	*/
