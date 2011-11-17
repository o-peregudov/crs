#ifndef CROSS_POSIX_COND_H_INCLUDED
#define CROSS_POSIX_COND_H_INCLUDED 1
//
// cond.h - interface of the class cConditionVariable (using POSIX API)
// (c) Sep 3, 2010 Oleg N. Peregudov
//
//	09/18/2010	expand errors using strerror function
//	01/17/2011	integer types
//

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
		void wait ( _LockIt & );
		
		template <class Predicate>
		bool wait_for ( _LockIt & lock, const unsigned long dwMilliseconds, Predicate pred )
		{
			timespec abstime;
			clock_gettime( CLOCK_REALTIME, &abstime );
			crs_int64_t nanoseconds = abstime.tv_nsec + dwMilliseconds * 1000000L;
			abstime.tv_sec += nanoseconds / 1000000000L;	// seconds
			abstime.tv_nsec = nanoseconds % 1000000000L;	// nanoseconds
			for( int errCode = 0; !pred(); )
			{
				errCode = pthread_cond_timedwait( &_condition, static_cast<pthread_mutex_t*>( *(lock.mutex()) ), &abstime );
				if( ( errCode == 0 ) || ( errCode == ETIMEDOUT ) )
					return pred();
				else
					throw std::runtime_error( strerror( errCode ) );
			}
			return pred();
		}
	};
} // namespace CrossClass
#endif // CROSS_POSIX_COND_H_INCLUDED

