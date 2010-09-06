#ifndef CROSS_SC_I41SERIAL_H_INCLUDED
#define CROSS_SC_I41SERIAL_H_INCLUDED 1
//////////////////////////////////////////////////////////////////////
//
// i41serial.h: interface for the i41serial class.
// (c) Oct 14, 2005 Oleg N. Peregudov
// (c) Jun 18, 2007 Oleg N. Peregudov - uniform locks
// (c) Nov 21, 2007 Oleg N. Peregudov - new place for cross-compiling routines
//
//////////////////////////////////////////////////////////////////////

#include <crs/sc/serialbyte.h>

namespace sc {
	#pragma pack(push, 1)
	struct eightBytePacket
	{
		unsigned char id,		// packet identification and controller code
				  cmd,	// command code
				  ext;	// command code extension
		unsigned long param;
		unsigned char crc;
	};
	#pragma pop(push)
	
	typedef serialByte<eightBytePacket> i41serial;
};

#endif // CROSS_SC_I41SERIAL_H_INCLUDED

