#ifndef CROSS_UDE_DEVMAN_H_INCLUDED
#define CROSS_UDE_DEVMAN_H_INCLUDED 1

#include <crs/ude/netpoint.h>
#include <crs/ude/basics.h>
#include <memory>

namespace ude {

class CROSS_EXPORT device_manager
	: public basic_device_manager
	, public netPoint
	, public CrossClass::cThread
{
private:
	class cDeliveryThread : public CrossClass::cThread
	{
	protected:
		device_manager * devman;
		const size_t batchSize;
		
		virtual bool Step ();
	
	public:
		cDeliveryThread( device_manager * dm, const size_t nBatchSize )
			: CrossClass::cThread( false )
			, devman( dm )
			, batchSize( nBatchSize )
		{ }
		
		virtual ~cDeliveryThread ()
		{
			kill();
		}
	};
	
protected:
	std::auto_ptr<cDeliveryThread> _deliveryThread;
	
      virtual void _resume_transmission ();	// signals to reactivate transmission
      virtual void _resume_delivery ();		// signals to reactivate delivery
	
      virtual bool _send ( const cTalkPacket & );
      virtual bool _receive ( cTalkPacket & );
	
	virtual bool Step ();
	
	// hide parent member
	netPoint::clientConnect;
	
public:
	device_manager ( const size_t nDeliveryBatchSize = 16 );
      virtual ~device_manager ();
      
	void	connect ( const CrossClass::cSockAddr & );
	
	//
      // proceed safe and gentle shutdown
      //
      virtual void shut_down ( const int = sdEverything );
};

} // namespace ude
#endif // CROSS_UDE_DEVMAN_H_INCLUDED
