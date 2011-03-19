//
// General Hardware-to-PC interface ( UDE information classes )
// Copyright (c) 2003-2010 Oleg N. Peregudov
// Support: op@pochta.ru
//	12/17/2010	new packet structure (size field first)
//	12/20/2010	char type for packet contents
//	01/02/2011	memory leak in cTalkPacket::assign
//	01/03/2011	integer types
//

#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#endif

#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif

#include <cstring>
#include <crs/ude/packets.h>

namespace ude {

using namespace CrossClass;

void	cTalkPacket::addhandle ()
{
	_LockIt lock ( hcounter->mutex );
	++(hcounter->nlinks);
}

long	cTalkPacket::releasehandle ()
{
	_LockIt lock ( hcounter->mutex );
	return --(hcounter->nlinks);
}

void	cTalkPacket::release ()
{
	delete [] contents;
	contents = 0;
	size = 0;
}

void	cTalkPacket::unbind ()
{
	if( *this )
	{
		if( releasehandle() == 0 )
		{
			delete hcounter;
			hcounter = 0;
			release ();
		}
	}
}

void	cTalkPacket::assign ( const cTalkPacket & o )
{
	if( contents != o.contents )
	{
		unbind();
		size = o.size;
		contents = o.contents;
		hcounter = o.hcounter;
		addhandle();
	}
}

void	cTalkPacket::construct ( const ushort pid, const ulong csz, const ushort dmn, const ushort rcp )
{
	//
	// allocate raw memory
	//
	size = csz + sizeof( cPacketHeader );
	ulong slack ( csz % sizeof( double ) );
	if( slack ) // round contents size if nesseccary
		size += sizeof( double ) - slack;
	contents = new char [ size ];
	//
	// set up packet header
	//
	header().sz = csz;
	header().id = pid;
	header().domain = dmn;
	header().recepient = rcp;
}

cTalkPacket::cTalkPacket ( const unsigned long rawSize, const char * rawContents )
	: size( 0 )
	, contents( 0 )
	, hcounter( 0 )
{
	if( ( *reinterpret_cast<const ulong *>( rawContents ) + sizeof( cPacketHeader ) ) > rawSize )
		throw std::range_error( "insufficient raw size" );
	else
	{
		size = rawSize;
		contents = new char [ size ];
		memcpy( contents, rawContents, size );
		hcounter = new handleCounter( 1 );
	}
}

cTalkPacket::cTalkPacket ( const ushort pid, const ulong csz, const ushort dmn, const ushort rcp )
	: size( 0 )
	, contents( 0 )
	, hcounter( new handleCounter( 1 ) )
{
	construct( pid, csz, dmn, rcp );
}

cTalkPacket::cTalkPacket ( const cPacketHeader & hdr )
	: size( 0 )
	, contents( 0 )
	, hcounter( new handleCounter( 1 ) )
{
	construct( hdr.id, hdr.sz, hdr.domain, hdr.recepient );
}

cTalkPacket::cTalkPacket ( const cTalkPacket & o )
	: size( 0 )
	, contents( 0 )
	, hcounter( 0 )
{
	assign( o );
}

cTalkPacket & cTalkPacket::operator = ( const cTalkPacket & o )
{
	assign( o );
	return *this;
}

} // namespace ude

