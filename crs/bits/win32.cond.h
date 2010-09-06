#ifndef CROSS_WIN32_COND_H_INCLUDED
#define CROSS_WIN32_COND_H_INCLUDED 1
//
// cond.h - interface of the class cConditionVariable (using Win32 API)
// (c) Aug 30, 2010 Oleg N. Peregudov
//

#include <crs/security.h>
#include <stdexcept>
#include <cstdio>
#include <ctime>

namespace CrossClass
{
	class CROSS_EXPORT cWin32ConditionVariable
	{
		struct dummyPredicate
		{
			bool operator () ()
			{
				return false;
			}
		};
		
	protected:
		HANDLE	_event;
		
	public:
		cWin32ConditionVariable ();
		~cWin32ConditionVariable ();
		
		template <class Predicate>
		bool wait_for ( _LockIt & lock, const unsigned long msTimeOut, Predicate pred )
		{
			lock.unlock();
			int waitRes = WaitForSingleObject( _event, msTimeOut );
			lock.lock();
			switch( waitRes )
			{
			case	WAIT_OBJECT_0:	// wait successfull
			case	WAIT_TIMEOUT:	// the time-out interval elapsed
				break;
			case	WAIT_FAILED:
				{
					char msgText [ 64 ];
					sprintf( msgText, "WaitForSingleObject returned 0x%X", GetLastError() );
					throw std::runtime_error( msgText );
				}
			}
			return pred();
		}
		
		void notify_one ()
		{
			SetEvent( _event );
		}
		
		void wait ( _LockIt & lock )
		{
			wait_for( lock, INFINITE, dummyPredicate() );
		}
	};
} // namespace CrossClass
#endif // CROSS_WIN32_COND_H_INCLUDED

