// (c) Apr 17, 2008 Oleg N. Peregudov
// Aug 23, 2010 - POSIX thread envelope
// Aug 27, 2010 - C++0x locks compartibility
// Aug 30, 2010 - using our condition variable wrapper

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
	_LockIt lockFlags ( pThreadClass->_flags_mutex );
	pThreadClass->_running_flag = true;
	lockFlags.unlock();
	try
	{
		pThreadClass->_thread_routine_result = pThreadClass->Run ();
	}
	catch( std::runtime_error & e )
	{
		pThreadClass->_error_text = e.what();
		pThreadClass->_thread_routine_result = reinterpret_cast<void *>( -1 );
	}
	catch( ... )
	{
		pThreadClass->_error_text = "unhandled exception in 'thread_routine'";
		pThreadClass->_thread_routine_result = reinterpret_cast<void *>( -1 );
	}
	lockFlags.lock();
	pThreadClass->_running_flag = false;
	lockFlags.unlock();
	return pThreadClass->_thread_routine_result;
}

bool cPosixThread::active ()
{
	_LockIt lockFlags ( _flags_mutex );
	return _running_flag;
}

bool cPosixThread::_check_terminate ()
{
	_LockIt lockFlags ( _flags_mutex );
	return _terminate_flag;
}

bool cPosixThread::_check_terminate ( bool & activationFlag )
{
	_LockIt lockFlags ( _flags_mutex );
	activationFlag = _activate_flag;
	return _terminate_flag;
}

bool cPosixThread::_check_activate ()
{
	_LockIt lockFlags ( _flags_mutex );
	return _activate_flag;
}

void cPosixThread::_wake_up ()
{
	// NOTE: if predictable scheduling behavior is required,
	// then that mutex shall be locked by the thread calling
	// pthread_cond_broadcast() or pthread_cond_signal()
	_LockIt condition_lock ( _condition_mutex );
	_wake_up_flag = true;
	_condition.notify_one();	// wake up the thread
}

void cPosixThread::_post_terminate ()
{
	_LockIt lockFlags ( _flags_mutex );
	_terminate_flag = true;
	lockFlags.unlock();
	_wake_up();
}

void cPosixThread::_post_activate ()
{
	_LockIt lockFlags ( _flags_mutex );
	_activate_flag = true;
	lockFlags.unlock();
	_wake_up();
}

void cPosixThread::_post_deactivate ()
{
	_LockIt lockFlags ( _flags_mutex );
	_activate_flag = false;
	lockFlags.unlock();
	_wake_up();
}

void * cPosixThread::Run ()
{
	for( bool runStep = false;; )
	{
		if( _check_terminate( runStep ) )
			return StepResult( 0 );
		
		if( runStep )
		{
			if( Step() )
				return StepResult( 0 );
		}
		else
		{
			_LockIt lockCondition ( _condition_mutex );
			_wake_up_flag = false;
			do
			{
				_condition.wait( lockCondition );
			}
			while( !_wake_up_flag );
		}
	}
}

void * cPosixThread::StepResult ( const int errCode )
{
	return reinterpret_cast<void *>( errCode );
}

void cPosixThread::Resume ()
{
	_post_activate();
}

void cPosixThread::Stop ()
{
	_post_deactivate();
}

cPosixThread::cPosixThread ( const int priority )
	: _thread( )
	, _condition_mutex( )
	, _flags_mutex( )
	, _condition( )
	, _terminate_flag( false )
	, _activate_flag( false )
	, _wake_up_flag( false )
	, _running_flag( false )
	, _thread_routine_result( 0 )
	, _error_text( )
{
	int errCode ( 0 );
	pthread_attr_t _thread_attributes;
	safe_thread_attr sta ( &_thread_attributes );
	if( ( errCode = pthread_create( &_thread, &_thread_attributes, &thread_routine, this ) ) )
	{
		char msgText [ 64 ];
		sprintf( msgText, "pthread_create returns %d", errCode );
		throw std::runtime_error( msgText );
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
	if( active() )
	{
		_post_terminate();
		switch( pthread_join( _thread, NULL ) )
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
	if( msDuration == 0 )
		sched_yield();
	else
	{
		timespec rqtp, rmtp;
		rqtp.tv_sec = msDuration / 1000L;
		rqtp.tv_nsec = ( msDuration % 1000L ) * 1000000L;
		while( nanosleep( &rqtp, &rmtp ) == -1 ) rqtp = rmtp;
	}
}

} // namespace CrossClass

