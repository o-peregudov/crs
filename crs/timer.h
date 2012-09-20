#ifndef CROSS_TIMER_H_INCLUDED
#define CROSS_TIMER_H_INCLUDED 1
/*
 *  TIMER.H - high performance timer
 *  Copyright (c) 2007-2012 Oleg N. Peregudov <o.peregudov@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 *	2007/11/21	high performance timer
 *	2009/04/23	Win/Posix defines
 *	2009/10/21	C++0x timers
 *	2010/06/25	C++0x mutexes
 *	2010/08/23	separate versions for each API
 *	2012/06/03	C++11 standard
 */

#include <string>
#include <crs/libexport.h>

#if USE_CXX11_CHRONO
#	include <chrono>

namespace CrossClass {

class CROSS_EXPORT cCXX11Timer
{
public:
	typedef std::chrono::high_resolution_clock clock_type;
	typedef clock_type::time_point host_time_type;
	
protected:
	host_time_type _tmStart;
	
public:
	cCXX11Timer ()
		: _tmStart (clock_type::now ())
	{
	}
	
	void reset ()
	{
		_tmStart = clock_type::now ();
	}
	
	double getStamp ()
	{
		return (clock_type::now () - _tmStart).count () / static_cast<double> (clock_type::period ().den);
	}
	
	const host_time_type & startTime () const
	{
		return _tmStart;
	}
};

	typedef cCXX11Timer cHostTimerType;
}
#elif USE_WIN32_API
#	include <crs/bits/win32.timer.h>
namespace CrossClass {
	typedef cWin32Timer cHostTimerType;
}
#elif USE_POSIX_API
#	include <crs/bits/posix.timer.h>
namespace CrossClass {
	typedef cPosixTimer cHostTimerType;
}
#endif

namespace CrossClass {

/*
 * class Hi-performance timer
 */

class CROSS_EXPORT cTimer : public cHostTimerType
{
public:
	typedef cHostTimerType::host_time_type host_time_type;
	
	cTimer ()
		: cHostTimerType ()
	{ }
	
	static std::string time2string ( const double & tm, const bool fMSecs = true );
};

}	/* namespace CrossClass		*/
#endif/* CROSS_TIMER_H_INCLUDED	*/

