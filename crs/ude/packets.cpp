/*
 *  crs/ude/packets.cpp - General Hardware-to-PC interface (UDE information classes)
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

#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif

#include <cstring>
#include <crs/ude/packets.h>

namespace ude {

using namespace CrossClass;

void	cTalkPacket::addhandle ()
{
	cLocker<cSpinLock> lock ( hcounter->spin_lock );
	++(hcounter->nlinks);
}

long	cTalkPacket::releasehandle ()
{
	cLocker<cSpinLock> lock ( hcounter->spin_lock );
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
	if (*this)
	{
		if (releasehandle () == 0)
		{
			delete hcounter;
			hcounter = 0;
			release ();
		}
	}
}

void	cTalkPacket::assign ( const cTalkPacket & o )
{
	if (contents != o.contents)
	{
		unbind ();
		size = o.size;
		contents = o.contents;
		hcounter = o.hcounter;
		addhandle ();
	}
}

void	cTalkPacket::construct ( const ushort pid, const ulong csz, const ushort dmn, const ushort rcp )
{
	/*
	 * allocate raw memory
	 */
	size = csz + sizeof (cPacketHeader);
	ulong slack ( csz % sizeof (double) );
	if (slack) /* round contents size if necessary	*/
		size += sizeof (double) - slack;
	contents = new char [ size ];
	/*
	 * set up packet header
	 */
	header ().sz = csz;
	header ().id = pid;
	header ().domain = dmn;
	header ().recepient = rcp;
}

cTalkPacket::cTalkPacket ( const unsigned long rawSize, const char * rawContents )
	: size (0)
	, contents (0)
	, hcounter (0)
{
	if ((*reinterpret_cast<const ulong *> (rawContents) + sizeof (cPacketHeader)) > rawSize)
		throw std::range_error ("insufficient raw size");
	else
	{
		size = rawSize;
		contents = new char [ size ];
		memcpy (contents, rawContents, size);
		hcounter = new handleCounter (1);
	}
}

cTalkPacket::cTalkPacket ( const ushort pid, const ulong csz, const ushort dmn, const ushort rcp )
	: size (0)
	, contents (0)
	, hcounter (new handleCounter (1))
{
	construct (pid, csz, dmn, rcp);
}

cTalkPacket::cTalkPacket ( const cPacketHeader & hdr )
	: size (0)
	, contents (0)
	, hcounter (new handleCounter (1))
{
	construct (hdr.id, hdr.sz, hdr.domain, hdr.recepient);
}

cTalkPacket::cTalkPacket ( const cTalkPacket & o )
	: size (0)
	, contents (0)
	, hcounter (0)
{
	assign (o);
}

cTalkPacket & cTalkPacket::operator = ( const cTalkPacket & o )
{
	assign (o);
	return *this;
}

}	/* namespace ude	*/
