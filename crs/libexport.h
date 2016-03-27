#ifndef CROSS_LIBEXPORT_H
#define CROSS_LIBEXPORT_H 1
// (c) Dec 26, 2010 Oleg N. Peregudov

#include <crs/defsys.h>

#if USE_POSIX_API
#	define LIBCRS_DECL_DLLEXPORT
#	define LIBCRS_DECL_DLLIMPORT
#elif USE_WIN32_API || defined( WIN32 )
#	define LIBCRS_DECL_DLLEXPORT	__declspec( dllexport )
#	define LIBCRS_DECL_DLLIMPORT	__declspec( dllimport )
#else
#	define LIBCRS_DECL_DLLEXPORT
#	define LIBCRS_DECL_DLLIMPORT
#endif

#if !defined( CROSS_EXPORT )
#	if defined( COMPILING_CROSS_DLL ) || defined( DLL_EXPORT )
#		define CROSS_EXPORT	LIBCRS_DECL_DLLEXPORT
#	else
#		define CROSS_EXPORT	LIBCRS_DECL_DLLIMPORT
#	endif
#endif

#endif // CROSS_LIBEXPORT_H
