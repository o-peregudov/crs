#ifndef CROSS_POSIX_COND_H_INCLUDED
#define CROSS_POSIX_COND_H_INCLUDED 1
/*
 *  crs/bits/posix.cond.h - interface of the class cConditionVariable (POSIX API)
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
 *	2010/09/03	first version
 *	2010/09/18	expand errors using strerror function
 *	2011/01/17	integer types
 *	2012/05/11	notify_all member
 *	2012/08/16	support for C++11 mutex::native_handle ()
 */

#include <crs/security.h>
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <ctime>
#include <pthread.h>

namespace CrossClass
{
	class CROSS_EXPORT cPosixConditionVariable
	{
	protected:
		pthread_cond_t	_condition;
		
	public:
		cPosixConditionVariable ();
		~cPosixConditionVariable ();
		
		void notify_one ();
		void notify_all ();
		void wait ( _LockIt & );
		
		template <class Predicate>
		bool wait_for ( _LockIt & lock, const unsigned long dwMilliseconds, Predicate pred )
		{
			timespec abstime;
			clock_gettime (CLOCK_REALTIME, &abstime);
			crs_int64_t nanoseconds = abstime.tv_nsec + dwMilliseconds * 1000000L;
			abstime.tv_sec += nanoseconds / 1000000000L;	// seconds
			abstime.tv_nsec = nanoseconds % 1000000000L;	// nanoseconds
			for (int errCode = 0; !pred();)
			{
				errCode = pthread_cond_timedwait
					(&_condition, static_cast<pthread_mutex_t*> (lock.mutex ()->native_handle () ), &abstime);
				if ((errCode == 0) || (errCode == ETIMEDOUT))
					return pred ();
				else
					throw std::runtime_error (strerror (errCode));
			}
			return pred ();
		}
	};
}	/* namespace CrossClass			*/
#endif/* CROSS_POSIX_COND_H_INCLUDED	*/
