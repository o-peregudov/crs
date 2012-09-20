/*
 *  crs/bits/posix.thread.cpp
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

#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif

#include <crs/bits/posix.thread.h>
#include <cstdio>
#include <ctime>
#include <cerrno>

namespace CrossClass {

//
// members of class cPosixThread
//
void * cPosixThread::thread_routine ( void * pParam )
{
	cPosixThread * pThreadClass = reinterpret_cast<cPosixThread *>( pParam );
	pThreadClass->_running_flag.store (true);
	try
	{
		pThreadClass->_thread_routine_result = pThreadClass->Run ();
	}
	catch (std::runtime_error & e)
	{
		pThreadClass->_error_text = e.what();
		pThreadClass->_thread_routine_result = reinterpret_cast<void *>( -1 );
	}
	catch (...)
	{
		pThreadClass->_error_text = "unhandled exception in 'thread_routine'";
		pThreadClass->_thread_routine_result = reinterpret_cast<void *>( -2 );
	}
	pThreadClass->_running_flag.store (false);
	return pThreadClass->_thread_routine_result;
}

void	cPosixThread::_post_activate ()
{
	_activate_flag.store (true);
	_wake_up ();
}

void	cPosixThread::_post_deactivate ()
{
	_activate_flag.store (false);
	_wake_up ();
}

void	cPosixThread::_post_terminate ()
{
	_terminate_flag.store (true);
	_wake_up ();
}

void cPosixThread::_wake_up ()
{
	// NOTE: if predictable scheduling behavior is required,
	// then that mutex shall be locked by the thread calling
	// pthread_cond_broadcast() or pthread_cond_signal()
	_LockIt _wakeup_lock ( _wakeup_mutex );
	_wakeup_flag = true;
	_wakeup_condition.notify_one ();	// wake up the thread
}

void * cPosixThread::Run ()
{
	while (!_terminate_flag.load ())
	{
		if (_activate_flag.load ())
		{
			if (Step ())
				return StepResult (0);
		}
		else
		{
			_LockIt _wakeup_lock ( _wakeup_mutex );
			while (!_wakeup_flag)
				_wakeup_condition.wait (_wakeup_lock);
			_wakeup_flag = false;
		}
	}
	return StepResult (0);
}

void * cPosixThread::StepResult ( const int errCode )
{
	return reinterpret_cast<void *>( errCode );
}

void cPosixThread::Resume ()
{
	_post_activate ();
}

void cPosixThread::Stop ()
{
	_post_deactivate ();
}

cPosixThread::cPosixThread ( const int priority )
	: _thread ( )
	, _wakeup_mutex ( )
	, _wakeup_condition ( )
	, _wakeup_flag (false)
	, _terminate_flag (false)
	, _activate_flag (false)
	, _running_flag (false)
	, _thread_routine_result (0)
	, _error_text ( )
{
	pthread_attr_t _thread_attributes;
	safe_thread_attr sta (&_thread_attributes);
	
	int errCode = pthread_create (&_thread, &_thread_attributes, &thread_routine, this);
	if (errCode)
	{
		char msgText [ 64 ];
		sprintf (msgText, "pthread_create returns %d", errCode);
		throw std::runtime_error (msgText);
	}
	/*
	int policy;
	sched_param param;
	pthread_getschedparam( _thread, &policy, &param );
	param.sched_priority = -10;
	pthread_setschedparam( _thread, policy, &param );
	*/
}

cPosixThread::~cPosixThread ()
{
}

void * cPosixThread::kill ()
{
	if (*this)
	{
		_post_terminate ();
		switch (pthread_join (_thread, NULL))
		{
		case	ESRCH:
			throw std::runtime_error(
				"No thread could be found corresponding to that "
				"specified by the given thread ID" );
		case	EDEADLK:
			throw std::runtime_error(
				"A deadlock was detected or the value of thread "
				"specifies the calling thread" );
		case	EINVAL:
			throw std::runtime_error(
				"The value specified by thread does not refer to a "
				"joinable thread" );
		default:
			break;
		}
	}
	return _thread_routine_result;
}

void sleep ( const unsigned long msDuration )
{
	if (msDuration == 0)
		sched_yield ();
	else
	{
		timespec rqtp, rmtp;
		rqtp.tv_sec = msDuration / 1000L;				// seconds
		rqtp.tv_nsec = ( msDuration % 1000L ) * 1000000L;	// nanoseconds
		while ((nanosleep (&rqtp, &rmtp) == -1) && (errno == EINTR))
			rqtp = rmtp;
	}
}

} /* namespace CrossClass */
