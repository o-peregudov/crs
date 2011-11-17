//
// barrier.cpp - implementation of the class cBarrier
// (c) Feb 20, 2009 Oleg N. Peregudov
//

#include "cross/barrier.h"
#include <sstream>

namespace CrossClass
{

#if defined( __GNUG__ )
cBarrier::cBarrier( const unsigned count )
	: _barrier( )
{
	int errCode ( 0 )
	if( ( errCode = pthread_barrier_init( &_barrier, NULL, count ) ) )
	{
		std::basic_ostringstream<char> errMsg;
		errMsg << "pthread_barrier_init returns " << errCode;
		throw std::runtime_error( errMsg.str() );
	}
}

cBarrier::~cBarrier ()
{
	int errCode ( 0 )
	if( ( errCode = pthread_barrier_destroy( &_barrier ) ) )
	{
		std::basic_ostringstream<char> errMsg;
		errMsg << "pthread_barrier_destroy returns " << errCode;
		throw std::runtime_error( errMsg.str() );
	}
}

bool cBarrier::operator () ()
{
	int errCode = pthread_barrier_wait( &_barrier );
	switch( errCode )
	{
	case	0:
		break;
	
	case	PTHREAD_BARRIER_SERIAL_THREAD:
		return true;
	
	case	EINVAL:
		throw std::runtime_error( "barrier was not initialized" );
	
	default:
		throw std::runtime_error( "unexpected code from pthread_barrier_wait" );
	}
	return false;
}

#elif defined( _MSC_VER )
	
#endif
	
} // namespace CrossClass
