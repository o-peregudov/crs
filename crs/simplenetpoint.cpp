// (c) Mar 3, 2009 Oleg N. Peregudov
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#endif
#include <crs/simplenetpoint.h>
#include <sstream>
#include <iomanip>

namespace SimpleNetPoint {

message::message ( const unsigned long sz )
	: size( sz )
	, contents( sz ? ( new char [ sz ] ) : static_cast<char *>( 0 ) )
{ }

message::message ( const message & msg )
	: size( msg.size )
	, contents( msg.size ? ( new char [ size ] ) : static_cast<char *>( 0 ) )
{
	memcpy( contents, msg.contents, size );
}

message::message ( const std::string & msg )
	: size( msg.size() + 1 )
	, contents( msg.size() ? ( new char [ size ] ) : static_cast<char *>( 0 ) )
{
	if( size == 1 )
		size = 0;
	else
		msg.copy( contents, size );
}

message::~message ( )
{
	delete [] contents;
}

message & message::operator = ( const message & msg )
{
	if( this != &msg )
	{
		delete [] contents;
		if( ( size = msg.size ) )
		{
			contents = new char [ size ];
			memcpy( contents, msg.contents, size );
		}
		else
			contents = 0;
	}
	return *this;
}

server::server ()
	: CrossClass::netPoint( )
	
	, _inboxLock( )
	, _inbox( )
	, _inMessage( 0 )
	, _inContents( false )
	, _inNextByte( reinterpret_cast<char *>( &_inMessage.size ) )
	, _inBytesRest( sizeof( _inMessage.size ) )
	
	, _newMessageCallBack( 0 )
	, _callBackData( 0 )
	, _callBackLock( )
	
	, _outboxLock( )
	, _outbox( )
	, _outContents( true )
	, _outNextByte( 0 )
	, _outBytesRest( 0 )
{
}

server::server ( CrossClass::cSocket & sckt )
	: CrossClass::netPoint( sckt )
	
	, _inboxLock( )
	, _inbox( )
	, _inMessage( 0 )
	, _inContents( false )
	, _inNextByte( reinterpret_cast<char *>( &_inMessage.size ) )
	, _inBytesRest( sizeof( _inMessage.size ) )
	
	, _newMessageCallBack( 0 )
	, _callBackData( 0 )
	, _callBackLock( )
	
	, _outboxLock( )
	, _outbox( )
	, _outContents( true )
	, _outNextByte( 0 )
	, _outBytesRest( 0 )
{
}

server::~server ()
{
}

void server::sendMessage ( const message & msg )
{
	CrossClass::_LockIt exclusive_access ( _outboxLock );
	_outbox.push_back( msg );
}

bool server::recvMessage ( message & msg )
{
	CrossClass::_LockIt exclusive_access ( _inboxLock );
	if( _inbox.empty() )
		return false;
	else
	{
		msg = _inbox.front();
		_inbox.pop_front();
		return true;
	}
}

void server::transmit ()
{
	if( _outBytesRest == 0 )
	{
		if( _outContents )
		{
			// start transmission of new packet
			CrossClass::_LockIt exclusive_access ( _outboxLock );
			if( _outbox.empty() )
				return;	// there is nothing to transmit!
			
			_outContents = false;
			_outNextByte = reinterpret_cast<char *>( &_outbox.front().size );
			_outBytesRest = sizeof( _inMessage.size );
		}
		else
		{
			// continue packet's transmission
			CrossClass::_LockIt exclusive_access ( _outboxLock );
			_outContents = true;
			_outNextByte = _outbox.front().contents;
			_outBytesRest = _outbox.front().size;
		}
	}
	
	long nBytesProcessed = send( _socket, _outNextByte, _outBytesRest, 0 );
	if( nBytesProcessed == -1 )
	{
		std::basic_ostringstream<char> errMsg;
		errMsg << "transmit (" << errno << ')';
		throw write_error( errMsg.str() );
	}
	
	_outBytesRest -= nBytesProcessed;
	_outNextByte += nBytesProcessed;
	
	if( _outContents && ( _outBytesRest == 0 ) )
	{
		// transmission of packet is done
		// so we can remove packet from the outbox queue
		CrossClass::_LockIt exclusive_access ( _outboxLock );
		_outbox.pop_front();
	}
}

void server::receive ()
{
	long nBytesProcessed = recv( _socket, _inNextByte, _inBytesRest, 0 );
	switch( nBytesProcessed )
	{
	case	-1:	// error state
		{
			std::basic_ostringstream<char> errMsg;
			errMsg << "receive (" << errno << ')';
			throw read_error( errMsg.str() );
		}
	
	case	0:	// end-of-file
		throw end_of_file ( "receive: end-of-file" );
	
	default:
		_inBytesRest -= nBytesProcessed;
		_inNextByte += nBytesProcessed;
		
		if( _inBytesRest == 0 )
		{
			if( _inContents )
			{
				_inContents = false;
				_inNextByte = reinterpret_cast<char *>( &_inMessage.size );
				_inBytesRest = sizeof( _inMessage.size );
				{
					CrossClass::_LockIt exclusive_access ( _inboxLock );
					_inbox.push_back( _inMessage );
				}
				{
					CrossClass::_LockIt exclusive_access ( _callBackLock );
					if( _newMessageCallBack )
						_newMessageCallBack( _callBackData );
				}
			}
			else
			{
				delete [] _inMessage.contents;
				_inMessage.contents = 0;
				if( _inMessage.size )
				{
					_inContents = true;
					_inMessage.contents = new char [ _inMessage.size ];
					_inBytesRest = _inMessage.size;
					_inNextByte = _inMessage.contents;
				}
			}
		}
	}
}

CrossClass::cHandle<CrossClass::basicNetPoint>
server::handleNewConnection ( CrossClass::cSocket & sckt, const CrossClass::cSockAddr & sa )
{
	return new server ( sckt );
}

void	server::setAsyncDataCallback ( asyncDataCallBackFunction func, void * pData )
{
	CrossClass::_LockIt exclusive_access ( _callBackLock );
	_newMessageCallBack = func;
	_callBackData = pData;
}

} // namespace SimpleNetPoint

