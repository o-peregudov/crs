#ifndef CROSS_CONDITION_VARIABLE_HPP_INCLUDED
#define CROSS_CONDITION_VARIABLE_HPP_INCLUDED 1
// (c) Aug 30, 2010 Oleg N. Peregudov

#include <crs/libexport.h>

#if defined( USE_WIN32_API )
#	include <crs/bits/win32.cond.h>
#elif defined( USE_CXX0X_API )
#	include <condition_variable>
#elif defined( USE_POSIX_API )
#	include <crs/bits/posix.cond.h>
#endif

namespace CrossClass
{
#if defined( USE_WIN32_API )
	typedef cWin32ConditionVariable	cConditionVariable;
#elif defined( USE_CXX0X_API )
	class CROSS_EXPORT cConditionVariable : std::condition_variable
	{
	public:
		cConditionVariable ()
			: std::condition_variable( )
		{ }
		
		~cConditionVariable ()
		{ }
		
		template <class Predicate>
		bool wait_for ( _LockIt & lock, const unsigned long msTimeOut, Predicate pred )
		{
			return std::condition_variable::wait_for( lock, chrono::milliseconds( msTimeOut ), pred );
		}
	};
#elif defined( USE_POSIX_API )
	typedef cPosixConditionVariable	cConditionVariable;
#endif
} // namespace CrossClass
#endif // CROSS_CONDITION_VARIABLE_HPP_INCLUDED
