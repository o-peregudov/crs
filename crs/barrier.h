#ifndef CRS_BARRIER_H_INCLUDED
#define CRS_BARRIER_H_INCLUDED 1
/*
 *  crs/barrier.h
 *  Copyright (c) 2009-2016 Oleg N. Peregudov <o.peregudov@gmail.com>
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
  class barrier
  {
    mutex_type   _mutex;
    condvar_type _condition;
    unsigned int _counter;

    barrier () = delete;
    barrier (const barrier &) = delete;
    barrier & operator = (const barrier &) = delete;

  public:
    explicit barrier (const unsigned int);
    void wait ();

    template <class Rep, class Period>
    bool wait_for (const std::chrono::duration<Rep, Period> & rel_time)
    {
      lock_type guard (_mutex);

      if (_counter == 0)
        {
          return true;
        }

      if (--_counter == 0)
        {
          _condition.notify_all ();
          return true;
        }

      return _condition.wait_for (guard, rel_time, [this]{ return (0 == _counter); });
    }

    template <class Clock, class Duration>
    bool wait_until (const std::chrono::time_point<Clock, Duration> & abs_time)
    {
      lock_type guard (_mutex);

      if (_counter == 0)
        {
          return true;
        }

      if (--_counter == 0)
        {
          _condition.notify_all ();
          return true;
        }

      return _condition.wait_until (guard, abs_time, [this]{ return (0 == _counter); });
    }
  };
} /* namespace crs */
#endif /* CRS_BARRIER_H_INCLUDED */
