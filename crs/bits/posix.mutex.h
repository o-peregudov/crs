#ifndef CROSS_POSIX_MUTEX_H_INCLUDED
#define CROSS_POSIX_MUTEX_H_INCLUDED 1
/*
 *  crs/bits/posix.mutex.h
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
 *	2008/01/22	wrapper for POSIX mutex
 *	2010/08/30	compartibility with std::mutex from C++0x standard
 *	2012/08/16	fixed cPosixMutex::native_handle_type
 */

#include <crs/libexport.h>
#include <stdexcept>
#include <pthread.h>

namespace CrossClass
{
	class CROSS_EXPORT cPosixMutex
	{
	protected:
		pthread_mutex_t _mutex;
		pthread_mutexattr_t _attr;
		
	public:
		typedef pthread_mutex_t * native_handle_type;
		
		cPosixMutex ();
		~cPosixMutex ();
		
		void lock ();
		bool try_lock ();
		void unlock ();
		
		native_handle_type native_handle ()
		{
			return &_mutex;
		}
	};
}	/* namespace CrossClass			*/
#endif/* CROSS_POSIX_MUTEX_H_INCLUDED	*/
