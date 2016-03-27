#ifndef CROSS_SC_I41SERIAL_H_INCLUDED
#define CROSS_SC_I41SERIAL_H_INCLUDED 1
//////////////////////////////////////////////////////////////////////
//
// i41serial.h: interface for the i41serial class.
//	2005/10/14
//	2007/06/18	uniform locks
//	2007/11/21	new place for cross-compiling routines
//	2011/12/03	portable integer types
//
//////////////////////////////////////////////////////////////////////

#include <crs/sc/serialbyte.h>

namespace sc {
	#pragma pack(push, 1)
	struct eightBytePacket
	{
		crs_uint8_t	 id,	// packet identification and controller code
				 cmd,	// command code
				 ext;	// command code extension
		crs_uint32_t param;
		crs_uint8_t	 crc;
	};
	#pragma pack(pop)
	
	typedef serialByte<eightBytePacket> i41serial;
};

#endif // CROSS_SC_I41SERIAL_H_INCLUDED

