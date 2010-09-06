#ifndef CROSS_POSIX_NETPOINT_H_INCLUDED
#define CROSS_POSIX_NETPOINT_H_INCLUDED 1
// (c) Jan 28, 2009 Oleg N. Peregudov
// Apr 23, 2009 - Win/Posix defines
// Aug 26, 2010 - new server termination algorithm based on pipes

#include <crs/bits/basic.netpoint.h>
#include <syslog.h>

namespace CrossClass {

class CROSS_EXPORT posixNetPoint : public basicNetPoint
{
protected:
	int ipcPipeEnd [ 2 ];
	char pipeInBuf [ 16 ];
	char * pipeInBufPtr;
	
	posixNetPoint ( cSocket & );
	
	virtual void setNonBlock ();
	virtual cHandle<basicNetPoint> handleNewConnection ( cSocket &, const cSockAddr & );
	
	virtual void logGracefulDisconnect ();
	virtual void logRuntimeError ( const std::string & msg );
	virtual void logUnhandledError ();
	
	virtual void bindSocket ( const cSockAddr & );
	virtual timeval * onStartServer ();
	virtual void buildSelectList ();
	virtual bool postCheckTerminate ();
	
public:
	posixNetPoint ();
	virtual ~posixNetPoint ();
	virtual void terminateServer ();
};

} // namespace CrossClass
#endif // CROSS_POSIX_NETPOINT_H_INCLUDED

