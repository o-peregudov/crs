#ifndef CROSS_WIN32_SEMAPHORE_H_INCLUDED
#define CROSS_WIN32_SEMAPHORE_H_INCLUDED 1
/*
 *  crs/bits/win32.semaphore.h
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
 *	2010/09/18	wrapper for the Win32 semaphore
 *	2012/08/19	cWin32Mutex::native_handle method
 *			follows to the general destructor exceptions macro
 */

#include <crs/libexport.h>
#include <stdexcept>

namespace CrossClass
{
	class CROSS_EXPORT cWin32Semaphore
	{
	protected:
		HANDLE _semaphore;
		
	public:
		typedef HANDLE native_handle_type;
		
		cWin32Semaphore ( unsigned value = 1 );
		~cWin32Semaphore ();
		
		void lock ();
		bool try_lock_for ( const unsigned long dwMilliseconds );
		void unlock ();
		
		bool try_lock ()
		{
			return try_lock_for (0);
		}
		
		native_handle_type native_handle ()
		{
			return _semaphore;
		}
	};
}	/* namespace CrossClass			*/
#endif/* CROSS_WIN32_SEMAPHORE_H_INCLUDED	*/
