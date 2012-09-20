#ifndef CROSS_POSIX_SPINLOCK_H_INCLUDED
#define CROSS_POSIX_SPINLOCK_H_INCLUDED 1
/*
 *  crs/bits/posix.spinlock.h
 *  Copyright (c) 2012 Oleg N. Peregudov <o.peregudov@gmail.com>
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
 *	2012-08-27	wrapper for the POSIX spin lock
 */

#include <crs/libexport.h>
#include <pthread.h>
#include <stdexcept>
#include <cstring>
#include <cstdio>

namespace CrossClass
{
	class CROSS_EXPORT cPosixSpinLock
	{
	protected:
		pthread_spinlock_t _spinlock;
		
	public:
		typedef pthread_spinlock_t * native_handle_type;
		
		cPosixSpinLock ();
		~cPosixSpinLock ();
		
		void lock ();
		bool try_lock ();
		void unlock ();
		
		native_handle_type native_handle ()
		{
			return &_spinlock;
		}
	};
}	/* namespace CrossClass			*/
#endif/* CROSS_POSIX_SPINLOCK_H_INCLUDED	*/
