/*
 *  crs/semaphore.cpp
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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "crs/semaphore.h"
#include <limits>

namespace crs
{
  semaphore::semaphore (const unsigned int cnt)
    : _mutex ()
    , _condition ()
    , _counter (cnt)
  { }

  void semaphore::post (const unsigned int inc)
  {
    lock_type guard (_mutex);
    const unsigned int allowed =
      std::numeric_limits<unsigned int>::max () - _counter;
    _counter += (inc < allowed) ? inc : allowed;
    _condition.notify_one ();
  }

  void semaphore::wait ()
  {
    lock_type guard (_mutex);
    _condition.wait (guard, [this]{ return (0 < _counter); });
    --_counter;
  }
} /* namespace crs */
