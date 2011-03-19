#ifndef CROSS_DEFSYS_H_INCLUDED
#define CROSS_DEFSYS_H_INCLUDED 1
//	2010-12-17	default integer types
//	2010-12-26	WINVER define for Win32API
//	2011-01-18	special case for MS VC compiler

#define HAVE_WIN32_API 1
#define HAVE_POSIX_API 0
#define HAVE_CPP0X_API 0

#define HAVE_STDINT_H 0
#define HAVE_INTTYPES_H 0
#define HAVE_SYS_TYPES_H 0

#if HAVE_STDINT_H
#	include <stdint.h>
#endif
#if HAVE_INTTYPES_H
#	include <inttypes.h>
#endif
#if HAVE_SYS_TYPES_H
#	include <sys/types.h>
#endif
#if STDC_HEADERS
#	include <stdlib.h>
#	include <stddef.h>
#endif

#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#	pragma warning( disable : 4996 )
#	define USE_WIN32_API 1
#	ifndef WINVER				// Allow use of features specific to Windows XP or later.
#		define WINVER 0x0502		// Change this to the appropriate value to target other versions of Windows.
#	endif
#	ifndef _WIN32_WINNT			// Allow use of features specific to Windows XP or later.
#		define _WIN32_WINNT 0x0502	// Change this to the appropriate value to target other versions of Windows.
#	endif
#	ifndef _WIN32_WINDOWS			// Allow use of features specific to Windows 98 or later.
#		define _WIN32_WINDOWS 0x0410	// Change this to the appropriate value to target Windows Me or later.
#	endif
#	ifndef _WIN32_IE				// Allow use of features specific to IE 6.0 or later.
#		define _WIN32_IE 0x0600		// Change this to the appropriate value to target other versions of IE.
#	endif
#	include <winsock2.h>
	
	typedef CHAR crs_int8_t;
	typedef SHORT crs_int16_t;
	typedef INT32 crs_int32_t;
	typedef INT64 crs_int64_t;
	
	typedef BYTE crs_uint8_t;
	typedef WORD crs_uint16_t;
	typedef DWORD crs_uint32_t;
#endif

#endif // CROSS_DEFSYS_H_INCLUDED
