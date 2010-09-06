#ifndef CROSS_LIBEXPORT_H
#define CROSS_LIBEXPORT_H 1
// (c) Aug 12, 2009 Oleg N. Peregudov

#include <crs/defsys.h>

#if defined( USE_WIN32_API ) || defined( WIN32 )
#	define LIBSRC_DECL_DLLEXPORT	__declspec( dllexport )
#	define LIBSRC_DECL_DLLIMPORT	__declspec( dllimport )
#else
#	define LIBSRC_DECL_DLLEXPORT
#	define LIBSRC_DECL_DLLIMPORT
#endif

#if !defined( CROSS_EXPORT )
#	if defined( COMPILING_CROSS_DLL ) || defined( DLL_EXPORT )
#		define CROSS_EXPORT	LIBSRC_DECL_DLLEXPORT
#	else
#		define CROSS_EXPORT	LIBSRC_DECL_DLLIMPORT
#	endif
#endif

#endif // CROSS_LIBEXPORT_H

