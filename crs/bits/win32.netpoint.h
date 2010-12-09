#ifndef CROSS_WIN32_NETPOINT_H_INCLUDED
#define CROSS_WIN32_NETPOINT_H_INCLUDED 1
// (c) Jan 28, 2009 Oleg N. Peregudov
// 04/23/2009 - Win/Posix defines
// 08/24/2010 - new server termination algorithm based on events
// 12/01/2010 - new name for termination method
//              new implementation

#include <crs/bits/basic.netpoint.h>

namespace CrossClass {

class CROSS_EXPORT win32NetPoint : public basicNetPoint
{
protected:
	fd_set readset,
		 writeset,
		 exceptset;
	cSocket::host_socket_type highsock;
	HANDLE evntTerminate;
	cTimeVal selectTimeOut;
	
protected:
	virtual basicNetPoint * handleNewConnection ( cSocket &, const cSockAddr & );
	virtual void buildSelectList ();
	virtual bool checkTerminate ();
	
	win32NetPoint ( cSocket & );
	
public:
	win32NetPoint ();
	virtual ~win32NetPoint ();
	
	virtual void postTerminate ();
	virtual bool clientSendRecv ();
	virtual bool serverSendRecv ();
};

} // namespace CrossClass
#endif // CROSS_WIN32_NETPOINT_H_INCLUDED

