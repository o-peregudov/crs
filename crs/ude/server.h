#ifndef CROSS_UDE_SERVER_H_INCLUDED
#define CROSS_UDE_SERVER_H_INCLUDED 1

//
// General Hardware-to-PC interface ( base classes )
// Copyright (c) Aug 5, 2003 Oleg N. Peregudov
// Support: op@pochta.ru
//
//	08/17/2005	optimized packet handling for multidomain devices
//	09/28/2005	VS 7.1 adaptation
//	03/09/2006	uncached delivery method (not fully functional yet)
//	10/19/2006	cachedTake is just an exception handler
//	05/22/2007	uniform locks
//	11/21/2007	new place for cross-compiling routines
//	01/30/2008	placed in cross library
//	01/18/2008	new send/recv interface to reduce the number of the threads
//	12/03/2010	some corrections in comments 
//			improved workflow
//	12/10/2010	using unsigned long instead of size_t for packet size
//	12/17/2010	default action on basic_device::reset & basic_device::invariant
//	12/18/2010	new names: _post/_send/_recv
//	01/03/2011	integer types
//			notification about posted packet & proceed_request_job
//

#include <map>
#include <list>
#include <algorithm>
#include <crs/ude/netpoint.h>
#include <crs/ude/packets.h>
#include <crs/myexcept.h>

namespace ude {

//
// common exceptions
//

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

//
// class device (abstract)
// general device interface
//
class CROSS_EXPORT device
{
	// not allowed!
	device ( const device & );
	device & operator = ( const device & );
	
protected:
	device_manager * _device_manager;
	
	void	postPacket ( const cTalkPacket & );
	
public:
	device ( device_manager * dm = 0 )
		: _device_manager( dm )
	{ }
	
	virtual ~device () {}
	
	virtual int take ( const cTalkPacket & ) = 0;
	// take must return one of the following results:
	//    0 - it's not my packet
	//    1 - it's my packet and i was satisfied (please, remove me from pool)
	//    2 - it's my packet but i'll wait another one
	
	virtual void reset ();
	// reset device to it's initial state
	
	virtual bool invariant ();
	// device' invariant check
	
	virtual bool poolControlled () const;	// { return true; }
	// return true for devices allocated in a heap memory
	// (dynamic allocation). this will indicate that device
	// should be destroyed automatically during pool destruction procedure
};

class CROSS_EXPORT client_stub	: public netPoint
						, public device
{
protected:
	// hide parent members
	netPoint::startServer;
	netPoint::clientConnect;
	netPoint::clientSendRecv;
	
	std::list<cTalkPacket>	_queue;
	CrossClass::cMutex	_queueMutex;
	cTalkPacket			_inPacket;
	
	virtual void transmit ();
	virtual void receive ();
	
public:
	client_stub ( device_manager * dm, CrossClass::cSocket & );
	virtual ~client_stub ();
	
	virtual int take ( const cTalkPacket & );
	virtual bool poolControlled () const { return false; }
	// this devices will be deleted by basicNetPoint::removeClient
};

//
// class device_manager (base, abstract)
//
class CROSS_EXPORT device_manager	: public netPoint
{
	typedef std::list<device*> tDeviceList;
	
	//
	// class device_pool (concrete, base)
	// provides safe memory release
	//
	class device_pool : public tDeviceList
	{
		// restrict member access
		tDeviceList::erase;
		
		struct device_destroyer {
			void operator () ( device * d ) const {
				// HERE: we will destroy only automatic devices
				//       (see description of device::poolControlled()
				//       member for additional explanations)
				if( d->poolControlled() )
				{
					delete d;
					d = 0;
				}
			}
		};
		
		// this will save two lines of code
		device_destroyer destroy_device;
		
	public:
		device_pool () : tDeviceList (), destroy_device () {}
		device_pool ( const device_pool & o ) : tDeviceList ( o ), destroy_device () {}
		
		virtual ~device_pool () {
			std::for_each( begin(), end(), destroy_device );
		}
		
		void	remove ( iterator pos ) {
			destroy_device( *pos );
			erase( pos );
		}
	};
	
public:
	typedef std::map<ushort, device_pool>	dnsmap;
	typedef dnsmap::iterator			dnsiter;
	
	struct terminate_request_thread {};
	
protected:
	dnsmap			_dns;
	std::list<cTalkPacket>	_outbox;	// packets to send
	CrossClass::cMutex	_dnsMutex,
					_outboxMutex;
	CrossClass::cConditionVariable _outboxNotify;
	
	CrossClass::cMutex	_requestThreadMutex;
	CrossClass::cConditionVariable  _requestThreadNotify;
	bool				_requestThreadFlag;
	
	void _delivery_within_domain ( const cTalkPacket &, device_pool & );
	
	virtual void _post ( const cTalkPacket & );	// place packet into outbox
	virtual bool _send ( const cTalkPacket & );	// translate & send packet to hardware
	virtual bool _recv ( cTalkPacket & );		// compile received response
	// if returns true, than packet's queue could be updated
	
	virtual CrossClass::basicNetPoint * handleNewConnection ( CrossClass::cSocket &, const CrossClass::cSockAddr & );
	
public:
	device_manager ();
	virtual ~device_manager ();
	
	//
	// domain name system manipulators
	//
	bool	register_device ( const ushort domain, device * );
	bool	unregister_device ( const ushort domain, device * );
	
	//
	// the way to send you request to hardware or another device
	//
	void	register_request ( device *, const cTalkPacket & );
	
	//
	// process requests from devices and send them to hardware
	// returns true if there are more packets to send
	//
	bool	proceed_request ();
	void	mark_request_thread_start ();
	void	notify_request_thread_termination ();
	void	wait_request_thread_activation ();
	
	//
	// NOTE: typical request thread function
	//
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
	
	//
	// handle hardware response (will cause imediate delivery)
	// returns true if something has been received and delivered
	//
	bool	proceed_response ();
	
	//
	// deliver packet to child devices
	//
	void	deliver_response ( const cTalkPacket & packet );
};

} // namespace ude
#endif // CROSS_UDE_SERVER_H_INCLUDED

