#ifndef CROSS_POSIX_TIMER_H_INCLUDED
#define CROSS_POSIX_TIMER_H_INCLUDED 1
//
// posix.timer.h: interface for the cTimer class (POSIX API).
// (c) Aug 23, 2010 Oleg N. Peregudov
//

#include <crs/libexport.h>
#include <ctime>

namespace CrossClass {

//
// class Hi-performance timer
//

class CROSS_EXPORT cPosixTimer
{
public:
	typedef timespec	host_time_type;
	
protected:
	host_time_type	_tmStart;
	double		_timeFromStart;
	host_time_type	_recent,
				_actual;
	
public:
	cPosixTimer();
	~cPosixTimer();
	
	void reset ();
	const double & getStamp ();
	const host_time_type & startTime () const;
};

} // namespace CrossClass
#endif // CROSS_POSIX_TIMER_H_INCLUDED

