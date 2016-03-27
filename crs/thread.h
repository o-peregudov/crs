#ifndef CROSS_THREAD_H_INCLUDED
#define CROSS_THREAD_H_INCLUDED 1
/*
 *  crs/thread.h
 *  Copyright (c) 2008-2012 Oleg N. Peregudov <o.peregudov@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * NOTES: (1) cThread is an abstract base class
 *        (2) always call cThread::kill() memeber in the destructor of your derived class
 *            in order to proceed all virtual members properly
 *        (3) the thread is always created in the inactive state,
 *            so you have to Resume it when necessary
 */

/*
 *	2008/02/08	interface of the CrossClass::cThread
 *	2010/08/21	separate versions for each API
 *	2011/08/07	sleep function prototype
 *	2012/08/15	new platform specific defines
 */

#include <crs/libexport.h>

#if USE_WIN32_API
#include <crs/bits/win32.thread.h>
namespace CrossClass
{
	typedef cWin32Thread cThread;
	inline void sleep ( const unsigned long msDuration )
	{
		::Sleep (msDuration);
	}
}
#elif USE_POSIX_API
#	include <crs/bits/posix.thread.h>
namespace CrossClass
{
	typedef cPosixThread cThread;
	extern CROSS_EXPORT void sleep ( const unsigned long msDuration );
}
#endif

#endif /* CROSS_THREAD_H_INCLUDED */
