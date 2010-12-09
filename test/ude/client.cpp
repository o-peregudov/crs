#include <crs/ude/client.h>

#include <crs/math/unimath.h>
#include <crs/timer.h>
#include <limits>

#include <csignal>
#include <iostream>
#include <iomanip>

class cdm : public ude::client_device_manager
{
protected:
	virtual bool checkTerminate ()
	{
		bool result = ude::client_device_manager::checkTerminate();
		if( result )
			std::cout << "Got terminate signal!" << std::endl;
		return result;
	}
	
public:
	cdm ()
		: ude::client_device_manager( )
	{
	}
	
	virtual ~cdm ()
	{
	}
};

cdm * netClient;

void	terminate ( int signal )
{
	netClient->postTerminate();
}

class pingTool : public ude::basic_device
{
protected:
	CrossClass::cTimer			_timer;
	ude::cTalkPacket				_packet;
	CrossClass::cMutex			_mutex;
	CrossClass::cConditionVariable	_notify;
	bool						_flag;
	
	virtual int take ( const ude::cTalkPacket & );
	
	struct waitpred
	{
		bool * flag;
		waitpred( bool * f ) : flag ( f ) { }
		bool operator () () { return !(*flag); }
	};
	
public:
	pingTool ( ude::basic_device_manager * bdm )
		: ude::basic_device( bdm )
		, _timer( )
		, _packet( 0, 2 * sizeof( double ), ude::cTalkPacket::domainPMeasure, 0 )
		, _mutex( )
		, _notify( )
		, _flag( false )
	{
		_packet[ 0 ] = _timer.getStamp();
		if( _device_manager )
			_device_manager->register_device( ude::cTalkPacket::domainPMeasure, this );
	}
	
	virtual ~pingTool ()
	{
		if( _device_manager )
			_device_manager->unregister_device( ude::cTalkPacket::domainPMeasure, this );
	}
	
	virtual bool poolControlled () const
	{
		return false;
	}
	
	virtual void reset ()
	{
		CrossClass::_LockIt _lock ( _mutex );
		_packet[ 0 ] = _timer.getStamp();
		_flag = false;
	}
	
	virtual bool invariant ()
	{
		return true;
	}
	
	double ping ( );	// non blocking version, returns -1 while waiting
	double ping ( const unsigned long msTimeOut );
};

int pingTool::take ( const ude::cTalkPacket & inPacket )
{
	CrossClass::_LockIt _lock ( _mutex );
	if( _flag )
	{
		if(	( inPacket.domain == _packet.domain )
			&& ( inPacket.recepient == _packet.recepient )
			&& UniMath::isZero( inPacket[ 0 ] - _packet[ 0 ] ) )
		{
			_flag = false;
			_packet[ 1 ] = _timer.getStamp() - inPacket[ 1 ];
			_notify.notify_one();
			return 2;
		}
	}
	return 0;	// not my packet
}

double pingTool::ping ()
{
	CrossClass::_LockIt _lock ( _mutex );
	if( _flag )
		return -1.0;
	else
	{
		if( UniMath::isZero( _packet[ 1 ] ) )
		{
			_packet[ 1 ] = _timer.getStamp();
			_device_manager->register_request( this, _packet );
			_flag = true;
			return -1.0;
		}
		else
		{
			double delta = _packet[ 1 ];
			_packet[ 1 ] = 0.0;
			return delta;
		}
	}
}

double pingTool::ping ( const unsigned long msTimeOut )
{
	CrossClass::_LockIt _lock ( _mutex );
	if( _flag )
		return -1.0;
	else
	{
		_packet.value( 1 ) = _timer.getStamp();
		_device_manager->register_request( this, _packet );
		_flag = true;
		if( msTimeOut == static_cast<const unsigned long>( -1 ) )
		{
			_notify.wait( _lock );
			double delta = _packet[ 1 ];
			_packet[ 1 ] = 0.0;
			_flag = false;
			return delta;
		}
		else if( _notify.wait_for( _lock, msTimeOut, waitpred( &_flag ) ) )
		{
			double delta = _packet[ 1 ];
			_packet[ 1 ] = 0.0;
			_flag = false;
			return delta;
		}
		else
			return std::numeric_limits<double>::infinity();
	}
}

int	main ()
{
#if defined( USE_WIN32_API )
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD( 2, 2 );
	int err = WSAStartup( wVersionRequested, &wsaData );
	if( err != 0 ) {
		std::cout << "Could not find a usable WinSock DLL" << std::endl;
		return 1;
	}
#endif
	
	char a;
	std::cout << "Ready ? ";
	std::cin >> a;
	
	cdm client;
	CrossClass::cSockAddr sa ( "127.0.0.1", 19802 );
	client.clientConnect( sa );
	
	pingTool ptool ( &client );
	
	//
	// set Ctrl+C signal handler
	//
	std::cout << "Press Ctrl+C to terminate ... " << std::endl;
	netClient = &client;
	signal( SIGINT, terminate );
	
	double rtDelay = 0;
	while( client.clientSendRecv() )
	{
		rtDelay = ptool.ping();
		if( rtDelay > -1.0 )
		{
			std::cout << "Round trip: " << std::setiosflags( std::ios_base::fixed ) << std::setprecision( 6 ) << rtDelay << std::endl;
#if defined( USE_WIN32_API )
			Sleep( 500 );
#else
			sleep( 1 );
#endif
		}
		
		client.proceed_response( );
		client.proceed_request( 1 );
	}

#if defined( USE_WIN32_API )
	WSACleanup();
#endif
	return 0;
}

