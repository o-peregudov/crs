#ifndef CROSS_SECURITY_H
#define CROSS_SECURITY_H 1
//
//  SECURITY.H - handle classes and templates
//  Copyright (c) Aug 20, 2007 Oleg N. Peregudov <op@pochta.ru>
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//	11/21/2007 - new place for cross-compiling routines
//	12/05/2007 - DLL
//	08/30/2010 - cLocker compartibility with std::unique_lock from C++0x standard
//	12/01/2010 - translation for some comments
//

//
// Here small classes dedicated for realizing resource requests via
// initialization are collected. Resource blocking/allocation is performed in
// constructor of such class and release of the resource is performed in
// destructor.
//

#include <iostream>
#include <crs/mutex.h>
#define ALL_IOS_EXCEPTIONS (std::ios_base::eofbit|std::ios_base::failbit|std::ios_base::badbit)

namespace CrossClass {

//
// class cSaveStreamExceptions
// sets new exception tracking state for IO stream
//
class CROSS_EXPORT cSaveStreamExceptions
{
	std::ios_base::iostate  _state;
	std::basic_ios<char, std::char_traits<char> > * _stream;
public:
	cSaveStreamExceptions ( std::basic_ios<char, std::char_traits<char> > * s, std::ios_base::iostate def_state = ALL_IOS_EXCEPTIONS );
	~cSaveStreamExceptions ();
};

//
// class cSaveIStreamPosition
// saves stream read position
//
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

//
// class cSaveOStreamPosition
// saves stream write position
//
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

#if defined( USE_CXX0X_API )
	struct defer_lock_t : std::defer_lock_t {};
	struct try_to_lock_t : std::try_to_lock_t {};
#else
	struct defer_lock_t {};
	struct try_to_lock_t {};
#endif

//
// class cLocker - lock template
//
template <typename MutexType> class cLocker
{
	MutexType & theLock;
	bool fAcquired;
	
	// not allowed!
	cLocker ( const cLocker & );
	cLocker & operator = ( const cLocker & );
	
public:
	cLocker ( MutexType & m )
		: theLock( m )
		, fAcquired( false )
	{
		lock();
	}
	
	cLocker ( MutexType & m, defer_lock_t )
		: theLock( m )
		, fAcquired( false )
	{
	}
	
	cLocker ( MutexType & m, try_to_lock_t )
		: theLock( m )
		, fAcquired( false )
	{
		try_lock();
	}
	
	~cLocker ()
	{
		unlock();
	}
	
	void lock ()
	{
		theLock.lock();
		fAcquired = true;
	}
	
	bool try_lock ()
	{
		return ( fAcquired = theLock.try_lock() );
	}
	
	void unlock ()
	{
		if( fAcquired )
		{
			theLock.unlock();
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

typedef CrossClass::cMutex 				LockType;
#if defined( CAN_USE_STD_UNIQUE_LOCK )
	typedef std::unique_lock<std::mutex>	_LockIt;
#else
	typedef CrossClass::cLocker<LockType>	_LockIt;
#endif

} // namespace CrossClass
#endif // CROSS_SECURITY_H
