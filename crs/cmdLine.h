#ifndef CROSS_CMDLINE_H
#define CROSS_CMDLINE_H 1
//
//  cmdLine.h - Base class for getting command line options
//  Copyright (c) Aug 5, 2003 Oleg N. Peregudov
//	Mar 20, 2007 - add some cosmetics
//	Sep 17, 2007 - switch parsing
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

#include <cstdlib>
#include <cstdio>
#include <cctype>

#include <iostream>
#include <fstream>
#include <string>
#include <list>

#include <crs/myexcept.h>
#include <crs/libexport.h>

namespace CommandLine {

////////////////////////////////////////////////////////////////////////////////
class CROSS_EXPORT reader
////////////////////////////////////////////////////////////////////////////////
{
public:
      // exceptions
      class BaseException {};
      
      struct ErrOpen : BaseException, std::open_stream_error
      {
            ErrOpen( const std::string & what_arg ):
                  BaseException(), std::open_stream_error( what_arg ) {}
      };
      
      struct ErrRead : BaseException, std::in_stream_error
      {
            ErrRead( const std::string & what_arg ):
                  BaseException(), std::in_stream_error( what_arg ) {}
      };
      
      struct ErrWrite : BaseException, std::out_stream_error
      {
            ErrWrite( const std::string & what_arg ):
                  BaseException(), std::out_stream_error( what_arg ) {}
      };
      
      struct ErrErase : BaseException, std::file_system_error
      {
            ErrErase( const std::string & what_arg ):
                  BaseException(), std::file_system_error( what_arg ) {}
      };
      
      typedef std::list<std::string>                  tDList;
      typedef std::list<std::string>::const_iterator  tDListIter;
      
private:
      // data members
      int     _ac,                  // command line arguments count
              _sw_position;         // command line switch position
      char ** _av;                  // command line memory pointer
      
      tDList     _script;           // script list
      tDListIter _script_iter;      // script iterator
      
      tDList     _command;          // command' switches
      
      // members
	int   get_sw ( std::string & );
	void  pprocess ( std::string );
      
      void  close_script ()
	{
	      _script.clear();
	      _script_iter = _script.begin();
	}
      
	void  use_script ( std::string s )
	{
	      // close previous script ...
	      close_script();
   		
	      // pre-process script ...
	      pprocess( s );
	      _script_iter = _script.begin();
	}
      
      int   read_script ( std::string & s )
	{
	      if( _script_iter != _script.end() )
	      {
	            s = *_script_iter++;
	            return 1;
	      }
	      return 0;
	}
      
public:
      reader ( int argc, char ** argv );
      ~reader ();
      
      int nswitches () const
	{
		return _command.size();
	}
      
	void reset ()
	{
		_sw_position = 0;
		next_command();
	}
      
      tDListIter begin ()
	{
		return _command.begin();
	}
      
	tDListIter end ()
	{
		return _command.end();
	}
	
      int next_command ();
	static bool parseSwitch ( std::string & sw, bool & swState );
	// return true if argument is a switch
};

typedef reader Reader;

} // namespace CommandLine
#endif // CROSS_CMDLINE_H
