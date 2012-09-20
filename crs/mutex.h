#ifndef CROSS_MUTEX_H_INCLUDED
#define CROSS_MUTEX_H_INCLUDED 1
/*
 *  crs/mutex.h
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
 *	2008/01/22	wrapper for mutex object
 *	2010/08/26	separate versions for each API
 *	2010/08/28	compartibility with std::mutex from C++0x standard
 *	2012/08/18	new host class selection
 */

#include <crs/libexport.h>

#if USE_CXX11_MUTEX
#	include <mutex>
namespace CrossClass
{
	typedef std::mutex	cMutex;
}
#elif USE_WIN32_API
#	include <crs/bits/win32.mutex.h>
namespace CrossClass
{
	typedef cWin32Mutex	cMutex;
}
#elif USE_POSIX_API
#	include <crs/bits/posix.mutex.h>
namespace CrossClass
{
	typedef cPosixMutex	cMutex;
}
#endif

#endif/* CROSS_MUTEX_H_INCLUDED	*/
