#ifndef CROSS_SC_TWELVEBYTE_H_INCLUDED
#define CROSS_SC_TWELVEBYTE_H_INCLUDED 1
//////////////////////////////////////////////////////////////////////
//
// twelvebyte.h:	interface for the M306 controller
//			with the 12-byte protocol from K. Melnik
// (c) Apr 25, 2010 Oleg N. Peregudov
//
//////////////////////////////////////////////////////////////////////

#include <crs/sc/serialbyte.h>

namespace sc {
	#pragma pack(push, 1)
	struct twelveBytePacket
	{
		unsigned char id,		// command identifier, usualy shows command destination
						//	0x01 - to controller
						//	0x10 - to pc
				  cmd,	// command code
				  ext;	// command extension
		unsigned long parama,	// command data
				  paramb;	// more command data
		unsigned char crc;	// CRC sum
	};
	#pragma pack(pop)
	
	typedef serialByte<twelveBytePacket> twByte;
};

#endif // CROSS_SC_I41SERIAL_H_INCLUDED
