#ifndef CROSS_WIN32_TIMER_H_INCLUDED
#define CROSS_WIN32_TIMER_H_INCLUDED 1
//
// win32.timer.h: base interface for the cTimer class (Win32 API).
// (c) Aug 24, 2010 Oleg N. Peregudov

#include <crs/libexport.h>
#include <windows.h>
#include <stdexcept>

namespace CrossClass {

//
// class Hi-performance timer
//

class CROSS_EXPORT cWin32Timer
{
public:
	typedef SYSTEMTIME host_time_type;
	
	struct noHardware : std::runtime_error {
		noHardware () : std::runtime_error( "no performance counter available" ) { }
	};
	
protected:
	host_time_type	_tmStart;
	double		_timeFromStart;
	LARGE_INTEGER	_recent,
				_actual,
				_freq;
	
public:
	cWin32Timer();
	~cWin32Timer();
	
	void reset ();
	const double & getStamp ();
	const host_time_type & startTime () const;
};

} // namespace CrossClass
#endif // CROSS_WIN32_TIMER_H_INCLUDED
