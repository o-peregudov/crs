/*
 *  crs/bits/posix.timer.cpp
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

#include <crs/bits/posix.timer.h>

namespace CrossClass {

cPosixTimer::cPosixTimer ()
	: _tmStart ()
	, _actual ()
{
	reset ();
}

const cPosixTimer::host_time_type & cPosixTimer::startTime () const
{
	return _tmStart;
}

double cPosixTimer::getStamp ()
{
	clock_gettime (CLOCK_REALTIME, &_actual);
	double delta = _actual.tv_sec - _tmStart.tv_sec;
	if( delta > 1 )
		delta += 1.0 - static_cast<double> (_tmStart.tv_nsec) / 1e9 + static_cast<double> (_actual.tv_nsec) / 1e9;
	else
		delta += static_cast<double> (_actual.tv_nsec - _tmStart.tv_nsec) / 1e9;
	return delta;
}

void cPosixTimer::reset ()
{
	clock_gettime (CLOCK_REALTIME, &_tmStart);
}

} /* namespace CrossClass */
