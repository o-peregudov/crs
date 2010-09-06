#ifndef CROSS_SC_SERIALBYTE_H_INCLUDED
#define CROSS_SC_SERIALBYTE_H_INCLUDED 1
//////////////////////////////////////////////////////////////////////
//
// serialbyte.h:	template class for serial interfaces
//			whos protocol based on packets
// (c)Apr 25, 2010 Oleg N. Peregudov
//	Jun 28, 2010 - c++0x mutexes, condition_variables & timers
//	Sep 4, 2010 - our mutexes & condition variables
//
//////////////////////////////////////////////////////////////////////

#include <crs/condition_variable.h>
#include <crs/sc/rs232port.h>
#include <sstream>
#include <iomanip>
#include <list>

namespace sc {

//
// template class serialByte
//
template <class packetType>
class serialByte : public rs232port
{
public:
	struct syncTimeOut : rs232port::errTimeout {
		syncTimeOut ( const std::string & wh ) : rs232port::errTimeout( wh ) { }
	};
	
	struct crcError : std::runtime_error {
		crcError ( const std::string & wh ) : std::runtime_error( wh ) { }
	};
	
	struct badAddress : std::runtime_error {
		badAddress ( const std::string & wh ) : std::runtime_error( wh ) { }
	};
	
	struct badCommand : std::runtime_error {
		badCommand ( const std::string & wh ) : std::runtime_error( wh ) { }
	};
	
	struct controllerOverflow : std::runtime_error {
		controllerOverflow ( const std::string & wh ) : std::runtime_error( wh ) { }
	};
	
	typedef packetType hostPacketType;
	
#pragma pack(push, 1)
	struct comPacket
	{
		union {
			packetType data;
			unsigned char byteArray [ sizeof( packetType ) ];
		};
		
		unsigned char buildCRC ( ) {
			// calculate CRC sum
			byteArray[ sizeof( packetType ) - 1 ] = 0;
			for( int i = 0; i < sizeof( packetType ) - 1;
				byteArray[ sizeof( packetType ) - 1 ] -= byteArray[ i++ ] );
			return byteArray[ sizeof( packetType ) - 1 ];
		}
		
		bool  checkIdentity ( ) const {
			// return true if the given CRC sum is correct
			unsigned char newCRC = 0;
			for( int i = 0; i < sizeof( packetType ) - 1;
				newCRC -= byteArray[ i++ ] );
			return ( newCRC == byteArray[ sizeof( packetType ) - 1 ] );
		}
		
		bool  syncPacket ( ) const {
			for( int i = 0; i < sizeof( packetType ); ++i )
				if( byteArray[ i ] != 0x55 )
					return false;
			return true;
		}
		
		std::string byteString () const;
		
		void  setZero () {
			memset( byteArray, 0x00, sizeof( packetType ) );
		}
		
		comPacket & operator = ( const comPacket & o ) {
			if( &o != this )
				memcpy( byteArray, o.byteArray, sizeof( packetType ) );
			return *this;
		}
	};
#pragma pack(pop)
	
protected:
	CrossClass::cConditionVariable	_notifySyncPacket,
							_notifyComSection;
	CrossClass::LockType	_mutexSyncPacket,
					_mutexComSection,
					_mutexInbox;
	comPacket			_incomingPacket,
					_outgoingPacket;
	std::list<comPacket>	_inbox;
	bool				_waitComSection,
					_waitSyncPacket;
	
	virtual size_t compileResponse ( const char * lpBuf, const size_t dwAvailable );
	virtual void checkPacket ( const comPacket & );
	virtual bool tracePacket ( const comPacket & );
	
	bool	isComSectionPacket ( comPacket & packet );
	bool	isSyncPacket ( comPacket & packet );
	
	void	resetWaitComSection ()
	{
		CrossClass::_LockIt lockComSection ( _mutexComSection );
		_waitComSection = false;
	}
	
	struct directPredicate
	{
		bool * _flag;
		directPredicate ( bool * flag ) : _flag( flag ) { }
		bool operator () () { return *_flag; }
	};
	
	struct inversePredicate
	{
		bool * _flag;
		inversePredicate ( bool * flag ) : _flag( flag ) { }
		bool operator () () { return !(*_flag); }
	};
	
public:
	virtual bool synchronize ( const unsigned long msTimeOut = 1500 );
	
	serialByte ( const size_t inBufSize = 2048 );
	virtual ~serialByte();
	
	//
	// handling of asynchronous packets
	//
	void pop ();
	bool peekPacket ( comPacket & );
	
	//
	// handling of synchronous packets
	//
	void send ( const comPacket & packet );
	
