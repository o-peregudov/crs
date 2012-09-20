#ifndef CROSS_SECURITY_H
#define CROSS_SECURITY_H 1
/*
 *  crs/security.h - RAII handle classes and templates
 *  Copyright (c) 2007-2012 Oleg N. Peregudov <o.peregudov@gmail.com>
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
 * Here small classes that implements RAII idiom are collected.
 * Resource blocking/allocation is performed in constructors of such classes
 * and release of resources is performed in destructor.
 *
 *	2007-11-21	new place for cross-compiling routines
 *	2007-12-05	DLL
 *	2010-08-30	cLocker compartibility with std::unique_lock from C++0x standard
 *	2010-12-01	translation for some comments
 *	2012-08-29	for C++11 mode cLocker is an alias for std::unique_lock
 */

#include <iostream>
#include <crs/mutex.h>
#define ALL_IOS_EXCEPTIONS (std::ios_base::eofbit|std::ios_base::failbit|std::ios_base::badbit)

namespace CrossClass {

/*
 * class cSaveStreamExceptions
 * sets new exception tracking state for IO stream
 */
class CROSS_EXPORT cSaveStreamExceptions
{
	std::ios_base::iostate  _state;
	std::basic_ios<char, std::char_traits<char> > * _stream;
public:
	cSaveStreamExceptions ( std::basic_ios<char, std::char_traits<char> > * s, std::ios_base::iostate def_state = ALL_IOS_EXCEPTIONS );
	~cSaveStreamExceptions ();
};

/*
 * class cSaveIStreamPosition
 * saves stream read position
 */
class CROSS_EXPORT cSaveIStreamPosition
{
	std::istream * _stream;
	std::streampos _position;
public:
	cSaveIStreamPosition ( std::istream * );
	~cSaveIStreamPosition ();
	
	operator std::streampos () const
	{
		return ( _stream ? _position : std::streampos( 0 ) );
	}
};

/*
 * class cSaveOStreamPosition
 * saves stream write position
 */
class CROSS_EXPORT cSaveOStreamPosition
{
	std::ostream * _stream;
	std::streampos _position;
public:
	cSaveOStreamPosition ( std::ostream * s );
	~cSaveOStreamPosition ();
	
	operator std::streampos () const
	{
		return ( _stream ? _position : std::streampos( 0 ) );
	}
};

#if USE_CXX11_MUTEX
typedef CrossClass::cMutex		LockType;
typedef std::unique_lock<LockType>	_LockIt;
template <class MutexType> using cLocker = std::unique_lock<MutexType>;
#else
struct defer_lock_t {};
struct try_to_lock_t {};

/*
 * class cLocker - unique_lock template
 */
template <typename MutexType> class cLocker
{
	MutexType & theLock;
	bool fAcquired;
	
	/* not allowed! */
	cLocker ( const cLocker & );
	cLocker & operator = ( const cLocker & );
	
public:
	cLocker ( MutexType & m )
		: theLock (m)
		, fAcquired (false)
	{
		lock();
	}
	
	cLocker ( MutexType & m, defer_lock_t )
		: theLock (m)
		, fAcquired (false)
	{
	}
	
	cLocker ( MutexType & m, try_to_lock_t )
		: theLock (m)
		, fAcquired (false)
	{
		try_lock ();
	}
	
	~cLocker ()
	{
		try
		{
			unlock ();
		}
		catch (...)
		{
		}
	}
	
	void lock ()
	{
		theLock.lock ();
		fAcquired = true;
	}
	
	bool try_lock ()
	{
		return ( fAcquired = theLock.try_lock () );
	}
	
	void unlock ()
	{
		if (fAcquired)
		{
			theLock.unlock ();
			fAcquired = false;
		}
	}
	
	MutexType * mutex () const
	{
		return &theLock;
	}
	
	operator bool () const
	{
		return fAcquired;
	}
	
	bool operator ! () const
	{
		return !fAcquired;
	}
};

typedef CrossClass::cMutex			LockType;
typedef CrossClass::cLocker<LockType>	_LockIt;
#endif

}	/* namespace CrossClass	*/
#endif/* CROSS_SECURITY_H	*/
