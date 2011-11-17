#ifndef CROSS_THREAD_H_INCLUDED
#define CROSS_THREAD_H_INCLUDED 1
//
// thread.h - interface of the class cThread
// (c) Feb 8, 2008 Oleg N. Peregudov
//
// NOTE: (1) cThread is an abstract base class
//       (2) always call cThread::kill() memeber in the destructor of your derived class
//           in order to proceed all virtual members properly
//
//	08/21/2010	separate versions for each API
//	08/07/2011	sleep function prototype
//

#include <crs/libexport.h>
#if defined( USE_WIN32_API )
#	include <crs/bits/win32.thread.h>
#elif defined( USE_POSIX_API )
#	include <crs/bits/posix.thread.h>
#endif

namespace CrossClass
{

#if defined( USE_WIN32_API )
	typedef cWin32Thread	cThread;
	
	inline void sleep ( const unsigned long msDuration )
	{
		::Sleep( msDuration );
	}
#elif defined( USE_POSIX_API )
	typedef cPosixThread	cThread;
	
	extern CROSS_EXPORT void sleep ( const unsigned long msDuration );
#endif

} // namespace CrossClass
#endif // CROSS_THREAD_H_INCLUDED

