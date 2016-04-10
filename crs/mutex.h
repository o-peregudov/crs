#ifndef CROSS_MUTEX_H_INCLUDED
#define CROSS_MUTEX_H_INCLUDED 1
/*
 *  crs/mutex.h
 *  Copyright (c) 2008-2016 Oleg N. Peregudov <o.peregudov@gmail.com>
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
 *	2008/01/22	wrapper for mutex object
 *	2010/08/26	separate versions for each API
 *	2010/08/28	compartibility with std::mutex from C++0x standard
 *	2012/08/18	new host class selection
 *	2013/09/21	ver. 2.0.0 refactoring
 *	2016/04/10	ver. 2.0.0 refactoring
 */

#include <crs/libexport.h>

#if USE_CXX11_MUTEX
#  include <mutex>
#  define LIBCRS_DEFINE_LOCKS 0

#elif PLATFORM_MINGW
#  include <crs/bits/win32.mutex.h>
#  define LIBCRS_DEFINE_LOCKS 1
namespace std
{
  typedef CrossClass::win32_mutex mutex;
}

#else
#  include <crs/bits/posix.mutex.h>
#  define LIBCRS_DEFINE_LOCKS 1
namespace std
{
  typedef CrossClass::posix_mutex mutex;
}
#endif /* USE_CXX11_MUTEX */

#if LIBCRS_DEFINE_LOCKS
namespace std
{
  struct defer_lock_t { };
  struct adopt_lock_t { };
  struct try_to_lock_t { };

  template <class Mutex>
  class lock_guard
  {
    lock_guard (const lock_guard &);
    lock_guard & operator = (const lock_guard &);

  public:
    typedef Mutex mutex_type;

    lock_guard (mutex_type & m)
      : pm (m)
    {
      pm.lock ();
    }

    lock_guard (mutex_type & m, adopt_lock_t)
      : pm (m)
    {
    }

    ~lock_guard ()
    {
      pm.unlock ();
    }

  private:
    mutex_type & pm;
  };
} /* namespace std */

#endif /* LIBCRS_DEFINE_LOCKS */
#undef LIBCRS_DEFINE_LOCKS

#endif /* CROSS_MUTEX_H_INCLUDED */
