#ifndef CROSS_UDE_PACKETS_H
#define CROSS_UDE_PACKETS_H 1
#define UDE_SENDER_IDENTIFICATION 1
//
// General Hardware-to-PC interface ( UDE information classes )
// Copyright (c) Feb 13, 2003 Oleg N. Peregudov
// Support: op@pochta.ru
//
// May 29, 2006 Oleg N. Peregudov
//        - support for __int64 data type
//
// Sep 4, 2006 Oleg N. Peregudov
//        - wrappers
//
// Oct 19, 2006 Oleg N. Peregudov
//        - IFPACKET filter macro
//
// Feb 6, 2007 Oleg N. Peregudov
//        - packet's sender id & header checksum
//
// Jun 12, 2007 Oleg N. Peregudov
//        - new data types
//
// Jan 21, 2008 Oleg N. Peregudov
//        - included in cross library
//
// Feb 12, 2008 Oleg N. Peregudov
//        - id for fast processing packet
//
// Apr 5, 2008 Oleg N. Peregudov
//        - rename UDE_SENDER_IDENTIFICATION macro
//

#include <cstring>
#include <stdexcept>
#include <crs/libexport.h>

namespace ude {

typedef unsigned char	uchar;
typedef unsigned short	ushort;
#if defined( __GNUG__ )
typedef long long		long64;
#elif defined( _MSC_VER )
typedef __int64		long64;
#endif

#pragma pack(push,1)
struct CROSS_EXPORT cPacketHeader
{
	enum reserved_id_bits {
		//
		// reserved bits in id
		//
		idLocal	= 0x8000,		// 1 for local messages
							// 0 for remote messages
		idPart	= 0x4000,		// this packet is only a part of long message
		idFinal	= 0x2000,		// this packet is a final part of long message
		idGet		= 0x1000,		// peek parameter (set by default)
		idLoopBack	= 0x0800,		// packet for device manager
							// address fields contains reply address
		idDTD		= 0x0400,		// device-to-device packet
		idWrapper	= 0x0200,		// packet header contains sender address (wrapper)
		idFast	= 0x0100		// packet for fast processing
	};
	
	enum reserved_domains {
		//
		// reserved domains
		//
		domainCommon	= 0xFFFF,	// for broadband packets
		domainRDevMan	= 0x8001,	// remote device manager
		domainStatus	= 0x8002,	// status messages
		domainPMeasure	= 0x8003	// performance measurements
	};
	
	enum reserved_recepients {
		//
		// reserved recepients
		//
		rcpError		= 0xFFFF	// life critical message
	};
	
	ushort id;					// packet type
	ushort domain;				// owner\destination location
	ushort recepient;				// owner\destination address
#if defined( UDE_SENDER_IDENTIFICATION )
	size_t sender;				// sender id
	ushort crc;					// checksum
	ushort crc2;
#endif
	
	cPacketHeader ( const ushort pid = 0x0000, const ushort dmn = domainCommon, const ushort rcp = 0x0000 )
		: id( pid )
		, domain( dmn )
		, recepient( rcp )
#if defined( UDE_SENDER_IDENTIFICATION )
		, sender( 0 )
		, crc( 0 )
		, crc2( 0 )
#endif
	{}
	
	cPacketHeader ( const cPacketHeader & ph )
		: id( ph.id )
		, domain( ph.domain )
		, recepient( ph.recepient )
#if defined( UDE_SENDER_IDENTIFICATION )
		, sender( ph.sender )
		, crc( ph.crc )
		, crc2( ph.crc2 )
#endif
	{}
};
#pragma pack(pop)

class cTalkPacket : public cPacketHeader
{
	size_t _size;
	
	union {
		uchar * _byte;
		ushort * _word;
		double * _value;
		long64 * _intvalue;
	};
	
	void	_allocate ()
	{
		if( _size > 0 )
		{
			size_t slack ( _size % sizeof( double ) );
			if( slack ) // round contents size if nesseccary
				_size += sizeof( double ) - slack;
			_byte = new uchar [ _size ];
			memset( _byte, 0x00, _size );
		}
	}
	
	void	_release ()
	{
		delete [] _byte;
		_byte = 0;
		_size = 0;
	}
	
	const size_t _byte_range ( const size_t idx ) const
	{
		if( idx >= byteSize() )
			throw std::range_error ( "cTalkPacket::_byte_range" );
		return idx;
	}
	
