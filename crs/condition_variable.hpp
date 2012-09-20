#ifndef CROSS_CONDITION_VARIABLE_HPP_INCLUDED
#define CROSS_CONDITION_VARIABLE_HPP_INCLUDED 1
/*
 *  CONDITION_VARIABLE.HPP
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
 *	2010/08/30	wrapper for condition variable
 *	2012/08/16	new platform specific defines
 */

#include <crs/libexport.h>
#include <crs/security.h>

#if USE_CXX11_CONDITION_VARIABLE
#	include <condition_variable>
#	include <chrono>
namespace CrossClass
{
	class CROSS_EXPORT cConditionVariable : public std::condition_variable
	{
	public:
		cConditionVariable ()
			: std::condition_variable ()
		{ }
		
		template <class Predicate>
		bool wait_for ( _LockIt & lock, const unsigned long msTimeOut, Predicate pred )
		{
			return std::condition_variable::wait_for (lock, std::chrono::milliseconds (msTimeOut), pred);
		}
	};
}
#elif USE_WIN32_API
#	include <crs/bits/win32.cond.h>
namespace CrossClass
{
	typedef cWin32ConditionVariable	cConditionVariable;
}
#elif USE_POSIX_API
#	include <crs/bits/posix.cond.h>
namespace CrossClass
{
	typedef cPosixConditionVariable	cConditionVariable;
}
#endif
#endif // CROSS_CONDITION_VARIABLE_HPP_INCLUDED
