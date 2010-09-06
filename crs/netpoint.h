#ifndef CROSS_NETPOINT_H_INCLUDED
#define CROSS_NETPOINT_H_INCLUDED 1
// (c) Aug 24, 2010 Oleg N. Peregudov

#include <crs/libexport.h>
#if defined( USE_POSIX_API )
#	include <crs/bits/posix.netpoint.h>
#elif defined( USE_WIN32_API )
#	include <crs/bits/win32.netpoint.h>
#endif

namespace CrossClass
{

#if defined( USE_POSIX_API )
	typedef posixNetPoint	netPoint;
#elif defined( USE_WIN32_API )
	typedef win32NetPoint	netPoint;
#endif

} // namespace CrossClass
#endif // CROSS_NETPOINT_H_INCLUDED

