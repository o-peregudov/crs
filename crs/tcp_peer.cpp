/*
 *  crs/tcp_peer.cpp
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

#if defined (HAVE_CONFIG_H)
#	include "config.h"
#endif

#include <crs/tcp_peer.h>
#include <algorithm>
#include <cstdio>
#include <cerrno>

#define EMSGLENGTH	256
#define MAXCLIENTS	256

namespace CrossClass {

/*
 * members of class tcp_peer
 */
tcp_peer::tcp_peer ( event_loop * ev_loop )
	: _socket (AF_INET, SOCK_STREAM, 0)
	, _socket_address ()
	, _event_loop (ev_loop)
	, _server_mode (true)
{
	if (_socket == -1)
		throw socket_allocation_error ("tcp_peer::tcp_peer");
}

tcp_peer::tcp_peer ( event_loop * ev_loop, int type, int protocol )
	: _socket (type, protocol, 0)
	, _socket_address ()
	, _event_loop (ev_loop)
	, _server_mode (true)
{
	if (_socket == -1)
		throw socket_allocation_error ("tcp_peer::tcp_peer");
}

tcp_peer::tcp_peer ( event_loop * ev_loop, cSocket & client_socket )
	: _socket (client_socket)
	, _socket_address ()
	, _event_loop (ev_loop)
	, _server_mode (false)
{
}

tcp_peer::~tcp_peer ()
{
}

void tcp_peer::set_non_block ()
{
}

void tcp_peer::bind_socket ( const cSockAddr & sa )
{
	if (_server_mode)
	{
		set_non_block ();
		
		_socket_address = sa;
		
		if (bind (_socket, sa, sizeof (sa)) == -1)
		{
			char msgText [ EMSGLENGTH ];
			sprintf (msgText, "%d: %s", errno, strerror (errno));
			throw socket_bind_error (msgText);
		}
		
		if (listen (_socket, 16) == -1)
		{
			char msgText [ EMSGLENGTH ];
			sprintf (msgText, "%d: %s", errno, strerror (errno));
			throw socket_listen_error (msgText);
		}
	}
	else
		throw std::logic_error ("must be in a server mode");
}

void tcp_peer::start_server ( const unsigned short int portNo )
{
	cSockAddr addrAny ( portNo );
	
	/* start listening for a new connections	*/
	bind_socket (addrAny);
	
	/* add listening socket to the event pool	*/
	_event_loop->add (*this);
}

void tcp_peer::connect_to_server ( const cSockAddr & sa )
{
	if (_server_mode)
		throw std::logic_error ("must be in a client mode");
	
	if (connect (_socket, sa, sizeof (sa)) == -1)
	{
		char msgText [ EMSGLENGTH ];
		sprintf (msgText, "%d: %s", errno, strerror (errno));
		throw socket_connect_error (msgText);
	}
	else
		set_non_block ();
}

cSocket & tcp_peer::get_socket ()
{
	return _socket;
}

bool tcp_peer::needs_prepare ()
{
	return !_server_mode;
}

bool tcp_peer::want2write ()
{
	return !_server_mode;
}

bool tcp_peer::auto_destroy ()
{
	return !_server_mode;
}

bool tcp_peer::handle_read ()
{
	if (_server_mode)
	{
		cSocket new_peer;
		cSockAddr new_peer_addr;
		if (_socket.accept (new_peer, new_peer_addr))
		{
			tcp_peer * new_client = handle_new_connection (new_peer, new_peer_addr);
			_event_loop->add (*new_client);
			return true;
		}
	}
	return false;
}

tcp_peer * tcp_peer::handle_new_connection ( cSocket & socket, const cSockAddr & socket_addr )
{
	return new tcp_peer (_event_loop, socket);
}

bool tcp_peer::handle_write ()
{
	return false;
}

}	/* namespace CrossClass */
