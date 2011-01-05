#ifndef CROSS_DEFSYS_H_INCLUDED
#define CROSS_DEFSYS_H_INCLUDED 1
//	2010-12-17	default integer types
//	2010-12-26	WINVER define for Win32API

#ifdef USE_POSIX_API
#undef USE_POSIX_API
#endif

#ifdef USE_WIN32_API
#undef USE_WIN32_API
#endif

#ifdef USE_CXX0X_API
#undef USE_CXX0X_API
#endif

#if defined( __GNUG__ ) || defined( __GNUC__ )
#	if defined( __MINGW32__ )
#		define USE_WIN32_API 1
#	else
#		define USE_POSIX_API 1
#	endif
#	if defined( __GXX_EXPERIMENTAL_CXX0X__ )
#		define USE_CXX0X_API 1
#	endif
#elif defined( _MSC_VER )
#	define USE_WIN32_API 1
#else
#	define USE_POSIX_API 1
#endif

#if defined( USE_WIN32_API )
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
#endif

#if HAVE_STDINT_H
#	include <stdint.h>
#endif
#if STDC_HEADERS
#	include <stdlib.h>
#	include <stddef.h>
#endif
#if HAVE_INTTYPES_H
#	include <inttypes.h>
#endif

#endif // CROSS_DEFSYS_H_INCLUDED

