/*
 *  crs/bits/win32.cond.cpp
 *  Copyright (c) 2010-2013 Oleg N. Peregudov <o.peregudov@gmail.com>
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
#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif
#include <crs/bits/win32.cond.h>

namespace CrossClass {

//
// members of class cWin32ConditionVariable
//
  cWin32ConditionVariable::cWin32ConditionVariable ()
    : _semaphore (NULL)
    , _num_waiters (0)
    , _spinlock ()
  {
    _semaphore = CreateSemaphore (NULL, 0, 0, NULL);
    if (_semaphore == NULL)
      {
	char msgText [ 64 ];
	sprintf (msgText, "cWin32ConditionVariable::cWin32ConditionVariable (%d)", GetLastError ());
	throw std::runtime_error (msgText);
      }
  }

  cWin32ConditionVariable::~cWin32ConditionVariable ()
  {
    BOOL result = CloseHandle (_semaphore);
#if defined (DESTRUCTOR_EXCEPTIONS_ALLOWED)
    if (result == 0)
      {
	char msgText [ 64 ];
	sprintf (msgText, "~cWin32ConditionVariable (%d)", GetLastError ());
	throw std::runtime_error (msgText);
      }
#endif
  }
  
  void cWin32ConditionVariable::notify_one ()
  {
    int res = 1;
    _spinlock.lock ();
    if (_num_waiters > 0)
      {
	res = ReleaseSemaphore (_semaphore, 1, NULL);
      }
    _spinlock.unlock ();
    if (res == 0)
      {
	char msgText [ 64 ];
	sprintf (msgText, "cWin32Semaphore::notify_one (%d)", GetLastError ());
	throw std::runtime_error (msgText);
      }
  }
  
  void cWin32ConditionVariable::notify_all ()
  {
    int res = 1;
    _spinlock.lock ();
    if (_num_waiters > 0)
      {
	res = ReleaseSemaphore (_semaphore, _num_waiters, NULL);
      }
    _spinlock.unlock ();
    if (res == 0)
      {
	char msgText [ 64 ];
	sprintf (msgText, "cWin32Semaphore::notify_all (%d)", GetLastError ());
	throw std::runtime_error (msgText);
      }
  }

} /* namespace CrossClass	*/
