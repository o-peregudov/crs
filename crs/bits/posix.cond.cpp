// (c) Aug 30, 2010 Oleg N. Peregudov
//
//	09/18/2010	expand errors using strerror function
//

#include <crs/bits/posix.cond.h>

namespace CrossClass {

//
// members of class cPosixConditionVariable
//
void cPosixConditionVariable::notify_one ()
{
	int errCode = pthread_cond_signal( &_condition );	// wake up the thread
	if( errCode )
		throw std::runtime_error( strerror( errCode ) );
}

void cPosixConditionVariable::wait ( _LockIt & lock )
{
	int errCode = pthread_cond_wait( &_condition, static_cast<pthread_mutex_t*>( *(lock.mutex()) ) );
	if( errCode )
		throw std::runtime_error( strerror( errCode ) );
}

cPosixConditionVariable::cPosixConditionVariable ()
	: _condition( )
{
	int errCode = pthread_cond_init( &_condition, NULL );
	if( errCode )
		throw std::runtime_error( strerror( errCode ) );
}

cPosixConditionVariable::~cPosixConditionVariable ()
{
	int errCode = pthread_cond_destroy( &_condition );
	if( errCode )
		throw std::runtime_error( strerror( errCode ) );
}

} // namespace CrossClass

