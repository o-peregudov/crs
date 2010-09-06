//
// posix.imer.cpp: interface for the cTimer class (POSIX API).
// (c) Aug 23, 2010 Oleg N. Peregudov
//

#include <crs/bits/posix.timer.h>
#include <cstring>

namespace CrossClass {

cPosixTimer::cPosixTimer()
	: _tmStart()
	, _timeFromStart( 0 )
	, _recent()
	, _actual()
{
	reset();
}

cPosixTimer::~cPosixTimer()
{
}

const cPosixTimer::host_time_type & cPosixTimer::startTime () const
{
	return _tmStart;
}

const double & cPosixTimer::getStamp ()
{
	clock_gettime( CLOCK_REALTIME, &_actual );
	double delta = _actual.tv_sec - _recent.tv_sec;
	if( delta > 1 )
		delta += 1.0 - static_cast<double>( _recent.tv_nsec ) / 1e9 + static_cast<double>( _actual.tv_nsec ) / 1e9;
	else
		delta += static_cast<double>( _actual.tv_nsec - _recent.tv_nsec ) / 1e9;
	memcpy( &_recent, &_actual, sizeof( timespec ) );
	_timeFromStart += delta;
	return _timeFromStart;
}

void  cPosixTimer::reset ()
{
	clock_gettime( CLOCK_REALTIME, &_tmStart );
	memcpy( &_recent, &_tmStart, sizeof( timespec ) );
	_timeFromStart = 0;
}

} // namespace CrossClass

