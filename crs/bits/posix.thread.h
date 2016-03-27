#ifndef CROSS_POSIX_THREAD_H_INCLUDED
#define CROSS_POSIX_THREAD_H_INCLUDED 1
/*
 *  crs/bits/posix.thread.h
 *  Copyright (c) 2008-2012 Oleg N. Peregudov <o.peregudov@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * IMPORTANT: see crs/thread.h for usage notes
 *
 *	2008/02/08	interface of the class cThread
 *	2009/04/23	Win/Posix defines
 *	2010/08/30	compartibility with STL condition_variable
 *	2010/09/04	using our condition_variable
 *			new running flag
 *	2011/01/05	slightly changed Step
 *	2011/01/21	separate running and activation flags observers
 *	2011/08/07	no sleep function prototype
 *	2012/08/17	atomic control flags
 */

#include <crs/security.h>
#include <crs/condition_variable.hpp>
#include <crs/atomic_flag.h>
#include <pthread.h>
#include <sched.h>

namespace CrossClass
{
	class CROSS_EXPORT cPosixThread
	{
	protected:
		pthread_t		 _thread;
		cMutex		 _wakeup_mutex;
		cConditionVariable _wakeup_condition;
		bool			 _wakeup_flag;
		cAtomicBool		 _terminate_flag,
					 _activate_flag,
					 _running_flag;
		void *		 _thread_routine_result;
		std::string		 _error_text;
		
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
				: _thread_attr (ta)
			{
				pthread_attr_init (_thread_attr);
			}
			
			~safe_thread_attr ()
			{
				pthread_attr_destroy (_thread_attr);
			}
		};
		
		cPosixThread ( const int priority = 0 );
		
	public:
		virtual ~cPosixThread ();
		virtual void Resume ();
		virtual void Stop ();
		
		operator bool ()
		{
			return _running_flag.load ();
		}
		
		bool active ()
		{
			return _activate_flag.load ();
		}
		
		void * kill ();
	};
}	/* namespace CrossClass			*/
#endif/* CROSS_POSIX_THREAD_H_INCLUDED	*/
