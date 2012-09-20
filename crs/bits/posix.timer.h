#ifndef CROSS_POSIX_TIMER_H_INCLUDED
#define CROSS_POSIX_TIMER_H_INCLUDED 1
/*
 *  crs/bits/posix.timer.h
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
 *	2010/08/23	interface for the cTimer class (POSIX API)
 *	2012/06/03	optimized version
 */

#include <crs/libexport.h>
#include <ctime>

namespace CrossClass {

/*
 * class Hi-performance timer
 */

class CROSS_EXPORT cPosixTimer
{
public:
	typedef timespec	host_time_type;
	
protected:
	host_time_type	_tmStart,
				_actual;
	
public:
	cPosixTimer ();
	
	void reset ();
	double getStamp ();
	const host_time_type & startTime () const;
};

}	/* namespace CrossClass			*/
#endif/* CROSS_POSIX_TIMER_H_INCLUDED	*/
