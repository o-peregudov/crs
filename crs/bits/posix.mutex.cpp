// (c) Nov 29, 2007 Oleg N. Peregudov
// Aug 24, 2010 - POSIX mutex envelope
// Aug 28, 2010 - compartibility with std::mutex from C++0x standard

#include <crs/bits/posix.mutex.h>
#include <cstdio>
#include <errno.h>

namespace CrossClass {

//
// members of class cPosixMutex
//

cPosixMutex::cPosixMutex ()
	: _mutex( )
	, _attr( )
{
	int errCode ( 0 );
	char msgText [ 64 ];
	
	if( ( errCode = pthread_mutexattr_init( &_attr ) ) != 0 )
	{
		sprintf( msgText, "pthread_mutexattr_init returns 0x%X", errCode );
		throw std::runtime_error( msgText );
	}
	
	if( ( errCode = pthread_mutexattr_settype( &_attr, PTHREAD_MUTEX_NORMAL ) ) != 0 )
	{
		sprintf( msgText, "pthread_mutexattr_settype returns 0x%X", errCode );
		throw std::runtime_error( msgText );
	}
	
	if( ( errCode = pthread_mutex_init( &_mutex, &_attr ) ) != 0 )
	{
		sprintf( msgText, "pthread_mutex_init returns 0x%X", errCode );
		throw std::runtime_error( msgText );
	}
}

cPosixMutex::~cPosixMutex ()
{
	int errCode ( 0 );
	char msgText [ 64 ];
	
	if( ( errCode = pthread_mutex_destroy( &_mutex ) ) != 0 )
	{
		sprintf( msgText, "pthread_mutex_destroy returns 0x%X", errCode );
		throw std::runtime_error( msgText );
	}
	
	if( ( errCode = pthread_mutexattr_destroy( &_attr ) ) != 0 )
	{
		sprintf( msgText, "pthread_mutexattr_destroy returns 0x%X", errCode );
		throw std::runtime_error( msgText );
	}
}

void cPosixMutex::lock ()
{
	int errCode ( 0 );
	if( ( errCode = pthread_mutex_lock( &_mutex ) ) != 0 )
	{
		char msgText [ 64 ];
		sprintf( msgText, "pthread_mutex_lock returns 0x%X", errCode );
		throw std::runtime_error( msgText );
	}
}

bool cPosixMutex::try_lock ()
{
	return ( pthread_mutex_trylock( &_mutex ) == 0 );
}

void cPosixMutex::unlock ()
{
	int errCode ( 0 );
	if( ( errCode = pthread_mutex_unlock( &_mutex ) ) != 0 )
	{
		char msgText [ 64 ];
		sprintf( msgText, "pthread_mutex_unlock returns 0x%X", errCode );
		throw std::runtime_error( msgText );
	}
}

} // namespace CrossClass

