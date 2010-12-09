#ifndef CROSS_SOCKET_H_INCLUDED
#define CROSS_SOCKET_H_INCLUDED 1
// (c) Dec 22, 2008 Oleg N. Peregudov
// Apr 23, 2009 - Win/Posix defines
// Aug 26, 2010 - simplified implementation

#include <crs/bits/hostsocket.hpp>
#if defined( USE_WIN32_API )
#	include <winsock2.h>
struct WSAStartWrapper
{
	WSAStartWrapper ()
	{
		WSADATA wsaData;
		WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
	}
	
	~WSAStartWrapper ()
	{
		WSACleanup();
	}
};
#elif defined( USE_POSIX_API )
#	include <unistd.h>
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <netdb.h>
#	include <fcntl.h>
#	if !defined( INFINITE )
#		define INFINITE static_cast<unsigned long>( -1 )
#	endif
#endif
#include <cstring>

namespace CrossClass
{
#if defined( USE_WIN32_API )
	struct win32closeSocket
	{
		int operator () ( SOCKET s )
		{
			return closesocket( s );
		}
	};
	typedef cHostSocket<SOCKET, int, win32closeSocket> cHostSocketType;
#elif defined( USE_POSIX_API )
	struct posixcloseSocket
	{
		int operator () ( int s )
		{
			return close( s );
		}
	};
	typedef cHostSocket<int, socklen_t, posixcloseSocket> cHostSocketType;
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
	
	struct CROSS_EXPORT cSockAddr : sockaddr_in
	{
		cSockAddr( const unsigned short int portNo = 0 )
			: sockaddr_in( )
		{
			memset( reinterpret_cast<char *>( static_cast<sockaddr_in *>( this ) ), 0, sizeof( sockaddr_in ) );
			sin_family = AF_INET;
			sin_port = htons( portNo );
			sin_addr.s_addr = INADDR_ANY;
		}
		
		cSockAddr( const sockaddr_in & sa )
			: sockaddr_in( )
		{
			memcpy( this, &sa, sizeof( sockaddr_in ) );
		}
		
		cSockAddr( const char * pchIP, const unsigned short int portNo )
			: sockaddr_in( )
		{
			memset( reinterpret_cast<char *>( static_cast<sockaddr_in *>( this ) ), 0, sizeof( sockaddr_in ) );
			sin_family = AF_INET;
			sin_port = htons( portNo );
			sin_addr.s_addr = inet_addr( pchIP );
		}
		
		bool createFromName ( const char * pchHOST, const unsigned short int portNo = 0 )
		{
			hostent * pHostEnt = gethostbyname( pchHOST );
			if( pHostEnt )
			{
				unsigned long * pulAddr = reinterpret_cast<unsigned long *>( pHostEnt->h_addr_list[0] );
				sin_family = AF_INET;
				sin_port = htons( portNo );
				sin_addr.s_addr = *pulAddr;
				return true;
			}
			return false;
		}
		
		const cSockAddr & operator = ( const sockaddr_in & sa )
		{
			if( this != &sa )
				memcpy( this, &sa, sizeof( sockaddr_in ) );
			return *this;
		}
		
		// Return the address in dotted-decimal format
		std::string DottedDecimal () const	{ return inet_ntoa(sin_addr); }
		
		// Get port and address (even though they're public)
		unsigned short int Port () const	{ return ntohs(sin_port); }
		unsigned long IPAddr () const		{ return ntohl(sin_addr.s_addr); }
		
		operator sockaddr * ()			{ return reinterpret_cast<sockaddr *>( this ); }
		operator const sockaddr * () const	{ return reinterpret_cast<const sockaddr *>( this ); }
	};
	
	class CROSS_EXPORT cSocket : public cHostSocketType
	{
	public:
		typedef cHostSocketType::host_socket_type host_socket_type;
	
	public:
		cSocket ( host_socket_type sock = 0 )
			: cHostSocketType( sock )
		{
		}
		
		cSocket ( int domain, int type, int protocol )
			: cHostSocketType( socket( domain, type, protocol ) )
		{
		}
		
		~cSocket ()
		{
		}
		
		cSocket ( cSocket & o )
			: cHostSocketType( o )
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
			cHostSocketType::host_socklen_type addrSize = sizeof( addr );
			host_socket_type newPeer = ::accept( _socket, addr, &addrSize );
			if( newPeer == -1 )
				return false;
			newSocket = newPeer;
			return true;
		}
	
	};
} // namespace CrossClass
#endif // CROSS_SOCKET_H_INCLUDED
