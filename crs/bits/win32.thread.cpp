// (c) Apr 17, 2008 Oleg N. Peregudov
//	08/23/2010	Win32 thread envelope
//	09/04/2010	running flag
//	01/21/2011	separate running and activation flags observers
#include <crs/defsys.h>
#include <crs/bits/win32.thread.h>
#include <cstdio>

namespace CrossClass {

//
// members of class cWin32Thread
//
unsigned long WINAPI cWin32Thread::thread_routine ( void * pParam )
{
	cWin32Thread * pThreadClass = reinterpret_cast<cWin32Thread *>( pParam );
	ResetEvent( pThreadClass->_evntStopped );
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
	SetEvent( pThreadClass->_evntStopped );
	return 0;
}

cWin32Thread::cWin32Thread ( const int priority )
	: _thread( NULL )
	, _evntActivate( CreateEvent( NULL, TRUE, FALSE, NULL ) )
	, _evntResume( CreateEvent( NULL, FALSE, FALSE, NULL ) )
	, _evntTerminate( CreateEvent( NULL, TRUE, FALSE, NULL ) )
	, _evntStopped( CreateEvent( NULL, TRUE, TRUE, NULL ) )
	, _error_text( )
{
	_thread = CreateThread( NULL, 0, thread_routine, this, 0, 0 );
	if( !_thread )
		throw std::runtime_error( "CreateThread returns NULL pointer" );
}

cWin32Thread::~cWin32Thread ()
{
	kill();
	CloseHandle( _evntStopped );
	CloseHandle( _evntTerminate );
	CloseHandle( _evntResume );
	CloseHandle( _evntActivate );
}

bool cWin32Thread::_check_terminate ()
{
	return ( WaitForSingleObject( _evntTerminate, 0 ) == WAIT_OBJECT_0 );
}

bool cWin32Thread::_check_terminate ( bool & activationFlag )
{
	activationFlag = _check_activate();
	return _check_terminate();
}

bool cWin32Thread::_check_activate ()
{
	return ( WaitForSingleObject( _evntActivate, 0 ) == WAIT_OBJECT_0 );
}

void cWin32Thread::_post_activate ()
{
	SetEvent( _evntActivate );
	SetEvent( _evntResume );
}

void cWin32Thread::_post_deactivate ()
{
	ResetEvent( _evntActivate );
	SetEvent( _evntResume );
}

void cWin32Thread::_post_terminate ()
{
	SetEvent( _evntTerminate );
	SetEvent( _evntResume );
}

void * cWin32Thread::Run ()
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
			WaitForSingleObject( _evntResume, INFINITE );
	}
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
	return _check_activate();
}

cWin32Thread::operator bool ()
{
	return ( WaitForSingleObject( _evntStopped, 0 ) == WAIT_TIMEOUT );
}

void * cWin32Thread::kill ()
{
	if( *this )
	{
		_post_terminate();
		WaitForSingleObject( _evntStopped, INFINITE );
		CloseHandle( _thread );
		_thread = 0;
	}
	return _thread_routine_result;
}

} // namespace CrossClass

