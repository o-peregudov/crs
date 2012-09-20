#ifndef CROSS_WIN32_TIMER_H_INCLUDED
#define CROSS_WIN32_TIMER_H_INCLUDED 1
/*
 *  crs/bits/win32.timer.h
 *  Copyright (c) 2010-2012 Oleg N. Peregudov <o.peregudov@gmail.com>
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
 *	2010/08/24	base interface for the cTimer class (Win32 API).
 *	2012/06/03	optimized version
 */
#include <crs/libexport.h>
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
		noHardware () : std::runtime_error ("no performance counter available") { }
	};
	
protected:
	host_time_type	_tmStart;
	LARGE_INTEGER	_recent,
				_actual,
				_freq;
	
public:
	cWin32Timer ();
	
	void reset ();
	double getStamp ();
	const host_time_type & startTime () const;
};

}	/* namespace CrossClass			*/
#endif/* CROSS_WIN32_TIMER_H_INCLUDED	*/
