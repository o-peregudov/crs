#ifndef CROSS_POSIX_COND_H_INCLUDED
#define CROSS_POSIX_COND_H_INCLUDED 1
//
// cond.h - interface of the class cConditionVariable (using POSIX API)
// (c) Sep 3, 2010 Oleg N. Peregudov
//

#include <crs/security.h>
#include <stdexcept>
#include <cstdio>
#include <ctime>

namespace CrossClass
{
	class CROSS_EXPORT cPosixConditionVariable
	{
	protected:
		pthread_cond_t	_condition;
		#if defined( __GNUG__ )
		typedef long long		long64;
		#elif defined( _MSC_VER )
		typedef __int64		long64;
		#endif
		
	public:
		cPosixConditionVariable ();
		~cPosixConditionVariable ();
		
		void notify_one ();
		void wait ( _LockIt & );
		
		template <class Predicate>
		bool wait_for ( _LockIt & lock, const unsigned long msTimeOut, Predicate pred )
		{
			timespec abstime;
			clock_gettime( CLOCK_REALTIME, &abstime );
			long64 nanoseconds = abstime.tv_nsec + msTimeOut * 1000000L;
			abstime.tv_sec += nanoseconds / 1000000000L;	// seconds
			abstime.tv_nsec = nanoseconds % 1000000000L;	// nanoseconds
			for( int errCode = 0; !pred(); )
			{
				errCode = pthread_cond_timedwait( &_condition, static_cast<pthread_mutex_t*>( *(lock.mutex()) ), &abstime );
				switch( errCode )
				{
				case	EINVAL:
					throw std::runtime_error( "The value specified by cond, mutex or abstime is invalid" );
				case	EPERM:
					throw std::runtime_error( "The mutex was not owned by the current thread at the time of the call" );
				case	ETIMEDOUT:	// The time specified by abstime to pthread_cond_timedwait() has passed
				default:
					return pred();
				}
			}
			return pred();
		}
	};
} // namespace CrossClass
#endif // CROSS_POSIX_COND_H_INCLUDED

