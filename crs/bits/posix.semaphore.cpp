/*
 *  crs/bits/posix.semaphore.cpp
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

#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif
#include <crs/bits/posix.semaphore.h>

namespace CrossClass
{

cPosixSemaphore::cPosixSemaphore ( unsigned value )
	: _semaphore( )
{
	if (sem_init (&_semaphore, 0, value) == -1)
	{
		char msgText [ 256 ];
		sprintf (msgText, "%d: %s in 'cPosixSemaphore::sem_init'", errno, strerror (errno));
		throw std::runtime_error (msgText);
	}
}

cPosixSemaphore::~cPosixSemaphore ()
{
	int result = sem_destroy (&_semaphore);
#if defined (DESTRUCTOR_EXCEPTIONS_ALLOWED)
	if (result == -1)
	{
		char msgText [ 256 ];
		sprintf (msgText, "%d: %s in '~cPosixSemaphore::sem_destroy'", errno, strerror (errno));
		throw std::runtime_error (msgText);
	}
#endif
}

void cPosixSemaphore::lock ()
{
	int retcode = 0;
	while (((retcode = sem_wait (&_semaphore)) == -1) && (errno == EINTR))
		continue;			/* restart if interrupted by signal */
	
	/* check what happened */
	if (retcode == -1)
	{
		char msgText [ 256 ];
		sprintf (msgText, "%d: %s in 'cPosixSemaphore::sem_wait'", errno, strerror (errno));
		throw std::runtime_error (msgText);
	}
}

bool cPosixSemaphore::try_lock ()
{
	int retcode = 0;
	while (((retcode = sem_trywait (&_semaphore)) == -1) && (errno == EINTR))
		continue;			/* restart if interrupted by signal */
	
	/* check what happened */
	if (retcode == -1)
	{
		if (errno == EAGAIN)	/* The semaphore was already locked, so it cannot be */
			return false;	/* immediately locked by the sem_trywait() operation */
		else
		{
			char msgText [ 256 ];
			sprintf (msgText, "%d: %s in 'cPosixSemaphore::sem_trywait'", errno, strerror (errno));
			throw std::runtime_error (msgText);
		}
	}
	else
		return true;
}

bool cPosixSemaphore::try_lock_for ( const unsigned long dwMilliseconds )
{
	struct timespec abstime;
	if (clock_gettime (CLOCK_REALTIME, &abstime) == -1)
	{
		char msgText [ 256 ];
		sprintf (msgText, "%d: %s in 'cPosixSemaphore::clock_gettime'", errno, strerror (errno));
		throw std::runtime_error (msgText);
	}
	
	crs_int64_t nanoseconds = abstime.tv_nsec + dwMilliseconds * 1000000L;
	abstime.tv_sec += nanoseconds / 1000000000L;	/* seconds		*/
	abstime.tv_nsec = nanoseconds % 1000000000L;	/* nanoseconds	*/
	
	int retcode = 0;
	while (((retcode = sem_timedwait (&_semaphore, &abstime)) == -1) && (errno == EINTR))
		continue;			/* restart if interrupted by signal */
	
	/* check what happened */
	if (retcode == -1)
	{
		if (errno == ETIMEDOUT)	/* The semaphore could not be locked	*/
			return false;	/* before the specified timeout expired	*/
		else
		{
			char msgText [ 256 ];
			sprintf (msgText, "%d: %s in 'cPosixSemaphore::sem_timedwait'", errno, strerror (errno));
			throw std::runtime_error (msgText);
		}
	}
	else
		return true;
}

void cPosixSemaphore::unlock ()
{
	if (sem_post (&_semaphore) == -1)
	{
		char msgText [ 256 ];
		sprintf (msgText, "%d: %s in 'cPosixSemaphore::sem_post'", errno, strerror (errno));
		throw std::runtime_error (msgText);
	}
}

} /* namespace CrossClass */
