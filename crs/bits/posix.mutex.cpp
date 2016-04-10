/*
 *  crs/bits/posix.mutex.cpp
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

#include <crs/bits/posix.mutex.h>
#include <cstdio>
#include <cerrno>

namespace CrossClass
{

  /*
   * members of class posix_mutex
   */

  posix_mutex::posix_mutex ()
    : _mutex( )
    , _attr( )
  {
    int errCode = pthread_mutexattr_init (&_attr);
    if (errCode)
      {
	char msgText [ 64 ];
	snprintf (msgText,
		  sizeof (msgText) / sizeof (char),
		  "pthread_mutexattr_init returns 0x%X",
		  errCode);
	throw std::runtime_error (msgText);
      }

    errCode = pthread_mutexattr_settype (&_attr, PTHREAD_MUTEX_NORMAL);
    if (errCode)
      {
	char msgText [ 64 ];
	snprintf (msgText,
		  sizeof (msgText) / sizeof (char),
		  "pthread_mutexattr_settype returns 0x%X",
		  errCode);
	throw std::runtime_error (msgText);
      }

    errCode = pthread_mutex_init (&_mutex, &_attr);
    if (errCode)
      {
	char msgText [ 64 ];
	snprintf (msgText,
		  sizeof (msgText) / sizeof (char),
		  "pthread_mutex_init returns 0x%X",
		  errCode);
	throw std::runtime_error (msgText);
      }
  }

  posix_mutex::~posix_mutex ()
  {
    try
      {
	int errCode = pthread_mutex_destroy (&_mutex);
	if (errCode)
	  {
	    char msgText [ 64 ];
	    snprintf (msgText,
		      sizeof (msgText) / sizeof (char),
		      "pthread_mutex_destroy returns 0x%X",
		      errCode);
	    throw std::runtime_error (msgText);
	  }

	errCode = pthread_mutexattr_destroy (&_attr);
	if (errCode)
	  {
	    char msgText [ 64 ];
	    snprintf (msgText,
		      sizeof (msgText) / sizeof (char),
		      "pthread_mutexattr_destroy returns 0x%X",
		      errCode);
	    throw std::runtime_error (msgText);
	  }
      }
    catch (...)
      {
#if DESTRUCTOR_EXCEPTIONS_ALLOWED
	throw;
#endif
      }
  }

  void posix_mutex::lock ()
  {
    int errCode = pthread_mutex_lock (&_mutex);
    if (errCode)
      {
	char msgText [ 64 ];
	snprintf (msgText,
		  sizeof (msgText) / sizeof (char),
		  "pthread_mutex_lock returns 0x%X",
		  errCode);
	throw std::runtime_error (msgText);
      }
  }

  bool posix_mutex::try_lock ()
  {
    int errCode = pthread_mutex_trylock (&_mutex);
    if (errCode == 0)
      {
	return true;
      }

    if ((errCode != EAGAIN) && (errCode != EBUSY))
      {
	char msgText [ 64 ];
	snprintf (msgText,
		  sizeof (msgText) / sizeof (char),
		  "pthread_mutex_trylock returns 0x%X",
		  errCode);
	throw std::runtime_error (msgText);
      }

    return false;
  }

  void posix_mutex::unlock ()
  {
    int errCode = pthread_mutex_unlock (&_mutex);
    if (errCode)
      {
	char msgText [ 64 ];
	snprintf (msgText,
		  sizeof (msgText) / sizeof (char),
		  "pthread_mutex_unlock returns 0x%X",
		  errCode);
	throw std::runtime_error (msgText);
      }
  }

} /* namespace CrossClass */
