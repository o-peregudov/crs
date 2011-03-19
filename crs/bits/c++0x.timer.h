#ifndef CROSS_CPP0X_TIMER_H_INCLUDED
#define CROSS_CPP0X_TIMER_H_INCLUDED 1
//
// c++0x.timer.h: interface for the cTimer class (STL).
// (c) Aug 23, 2010 Oleg N. Peregudov
//

#include <crs/libexport.h>
#include <chrono>

namespace CrossClass {

//
// class Hi-performance timer
//

class CROSS_EXPORT cCpp0xTimer
{
public:
	typedef std::chrono::monotonic_clock clock_type;
	typedef clock_type::time_point host_time_type;
	
protected:
	host_time_type	_tmStart;
	double		_timeFromStart;
	clock_type		_mtc;
	host_time_type	_recent,
				_actual;
	
public:
	cCpp0xTimer();
	~cCpp0xTimer();
	
	void reset ();
	const double & getStamp ();
	const host_time_type & startTime () const;
};

} // namespace CrossClass
#endif // CROSS_CPP0X_TIMER_H_INCLUDED

