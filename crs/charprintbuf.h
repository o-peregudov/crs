#ifndef CROSS_CHARPRINTBUF_H
#define CROSS_CHARPRINTBUF_H 1
//
//  CHARPRINTBUF.H - intelligent character buffer & print buffer
//  Copyright (c) Jun 14, 2007 Oleg N. Peregudov <op@pochta.ru>
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
//	Nov 21, 2007 - new place for cross-compiling routines
//

//
// эти классы используют локальный символьный буфер и std::ostrstream
// для преобразования чего-либо в строку при помощи operator <<
//

#include <iostream>
#include <sstream>
#include <cstring>

namespace CrossClass {

//
// class cCharBuffer (concrete)
//
class cCharBuffer
{
      size_t allocated;
      char * memptr;
      
      // not allowed!
      cCharBuffer ( const cCharBuffer & );
      cCharBuffer & operator = ( const cCharBuffer & );
      
public:
      cCharBuffer ( const size_t sz = 0xFFFF )
		: allocated( sz )
		, memptr( new char [ sz ] )
      { }
      
      ~cCharBuffer ()                     { delete [] memptr; }
      
      size_t size () const                { return strlen( memptr ); }
      size_t length () const              { return size(); }
      size_t capacity () const            { return allocated; }
      
      cCharBuffer clone () const;
      
      operator char * ()                  { return memptr; }
      operator const char * () const      { return memptr; }
      
      const char * c_str () const         { return memptr; }
      
      char * upcase ();
      char * locase ();
      
      friend std::istream & operator >> ( std::istream &, cCharBuffer & );
};

inline
cCharBuffer cCharBuffer::clone () const
{
      cCharBuffer result ( allocated );
      memcpy ( result.memptr, memptr, allocated );
      return result;
}

inline
char * cCharBuffer::upcase ()
{
      for( size_t i = 0; i < size(); ++i )
            memptr[ i ] = toupper( memptr[ i ] );
      return memptr;
}

inline
char * cCharBuffer::locase ()
{
      for( size_t i = 0; i < size(); ++i )
            memptr[ i ] = tolower( memptr[ i ] );
      return memptr;
}

inline
std::istream & operator >> ( std::istream & is, cCharBuffer & s )
{
      if( s.allocated != 0 )
		is.getline( s.memptr, static_cast<std::streamsize>( s.allocated ) );
      return is;
}

//
// class cPrintBuf (concrete)
//
class cPrintBuf : public std::basic_ostringstream<char>
{
protected:
	mutable std::string stringBuf;
	
	cPrintBuf ( const cPrintBuf & );
	cPrintBuf & operator = ( const cPrintBuf & );
	
public:
	cPrintBuf ( )
		: std::basic_ostringstream<char>( )
		, stringBuf( )
	{ }
	
	void	reset ()
	{
		std::basic_ostringstream<char>::str( std::string() );
	}
	
	const char * c_str () const
	{
		return str().c_str();
	}
	
	std::basic_ostringstream<char> * operator -> ()
	{
		return this;
	}
	
	const std::string & str () const
	{
		return ( stringBuf = std::basic_ostringstream<char>::str() );
	}
	
      size_t size () const
	{
		return str().size();
	}
};

} // namespace CrossClass
#endif // CROSS_CHARPRINTBUF_H
