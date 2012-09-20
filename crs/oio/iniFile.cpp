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

#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif

#include <crs/oio/iniFile.h>

namespace ObjectIO {

const char SLAP_L [] = "[";
const char SLAP_R [] = "]";
const char REM [] = ";";
const char EQU [] = "=";
const char EOL [] = "\n";

using namespace std;
using namespace CrossClass;

static
string cutfrom( std::string & line, const char * from )
{
	string result ( "" );
	size_t position = line.find_first_of( from );
	if( position != std::string::npos )
	{
		result = line.substr( position + 1 );
		line.erase( position );
	}
	return result;
}

// eat whitespaces from the line head ...
static
string firstchar ( const string & buf )
{
	size_t i = 0;
	for(; ( i < buf.size() ) && isspace( buf[ i ] ); ++i );
	return buf.substr( i );
}

//
// members of Var
//
void	Var::_set_stream ()
{
	_stream.str( _strContents );
	_stream.clear();
}

const char * Var::c_str () const
{
	_LockIt lock ( _mutex );
	return _strContents.c_str();
}

Var & Var::operator = ( const char * p )
{
	base_assign( p );
	return *this;
}

void	Var::clear ()
{
	_LockIt lock ( _mutex );
	_modified = 1;
	_strContents = "";
	_set_stream();
}

void	Var::base_assign ( const string & o )
{
	_LockIt lock ( _mutex );
	_strContents = o;
	_set_stream();
}

size_t Var::size ()
{
	_LockIt lock ( _mutex );
	return _strContents.size();
}

void	Var::create ( const string & str )
// creates variable a-la : "Name=Contents;Comment\n"
{
	string line ( str );
	_LockIt lock ( _mutex );
	_description = cutfrom( line, REM );		// comment ...
	lock.unlock();
	base_assign( cutfrom( line, EQU ).c_str() );	// contents ...
	lock.lock();
	_name = line;						// name ...
}

void	Var::read ( basic_istream<char> & is )
// read variable a-la : "Name=Contents;Comment\n"
{
	// allocate memory for line buffer
	string buf;
	
	// clear stream exceptions
	cSaveStreamExceptions es ( &is, ios_base::goodbit );
	
	// read one line
	getline( is, buf );
	
	if( is.bad() )
		// report fatal error
		throw ErrRead( "Var::read" );
	else
	{
		// filling internal fields
		create( firstchar( buf.substr( 0, buf.find( '\r' ) ) ) );
		_LockIt lock ( _mutex );
		_modified = false;
		
		if( is.eof() )
			throw msgEOF( "Var::read" );
	}
}

void	Var::write ( basic_ostream<char> & os )
// writes variable a-la : "Name=Contents;description"
{
	static const char * my_name = "Var::write";
	try
	{
		_LockIt lock ( _mutex );
		cSaveStreamExceptions es ( &os );
		if( _name.length() != 0 )
		{
			os.write( _name.c_str(), _name.length() );
			os.write( EQU, strlen( EQU ) );
			if( _strContents.size() )
				os.write( _strContents.c_str(), _strContents.size() );
		}
		if( _description.length() != 0 )
		{
			os.write( REM, strlen( REM ) );
			os.write( _description.c_str(), _description.length() );
		}
	}
	catch ( ios_base::failure )
	{
		throw ErrWrite( my_name );
	}
}

//
// members of INIFile::Hub::get_hub_size
//
void	Hub::get_hub_size::operator () ( base::value_type & v )
{
	if( v.second )
	{
		if( v.second->name().length() != 0 )
		{
			sz += v.second->name().length()
			   + strlen( EQU )
			   + v.second->size();
		}
		if( v.second->description().length() != 0 )
		{
			sz += strlen( REM )
			   + v.second->description().length();
		}
		sz += strlen( EOL );
	}
}

size_t Hub::size ()
{
	_LockIt lock ( _mutex );
	return ( _size = for_each( begin(), end(), get_hub_size() ) );
}

//
// members of INIFile::Hub
//
void	Hub::read ( basic_istream<char> & is )
{
	cVarHandle v;
	string buf;
	streampos line_start_position;
	
	// clear stream exceptions
	cSaveStreamExceptions es ( &is, ios_base::goodbit );
	
	// read section line by line
	bool last = false;
	while( !last )
	{
		// save file read position
		line_start_position = is.tellg();
		
		// read one line
		getline( is, buf );
		
		if( is.bad() )
			// report fatal error
			throw ErrRead( "Hub::read" );
		else
			// check EOF sign
			last = is.eof();
		
		// skip leading whitespaces and remove '\r' from the end
		buf = firstchar( buf.substr( 0, buf.find( '\r' ) ) );
		if( buf.size() && ( buf[ 0 ] == *SLAP_L ) )
		{
			// roll back to first position and exit
			// because this is a new hub section
			is.clear();
			is.seekg( line_start_position );
			break;
		}
		
		// creating new variable
		v.bind( new Var () );
		v->create( buf );
		
		// append variable to hub
		append( v );
	}
	
	if( last )
		throw msgEOF( "Hub::read" );
	
	_LockIt lock ( _mutex ); 
	_modified = false;
}

void	Hub::_write_contents ( basic_ostream<char> & os, const bool startFromNewLine )
{
	// perform bubble sort on variables
	tDListIter p, i = olist.begin();
	while( ( i != olist.end() ) && ( (*i)->entries() == -1 ) ) ++i;
	for( p = i; p != olist.end(); ++p )
	{
		if( (*p)->entries() == -1 )
		{
			tDListIter s, q;
			data_type var = *p;
			for( --( q = s = p ); s != i; *s-- = *q-- );
			*i++ = var;
		}
	}
	
	// write Hub's contents line by line
	bool first = true;
	for( p = olist.begin(); p != olist.end(); ++p )
	{
		if( *p )
		{
			if( !first || startFromNewLine )
				os.write( EOL, strlen( EOL ) );
			(*p)->write( os );
			first = false;
		}
	}
}

void	Hub::write ( basic_ostream<char> & os )
{
	try
	{
		_LockIt lock ( _mutex );
		cSaveStreamExceptions es ( &os );
		os.write( SLAP_L, strlen( SLAP_L ) );
		os.write( _name.c_str(), _name.length() );
		os.write( SLAP_R, strlen( SLAP_R ) );
		if( _description.length() != 0 )
		{
			os.write( REM, strlen( REM ) );
			os.write( _description.c_str(), _description.length() );
		}
		_write_contents( os, true );
	}
	catch ( ios_base::failure )
	{
		// translate an exception ...
		throw ErrWrite( "Hub::write" );
	}
}

//
// members of INIFile::cINIFile
//
void	cINIFile::read ( basic_istream<char> & is )
{
	cBaseVarHandle v;
	string buf;
	
	// clear stream exceptions
	cSaveStreamExceptions es ( &is, ios_base::goodbit );
	
	// read file line by line
	for( bool last = false; !last; )
	{
		// read entire line
		getline( is, buf );
		
		if( is.bad() )
			// report fatal error
			throw ErrRead( "cINIFile::read" );
		else
			// check EOF sign
			last = is.eof();
		
		// skip whitespaces
		buf = firstchar( buf.substr( 0, buf.find( '\r' ) ) );
		if( buf.size() && ( buf[ 0 ] == *SLAP_L ) )
		{
			// creating new hub
			v.bind( new Hub () );
			string line = buf.substr( 1 );
			
			// get comment
			v->description( cutfrom( line, REM ) );
			
			// get name
			cutfrom( line, SLAP_R );
			v->name( line );
			
			// read hub contents
			try
			{
				v->read( is );
			}
			catch( msgEOF )
			{
				// it's ok, don't panic!
				// stream is clear because of
				// class cSaveStreamExceptions
				last = true;
			}
		}
		else
		{
			// creating new var
			Var * var = new Var ();
			var->create( buf );
			v.bind( var );
		}
		
		// append variable to list
		append( v );
	}
	_LockIt lock ( _mutex );
	_modified = false;
}

void	cINIFile::write ( basic_ostream<char> & os )
{
	try
	{
		_LockIt lock ( _mutex );
		cSaveStreamExceptions es ( &os );
		_write_contents( os, false );
	}
	catch( ios_base::failure )
	{
		// translate an exception ...
		throw ErrWrite( "cINIFile::write" );
	}
}

void	cINIFile::readvar ( Hub & hub, const std::string & name, bool & x )
{
	cVarHandle var ( hub.get( name ) );
	if( var )
	{
		var->instream()->clear();
		var->instream()->seekg( 0 );
		*var >> std::boolalpha >> x;
	}
}

void	cINIFile::readvar ( Hub & hub, const std::string & name, std::string & x )
{
	cVarHandle var ( hub.get( name ) );
	if( var )
		x = *var;
}

void	cINIFile::writevar ( Hub & hub, CrossClass::cPrintBuf & pbuf, const std::string & name, const bool x )
{
	pbuf.reset();
	pbuf << std::setiosflags( std::ios_base::boolalpha ) << x << std::resetiosflags( std::ios_base::boolalpha );
	cVarHandle var ( hub.get( name, Var() ) );
	*var = pbuf.c_str();
}

void	cINIFile::writevar ( Hub & hub, CrossClass::cPrintBuf & pbuf, const std::string & name, const char * str )
{
	cVarHandle var ( hub.get( name, Var() ) );
	*var = str;
}

void	cINIFile::writevar ( Hub & hub, CrossClass::cPrintBuf & pbuf, const std::string & name, const unsigned char x )
{
	pbuf.reset();
	pbuf << std::setw( 2 ) << std::setfill( '0' ) << static_cast< int >( x );
	cVarHandle var ( hub.get( name, Var() ) );
	*var = pbuf.c_str();
}

} /* namespace ObjectIO	*/
