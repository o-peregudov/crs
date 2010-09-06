// (c) Nov 29, 2007 Oleg N. Peregudov
// Aug 23, 2010 - Win32 mutex envelope
// Aug 28, 2010 - compartibility with std::mutex from C++0x standard
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#	pragma warning( disable : 4996 )
#endif
#include <crs/bits/win32.mutex.h>
#include <cstdlib>

namespace CrossClass {

//
// members of class cWin32Mutex
//

cWin32Mutex::cWin32Mutex ()
	: _mutex( CreateMutex( NULL, FALSE, NULL ) )
{ }

cWin32Mutex::~cWin32Mutex ()
{
	CloseHandle( _mutex );
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
			sprintf( msgText, "mutex wait failed with code 0x%X", GetLastError() );
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
			sprintf( msgText, "mutex wait failed with code 0x%X", GetLastError() );
			throw std::runtime_error( msgText );
		}
	
	case	WAIT_TIMEOUT:
	default:
		return false;
	}
}

void cWin32Mutex::unlock ()
{
	ReleaseMutex( _mutex );
}

} // namespace CrossClass
