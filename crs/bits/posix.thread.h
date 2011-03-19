#ifndef CROSS_POSIX_THREAD_H_INCLUDED
#define CROSS_POSIX_THREAD_H_INCLUDED 1
//
// thread.h - interface of the class cThread
// (c) Feb 8, 2008 Oleg N. Peregudov
// Apr 23, 2009 - Win/Posix defines
//
// NOTE: (1) cThread is an abstract base class
//       (2) always call cThread::kill() memeber in the destructor of your derived class
//           in order to proceed all virtual members properly
//
//	08/30/2010	compartibility with STL condition_variable
//	09/04/2010	using our condition_variable
//			new running flag
//	01/05/2011	slightly changed Step
//	01/21/2011	separate running and activation flags observers
//

#include <crs/security.h>
#include <crs/condition_variable.hpp>
#include <sched.h>

namespace CrossClass
{
	class CROSS_EXPORT cPosixThread
	{
	protected:
		pthread_t	_thread;
		cMutex	_condition_mutex,
				_flags_mutex;
		cConditionVariable _condition;
		bool		_terminate_flag,
				_activate_flag,
				_wake_up_flag,
				_running_flag;
		void *	_thread_routine_result;
		std::string	_error_text;
		
		bool	_check_terminate ();
		bool	_check_terminate ( bool & activationFlag );
		bool	_check_activate ();
		
		void	_wake_up ();
		
		void	_post_activate ();
		void	_post_deactivate ();
		void	_post_terminate ();
		
		virtual void * Run ();
		virtual void * StepResult ( const int errCode );
		virtual bool Step () = 0;
		
		static void * thread_routine ( void * pParam );
		
		struct safe_thread_attr
		{
			pthread_attr_t * _thread_attr;
			
			safe_thread_attr ( pthread_attr_t * ta )
				: _thread_attr( ta )
			{
				pthread_attr_init( _thread_attr );
			}
			
			~safe_thread_attr ()
			{
				pthread_attr_destroy( _thread_attr );
			}
		};
		
		cPosixThread ( const int priority = 0 );
		
	public:
		virtual ~cPosixThread ();
		virtual void Resume ();
		virtual void Stop ();
		
		operator bool ();
		
		bool active ();
		void * kill ();
	};
	
	extern CROSS_EXPORT void sleep ( const unsigned long msDuration );
} // namespace CrossClass
#endif // CROSS_POSIX_THREAD_H_INCLUDED
