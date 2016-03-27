#ifndef CROSS_POSIX_NETPOINT_H_INCLUDED
#define CROSS_POSIX_NETPOINT_H_INCLUDED 1
/*
 *  crs/bits/posix.netpoint.h
 *  Copyright (c) 2009-2012 Oleg N. Peregudov <o.peregudov@gmail.com>
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
 *	2009-01-28
 *	2009-04-23	Win/Posix defines
 *	2010-08-26	new server termination algorithm based on pipes
 *	2010-11-30	usage of the poll system call
 *	2010-12-04	new pipe creation concept
 *			checkTerminate bug fixed (pipe close)
 *	2010-12-05	buildClientList bug fixed
 *			extended error info
 *	2010-12-09	postRestart member
 *			observer for transmission flag
 *	2010-08-26	new server termination algorithm based on pipes
 *	2010-12-09	postRestart member
 *	2010-12-12	postRestart status flag to avoid request stacking
 *			clientSendRecv now checks for wait2transmit
 */

#include <crs/bits/basic.netpoint.h>
#include <crs/security.h>
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
	
	cMutex	postRestartMutex;
	bool		postRestartFlag;
	
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
	virtual void postRestart ();
	virtual void postTerminate ();
	virtual bool clientSendRecv ();
	virtual bool serverSendRecv ();
};

}	/* namespace CrossClass			*/
#endif/* CROSS_POSIX_NETPOINT_H_INCLUDED	*/
