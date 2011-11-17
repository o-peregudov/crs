#ifndef CROSS_TIMER_H_INCLUDED
#define CROSS_TIMER_H_INCLUDED 1
//
// Timer.h: interface for the cTimer class.
// (c) Nov 21, 2007 Oleg N. Peregudov
// Jan 3, 2009
// Apr 23, 2009 - Win/Posix defines
// Oct 21, 2009 - C++0x timers
// Jun 25, 2010 - C++0x mutexes
// Aug 23, 2010 - separate versions for each API
//

#include <string>
#include <crs/libexport.h>

#if defined( USE_WIN32_API )
#	include <crs/bits/win32.timer.h>
#elif defined( USE_CXX0X_API )
#	include <crs/bits/c++0x.timer.h>
#elif defined( USE_POSIX_API )
#	include <crs/bits/posix.timer.h>
#endif

namespace CrossClass {

#if defined( USE_WIN32_API )
	typedef cWin32Timer	cHostTimerType;
#elif defined( USE_CXX0X_API )
	typedef cCpp0xTimer	cHostTimerType;
#elif defined( USE_POSIX_API )
	typedef cPosixTimer	cHostTimerType;
#endif

//
// class Hi-performance timer
//

class CROSS_EXPORT cTimer : public cHostTimerType
{
public:
	typedef cHostTimerType::host_time_type host_time_type;
	
	cTimer() : cHostTimerType( )	{ }
	~cTimer()				{ }
      
	const double & operator () ()	{ return getStamp(); }
	static std::string time2string ( const double & tm, const bool fMSecs = true );
};

} // namespace CrossClass
#endif // CROSS_TIMER_H_INCLUDED

