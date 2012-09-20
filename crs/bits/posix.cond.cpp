/*
 *  crs/bits/posix.cond.cpp
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

#include <crs/bits/posix.cond.h>
#include <cstdio>

namespace CrossClass {

/*
 * members of class cPosixConditionVariable
 */
void cPosixConditionVariable::notify_one ()
{
	int errCode = pthread_cond_signal (&_condition);	/* wake up the thread */
	if (errCode)
	{
		char errText [ 128 ];
		sprintf (errText, "%d: %s in 'cPosixConditionVariable::notify_one'", errCode, strerror (errCode));
		throw std::runtime_error (errText);
	}
}

void cPosixConditionVariable::notify_all ()
{
	int errCode = pthread_cond_broadcast (&_condition);	/* wake up threads */
	if (errCode)
	{
		char errText [ 128 ];
		sprintf (errText, "%d: %s in 'cPosixConditionVariable::notify_all'", errCode, strerror (errCode));
		throw std::runtime_error (errText);
	}
}

void cPosixConditionVariable::wait ( _LockIt & lock )
{
	int errCode = pthread_cond_wait (&_condition, static_cast<pthread_mutex_t*> (lock.mutex()->native_handle ()));
	if (errCode)
	{
		char errText [ 128 ];
		sprintf (errText, "%d: %s in 'cPosixConditionVariable::wait'", errCode, strerror (errCode));
		throw std::runtime_error (errText);
	}
}

cPosixConditionVariable::cPosixConditionVariable ()
	: _condition ()
{
	int errCode = pthread_cond_init (&_condition, NULL);
	if (errCode)
	{
		char errText [ 128 ];
		sprintf (errText, "%d: %s in 'cPosixConditionVariable::constructor'", errCode, strerror (errCode));
		throw std::runtime_error (errText);
	}
}

cPosixConditionVariable::~cPosixConditionVariable ()
{
	int errCode = pthread_cond_destroy (&_condition);
#if defined (DESTRUCTOR_EXCEPTIONS_ALLOWED)
	if (errCode)
	{
		char errText [ 128 ];
		sprintf (errText, "%d: %s in 'cPosixConditionVariable::destructor'", errCode, strerror (errCode));
		throw std::runtime_error (errText);
	}
#endif
}

} /* namespace CrossClass */
