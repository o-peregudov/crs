#ifndef CROSS_POSIX_SEMAPHORE_H_INCLUDED
#define CROSS_POSIX_SEMAPHORE_H_INCLUDED 1
/*
 *  crs/bits/posix.semaphore.h
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
 *	2010-09-18	wrapper for the POSIX semaphore
 *	2011-01-17	integer types
 *	2012-03-17	semaphore is shared within process by default
 *			all wrappers are inline and becomes more sophisticated
 *	2012-08-17	implementation was moved into a separate file
 *			native_handle_type
 */

#include <crs/libexport.h>
#include <semaphore.h>
#include <stdexcept>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <ctime>

namespace CrossClass
{
	class CROSS_EXPORT cPosixSemaphore
	{
	protected:
		sem_t	_semaphore;
		
	public:
		typedef sem_t * native_handle_type;
		
		cPosixSemaphore ( unsigned value = 1 );
		~cPosixSemaphore ();
		
		void lock ();
		bool try_lock ();
		bool try_lock_for ( const unsigned long dwMilliseconds );
		void unlock ();
		
		native_handle_type native_handle ()
		{
			return &_semaphore;
		}
	};
}	/* namespace CrossClass			*/
#endif/* CROSS_POSIX_SEMAPHORE_H_INCLUDED	*/
