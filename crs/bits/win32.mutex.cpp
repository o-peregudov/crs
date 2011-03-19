// (c) Nov 29, 2007 Oleg N. Peregudov
// Aug 23, 2010 - Envelope for the Win32 mutex
//	09/09/2010	compartibility with std::mutex from C++0x standard
//	09/18/2010	uniform error handling
//
#include <crs/defsys.h>
#include <crs/bits/win32.mutex.h>
#include <cstdio>

namespace CrossClass {

//
// members of class cWin32Mutex
//

cWin32Mutex::cWin32Mutex ()
	: _mutex( NULL )
{
	_mutex = CreateMutex( NULL, FALSE, NULL );
	if( _mutex == NULL )
	{
		char msgText [ 64 ];
		sprintf( msgText, "cWin32Mutex::cWin32Mutex (%d)", GetLastError() );
		throw std::runtime_error( msgText );
	}
}

cWin32Mutex::~cWin32Mutex ()
{
	if( CloseHandle( _mutex ) == 0 )
	{
		char msgText [ 64 ];
		sprintf( msgText, "cWin32Mutex::~cWin32Mutex (%d)", GetLastError() );
		throw std::runtime_error( msgText );
	}
}

void cWin32Mutex::lock ()
{
	switch( WaitForSingleObject( _mutex, INFINITE ) )
	{
	case	WAIT_OBJECT_0:
		return;
	
	case	WAIT_ABANDONED:
		throw std::runtime_error( "Got ownership of the abandoned mutex object" );
	
	case	WAIT_FAILED:
		{
			char msgText [ 64 ];
			sprintf( msgText, "cWin32Mutex::lock (%d)", GetLastError() );
			throw std::runtime_error( msgText );
		}
	}
}

bool cWin32Mutex::try_lock ()
{
	switch( WaitForSingleObject( _mutex, 0 ) )
	{
	case	WAIT_OBJECT_0:
		return true;
	
	case	WAIT_ABANDONED:
		throw std::runtime_error( "Got ownership of the abandoned mutex object" );
	
	case	WAIT_FAILED:
		{
			char msgText [ 64 ];
			sprintf( msgText, "cWin32Mutex::try_lock (%d)", GetLastError() );
			throw std::runtime_error( msgText );
		}
	
	case	WAIT_TIMEOUT:
	default:
		return false;
	}
}

void cWin32Mutex::unlock ()
{
	if( ReleaseMutex( _mutex ) == 0 )
	{
		char msgText [ 64 ];
		sprintf( msgText, "cWin32Mutex::unlock (%d)", GetLastError() );
		throw std::runtime_error( msgText );
	}
}

} // namespace CrossClass
