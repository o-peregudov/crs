// (c) Sep 18, 2010 Oleg N. Peregudov
// Envelop for the Win32 semaphore
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#	pragma warning( disable : 4996 )
#endif

#include <crs/bits/win32.semaphore.h>
#include <cstdio>

namespace CrossClass {

//
// members of class cWin32Semaphore
//

cWin32Semaphore::cWin32Semaphore ( unsigned value )
	: _semaphore( NULL )
{
	_semaphore = CreateSemaphore( NULL, value, value, NULL );
	if( _semaphore == NULL )
	{
		char msgText [ 64 ];
		sprintf( msgText, "cWin32Semaphore::cWin32Semaphore (%d)", GetLastError() );
		throw std::runtime_error( msgText );
	}
}

cWin32Semaphore::~cWin32Semaphore ()
{
	if( CloseHandle( _semaphore ) == 0 )
	{
		char msgText [ 64 ];
		sprintf( msgText, "cWin32Semaphore::~cWin32Semaphore (%d)", GetLastError() );
		throw std::runtime_error( msgText );
	}
}

void cWin32Semaphore::lock ()
{
	switch( WaitForSingleObject( _semaphore, INFINITE ) )
	{
	case	WAIT_OBJECT_0:
		return;
	
	case	WAIT_FAILED:
		{
			char msgText [ 64 ];
			sprintf( msgText, "cWin32Semaphore::lock (%d)", GetLastError() );
			throw std::runtime_error( msgText );
		}
	}
}

bool cWin32Semaphore::try_lock_for ( const unsigned long dwMilliseconds )
{
	switch( WaitForSingleObject( _semaphore, dwMilliseconds ) )
	{
	case	WAIT_OBJECT_0:
		return true;
	
	case	WAIT_FAILED:
		{
			char msgText [ 64 ];
			sprintf( msgText, "cWin32Semaphore::try_lock (%d)", GetLastError() );
			throw std::runtime_error( msgText );
		}
	
	case	WAIT_TIMEOUT:
	default:
		return false;
	}
}

void cWin32Semaphore::unlock ()
{
	if( ReleaseSemaphore( _semaphore, 1, NULL ) == 0 )
	{
		char msgText [ 64 ];
		sprintf( msgText, "cWin32Semaphore::unlock (%d)", GetLastError() );
		throw std::runtime_error( msgText );
	}
}

} // namespace CrossClass