	//
	// returns false, if timed out
	//
	bool comSection ( const comPacket & outPacket, comPacket & inPacket, const unsigned long msTimeOut = 1500 );
};

template <class packetType>
std::string serialByte<packetType>::comPacket::byteString () const
{
	std::basic_ostringstream<char> stream;
	stream.setf( std::ios_base::uppercase );
	stream.fill( '0' );
	for( size_t i = 0; i < sizeof( packetType ); ++i )
	{
		if( i )
			stream << ' ';
		stream << std::setw( 2 ) << std::hex << static_cast<int>( byteArray[ i++ ] );
		stream << ' ' << std::setw( 2 ) << std::hex << static_cast<int>( byteArray[ i ] );
	}
	return stream.str();
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

template <class packetType>
serialByte<packetType>::serialByte( const size_t inBufSize )
	: rs232port( inBufSize )
	, _notifySyncPacket( )
	, _notifyComSection( )
	, _mutexSyncPacket( )
	, _mutexComSection( )
	, _mutexInbox( )
	, _incomingPacket( )
	, _outgoingPacket( )
	, _inbox( )
	, _waitComSection( false )
	, _waitSyncPacket( false )
{
}

template <class packetType>
serialByte<packetType>::~serialByte()
{
}

template <class packetType>
void serialByte<packetType>::pop ()
{
	CrossClass::_LockIt exclusive_access ( _mutexInbox );
	if( !_inbox.empty() )
		_inbox.pop_front();
}

template <class packetType>
bool serialByte<packetType>::peekPacket ( comPacket & packet )
{
	CrossClass::_LockIt exclusive_access ( _mutexInbox );
	if( _inbox.empty() )
		return false;
	memcpy( packet.byteArray, _inbox.front().byteArray, sizeof( packetType ) );
	return true;
}

template <class packetType>
bool serialByte<packetType>::tracePacket ( const comPacket & inPacket )
{
	// check if inPacket is a synchronous reply for the command being posted
	unsigned char TmpByte ( (inPacket.byteArray[ 0 ] << 4)|(inPacket.byteArray[ 0 ] >> 4) );
	if( _outgoingPacket.byteArray[ 0 ] != TmpByte )
		return false;						// this is an asynchronous packet
	else if( inPacket.byteArray[ 1 ] != _outgoingPacket.byteArray[ 1 ] )
		return ( inPacket.byteArray[ 1 ] > 0xAC );	// treat error code as a synchronous reply
										// known error codes starts from 0xAD
	else
		return true;						// this is a synchronous reply
}

template <class packetType>
void serialByte<packetType>::checkPacket ( const comPacket & inPacket )
{
	if( inPacket.checkIdentity() )
	{
		if( inPacket.byteArray[ 1 ] != _outgoingPacket.byteArray[ 1 ] )
		{
			std::string msgText = "Request: ";
			msgText += _outgoingPacket.byteString();
			switch( inPacket.byteArray[ 1 ] )
			{
			case  0xF0:
				throw crcError( ( msgText += "; controller code 0xF0" ) );
			
			case  0xF1:
				throw badAddress( ( msgText += "; controller code 0xF1" ) );
			
			case  0xF2:
				throw badCommand( ( msgText += "; controller code 0xF2" ) );
			
			case  0xF3:
				throw controllerOverflow( ( msgText += "; controller code 0xF3" ) );
			}
		}
	}
	else
		throw crcError( inPacket.byteString() );
}

template <class packetType>
bool serialByte<packetType>::comSection ( const comPacket & outPacket, comPacket & inPacket, const unsigned long msTimeOut )
{
	send( outPacket );
	CrossClass::_LockIt lockComSection ( _mutexComSection );
	if( _notifyComSection.wait_for( lockComSection, msTimeOut, inversePredicate( &_waitComSection ) ) )
	{
		checkPacket( ( inPacket = _incomingPacket ) );
		return true;
	}
	else
		return false;
}

template <class packetType>
void serialByte<packetType>::send ( const comPacket & outPacket )
{
	CrossClass::_LockIt lockComSection ( _mutexComSection );
	write( reinterpret_cast<const char *>( outPacket.byteArray ), sizeof( packetType ) );
	_outgoingPacket = outPacket;
	_waitComSection = true;
}

template <class packetType>
bool serialByte<packetType>::isComSectionPacket ( comPacket & packet )
{
	CrossClass::_LockIt lockComSection ( _mutexComSection );
	if( _waitComSection && tracePacket( packet ) )
	{
		_incomingPacket = packet;
		_waitComSection = false;
		_notifyComSection.notify_one();
		return true;
	}
	return false;
}

template <class packetType>
bool serialByte<packetType>::isSyncPacket ( comPacket & packet )
{
	CrossClass::_LockIt lockSyncPacket ( _mutexSyncPacket );
	if( _waitSyncPacket && packet.syncPacket() )
	{
		/* a synchronization packet which consists of 'U' chars */
		_waitSyncPacket = false;
		_notifySyncPacket.notify_one();
		return true;
	}
	else
		return false;
}

template <class packetType>
size_t serialByte<packetType>::compileResponse ( const char * lpBuf, const size_t dwAvailable )
{
	comPacket packet;
	size_t nBytesProcessed = 0;
	while( ( dwAvailable - nBytesProcessed ) >= sizeof( packetType ) )
	{
		memcpy( packet.byteArray, lpBuf + nBytesProcessed, sizeof( packetType ) );
		nBytesProcessed += sizeof( packetType );
		if( isComSectionPacket( packet ) || isSyncPacket( packet ) )
			continue;
		else
		{
			CrossClass::_LockIt lockInbox ( _mutexInbox );
			_inbox.push_back( packet );
			lockInbox.unlock();
			incomingData();
		}
	}
	return nBytesProcessed;
}

template <class packetType>
bool serialByte<packetType>::synchronize ( const unsigned long msTimeOut )
{
	unsigned long msBitTimeOut = msTimeOut / 15;
	if( msBitTimeOut < 10 )
		msBitTimeOut = 10;
	
	resetWaitComSection();
	inversePredicate pred ( &_waitSyncPacket );
	CrossClass::_LockIt lockSyncPacket ( _mutexSyncPacket );
	_waitSyncPacket = true;
	for( int i = 0; i < 15; ++i )
	{
		write( "U", 1 );
		if( _notifySyncPacket.wait_for( lockSyncPacket, msBitTimeOut, pred ) )
			return true;
	}
	return false;
}

} // namespace sc
#endif // CROSS_SC_SERIALBYTE_H_INCLUDED
