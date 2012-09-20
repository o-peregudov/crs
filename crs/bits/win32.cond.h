#ifndef CROSS_WIN32_COND_H_INCLUDED
#define CROSS_WIN32_COND_H_INCLUDED 1
/*
 *  crs/bits/win32.cond.h
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
 *	2010/08/30	emulation of condition variable for Win32 API
 *			NOTE: functionality is not complete, because
 *				of absence of the notify_all () member and
 *				not fully atomic mutex lock/unlock
 *	2012/08/19	follows to the general destructor exceptions macro
 */

#include <crs/libexport.h>
#include <crs/security.h>
#include <stdexcept>
#include <cstdio>
#include <ctime>

namespace CrossClass
{
	class CROSS_EXPORT cWin32ConditionVariable
	{
		struct dummyPredicate
		{
			bool operator () ()
			{
				return false;
			}
		};
		
	protected:
		HANDLE _event;
		
	public:
		cWin32ConditionVariable ();
		~cWin32ConditionVariable ();
		
		template <class Predicate>
		bool wait_for ( _LockIt & lock, const unsigned long msTimeOut, Predicate pred )
		{
			lock.unlock ();
			int waitRes = WaitForSingleObject (_event, msTimeOut);
			lock.lock ();
			switch (waitRes)
			{
			case	WAIT_OBJECT_0:	// wait successfull
			case	WAIT_TIMEOUT:	// the time-out interval elapsed
				break;
			case	WAIT_FAILED:
				{
					char msgText [ 64 ];
					sprintf (msgText, "WaitForSingleObject returned 0x%X", GetLastError ());
					throw std::runtime_error (msgText);
				}
			}
			return pred();
		}
		
		void notify_one ()
		{
			SetEvent (_event);
		}
		
		void wait ( _LockIt & lock )
		{
			wait_for (lock, INFINITE, dummyPredicate ());
		}
	};
}	/* namespace CrossClass			*/
#endif/* CROSS_WIN32_COND_H_INCLUDED	*/
