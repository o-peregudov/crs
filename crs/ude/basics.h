#ifndef CROSS_UDE_BASICS_H
#define CROSS_UDE_BASICS_H 1

//
// General Hardware-to-PC interface ( base classes )
// Copyright (c) Aug 5, 2003 Oleg N. Peregudov
// Support: op@pochta.ru
//
// 08/17/2005 - optimized packet handling for multidomain devices
// 09/28/2005 - -VS 7.1 adaptation
// 03/09/2006 - uncached delivery method (not fully functional yet)
// 10/19/2006 - cachedTake is just an exception handler
// 05/22/2007 - uniform locks
// 11/21/2007 - new place for cross-compiling routines
// 01/30/2008 - placed in cross library
// 01/18/2008 - new send/recv interface to reduce the number of the threads
// 12/03/2010 - some corrections in comments 
//              improved workflow
//

#include <iostream>
#include <algorithm>
#include <list>
#include <map>

#include <crs/myexcept.h>
#include <crs/security.h>
#include <crs/ude/packets.h>

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
class CROSS_EXPORT basic_device_manager;

//
// class basic_device (abstract)
// general device interface
//
class CROSS_EXPORT basic_device
{
	// not allowed!
	basic_device ( const basic_device & );
	basic_device & operator = ( const basic_device & );
	
protected:
	basic_device_manager * _device_manager;
	
public:
	basic_device ( basic_device_manager * dm = 0 )
		: _device_manager( dm )
	{ }
	
	virtual ~basic_device () {}
	
	int cachedTake ( const cTalkPacket & );
	// prevent processing of the same packet with serial take calls
	// and perform standard actions for predefined packets
	
	virtual int take ( const cTalkPacket & ) = 0;
	// take must return one of the following results:
	//    0 - it's not my packet
	//    1 - it's my packet and i was satisfied (please, remove me from pool)
	//    2 - it's my packet but i'll wait another one
	
	virtual void reset () = 0;
	// reset device to it's initial state
	
	virtual bool invariant () = 0;
	// device' invariant check
	
	virtual bool poolControlled () const { return true; }
	// return true for devices allocated in a heap memory
	// (dynamic allocation). this will indicate that device
	// should be destroyed automatically during pool destruction procedure
};

//
// class basic_device_manager (base, abstract)
//
class CROSS_EXPORT basic_device_manager
{
	typedef std::list<basic_device*> tDeviceList;
	
	//
	// class device_pool (concrete, base)
	// provides safe memory release
	//
	class device_pool : public tDeviceList
	{
		// restrict member access
		tDeviceList::erase;
		
		struct device_destroyer {
			void operator () ( basic_device * d ) const {
				// HERE: we will destroy only automatic devices
				//       (see description of basic_device::poolControlled()
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
	
	//
	// class _internal_simple_counter (concrete)
	//
	struct _internal_simple_counter
	{
		size_t nDone,
			nBatchSize;
		
		_internal_simple_counter ( const size_t nCountMax )
			: nDone( 0 )
			, nBatchSize( nCountMax )
		{ }
		
		bool operator () ()
		{
			return ( (nBatchSize == static_cast<size_t>( -1 )) || (nDone++ < nBatchSize) );
		}
	};

public:
	enum shutdownmodes {
		sdReceive		= 0x01,
		sdTransmit		= 0x02,
		sdDelivery		= 0x04,
		
		sdEverything	= sdReceive | sdTransmit | sdDelivery
	};
	
	enum unregisteringcodes {
		udNotFound		= 0x00,	// device was not found in a specified domain
		udBusyDNS		= 0x01,	// DNS is currently busy ** OBSOLET **
		udSuccess		= 0x02	// device was successfully dettached
	};
	
	enum jobcodes {
		jcAllDone		= 0x00,	// the job was completely done
		jcBusy		= 0x01,	// the job is under processing now
		jcPartialDone	= 0x02	// the job was partially done
	};
	
	typedef std::map<ushort, device_pool> dnsmap;
	typedef dnsmap::iterator		  dnsiter;

protected:
	dnsmap			_dns;
	std::list<cTalkPacket>	_outbox;	// packets to send
	CrossClass::LockType	_dnsMutex,
					_outboxMutex;
	
	void _delivery_within_domain ( const cTalkPacket &, device_pool & );
	
	virtual void _post ( const cTalkPacket & );	// place packet into outbox
	virtual bool _send ( const cTalkPacket & ) = 0;	// translate & send packet to hardware
	virtual bool _receive ( cTalkPacket & ) = 0;	// compile received response
	// if returns true, than packet's queue could be updated
	
public:
	basic_device_manager ();
	virtual ~basic_device_manager ();
	
public:
	//
	// domain name system
	//
	
	bool	register_device ( const ushort domain, basic_device * );
	// this will not cause any problems because new devices
	// will be puted at list's tail
	
	int	unregister_device ( const ushort domain, basic_device * );
	// NOTE: after this call you must manually release memory
	//       allocated for your device
	// function returns:
	//    udNotFound  -     device was not found in a specified domain
	//    udSuccess   -     device was successfully dettached
	
public:
	//
	// the way to send you request to hardware or another device
	// (short message version)
	//
	void	register_request ( basic_device * device, const cTalkPacket & packet )
	{
		if( ( packet.id & (cTalkPacket::idLocal|cTalkPacket::idDTD) ) == (cTalkPacket::idLocal|cTalkPacket::idDTD) )
			proceed_response( packet );	// device-to-device interaction
								// don't send it to hardware!
		else
			_post( packet );			// remote packet - send it!
	}
	
	//
	// communication driver should call this member to send
	// all requests to hardware
	//
	
	// predicate based function - runs until predicate is true
	template <class Pred> int predicate_proceed_request ( Pred p )
	{
		CrossClass::_LockIt _outboxLock ( _outboxMutex );
		while( !_outbox.empty() )
		{
			if( p() )
			{
				if( _send( _outbox.front() ) )
					_outbox.pop_front();
			}
			else
				return jcPartialDone;
		}
		return jcAllDone;
	}
	
	// simple counter based version to proceed requests
	// returns job code
	int	proceed_request ( const size_t nPackets = static_cast<size_t>( -1 ) )
	{
		_internal_simple_counter counter ( nPackets );
		return predicate_proceed_request( counter );
	}
	
public:
	//
	// communication driver should call this members
	// if it'll be notified about hardware's response arrival
	//
	void	proceed_response ( const size_t nPacketSize = 0 );
	void	proceed_response ( const cTalkPacket & packet );
};

} // namespace ude
#endif // CROSS_UDE_BASICS_H
