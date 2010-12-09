#ifndef CROSS_DEFSYS_H_INCLUDED
#define CROSS_DEFSYS_H_INCLUDED 1

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
#	ifndef WINVER				// Allow use of features specific to Windows XP or later.
#		define WINVER 0x0600		// Change this to the appropriate value to target other versions of Windows.
#	endif
#	ifndef _WIN32_WINNT			// Allow use of features specific to Windows XP or later.
#		define _WIN32_WINNT 0x0600	// Change this to the appropriate value to target other versions of Windows.
#	endif
#	ifndef _WIN32_WINDOWS			// Allow use of features specific to Windows 98 or later.
#		define _WIN32_WINDOWS 0x0410	// Change this to the appropriate value to target Windows Me or later.
#	endif
#	ifndef _WIN32_IE				// Allow use of features specific to IE 6.0 or later.
#		define _WIN32_IE 0x0600		// Change this to the appropriate value to target other versions of IE.
#	endif
#else
#	define USE_POSIX_API 1
#endif

#if defined( __GNUG__ ) || defined( __GNUC__ )
typedef long long		long64;
#elif defined( _MSC_VER )
typedef __int64		long64;
#endif
#define LONG64_TYPE	1

#endif // CROSS_DEFSYS_H_INCLUDED

