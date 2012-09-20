#ifndef CROSS_OIO_INIFILE_H
#define CROSS_OIO_INIFILE_H 1
/*
 *  This file is a part of Object I/O library
 *
 *  crs/oio/iniFile.h - INI-file implementation
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
 *	2007/08/03	read and write operations are for istream|ostream
 *	2007/11/21	new place for cross-compiling routines
 *	2007/12/06	new project name & DLL
 *	2008/01/24	no throw specificator for read\write members
 *	2010/09/08	C++0x compartible locks
 */

#include <crs/oio/FileVar.h>
#include <crs/charprintbuf.h>
#include <iomanip>

// #define _DEBUG_PURPOSE_NO_HUBS_

namespace ObjectIO {

// forward definition
class CROSS_EXPORT cINIFile;

//
// class Var (concrete)
// represent the concept of a simple text variable
// (namely - .ini-file variable).
//
class CROSS_EXPORT Var : public BaseVar
{
	friend class cINIFile;
	
	Var ( const Var & );			// not allowed!
	Var & operator = ( const Var & );	// not allowed!
	
	void	_set_stream ();
	
protected:
	std::string _strContents;
	std::basic_istringstream<char> _stream;
	
	void base_assign ( const std::string & );
	
public:
	Var ()
		: BaseVar()
		, _strContents()
		, _stream()
	{
		_set_stream();
	}
	
	Var ( const std::string & n )
		: BaseVar( n )
		, _strContents()
		, _stream()
	{
		_set_stream();
	}
	
	virtual ~Var () { clear(); }
	
	virtual size_t size ();
	virtual int entries () const { return -1; }
	
	virtual void read ( std::basic_istream<char> & );
	virtual void write ( std::basic_ostream<char> & );
	virtual void loadAll () {}
	virtual void clear ();
	
	const char * c_str () const;
	Var & operator = ( const char * );
	
	operator const std::string & () const
	{
		return _strContents;
	}
	
	void	create ( const std::string & );
	
public:
	//
	// stream-like input and related functions
	//
	std::basic_istringstream<char> * instream ()
	{
		return &_stream;
	}
	
	template <class Type>
	std::basic_istream<char> & operator >> ( Type & v )
	{
		CrossClass::_LockIt lock ( _mutex );
		return ( _stream >> v );
	}
};

typedef CrossClass::cHandle< Var > cVarHandle;

//
// class Hub (concrete)
// represent the structure (a finite set of named variables)
//
class CROSS_EXPORT Hub : public BaseHub
{
	typedef BaseHub base;
	
	//
	// An internal class
	// You should not attempt to use it directly
	// Special implementation for .INI files ("Name=Contents;Comment\n")
	//
	class get_hub_size
	{
		size_t sz;
	public:
		get_hub_size () : sz( 0 ) {}
		operator const size_t () const { return sz; }
		void operator () ( base::value_type & );
	};
	
	Hub ( const Hub & );			// not allowed!
	Hub & operator = ( const Hub & );	// not allowed!
	
public:
	Hub () : base() {}
	Hub ( const std::string & n ) : base( n ) {}
	
	virtual size_t size ();
	virtual void read ( std::basic_istream<char> & );
	virtual void write ( std::basic_ostream<char> & );
	
protected:
	void _write_contents ( std::basic_ostream<char> &, const bool startFromNewLine = true );
};

typedef CrossClass::cHandle< Hub > cHubHandle;

//
// class cINIFile (concrete)
//
class CROSS_EXPORT cINIFile : public Hub
{
public:
	cINIFile () : Hub() {}
	cINIFile ( const std::string & n ) : Hub( n ) {}
	virtual void read ( std::basic_istream<char> & );
	virtual void write ( std::basic_ostream<char> & );
	
public:
	struct hex {};
	
	template <typename Type>
	static void readvar ( Hub & hub, const std::string & name, Type & x )
	{
		cVarHandle var ( hub.get( name ) );
		if( var )
		{
			var->instream()->clear();
			var->instream()->seekg( 0 );
			*var >> x;
		}
	}
	
	template <typename Type>
	static void readvar ( Hub & hub, const std::string & name, Type & x, hex )
	{
		cVarHandle var ( hub.get( name ) );
		if( var )
		{
			var->instream()->clear();
			var->instream()->seekg( 2 );
			*var >> std::hex >> x;
		}
	}
	
	template <typename Type>
	static void writevar ( Hub & hub, CrossClass::cPrintBuf & pbuf, const std::string & name, const Type & x )
	{
		pbuf.reset();
		pbuf << x;
		
		cVarHandle var ( hub.get( name, Var() ) );
		*var = pbuf.c_str();
	}
	
	template <typename Type>
	static void writevar ( Hub & hub, CrossClass::cPrintBuf & pbuf, const std::string & name, const Type & x, const int w )
	{
		pbuf.reset();
		pbuf << "0x" << std::setw( w ) << std::setfill( '0' ) << std::hex << x << std::dec;
		cVarHandle var ( hub.get( name, Var() ) );
		*var = pbuf.c_str();
	}
	
	static void readvar ( Hub & hub, const std::string & name, bool & x );
	static void readvar ( Hub & hub, const std::string & name, std::string & x );
	static void writevar ( Hub & hub, CrossClass::cPrintBuf & pbuf, const std::string & name, const bool x );
	static void writevar ( Hub & hub, CrossClass::cPrintBuf & pbuf, const std::string & name, const char * str );
	static void writevar ( Hub & hub, CrossClass::cPrintBuf & pbuf, const std::string & name, const unsigned char x );
	
	template <typename Type>
	static void readvar ( const cHubHandle & hubHandle, const std::string & name, Type & x )
	{
		readvar( *hubHandle, name, x );
	}
	
	template <typename Type>
	static void readvar ( const cHubHandle & hubHandle, const std::string & name, Type & x, hex )
	{
		readvar( *hubHandle, name, x, hex() );
	}
	
	template <typename Type>
	static void writevar ( const cHubHandle & hubHandle, CrossClass::cPrintBuf & pbuf, const std::string & name, const Type & x )
	{
		writevar( *hubHandle, pbuf, name, x );
	}
	
	template <typename Type>
	static void writevar ( const cHubHandle & hubHandle, CrossClass::cPrintBuf & pbuf, const std::string & name, const Type & x, const int w )
	{
		writevar( *hubHandle, pbuf, name, x, w );
	}
};

typedef CrossClass::cHandle< cINIFile > cINIFileHandle;

}	/* namespace ObjectIO	*/
#endif/* CROSS_OIO_INIFILE_H	*/
