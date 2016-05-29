#ifndef CRS_SEMAPHORE_H_INCLUDED
#define CRS_SEMAPHORE_H_INCLUDED 1
/*
 *  crs/semaphore.h
 *  Copyright (c) 2010-2016 Oleg N. Peregudov <o.peregudov@gmail.com>
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

#include <crs/libexport.h>
#include <crs/mutex.h>
#include <crs/condition_variable.h>

namespace crs
{
  class semaphore
  {
    mutex_type   _mutex;
    condvar_type _condition;
    unsigned int _counter;

    semaphore (const semaphore &) = delete;
    semaphore & operator = (const semaphore &) = delete;

  public:
    explicit semaphore (const unsigned int cnt = 0);

    void post (const unsigned int inc = 1);
    void wait ();

    template< class Rep, class Period>
    bool wait_for (const std::chrono::duration<Rep, Period> & rel_time)
    {
      lock_type guard (_mutex);
      if (_condition.wait_for (guard, rel_time, [this]{ return (0 < _counter); }))
	{
	  --_counter;
	  return true;
	}
      return false;
    }

    template< class Clock, class Duration, class Predicate >
    bool wait_until (const std::chrono::time_point<Clock, Duration> & abs_time)
    {
      lock_type guard (_mutex);
      if (_condition.wait_until (guard, abs_time, [this]{ return (0 < _counter); }))
	{
	  --_counter;
	  return true;
	}
      return false;
    }
  };
} /* namespace crs */
#endif /* CRS_SEMAPHORE_H_INCLUDED */
