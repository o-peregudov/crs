#ifndef CROSS_ATOMIC_FLAG_H_INCLUDED
#define CROSS_ATOMIC_FLAG_H_INCLUDED 1
/*
 *  crs/atomic_flag.h - atomic boolean flag
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

/*
 *	2012/08/16	atomic boolean flag
 *	2012/08/27	using spin locks
 */

#include <crs/libexport.h>
#if USE_CXX11_ATOMIC
#	include <atomic>
namespace CrossClass
{
	typedef std::atomic<bool>	cAtomicBool;
}
#else
#	include <crs/security.h>
#	include <crs/spinlock.h>
namespace CrossClass
{
	class cAtomicBool
	{
	protected:
		mutable cSpinLock	_spinlock;
		bool			_flag;
		
	public:
		cAtomicBool ( const bool fNewState = false )
			: _spinlock ()
			, _flag (fNewState)
		{ }
		
		void store ( bool fNewState )
		{
			cLocker<cSpinLock> lock (_spinlock);
			_flag = fNewState;
		}
		
		bool load () const
		{
			cLocker<cSpinLock> lock (_spinlock);
			return _flag;
		}
		
		bool operator = ( bool fNewState )
		{
			store (fNewState);
			return fNewState;
		}
		
		operator bool () const
		{
			return load ();
		}
		
	private:
		cAtomicBool (const cAtomicBool &);
		cAtomicBool & operator = (const cAtomicBool &);
	};
}
#endif

#endif /* CROSS_ATOMIC_FLAG_H_INCLUDED */
