/*
 *  TIMER.CPP - high performance timer
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
 *	2007/05/22	high performance timer
 *	2007/10/17	heavy usage of C++ library
 *	2007/11/21	new place for cross-compiling routines
 *	2009/04/23	Win/Posix defines
 *	2009/10/21	C++0x timers
 *	2010/06/25	C++0x mutexes
 *	2010/08/21	separate versions for each API
 */

#include <crs/timer.h>
#include <sstream>
#include <iomanip>

namespace CrossClass {

std::string cTimer::time2string ( const double & tm, const bool fMSecs )
{
	double msTime = tm;
	std::ostringstream stream;
	int nHours = static_cast<int> (msTime / 3.6e6);
	msTime -= nHours * 3.6e6;
	int nMinutes = static_cast<int> (msTime / 6e4);
	msTime -= nMinutes * 6e4;
	int nSeconds = static_cast<int> (msTime / 1e3);
	stream.fill ('0');
	stream << nHours << ':' << std::setw (2) << nMinutes << ':' << std::setw (2) << nSeconds;
	if( fMSecs )
	{
		msTime -= nSeconds * 1e3;
		stream << '.' << std::setw (3) << static_cast<int> (msTime);
	}
	return stream.str();
}

} // namespace CrossClass

