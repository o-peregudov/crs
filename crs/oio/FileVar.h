#ifndef CROSS_OIO_FILEVAR_H
#define CROSS_OIO_FILEVAR_H 1
/*
 *  This file is a part of Object I/O library
 *
 *  crs/oio/FileVar.h - base OIO classes and templates
 *  Copyright (C) 2004-2012 Oleg N. Peregudov <o.peregudov@gmail.com>
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
 *	2004/11/16	base OIO classes and templates
 *	2007/07/13	uniform locks & handles
 *	2007/08/03	default typedef for cBaseHubHandle
 *			read and write operations are for istream|ostream
 *	2007/11/21	new place for cross-compiling routines
 *	2007/12/06	new project name & DLL
 *	2008/01/24	no throw specificator for read\write members
 *	2010/09/08	C++0x compartible locks
 *	2011/01/21	bug in BaseHub::get causes a deadlock
 */

#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <map>

#include <crs/handle.h>
#include <crs/myexcept.h>

namespace ObjectIO {

//
// common ObjectIO exceptions
//

class BaseException {};

struct msgEOF : BaseException, std::logic_error
{
	msgEOF ( const std::string & what_arg ):
		BaseException(), std::logic_error( what_arg ) {}
};

struct ErrRead : BaseException, std::in_stream_error
{
	ErrRead ( const std::string & what_arg ):
		BaseException(), std::in_stream_error( what_arg ) {}
};

struct ErrWrite : BaseException, std::out_stream_error
{
	ErrWrite ( const std::string & what_arg ):
		BaseException(), std::out_stream_error( what_arg ) {}
};

//
// class BaseVar (abstract)
// представляет общее понятие переменной
//
class CROSS_EXPORT BaseVar
{
protected:
	mutable CrossClass::LockType	_mutex;
	bool		_modified;		// TRUE if contents was changed
	size_t	_size;		// current length
	std::string	_name,		// variable name
			_description;	// variable description
	
public:
	BaseVar ()
		: _mutex()
		, _modified( false )
		, _size( 0 )
		, _name()
		, _description()
	{ }
	
	BaseVar ( const std::string & n )
		: _mutex()
		, _modified( false )
		, _size( 0 )
		, _name( n )
		, _description()
	{ }
	
	virtual ~BaseVar () { clear(); }
	
	//  -1   - for simple variable
	// other - number of members in structure
	virtual int entries () const = 0;
	
	// current size
	virtual size_t size () = 0;
	
	virtual void read ( std::basic_istream<char> & ) = 0;		// reads variable contents from file
	virtual void write ( std::basic_ostream<char> & ) = 0;	// writes variable contents to file
	virtual void loadAll () = 0;						// loads entire variable in memory
	virtual void clear () {}
	
	virtual bool modified () const;
	
	const std::string & name ( const std::string & );
	const std::string & description ( const std::string & );
	
	const std::string & name () const;
	const std::string & description () const;
};

typedef CrossClass::cHandle< BaseVar > cBaseVarHandle;

//
// class BaseHub (abstract)
// представляет общее понятие узловой переменной (т. е. структуры)
//
class CROSS_EXPORT BaseHub :	public BaseVar,
					public std::multimap< const std::string, cBaseVarHandle >
{
public:
	typedef std::multimap< const std::string, cBaseVarHandle > hubcore;
	
	typedef hubcore::iterator		hubiter;
	typedef hubcore::key_type		key_type;
	typedef hubcore::value_type		value_type;
	typedef cBaseVarHandle			data_type;
	
	typedef std::list< data_type >	tDList;
	typedef tDList::iterator		tDListIter;
	
protected:
	tDList olist;
	
	// An internal class. You should not attempt to use it directly
	struct get_modified {
		bool	operator () ( value_type & v ) const {
			return ( v.second ? v.second->modified() : false );
		}
	};
	
	// An internal class. You should not attempt to use it directly
	struct get_all {
		void  operator () ( const data_type & v ) const {
			if( v ) v->loadAll();
		}
	};
	
public:
	BaseHub ()
		: BaseVar()
		, hubcore()
		, olist()
	{ }
	
	BaseHub ( const key_type & n )
		: BaseVar( n )
		, hubcore()
		, olist()
	{ }
	
	virtual ~BaseHub ()
	{
		clear();
	}
	
	virtual int entries () const
	{
		return static_cast<int>( hubcore::size() );
	}
	
	virtual void clear ();
	virtual bool modified ();
	virtual void loadAll ();
	
	void	append ( const data_type & );
	void	erase ( const key_type & );
	
	data_type get ( const key_type & name )
	{
		CrossClass::_LockIt lock ( _mutex );
		hubiter p ( find( name ) );
		return ( ( p != end() ) ? p->second : data_type() );
	}
	
	template <typename VarType>
	data_type get ( const key_type & name, const VarType & )
	{
		data_type var ( get( name ) );
		if( !var )
		{
			var = new VarType ( name );
			append( var );
		}
		return var;
	}
};

typedef CrossClass::cHandle< BaseHub > cBaseHubHandle;

}	/* namespace ObjectIO	*/
#endif/* CROSS_OIO_FILEVAR_H	*/
