#ifndef CROSS_WIN32_SEMAPHORE_H_INCLUDED
#define CROSS_WIN32_SEMAPHORE_H_INCLUDED 1
// (c) Sep 18, 2010 Oleg N. Peregudov
// Envelop for the Win32 semaphore

#include <crs/libexport.h>
#include <stdexcept>

namespace CrossClass
{
	class CROSS_EXPORT cWin32Semaphore
	{
	protected:
		HANDLE	_semaphore;
		
	public:
		typedef HANDLE native_handle_type;
		
		cWin32Semaphore ( unsigned value = 1 );
		~cWin32Semaphore ();
		
		void lock ();
		bool try_lock_for ( const unsigned long dwMilliseconds );
		bool try_lock ()						{ return try_lock_for( 0 ); }
		void unlock ();
		
		operator native_handle_type & ()			{ return _semaphore; }
		operator const native_handle_type & () const	{ return _semaphore; }
		
		operator native_handle_type * ()			{ return &_semaphore; }
		operator const native_handle_type * ()	const { return &_semaphore; }
	};
} // namespace CrossClass
#endif // CROSS_WIN32_SEMAPHORE_H_INCLUDED
