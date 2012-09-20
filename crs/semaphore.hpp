#ifndef CROSS_SEMAPHORE_HPP_INCLUDED
#define CROSS_SEMAPHORE_HPP_INCLUDED 1
/*
 *  crs/semaphore.hpp
 *  Copyright (c) 2010-2012 Oleg N. Peregudov <o.peregudov@gmail.com>
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
 *	2010/09/18	interface of the CrossClass::cSemaphore
 *	2012/08/15	new platform specific defines
 */

#include <crs/libexport.h>
#if USE_WIN32_API
#	include <crs/bits/win32.semaphore.h>
namespace CrossClass
{
	typedef cWin32Semaphore	cSemaphore;
}
#elif USE_POSIX_API
#	include <crs/bits/posix.semaphore.h>
namespace CrossClass
{
	typedef cPosixSemaphore	cSemaphore;
}
#endif

#endif /* CROSS_SEMAPHORE_HPP_INCLUDED */
