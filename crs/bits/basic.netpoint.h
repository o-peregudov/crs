#ifndef CROSS_BASICNETPOINT_H_INCLUDED
#define CROSS_BASICNETPOINT_H_INCLUDED 1
/*
 *  crs/bits/basic.netpoint.h
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
 *	2009-01-28	basic netpoint
 *	2009-04-23	Win/Posix defines
 *	2010-08-24	new server termination algorithm
 *	2010-11-30	new name for termination method
 *			new implementation
 *	2010-12-04	stored socket address
 *	2010-12-09	postRestart member
 *			observer for transmission flag
 *	2010-12-19	using std::vector as a clients list container
 *	2011-01-03	unused member _nClientList was removed
 *	2012-08-15	new platform specific defines
 */

#include <crs/socket.h>
#include <vector>

#define NETPOINT_EXPORTED_EXCEPTION( exception_name )			\
	struct CROSS_EXPORT exception_name : std::runtime_error	\
	{										\
		exception_name ( const std::string & what_arg )		\
			: std::runtime_error (what_arg) { }			\
	}

namespace CrossClass {

class CROSS_EXPORT basicNetPoint
{
protected:
	typedef std::vector<basicNetPoint*> clListType;
	
	cSocket	_socket;
	cSockAddr	_sockAddress;
	clListType	_clientList;
	
	basicNetPoint ( cSocket & );
	
	void addClient ( basicNetPoint * );
	void removeClient ( const size_t );
	void clientReceive ( const size_t );
	void clientTransmit ( const size_t );
	void disconnectAll ();
	
protected:
	virtual void setNonBlock ();
	
	virtual void transmit ();
	virtual void receive ();
	
	virtual basicNetPoint * handleNewConnection ( cSocket &, const cSockAddr & );
	virtual size_t enumerateDescriptors ();
	virtual void buildSelectList ();
	virtual bool handleDisconnect ( basicNetPoint * );
	virtual bool checkTerminate ();
	
public:
	NETPOINT_EXPORTED_EXCEPTION (socket_allocation_error);
	NETPOINT_EXPORTED_EXCEPTION (socket_select_error);
	NETPOINT_EXPORTED_EXCEPTION (socket_options_error);
	NETPOINT_EXPORTED_EXCEPTION (socket_listen_error);
	NETPOINT_EXPORTED_EXCEPTION (socket_bind_error);
	NETPOINT_EXPORTED_EXCEPTION (socket_connect_error);
	NETPOINT_EXPORTED_EXCEPTION (server_startup_error);
	NETPOINT_EXPORTED_EXCEPTION (read_crc_error);
	NETPOINT_EXPORTED_EXCEPTION (read_error);
	NETPOINT_EXPORTED_EXCEPTION (write_error);
	NETPOINT_EXPORTED_EXCEPTION (cancel_read);
	NETPOINT_EXPORTED_EXCEPTION (cancel_write);
	NETPOINT_EXPORTED_EXCEPTION (end_of_file);
	NETPOINT_EXPORTED_EXCEPTION (shutdown_error);
	NETPOINT_EXPORTED_EXCEPTION (pipe_open_error);
	NETPOINT_EXPORTED_EXCEPTION (pipe_close_error);
	
	basicNetPoint ();
	virtual ~basicNetPoint ();
	
	void startServer ( const unsigned short int portNo );
	virtual void bindSocket ( const cSockAddr & );
	virtual void clientConnect ( const cSockAddr & );
	
	virtual void postRestart ();
	virtual void postTerminate ();
	virtual bool clientSendRecv ();
	virtual bool serverSendRecv ();
	
	virtual bool want2transmit ();
	
	cSocket & getSocket ();
};

}	/* namespace CrossClass			*/
#undef NETPOINT_EXPORTED_EXCEPTION
#endif/* CROSS_BASICNETPOINT_H_INCLUDED	*/