	const size_t _word_range ( const size_t idx ) const
	{
		if( idx >= wordSize() )
			throw std::range_error ( "cTalkPacket::_word_range" );
		return idx;
	}
	
	const size_t _value_range ( const size_t idx ) const
	{
		if( idx >= valueSize() )
			throw std::range_error ( "cTalkPacket::_value_range" );
		return idx;
	}
	
	const size_t _int64_range ( const size_t idx ) const
	{
		if( idx >= int64Size() )
			throw std::range_error ( "cTalkPacket::_int64_range" );
		return idx;
	}
	
public:
	cTalkPacket ( const ushort pid = 0x0000, const size_t sz = 0x0F0,
			const ushort dmn = domainCommon, const ushort rcp = 0x0000 )
		: cPacketHeader( pid, dmn, rcp )
		, _size( sz )
		, _byte( 0 )
	{
		_allocate();
	}
	
	cTalkPacket ( const cPacketHeader & hdr, const size_t sz = 0x0F0 )
		: cPacketHeader( hdr )
		, _size( sz )
		, _byte( 0 )
	{
		_allocate();
	}
	
	cTalkPacket ( const cTalkPacket & packet )
		: cPacketHeader( packet )
		, _size( packet._size )
		, _byte( 0 )
	{
		if( _size )
		{
			_byte = new uchar [ _size ];
			memcpy( _byte, packet._byte, _size );
		}
	}
	
	cTalkPacket ( const ushort pid, const ushort dmn, const ushort rcp, const cTalkPacket & packet )
		: cPacketHeader( pid|cPacketHeader::idWrapper, dmn, rcp )
		, _size( packet.fullSize() )
		, _byte( 0 )
	{
		_byte = new uchar [ _size ];
		memcpy( _byte, &packet, _size );
	}
	
	cTalkPacket & operator = ( const cTalkPacket & packet )
	{
		if( this != &packet )
		{
			_release();
			if( ( _size = packet._size ) )
			{
				_byte = new uchar [ _size ];
				memcpy( _byte, packet._byte, _size );
			}
			cPacketHeader::operator = ( packet );
		}
		return *this;
	}
	
	~cTalkPacket ()							{ _release(); }
	
	const size_t & byteSize () const				{ return _size; }
	const size_t wordSize () const				{ return ( _size / sizeof( ushort ) ); }
	const size_t valueSize () const				{ return ( _size / sizeof( double ) ); }
	const size_t int64Size () const				{ return ( _size / sizeof( long64 ) ); }
	
	uchar & byte ( const size_t idx )				{ return _byte[ _byte_range( idx ) ]; }
	ushort & word ( const size_t idx )				{ return _word[ _word_range( idx ) ]; }
	double & value ( const size_t idx )				{ return _value[ _value_range( idx ) ]; }
	long64 & int64 ( const size_t idx )				{ return _intvalue[ _int64_range( idx ) ]; }
	
	const uchar & byte ( const size_t idx ) const		{ return _byte[ _byte_range( idx ) ]; }
	const ushort & word ( const size_t idx ) const		{ return _word[ _word_range( idx ) ]; }
	const double & value ( const size_t idx ) const		{ return _value[ _value_range( idx ) ]; }
	const long64 & int64 ( const size_t idx ) const		{ return _intvalue[ _int64_range( idx ) ]; }
	
	double & operator [] ( const size_t idx )			{ return value( idx ); }
	const double & operator [] ( const size_t idx ) const	{ return value( idx ); }
	
	uchar * const byte () const					{ return _byte; }
	ushort * const word () const					{ return _word; }
	double * const value () const					{ return _value; }
	long64 * const int64 () const					{ return _intvalue; }
	
	const cTalkPacket & unwrap () const				{ return *reinterpret_cast<const cTalkPacket*>( _byte ); }
	size_t fullSize () const					{ return _size + sizeof( size_t ) + sizeof( cPacketHeader ); }
};

} // namespace ude

#define IFPACKET( packet, dmn, rcp, cmd )		\
	if( ( (packet).domain == (dmn) )		\
	 && ( (packet).recepient == (rcp) )		\
	 && ( (packet).word( 0 ) == (cmd) ) )

#endif // CROSS_UDE_PACKETS_H

