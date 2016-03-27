#ifndef CROSS_TCP_PEER_H_INCLUDED
#define CROSS_TCP_PEER_H_INCLUDED 1
/*
 *  crs/tcp_peer.h
 *  Copyright (c) 2012 Oleg N. Peregudov <o.peregudov@gmail.com>
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
 *	2012/08/30	external event loop version for tcp_peer
 */

#include <crs/libexport.h>
#include <crs/socket.h>
#include <crs/eveloop.h>

namespace CrossClass {

class CROSS_EXPORT tcp_peer : public event_descriptor
{
	tcp_peer ();
	tcp_peer ( const tcp_peer & );
	tcp_peer & operator = ( const tcp_peer & );
	
protected:
	cSocket	_socket;
	cSockAddr	_socket_address;
	event_loop	* _event_loop;
	const bool	_server_mode;
	
	tcp_peer ( event_loop * ev_loop, cSocket & socket );
	virtual void set_non_block ();
	
public:
	tcp_peer ( event_loop * ev_loop );
	tcp_peer ( event_loop * ev_loop, int type, int protocol );
	virtual ~tcp_peer ();
	
	virtual void bind_socket ( const cSockAddr & );
	virtual void connect_to_server ( const cSockAddr & );
	virtual tcp_peer * handle_new_connection ( cSocket &, const cSockAddr & );
	
	virtual bool handle_read ();		/* return true if a message was received	*/
	virtual bool handle_write ();		/* return true if a send should be pending */
	
	virtual bool needs_prepare ();	/* needs preprocessing before polling	*/
	virtual bool want2write ();		/* transmission is pending			*/
	virtual bool auto_destroy ();		/* destroy object by 'event_loop' class	*/
	
	virtual crs_fd_t get_descriptor ()
	{
		return static_cast<crs_fd_t> (get_socket ());
	}
	
	void start_server ( const unsigned short int portNo );
	cSocket & get_socket ();
	
#define TCP_PEER_EXPORTED_EXCEPTION( exception_name )			\
	struct CROSS_EXPORT exception_name : std::runtime_error	\
	{										\
		exception_name ( const std::string & what_arg )		\
			: std::runtime_error (what_arg) { }			\
	}
	
	TCP_PEER_EXPORTED_EXCEPTION (socket_allocation_error);
	TCP_PEER_EXPORTED_EXCEPTION (socket_select_error);
	TCP_PEER_EXPORTED_EXCEPTION (socket_options_error);
	TCP_PEER_EXPORTED_EXCEPTION (socket_listen_error);
	TCP_PEER_EXPORTED_EXCEPTION (socket_bind_error);
	TCP_PEER_EXPORTED_EXCEPTION (socket_connect_error);
	TCP_PEER_EXPORTED_EXCEPTION (server_startup_error);
	TCP_PEER_EXPORTED_EXCEPTION (read_crc_error);
	TCP_PEER_EXPORTED_EXCEPTION (read_error);
	TCP_PEER_EXPORTED_EXCEPTION (write_error);
	TCP_PEER_EXPORTED_EXCEPTION (cancel_read);
	TCP_PEER_EXPORTED_EXCEPTION (cancel_write);
	TCP_PEER_EXPORTED_EXCEPTION (end_of_file);
	TCP_PEER_EXPORTED_EXCEPTION (shutdown_error);
	TCP_PEER_EXPORTED_EXCEPTION (pipe_open_error);
	TCP_PEER_EXPORTED_EXCEPTION (pipe_close_error);
#undef TCP_PEER_EXPORTED_EXCEPTION
};

}	/* namespace CrossClass		*/
#endif/* CROSS_TCP_PEER_H_INCLUDED	*/
