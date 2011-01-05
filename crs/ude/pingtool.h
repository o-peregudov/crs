#ifndef CROSS_UDE_PINGTOOL_H_INCLUDED
#define CROSS_UDE_PINGTOOL_H_INCLUDED 1
// (c) Oleg N. Peregudov
//	12/12/2010	simple tool for ude-network performance measurement
//	12/19/2010	new name for base ude classes

#include <crs/ude/server.h>
#include <crs/timer.h>
#include <limits>

namespace ude {

class CROSS_EXPORT pingTool : public device
{
protected:
	CrossClass::cTimer	_timer;
	CrossClass::cMutex	_mutex;
	CrossClass::cConditionVariable _notify;
	bool				_flag;
	cTalkPacket			_packet;
	
	virtual int take ( const cTalkPacket & );
	
	struct waitpred
	{
		bool * flag;
		waitpred( bool * f ) : flag ( f ) { }
		bool operator () () { return !(*flag); }
	};
	
public:
	pingTool ( device_manager * );
	virtual ~pingTool ();
	
	virtual bool poolControlled () const;	// { return false; }
	virtual void reset ();
	
	double ping ( );	// non blocking version, returns -1 while waiting
	double ping ( const unsigned long msTimeOut );
};

} // namespace ude
#endif // CROSS_UDE_PINGTOOL_H_INCLUDED

