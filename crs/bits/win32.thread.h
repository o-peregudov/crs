#ifndef CROSS_WIN32_THREAD_H_INCLUDED
#define CROSS_WIN32_THREAD_H_INCLUDED 1
/*
 *  crs/bits/win32.thread.h
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
 *	2010/09/04	running flag
 *	2011/01/21	separate running and activation flags observers
 *	2011/08/07	no sleep function prototype
 *	2012/08/19	atomic control flags
 */

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
		
		operator bool ();
		
		bool active ();
		void * kill ();
	};
}	/* namespace CrossClass			*/
#endif/* CROSS_WIN32_THREAD_H_INCLUDED	*/
