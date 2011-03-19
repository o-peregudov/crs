// (c) Sep 18, 2010 Oleg N. Peregudov
// Envelop for the POSIX semaphore
//	01/17/2011	integer types
//

#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif

#include <crs/bits/posix.semaphore.h>
#include <cstring>
#include <cerrno>
#include <ctime>

namespace CrossClass {

//
// members of class cPosixSemaphore
//

cPosixSemaphore::cPosixSemaphore ( unsigned value )
	: _semaphore( )
{
	if( sem_init( &_semaphore, 1, value ) == -1 )
		throw std::runtime_error( strerror( errno ) );
}

cPosixSemaphore::~cPosixSemaphore ()
{
	if( sem_destroy( &_semaphore ) == -1 )
		throw std::runtime_error( strerror( errno ) );
}

void cPosixSemaphore::lock ()
{
	if( sem_wait( &_semaphore ) == -1 )
		throw std::runtime_error( strerror( errno ) );
}

bool cPosixSemaphore::try_lock ()
{
	if( sem_wait( &_semaphore ) == -1 )
	{
		if( errno == EAGAIN )	// The semaphore was already locked, so it cannot be
			return false;	// immediately locked by the sem_trywait() operation
		else
			throw std::runtime_error( strerror( errno ) );
	}
	else
		return true;
}

bool cPosixSemaphore::try_lock_for ( const unsigned long dwMilliseconds )
{
	timespec abstime;
	clock_gettime( CLOCK_REALTIME, &abstime );
	crs_int64_t nanoseconds = abstime.tv_nsec + dwMilliseconds * 1000000L;
	abstime.tv_sec += nanoseconds / 1000000000L;	// seconds
	abstime.tv_nsec = nanoseconds % 1000000000L;	// nanoseconds
	if( sem_timedwait( &_semaphore, &abstime ) == -1 )
	{
		if( errno == ETIMEDOUT )// The semaphore could not be locked
			return false;	// before the specified timeout expired
		else
			throw std::runtime_error( strerror( errno ) );
	}
	else
		return true;
}

void cPosixSemaphore::unlock ()
{
	if( sem_post( &_semaphore ) == -1 )
		throw std::runtime_error( strerror( errno ) );
}

} // namespace CrossClass

