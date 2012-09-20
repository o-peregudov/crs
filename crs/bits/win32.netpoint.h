#ifndef CROSS_WIN32_NETPOINT_H_INCLUDED
#define CROSS_WIN32_NETPOINT_H_INCLUDED 1
/*
 *  crs/bits/win32.netpoint.h
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
 *	2009/01/28
 *	2009/04/23	Win/Posix defines
 *	2010/08/24	new server termination algorithm based on events
 *	2010/12/01	new name for termination method
 *			new implementation
 *	2010/12/10	observer for transmission flag
 *	2010/12/12	clientSendRecv now checks for wait2transmit
 *	2011/01/03	bug fixed in the clients' loop in serverSendRecv
 */

#include <crs/libexport.h>
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

}	/* namespace CrossClass			*/
#endif/* CROSS_WIN32_NETPOINT_H_INCLUDED	*/
