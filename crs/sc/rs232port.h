#ifndef CROSS_SC_RS232PORT_H_INCLUDED
#define CROSS_SC_RS232PORT_H_INCLUDED 1
//////////////////////////////////////////////////////////////////////
//
// rs232port.h: interface for the rs232port class.
// (c) Aug 27, 2010 Oleg N. Peregudov
//
//////////////////////////////////////////////////////////////////////

#include <crs/libexport.h>
#if defined( USE_POSIX_API )
#	include <crs/sc/posix.rs232port.h>
#elif defined( USE_WIN32_API )
#	include <crs/sc/win32.rs232port.h>
#endif

namespace sc
{

#if defined( USE_POSIX_API )
	typedef posixRS232port	rs232port;
#elif defined( USE_WIN32_API )
	typedef win32RS232port	rs232port;
#endif

} // namespace sc
#endif // CROSS_SC_RS232PORT_H_INCLUDED

