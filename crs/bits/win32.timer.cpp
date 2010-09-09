//
// win32.timer.cpp: interface for the cTimer class (Win32 API)
// (c) Aug 23, 2010 Oleg N. Peregudov
//

#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#endif
#include <crs/bits/win32.timer.h>

namespace CrossClass {

cWin32Timer::cWin32Timer()
	: _tmStart()
	, _timeFromStart( 0 )
	, _recent()
	, _actual()
	, _freq()
{
	if( !QueryPerformanceFrequency( &_freq ) )
		throw noHardware();
	reset();
}

cWin32Timer::~cWin32Timer()
{
}

const cWin32Timer::host_time_type & cWin32Timer::startTime () const
{
	return _tmStart;
}

const double & cWin32Timer::getStamp ()
{
	QueryPerformanceCounter( &_actual );
	_timeFromStart += static_cast<double>( _actual.QuadPart - _recent.QuadPart ) / _freq.QuadPart;
	_recent.QuadPart = _actual.QuadPart;
	return _timeFromStart;
}

void  cWin32Timer::reset ()
{
	QueryPerformanceCounter( &_recent );
	GetSystemTime( &_tmStart );
	_timeFromStart = 0;
}

} // namespace CrossClass

