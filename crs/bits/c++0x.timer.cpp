//
// c++0x.timer.cpp: interface for the cTimer class (STL).
// (c) Aug 23, 2010 Oleg N. Peregudov
//

#include <crs/bits/c++0x.timer.h>

namespace CrossClass {

cCpp0xTimer::cCpp0xTimer()
	: _tmStart()
	, _timeFromStart( 0 )
	, _mtc()
	, _recent()
	, _actual()
{
	_recent = _actual = _mtc.now();
	reset();
}

cCpp0xTimer::~cCpp0xTimer()
{
}

const cCpp0xTimer::host_time_type & cCpp0xTimer::startTime () const
{
	return _tmStart;
}

const double & cCpp0xTimer::getStamp ()
{
	_actual = _mtc.now();
	_timeFromStart = static_cast<double>( ( _actual - _recent ).count() ) / clock_type::period().den;
	_recent = _actual;
	return _timeFromStart;
}

void  cCpp0xTimer::reset ()
{
	_tmStart = _recent = _mtc.now();
	_timeFromStart = 0;
}

} // namespace CrossClass

