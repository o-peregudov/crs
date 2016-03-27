#ifndef CROSS_WIN32_LOCK_H_INCLUDED
#define CROSS_WIN32_LOCK_H_INCLUDED 1
/*
 *  crs/bits/win32.mutex.h
 *  Copyright (c) 2008-2013 Oleg N. Peregudov <o.peregudov@gmail.com>
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
 *	2010/08/24	wrapper for Win32 mutex
 *	2010/08/27	compartibility with std::mutex from C++0x standard
 *	2010/09/18	uniform error handling
 *	2012/08/19	cWin32Mutex::native_handle method
 *			follows to the general destructor exceptions macro
 *	2013/09/21	ver. 2.0.0 refactoring
 */

#include <crs/libexport.h>
#include <stdexcept>

namespace CrossClass
{
  class CROSS_EXPORT cWin32Mutex
  {
  protected:
    HANDLE _mutex;
    
  public:
    typedef HANDLE native_handle_type;
    
    cWin32Mutex ();
    ~cWin32Mutex ();
    
    void lock ();
    bool try_lock ();
    void unlock ();
    
    native_handle_type native_handle ()
    {
      return _mutex;
    }
  };
}	/* namespace CrossClass		*/
#endif	/* CROSS_WIN32_LOCK_H_INCLUDED	*/
