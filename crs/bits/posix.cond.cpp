// (c) Aug 30, 2010 Oleg N. Peregudov
//
//	09/18/2010	expand errors using strerror function
//	01/03/2011	integer types
//	01/21/2011	extended error info
//

#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif

#include <crs/bits/posix.cond.h>
#include <cstdio>

namespace CrossClass {

//
// members of class cPosixConditionVariable
//
void cPosixConditionVariable::notify_one ()
{
	int errCode = pthread_cond_signal( &_condition );	// wake up the thread
	if( errCode )
	{
		char errText [ 512 ];
		sprintf( errText, "%d: %s (cPosixConditionVariable::notify_one)", errCode, strerror( errCode ) );
		throw std::runtime_error( errText );
	}
}

void cPosixConditionVariable::wait ( _LockIt & lock )
{
	int errCode = pthread_cond_wait( &_condition, static_cast<pthread_mutex_t*>( *(lock.mutex()) ) );
	if( errCode )
	{
		char errText [ 512 ];
		sprintf( errText, "%d: %s (cPosixConditionVariable::wait)", errCode, strerror( errCode ) );
		throw std::runtime_error( errText );
	}
}

cPosixConditionVariable::cPosixConditionVariable ()
	: _condition( )
{
	int errCode = pthread_cond_init( &_condition, NULL );
	if( errCode )
	{
		char errText [ 512 ];
		sprintf( errText, "%d: %s (cPosixConditionVariable::constructor)", errCode, strerror( errCode ) );
		throw std::runtime_error( errText );
	}
}

cPosixConditionVariable::~cPosixConditionVariable ()
{
	int errCode = pthread_cond_destroy( &_condition );
	if( errCode )
	{
		char errText [ 512 ];
		sprintf( errText, "%d: %s (cPosixConditionVariable::destructor)", errCode, strerror( errCode ) );
		throw std::runtime_error( errText );
	}
}

} // namespace CrossClass
