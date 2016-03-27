#ifndef CROSS_SOCKET_H_INCLUDED
#define CROSS_SOCKET_H_INCLUDED 1
/*
 *  crs/socket.h
 *  Copyright (c) 2008-2012 Oleg N. Peregudov <o.peregudov@gmail.com>
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
 *	2008-12-22	general socket wrapper
 *	2009-04-23	Win/Posix defines
 *	2010-08-26	simplified implementation
 *	2011-01-03	cSockAddr::setPort member
 *	2012-08-15	new platform specific defines
 */

#include <cstring>
#include <crs/bits/hostsocket.hpp>
#if USE_WIN32_API
#	include <winsock2.h>
	struct WSAStartWrapper
	{
		WSAStartWrapper ()
		{
			WSADATA wsaData;
			WSAStartup (MAKEWORD (2, 2), &wsaData);
		}
		
		~WSAStartWrapper ()
		{
			WSACleanup ();
		}
	};
#elif USE_POSIX_API
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <netdb.h>
#	include <fcntl.h>
#	include <unistd.h>
#	if !defined (INFINITE)
#		define INFINITE static_cast<unsigned long> (-1)
#	endif
#endif

namespace CrossClass
{
#if USE_WIN32_API
	struct win32_closeSocket
	{
		int operator () ( SOCKET s )
		{
			return closesocket (s);
		}
	};
	typedef cHostSocket<SOCKET, int, win32_closeSocket> cHostSocketType;
#elif USE_POSIX_API
	struct posix_closeSocket
	{
		int operator () ( int s )
		{
			return close (s);
		}
	};
	typedef cHostSocket<int, socklen_t, posix_closeSocket> cHostSocketType;
#endif
	
	class cTimeVal : public timeval
	{
	public:
		cTimeVal ( const long nSeconds = 0, const long nUSeconds = 0 )
			: timeval( )
		{
			tv_sec = nSeconds;
			tv_usec = nUSeconds;
		}
		
		void	milliseconds ( const unsigned long dwMilliseconds )
		{
			tv_sec = dwMilliseconds / 1000L;
			tv_usec = ( dwMilliseconds % 1000L ) * 1000L;
		}
		
		void	microseconds ( const unsigned long dwMicroseconds )
		{
			tv_sec = dwMicroseconds / 1000000L;
			tv_usec = dwMicroseconds % 1000000L;
		}
	};
	
	struct cSockAddr : sockaddr_in
	{
		cSockAddr (const unsigned short int portNo = 0)
			: sockaddr_in ()
		{
			memset (reinterpret_cast<char *> (static_cast<sockaddr_in *> (this)), 0, sizeof (sockaddr_in));
			sin_family = AF_INET;
			sin_port = htons (portNo);
			sin_addr.s_addr = INADDR_ANY;
		}
		
		cSockAddr (const sockaddr_in & sa)
			: sockaddr_in ()
		{
			memcpy (this, &sa, sizeof (sockaddr_in));
		}
		
		cSockAddr (const char * pchIP, const unsigned short int portNo)
			: sockaddr_in ()
		{
			memset (reinterpret_cast<char *> (static_cast<sockaddr_in *> (this)), 0, sizeof (sockaddr_in));
			sin_family = AF_INET;
			sin_port = htons (portNo);
			sin_addr.s_addr = inet_addr (pchIP);
		}
		
		bool createFromName ( const char * pchHOST, const unsigned short int portNo = 0 )
		{
			hostent * pHostEnt = gethostbyname (pchHOST);
			if (pHostEnt)
			{
				unsigned long * pulAddr = reinterpret_cast<unsigned long *> (pHostEnt->h_addr_list[0]);
				sin_family = AF_INET;
				sin_port = htons (portNo);
				sin_addr.s_addr = *pulAddr;
				return true;
			}
			return false;
		}
		
		const cSockAddr & operator = ( const sockaddr_in & sa )
		{
			if (this != &sa)
				memcpy (this, &sa, sizeof (sockaddr_in));
			return *this;
		}
		
		unsigned short int setPort ( const unsigned short int portNo )
		{
			unsigned short int oldPortNo = Port ();
			sin_port = htons (portNo);
			return oldPortNo;
		}
		
		/* Returns the address in dotted-decimal format			*/
		std::string DottedDecimal () const	{ return inet_ntoa (sin_addr); }
		
		/* Returns port and address (even though they're public)	*/
		unsigned short int Port () const	{ return ntohs (sin_port); }
		unsigned long IPAddr () const		{ return ntohl (sin_addr.s_addr); }
		
		operator sockaddr * ()			{ return reinterpret_cast<sockaddr *> (this); }
		operator const sockaddr * () const	{ return reinterpret_cast<const sockaddr *> (this); }
	};
	
	class cSocket : public cHostSocketType
	{
	public:
		typedef cHostSocketType::host_socket_type host_socket_type;
	
	public:
		cSocket ( host_socket_type sock = 0 )
			: cHostSocketType (sock)
		{
		}
		
		cSocket ( int domain, int type, int protocol )
			: cHostSocketType (socket (domain, type, protocol))
		{
		}
		
		~cSocket ()
		{
		}
		
		cSocket ( cSocket & o )
			: cHostSocketType (o)
		{
		}
		
		cSocket & operator = ( cSocket & o )
		{
			cHostSocketType::operator = ( o );
			return *this;
		}
		
		cSocket & operator = ( const host_socket_type sock )
		{
			cHostSocketType::operator = ( sock );
			return *this;
		}
		
		operator host_socket_type ()
		{
			return cHostSocketType::operator host_socket_type ();
		}
		
		operator const host_socket_type () const
		{
			return cHostSocketType::operator const host_socket_type ();
		}
		
		bool accept ( cSocket & newSocket, cSockAddr & addr )
		{
			cHostSocketType::host_socklen_type addrSize = sizeof (addr);
			host_socket_type newPeer = ::accept (_socket, addr, &addrSize);
			if( newPeer == -1 )
				return false;
			newSocket = newPeer;
			return true;
		}
	};
}	/* namespace CrossClass		*/
#endif/* CROSS_SOCKET_H_INCLUDED	*/
