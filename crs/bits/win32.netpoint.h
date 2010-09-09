#ifndef CROSS_WIN32_NETPOINT_H_INCLUDED
#define CROSS_WIN32_NETPOINT_H_INCLUDED 1
// (c) Jan 28, 2009 Oleg N. Peregudov
// Apr 23, 2009 - Win/Posix defines
// Aug 24, 2010 - new server termination algorithm based on events

#include <crs/bits/basic.netpoint.h>

namespace CrossClass {

class CROSS_EXPORT win32NetPoint : public basicNetPoint
{
protected:
	HANDLE evntTerminate;
	cTimeVal selectTimeOut;
	
	virtual void setNonBlock ();
	virtual cHandle<basicNetPoint> handleNewConnection ( cSocket &, const cSockAddr & );
	virtual bool preCheckTerminate ();
	virtual timeval * onStartServer ();
	
	win32NetPoint ( cSocket & );
	
public:
	win32NetPoint ();
	virtual ~win32NetPoint ();
	virtual void terminateServer ();
};

} // namespace CrossClass
#endif // CROSS_WIN32_NETPOINT_H_INCLUDED

