#ifndef MYEXCEPT_H
#define MYEXCEPT_H 1
//
//  MYEXCEPT.H - Common exceptions
//  Copyright (c) August 18, 2000 Oleg N. Peregudov
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
//	Feb 11, 2008
//

#include <stdexcept>
#include <iostream>

namespace std {

struct resource_error : runtime_error {
      resource_error( const string& what_arg ) : runtime_error( what_arg ) {}
};

struct file_system_error : resource_error {
      file_system_error( const string& what_arg ) : resource_error( what_arg ) {}
};

struct in_stream_error : file_system_error {
      in_stream_error( const string& what_arg ) : file_system_error( what_arg ) {}
};

struct out_stream_error : file_system_error {
      out_stream_error( const string& what_arg ) : file_system_error( what_arg ) {}
};

struct open_stream_error : file_system_error {
      open_stream_error( const string& what_arg ) : file_system_error( what_arg ) {}
};

struct conversion_error : runtime_error {
      conversion_error( const string & from ) : runtime_error( from ) {}
};

struct invalid_command: invalid_argument {
      invalid_command( const string & what_arg ) : invalid_argument( what_arg ) {}
};

struct invalid_switch: invalid_argument {
      invalid_switch( const string & what_arg ) : invalid_argument( what_arg ) {}
};

//
// Error reporting code
//
inline void error ( const char * m ) { cerr << m; }

inline void error ( const runtime_error & e )
{
      cerr << "Runtime error: " << e.what() << "!" << endl;
}

inline void error ( const logic_error & e )
{
      cerr << "Logic error: " << e.what() << "!" << endl;
}

} // namespace std
#endif // MYEXCEPT_H
