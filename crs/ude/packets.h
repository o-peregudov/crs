#ifndef CROSS_UDE_PACKETS_H
#define CROSS_UDE_PACKETS_H 1
#define UDE_SENDER_IDENTIFICATION 1
/*
 *  crs/ude/packets.h - General Hardware-to-PC interface (UDE information classes)
 *  Copyright (c) 2003-2012 Oleg N. Peregudov <o.peregudov@gmail.com>
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
 *	2003/02/13	first version
 *	2006/05/29	support for __int64 data type
 *	2006/09/04	wrappers
 *	2006/10/19	IFPACKET filter macro
 *	2007/02/06	packet's sender id & header checksum
 *	2007/06/12	new data types
 *	2008/01/21	included in cross library
 *	2008/02/12	id for fast processing packet
 *	2008/04/05	rename UDE_SENDER_IDENTIFICATION macro
 *	2010/12/03	packet number field
 *	2010/12/10	using unsigned long instead of size_t for packet size
 *	2010/12/12	domainStat
 *	2010/12/17	new packet structure (size field first)
 *	2010/12/20	char type for packet contents
 *	2011/01/02	memory leak in cTalkPacket::assign
 *	2011/01/17	integer types
 *	2012/09/13	cPacketHeader::id_check_bits
 */

#include <cstring>
#include <stdexcept>
#include <crs/libexport.h>
#include <crs/handle.h>

namespace ude {

typedef crs_uint16_t	ushort;
typedef crs_uint32_t	ulong;
typedef crs_int64_t	long64;

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
		domainStat		= 0x8004,	// performance measurements & statistics
		domainSDevMan	= 0x8008	// packets for server device manager
	};
	
	enum reserved_recepients {
		//
		// reserved recepients
		//
		rcpError		= 0xEEEE,	// life critical message
		rcpThread		= 0xFEAD	// service threads of device manager
	};
	
	ulong sz;					// packet's contents size
	ushort id;					// packet type
	ushort domain;				// owner\destination location
	ushort recepient;				// owner\destination address
	
	cPacketHeader (
			const ushort pid = 0x0000,
			const ushort dmn = domainCommon,
			const ushort rcp = 0x0000,
			const ulong csz = 0x00F0 )
		: sz( csz )
		, id( pid )
		, domain( dmn )
		, recepient( rcp )
	{}
	
	cPacketHeader ( const cPacketHeader & ph )
		: sz( ph.sz )
		, id( ph.id )
		, domain( ph.domain )
		, recepient( ph.recepient )
	{}
	
	bool	id_check_bits ( const ushort bit_mask ) const
	{
		return ((id & bit_mask) == bit_mask);
	}
};
#pragma pack(pop)

class CROSS_EXPORT cTalkPacket
{
protected:
	ulong size;						// RAW packet size
	char * contents;					// RAW packet's contents
	CrossClass::handleCounter * hcounter;	// handle counter
	
	void	addhandle ();
	long	releasehandle ();
	void	release ();
	void	unbind ();
	void	assign ( const cTalkPacket & );
	void	construct ( const ushort pid, const ulong csz, const ushort dmn, const ushort rcp );
	
	const ulong _byte_range ( const ulong idx ) const
	{
		if( idx >= byteSize() )
			throw std::range_error ( "cTalkPacket::_byte_range" );
		return idx;
	}
	
	const ulong _word_range ( const ulong idx ) const
	{
		if( idx >= wordSize() )
			throw std::range_error ( "cTalkPacket::_word_range" );
		return idx;
	}
	
	const ulong _value_range ( const ulong idx ) const
	{
		if( idx >= valueSize() )
			throw std::range_error ( "cTalkPacket::_value_range" );
		return idx;
	}
	
	const ulong _int64_range ( const ulong idx ) const
	{
		if( idx >= int64Size() )
			throw std::range_error ( "cTalkPacket::_int64_range" );
		return idx;
	}
	
public:
	cTalkPacket ( const ushort pid = 0x0000,
			  const ulong csz = 0x0F0,
			  const ushort dmn = cPacketHeader::domainCommon,
			  const ushort rcp = 0x0000 );
	cTalkPacket ( const unsigned long rawSize, const char * rawContents );
	cTalkPacket ( const cPacketHeader & );
	cTalkPacket ( const cTalkPacket & );
	
	cTalkPacket & operator = ( const cTalkPacket & );
	
	~cTalkPacket ()
	{
		unbind();
	}
	
	operator const void * () const
	{
		return contents;
	}
	
	operator void * ()
	{
		return contents;
	}
	
	const cPacketHeader & header () const
	{
		return *(reinterpret_cast<const cPacketHeader *> (contents));
	}
	
	cPacketHeader & header ()
	{
		return *(reinterpret_cast<cPacketHeader *> (contents));
	}
	
	ulong rawSize () const				{ return size; }
	
	const ulong & byteSize () const		{ return header().sz; }
	const ulong wordSize () const			{ return ( byteSize() / sizeof( ushort ) ); }
	const ulong valueSize () const		{ return ( byteSize() / sizeof( double ) ); }
	const ulong int64Size () const		{ return ( byteSize() / sizeof( long64 ) ); }
	
	char * const byte () const			{ return ( contents + sizeof( cPacketHeader ) ); }
	ushort * const word () const			{ return reinterpret_cast<ushort * const>( byte() ); }
	double * const value () const			{ return reinterpret_cast<double * const>( byte() ); }
	long64 * const int64 () const			{ return reinterpret_cast<long64 * const>( byte() ); }
	
	char * const byte ()				{ return ( contents + sizeof( cPacketHeader ) ); }
	ushort * const word ()				{ return reinterpret_cast<ushort *>( byte() ); }
	double * const value ()				{ return reinterpret_cast<double *>( byte() ); }
	long64 * const int64 ()				{ return reinterpret_cast<long64 *>( byte() ); }
	
	char & byte ( const ulong idx )		{ return byte()[ _byte_range( idx ) ]; }
	ushort & word ( const ulong idx )		{ return word()[ _word_range( idx ) ]; }
	double & value ( const ulong idx )		{ return value()[ _value_range( idx ) ]; }
	long64 & int64 ( const ulong idx )		{ return int64()[ _int64_range( idx ) ]; }
	
	const char & byte ( const ulong idx ) const	{ return byte()[ _byte_range( idx ) ]; }
	const ushort & word ( const ulong idx ) const	{ return word()[ _word_range( idx ) ]; }
	const double & value ( const ulong idx ) const	{ return value()[ _value_range( idx ) ]; }
	const long64 & int64 ( const ulong idx ) const	{ return int64()[ _int64_range( idx ) ]; }
};

} // namespace ude

#define IFPACKET( packet, dmn, rcp, cmd )		\
	if( ( (packet).domain == (dmn) )		\
	 && ( (packet).recepient == (rcp) )		\
	 && ( (packet).word( 0 ) == (cmd) ) )

#endif // CROSS_UDE_PACKETS_H

