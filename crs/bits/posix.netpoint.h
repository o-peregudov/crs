#ifndef CROSS_POSIX_NETPOINT_H_INCLUDED
#define CROSS_POSIX_NETPOINT_H_INCLUDED 1
// (c) Jan 28, 2009 Oleg N. Peregudov
// 04/23/2009 - Win/Posix defines
// 08/26/2010 - new server termination algorithm based on pipes
// 11/30/2010 - usage of the poll system call
// 12/04/2010 - new pipe creation concept

#include <crs/bits/basic.netpoint.h>
#include <poll.h>

namespace CrossClass {

class CROSS_EXPORT posixNetPoint : public basicNetPoint
{
protected:
	int ipcPipeEnd [ 2 ];
	char pipeInBuf [ 16 ];
	char * pipeInBufPtr;
	
	pollfd *	fds;
	size_t	nfdsAllocated,
			nfdsUsed;
	
	void createPipe ();
	void closePipe ();
	posixNetPoint ( cSocket & );
	
protected:
	virtual void setNonBlock ();
	
	virtual basicNetPoint * handleNewConnection ( cSocket &, const cSockAddr & );
	virtual size_t enumerateDescriptors ();
	virtual void buildSelectList ();
	virtual bool checkTerminate ();
	
public:
	posixNetPoint ();
	virtual ~posixNetPoint ();
	
	virtual void bindSocket ( const cSockAddr & );
	virtual void clientConnect ( const cSockAddr & );
	virtual void postTerminate ();
	virtual bool clientSendRecv ();
	virtual bool serverSendRecv ();
};

} // namespace CrossClass
#endif // CROSS_POSIX_NETPOINT_H_INCLUDED

