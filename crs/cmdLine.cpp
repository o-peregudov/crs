//
//  cmdLine.cpp - Base class for getting command line options
//  Copyright (c) Aug 5, 2003 Oleg N. Peregudov
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
//  Send any questions and wishes by e-mail: op@pochta.ru
//
//	Sep 19, 2007 - switch parser
//	Nov 21, 2007 - new place for cross-compiling routines
//
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#endif
#include "cmdLine.h"
#include "parsing.h"
#include "security.h"

namespace CommandLine {

#define NEXT_COMMAND	'&'
#define SCRIPT_PREFFIX	'@'
#define MINUS		'-'
#define SLASH		'/'

using namespace std;
using namespace CrossClass;

reader::reader ( int argc, char ** argv )
	: _ac( argc )
	, _sw_position( 0 )
	, _av( argv )
	, _script()
	, _script_iter()
	, _command()
{
	_script_iter = _script.begin();
}

reader::~reader ()
{
	close_script();
	_command.clear();
}

void
reader::pprocess ( string s_name )
{
	fstream subscript_file( s_name.c_str(), ios_base::in );
	if( !subscript_file )
		throw ErrOpen( s_name );
	
	try
	{
		// pre-process subscript ...
		for( string one_token( "" );; )
		{
			one_token = parsing::scan_token( subscript_file );
			
			if( one_token.size() == 0 )
				break;
			else if( one_token[0] == SCRIPT_PREFFIX )
			{
				if( (one_token.size() > 1) && (one_token[1] == SCRIPT_PREFFIX) )
					_script.push_back( one_token.substr( 1 ) );
				else
					pprocess( one_token.substr( 1 ) );
			}
			else
				_script.push_back( one_token );
		}
	}
	catch ( in_stream_error )
	{
		throw ErrRead( s_name );
	}
}

int
reader::get_sw ( string & csw )
{
	while( !read_script( csw ) )
	{
		if( ++_sw_position < _ac )
		{
			csw = _av [ _sw_position ];
			
			if( csw[0] == SCRIPT_PREFFIX )
				use_script( csw.substr( 1 ) );
			else
				break;
		}
		else
			return 0;
	}
	
	return 1;
}

int
reader::next_command ()
{
      string cur_sw;
      _command.clear ();
      while( get_sw( cur_sw ) )
      {
		if( cur_sw.size() )
		{
	            if( cur_sw[0] == NEXT_COMMAND )
	            {
	                  if( cur_sw.size () > 1 && cur_sw[1] == NEXT_COMMAND )
	                        _command.push_back( cur_sw.substr( 1 ) );
	                  else
	                        break;
	            }
	            else
	                  _command.push_back( cur_sw );
		}
      }
      return _command.size();
}

bool
reader::parseSwitch ( string & sw, bool & swState )
{
	if( ( sw[ 0 ] == MINUS ) || ( sw[ 0 ] == SLASH ) )
	{
		if( sw[ sw.size() - 1 ] == MINUS )
		{
			swState = false;
			sw = sw.substr( 1, sw.size() - 2 );
		}
		else
		{
			swState = true;
			sw = sw.substr( 1 );
		}
		return true;
	}
	else
		return false;
}

} // namespace CommandLine
