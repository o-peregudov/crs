#ifndef CROSS_SEMAPHORE_HPP_INCLUDED
#define CROSS_SEMAPHORE_HPP_INCLUDED 1
//
// semaphore.hpp - interface of the class cSemaphore
// (c) Sep 18, 2010 Oleg N. Peregudov
//

#include <crs/libexport.h>
#if defined( USE_WIN32_API )
#	include <crs/bits/win32.semaphore.h>
#elif defined( USE_POSIX_API )
#	include <crs/bits/posix.semaphore.h>
#endif

namespace CrossClass
{

#if defined( USE_WIN32_API )
	typedef cWin32Semaphore	cSemaphore;
#elif defined( USE_POSIX_API )
	typedef cPosixSemaphore	cSemaphore;
#endif

} // namespace CrossClass
#endif // CROSS_SEMAPHORE_HPP_INCLUDED
