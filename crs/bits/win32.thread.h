#ifndef CROSS_WIN32_THREAD_H_INCLUDED
#define CROSS_WIN32_THREAD_H_INCLUDED 1
//
// thread.h - interface of the class cThread
// (c) Feb 8, 2008 Oleg N. Peregudov
// Apr 23, 2009 - Win/Posix defines
//
// NOTE: (1) cThread is an abstract base class
//       (2) always call cThread::kill() memeber in the destructor of your derived class
//           in order to proceed all virtual members properly
//
//	Sep 4, 2010 - running flag
//

#include <crs/libexport.h>
#include <crs/security.h>

namespace CrossClass
{
	class CROSS_EXPORT cWin32Thread
	{
	protected:
		HANDLE	_thread,
				_evntActivate,
				_evntResume,
				_evntTerminate,
				_evntStopped;
		void *	_thread_routine_result;
		std::string	_error_text;
		
		static unsigned long WINAPI thread_routine ( void * pParam );
		
		bool	_check_terminate ();
		bool	_check_terminate ( bool & activationFlag );
		bool	_check_activate ();
		
		void	_post_activate ();
		void	_post_deactivate ();
		void	_post_terminate ();
		
		virtual void * Run ();
		virtual void * StepResult ( const int errCode );
		virtual bool Step () = 0;
		
		cWin32Thread ( const int priority = 0 );
		
	public:
		virtual ~cWin32Thread ();
		virtual void Resume ();
		virtual void Stop ();
		
		bool active ();
		void * kill ();
	};
	
	inline void sleep ( const unsigned long msDuration )
	{
		::Sleep( msDuration );
	}
} // namespace CrossClass
#endif // CROSS_WIN32_THREAD_H_INCLUDED

