// (c) Aug 30, 2010 Oleg N. Peregudov

#include <crs/bits/posix.cond.h>

namespace CrossClass {

//
// members of class cPosixConditionVariable
//
void cPosixConditionVariable::notify_one ()
{
	int errCode = pthread_cond_signal( &_condition );	// wake up the thread
	if( errCode == EINVAL )
		throw std::runtime_error( "The value cond does not refer to an initialized condition variable" );
}

void cPosixConditionVariable::wait ( _LockIt & lock )
{
	int errCode = pthread_cond_wait( &_condition, static_cast<pthread_mutex_t*>( *(lock.mutex()) ) );
	switch( errCode )
	{
	case	EINVAL:
		throw std::runtime_error( "The value specified by cond or mutex is invalid" );
	case	EPERM:
		throw std::runtime_error( "The mutex was not owned by the current thread at the time of the call" );
	default:
		break;
	}
}

cPosixConditionVariable::cPosixConditionVariable ()
	: _condition( )
{
	int errCode = pthread_cond_init( &_condition, NULL );
	switch( errCode )
	{
	case	EAGAIN:
		throw std::runtime_error(
			"The system lacked the necessary resources (other than memory) "
			"to initialize another condition variable" );
	case	ENOMEM:
		throw std::runtime_error(
			"Insufficient memory exists to initialize the condition variable" );
	case	EBUSY:
		throw std::runtime_error(
			"The implementation has detected an attempt to reinitialize "
			"the object referenced by cond, a previously initialized, "
			"but not yet destroyed, condition variable" );
	case	EINVAL:
		throw std::runtime_error( "The value specified by attr is invalid" );
	
	default:
		break;
	}
}

cPosixConditionVariable::~cPosixConditionVariable ()
{
	int errCode = pthread_cond_destroy( &_condition );
	switch( errCode )
	{
	case	EBUSY:
		throw std::runtime_error(
			"The implementation has detected an attempt to destroy "
			"the object referenced by cond while it is referenced "
			"(for example, while being used in a pthread_cond_wait() "
			"or pthread_cond_timedwait()) by another thread" );
	case	EINVAL:
		throw std::runtime_error( "The value specified by cond is invalid" );
	default:
		break;
	}
}

} // namespace CrossClass

