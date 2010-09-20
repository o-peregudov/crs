#ifndef CROSS_POSIX_SEMAPHORE_H_INCLUDED
#define CROSS_POSIX_SEMAPHORE_H_INCLUDED 1
// (c) Sep 18, 2010 Oleg N. Peregudov
// Envelop for the POSIX semaphore

#include <crs/libexport.h>
#include <stdexcept>
#include <semaphore.h>

namespace CrossClass
{
	class CROSS_EXPORT cPosixSemaphore
	{
	protected:
		sem_t	_semaphore;
		
	public:
		typedef sem_t native_handle_type;
		
		cPosixSemaphore ( unsigned value = 1 );
		~cPosixSemaphore ();
		
		void lock ();
		bool try_lock ();
		bool try_lock_for ( const unsigned long dwMilliseconds );
		void unlock ();
		
		operator native_handle_type & ()			{ return _semaphore; }
		operator const native_handle_type & () const	{ return _semaphore; }
		
		operator native_handle_type * ()			{ return &_semaphore; }
		operator const native_handle_type * ()	const { return &_semaphore; }
	};
} // namespace CrossClass
#endif // CROSS_POSIX_SEMAPHORE_H_INCLUDED
