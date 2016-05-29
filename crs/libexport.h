#ifndef CRS_LIBEXPORT_H
#define CRS_LIBEXPORT_H 1
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

#if !defined( CRS_EXPORT )
#	if defined( COMPILING_CRS_DLL ) || defined( DLL_EXPORT )
#		define CRS_EXPORT	LIBCRS_DECL_DLLEXPORT
#	else
#		define CRS_EXPORT	LIBCRS_DECL_DLLIMPORT
#	endif
#endif

#endif /* CRS_LIBEXPORT_H */
