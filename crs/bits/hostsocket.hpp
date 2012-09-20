#ifndef CROSS_HOSTSOCKET_HPP_INCLUDED
#define CROSS_HOSTSOCKET_HPP_INCLUDED 1
/*
 *  crs/bits/hostsocket.hpp
 *  Copyright (c) 2010-2012 Oleg N. Peregudov <o.peregudov@gmail.com>
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
 *	2010-08-26	general socket handling template
 *	2012-08-15	new platform specific defines
 */

#include <crs/libexport.h>
#include <stdexcept>

namespace CrossClass
{
	template <class SockType, class SockLenType, class SockCloseFunction> class cHostSocket
	{
	public:
		typedef SockType		host_socket_type;
		typedef SockLenType	host_socklen_type;
	
	protected:
		host_socket_type	_socket;
		bool			_own;
		
		void	doClose ()
		{
			if (_own)
			{
				SockCloseFunction closeSocket;
				closeSocket (_socket);
				_socket = -1;
				_own = false;
			}
		}
	
	public:
		cHostSocket ( host_socket_type sock = 0 )
			: _socket (sock)
			, _own (true)
		{
			if (_socket == 0)
				_own = false;
			else if (_socket == -1)
				throw std::runtime_error ("create socket");
		}
		
		cHostSocket ( cHostSocket<SockType, SockLenType, SockCloseFunction> & o )
			: _socket (o._socket)
			, _own (o._own)
		{
			o._own = false;
		}
		
		~cHostSocket ()
		{
			doClose ();
		}
		
		cHostSocket<SockType, SockLenType, SockCloseFunction> & operator = ( cHostSocket<SockType, SockLenType, SockCloseFunction> & o )
		{
			if (this != &o)
			{
				doClose ();
				_socket = o._socket;
				_own = o._own;
				o._own = false;
			}
			return *this;
		}
		
		cHostSocket<SockType, SockLenType, SockCloseFunction> & operator = ( const host_socket_type sock )
		{
			if (_socket != sock)
			{
				doClose ();
				_socket = sock;
				_own = true;
			}
			return *this;
		}
		
		operator host_socket_type ()			{ return _socket; }
		operator const host_socket_type () const	{ return _socket; }
	};
}	/* namespace CrossClass			*/
#endif/* CROSS_HOSTSOCKET_HPP_INCLUDED	*/
