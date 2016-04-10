/*
 *  crs/bits/win32.mutex.cpp
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

#if defined( HAVE_CONFIG_H )
#  include "config.h"
#endif
#include <crs/bits/win32.mutex.h>
#include <cstdio>

namespace CrossClass
{
  /*
   * members of class win32_mutex
   */

  win32_mutex::win32_mutex ()
    : _mutex( NULL )
  {
    _mutex = CreateMutex (NULL, FALSE, NULL);
    if (_mutex == NULL)
      {
	char msgText [ 64 ];
	snprintf (msgText,
		  sizeof (msgText) / sizeof (char),
		  "win32_mutex::win32_mutex (%lu)",
		  GetLastError ());
	throw std::runtime_error (msgText);
      }
  }

  win32_mutex::~win32_mutex ()
  {
    try
      {
	BOOL result = CloseHandle (_mutex);
	if (result == 0)
	  {
	    char msgText [ 64 ];
	    snprintf (msgText,
		      sizeof (msgText) / sizeof (char),
		      "~win32_mutex (%lu)",
		      GetLastError ());
	    throw std::runtime_error (msgText);
	  }
      }
    catch (...)
      {
#if defined (DESTRUCTOR_EXCEPTIONS_ALLOWED)
	throw;
#endif
      }
  }

  void win32_mutex::lock ()
  {
    DWORD rv = WaitForSingleObject (_mutex, INFINITE);
    if (rv == WAIT_ABANDONED)
      {
	throw std::runtime_error ("Got ownership of the abandoned mutex object");
      }

    if (rv == WAIT_FAILED)
      {
	char msgText [ 64 ];
	snprintf (msgText,
		  sizeof (msgText) / sizeof (char),
		  "win32_mutex::lock (%lu)",
		  GetLastError ());
	throw std::runtime_error (msgText);
      }
  }

  bool win32_mutex::try_lock ()
  {
    DWORD rv = WaitForSingleObject (_mutex, 0);
    if (rv == WAIT_OBJECT_0)
      {
	return true;
      }

    if (rv == WAIT_ABANDONED)
      {
	throw std::runtime_error ("Got ownership of the abandoned mutex object");
      }

    if (rv == WAIT_FAILED)
      {
	char msgText [ 64 ];
	snprintf (msgText,
		  sizeof (msgText) / sizeof (char),
		  "win32_mutex::try_lock (%lu)",
		  GetLastError ());
	throw std::runtime_error (msgText);
      }

    return false; /* WAIT_TIMEOUT is also here */
  }

  void win32_mutex::unlock ()
  {
    if (ReleaseMutex (_mutex) == 0)
      {
	char msgText [ 64 ];
	snprintf (msgText,
		  sizeof (msgText) / sizeof (char),
		  "win32_mutex::unlock (%lu)",
		  GetLastError ());
	throw std::runtime_error (msgText);
      }
  }

} /* namespace CrossClass */
