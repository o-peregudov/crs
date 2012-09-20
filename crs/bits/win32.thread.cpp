/*
 *  crs/bits/win32.thread.cpp
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
#if defined (HAVE_CONFIG_H)
#	include "config.h"
#endif
#include <crs/bits/win32.thread.h>
#include <cstdio>

namespace CrossClass {

//
// members of class cWin32Thread
//
unsigned long WINAPI cWin32Thread::thread_routine ( void * pParam )
{
	cWin32Thread * pThreadClass = reinterpret_cast<cWin32Thread *>( pParam );
	ResetEvent (pThreadClass->_evntStopped);
	try
	{
		pThreadClass->_thread_routine_result = pThreadClass->Run ();
	}
	catch (std::runtime_error & e)
	{
		pThreadClass->_error_text = e.what ();
		pThreadClass->_thread_routine_result = reinterpret_cast<void *> (-1);
	}
	catch (...)
	{
		pThreadClass->_error_text = "unhandled exception in 'thread_routine'";
		pThreadClass->_thread_routine_result = reinterpret_cast<void *> (-1);
	}
	SetEvent (pThreadClass->_evntStopped);
	return 0;
}

cWin32Thread::cWin32Thread ( const int priority )
	: _thread (NULL)
	, _evntActivate (NULL)
	, _evntResume (NULL)
	, _evntTerminate (NULL)
	, _evntStopped (NULL)
	, _error_text ( )
{
	try
	{
		_evntActivate = CreateEvent (NULL, TRUE, FALSE, NULL);
		if (_evntActivate == NULL)
			throw "activate";
		
		_evntResume = CreateEvent (NULL, TRUE, FALSE, NULL);
		if (_evntResume == NULL)
			throw "resume";
		
		_evntTerminate = CreateEvent (NULL, TRUE, FALSE, NULL);
		if (_evntTerminate == NULL)
			throw "terminate";
		
		_evntStopped = CreateEvent (NULL, TRUE, FALSE, NULL);
		if (_evntStopped == NULL)
			throw "stopped";
		
		_thread = CreateThread (NULL, 0, thread_routine, this, 0, 0);
		if (_thread == NULL)
			throw "thread";
	}
	catch (const char * what)
	{
		char msgText [128];
		sprintf (msgText, "Create object (%s) failed in cWin32Thread::cWin32Thread (code %d)", what, GetLastError ());
		throw std::runtime_error (msgText);
	}
}

cWin32Thread::~cWin32Thread ()
{
	kill ();
	CloseHandle (_evntStopped);
	CloseHandle (_evntTerminate);
	CloseHandle (_evntResume);
	CloseHandle (_evntActivate);
}

bool cWin32Thread::_check_terminate ()
{
	return (WaitForSingleObject (_evntTerminate, 0) == WAIT_OBJECT_0);
}

bool cWin32Thread::_check_activate ()
{
	return (WaitForSingleObject (_evntActivate, 0) == WAIT_OBJECT_0);
}

void cWin32Thread::_post_activate ()
{
	SetEvent (_evntActivate);
	SetEvent (_evntResume);
}

void cWin32Thread::_post_deactivate ()
{
	ResetEvent (_evntActivate);
}

void cWin32Thread::_post_terminate ()
{
	SetEvent (_evntTerminate);
	SetEvent (_evntResume);
}

void * cWin32Thread::Run ()
{
	while (!_check_terminate ())
	{
		if (_check_activate ())
		{
			if (Step ())
				return StepResult (0);
		}
		else
		{
			WaitForSingleObject (_evntResume, INFINITE);
		}
	}
	return StepResult (0);
}

void * cWin32Thread::StepResult ( const int errCode )
{
	return (void*)( errCode );
}

void cWin32Thread::Resume ()
{
	_post_activate();
}

void cWin32Thread::Stop ()
{
	_post_deactivate();
}

bool cWin32Thread::active ()
{
	return _check_activate ();
}

cWin32Thread::operator bool ()
{
	return (WaitForSingleObject (_evntStopped, 0) == WAIT_TIMEOUT);
}

void * cWin32Thread::kill ()
{
	if (*this)
	{
		_post_terminate ();
		WaitForSingleObject (_evntStopped, INFINITE);
		CloseHandle (_thread);
		_thread = 0;
	}
	return _thread_routine_result;
}

} /* namespace CrossClass	*/
