/*
 *  crs/bits/win32.semaphore.cpp
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
#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif
#include <crs/bits/win32.semaphore.h>
#include <cstdio>

namespace CrossClass {

//
// members of class cWin32Semaphore
//

cWin32Semaphore::cWin32Semaphore ( unsigned value )
	: _semaphore (NULL)
{
	_semaphore = CreateSemaphore (NULL, value, value, NULL);
	if (_semaphore == NULL)
	{
		char msgText [ 64 ];
		sprintf (msgText, "cWin32Semaphore::cWin32Semaphore (%d)", GetLastError ());
		throw std::runtime_error (msgText);
	}
}

cWin32Semaphore::~cWin32Semaphore ()
{
	BOOL result = CloseHandle (_semaphore);
#if defined (DESTRUCTOR_EXCEPTIONS_ALLOWED)
	if (result == 0)
	{
		char msgText [ 64 ];
		sprintf (msgText, "~cWin32Semaphore (%d)", GetLastError ());
		throw std::runtime_error (msgText);
	}
#endif
}

void cWin32Semaphore::lock ()
{
	switch (WaitForSingleObject (_semaphore, INFINITE))
	{
	case	WAIT_OBJECT_0:
		return;
	
	case	WAIT_FAILED:
		{
			char msgText [ 64 ];
			sprintf (msgText, "cWin32Semaphore::lock (%d)", GetLastError ());
			throw std::runtime_error (msgText);
		}
	}
}

bool cWin32Semaphore::try_lock_for ( const unsigned long dwMilliseconds )
{
	switch (WaitForSingleObject (_semaphore, dwMilliseconds))
	{
	case	WAIT_OBJECT_0:
		return true;
	
	case	WAIT_FAILED:
		{
			char msgText [ 64 ];
			sprintf (msgText, "cWin32Semaphore::try_lock (%d)", GetLastError ());
			throw std::runtime_error (msgText);
		}
	
	case	WAIT_TIMEOUT:
	default:
		return false;
	}
}

void cWin32Semaphore::unlock ()
{
	if (ReleaseSemaphore (_semaphore, 1, NULL) == 0)
	{
		char msgText [ 64 ];
		sprintf (msgText, "cWin32Semaphore::unlock (%d)", GetLastError ());
		throw std::runtime_error (msgText);
	}
}

} /* namespace CrossClass	*/
