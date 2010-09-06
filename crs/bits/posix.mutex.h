#ifndef CROSS_POSIX_MUTEX_H_INCLUDED
#define CROSS_POSIX_MUTEX_H_INCLUDED 1
// (c) Jan 22, 2008 Oleg N. Peregudov
// Aug 23, 2010 - POSIX mutex envelope
// Aug 30, 2010 - compartibility with std::mutex from C++0x standard

#include <crs/libexport.h>
#include <stdexcept>
#include <cstdlib>
#include <pthread.h>
#include <errno.h>

namespace CrossClass
{
	class CROSS_EXPORT cPosixMutex
	{
	protected:
		pthread_mutex_t _mutex;
		pthread_mutexattr_t _attr;
		
	public:
		typedef pthread_mutex_t native_handle_type;
		
		cPosixMutex ();
		~cPosixMutex ();
		
		void lock ();
		bool try_lock ();
		void unlock ();
		
		operator native_handle_type & ()			{ return _mutex; }
		operator const native_handle_type & () const	{ return _mutex; }
		
		operator native_handle_type * ()			{ return &_mutex; }
		operator const native_handle_type * ()	const { return &_mutex; }
	};
} // namespace CrossClass
#endif // CROSS_POSIX_MUTEX_H_INCLUDED
