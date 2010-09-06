#ifndef CROSS_HOSTSOCKET_HPP_INCLUDED
#define CROSS_HOSTSOCKET_HPP_INCLUDED 1
// (c) Aug 26, 2010 Oleg N. Peregudov

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
			if( _own )
			{
				SockCloseFunction closeSocket;
				closeSocket( _socket );
				_socket = -1;
				_own = false;
			}
		}
	
	public:
		cHostSocket ( host_socket_type sock = 0 )
			: _socket( sock )
			, _own( true )
		{
			if( _socket == 0 )
				_own = false;
			else if( _socket == -1 )
				throw std::runtime_error( "create socket" );
		}
		
		cHostSocket ( cHostSocket<SockType, SockLenType, SockCloseFunction> & o )
			: _socket( o._socket )
			, _own( o._own )
		{
			o._own = false;
		}
		
		~cHostSocket ()
		{
			doClose();
		}
		
		cHostSocket<SockType, SockLenType, SockCloseFunction> & operator = ( cHostSocket<SockType, SockLenType, SockCloseFunction> & o )
		{
			if( &o != this )
			{
				doClose();
				_socket = o._socket;
				_own = o._own;
				o._own = false;
			}
			return *this;
		}
		
		cHostSocket<SockType, SockLenType, SockCloseFunction> & operator = ( const host_socket_type sock )
		{
			if( _socket != sock )
			{
				doClose();
				_socket = sock;
				_own = true;
			}
			return *this;
		}
		
		operator host_socket_type ()			{ return _socket; }
		operator const host_socket_type () const	{ return _socket; }
	};
} // namespace CrossClass
#endif // CROSS_HOSTSOCKET_HPP_INCLUDED
