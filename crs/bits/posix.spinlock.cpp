/*
 *  crs/bits/posix.spinlock.cpp
 *  Copyright (c) 2012 Oleg N. Peregudov <o.peregudov@gmail.com>
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
#include <crs/bits/posix.spinlock.h>
#include <sys/errno.h>

namespace CrossClass
{

cPosixSpinLock::cPosixSpinLock ()
	: _spinlock( )
{
	int errCode = pthread_spin_init (&_spinlock, PTHREAD_PROCESS_PRIVATE);
	if (errCode != 0)
	{
		char msgText [ 256 ];
		sprintf (msgText, "pthread_spin_init returns %d", errCode);
		throw std::runtime_error (msgText);
	}
}

cPosixSpinLock::~cPosixSpinLock ()
{
	int errCode = pthread_spin_destroy (&_spinlock);
#if defined (DESTRUCTOR_EXCEPTIONS_ALLOWED)
	if (errCode != 0)
	{
		char msgText [ 256 ];
		sprintf (msgText, "pthread_spin_destroy returns %d", errCode);
		throw std::runtime_error (msgText);
	}
#endif
}

void cPosixSpinLock::lock ()
{
	int errCode = pthread_spin_lock (&_spinlock);
	if (errCode != 0)
	{
		char msgText [ 256 ];
		sprintf (msgText, "pthread_spin_lock returns %d", errCode);
		throw std::runtime_error (msgText);
	}
}

bool cPosixSpinLock::try_lock ()
{
	int errCode = pthread_spin_trylock (&_spinlock);
	if (errCode == EBUSY)
		return false;
	else if (errCode != 0)
	{
		char msgText [ 256 ];
		sprintf (msgText, "pthread_spin_trylock returns %d", errCode);
		throw std::runtime_error (msgText);
	}
	else
		return true;
}

void cPosixSpinLock::unlock ()
{
	int errCode = pthread_spin_unlock (&_spinlock);
	if (errCode != 0)
	{
		char msgText [ 256 ];
		sprintf (msgText, "pthread_spin_unlock returns %d", errCode);
		throw std::runtime_error (msgText);
	}
}

} /* namespace CrossClass */
