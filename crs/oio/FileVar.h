#ifndef CROSS_OIO_FILEVAR_H_INCLUDED
#define CROSS_OIO_FILEVAR_H_INCLUDED 1
/*
 *  This file is a part of Object I/O library
 *
 *  crs/oio/FileVar.h - base OIO classes and templates
 *  Copyright (C) 2004-2016 Oleg N. Peregudov <o.peregudov@gmail.com>
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

#include <crs/libexport.h>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <list>
#include <map>

namespace ObjectIO
{

struct exception {};

struct msg_eof : exception, std::logic_error
{
  msg_eof (const std::string & what_arg)
    : exception ()
    , std::logic_error (what_arg)
  { }
};

struct read_error : exception, std::runtime_error
{
  read_error (const std::string & what_arg)
    : exception ()
    , std::runtime_error (what_arg)
  { }
};

struct write_error : exception, std::runtime_error
{
  write_error (const std::string & what_arg)
    : exception ()
    , std::runtime_error (what_arg)
  { }
};

class var
{
protected:
  mutable mutex_type _mutex;
  bool               _modified;
  size_t             _size;
  std::string        _name;
  std::string        _description;

public:
  typedef std::unique_ptr<var> upointer_type;

  var ()
    : _mutex ()
    , _modified (false)
    , _size (0)
    , _name ()
    , _description ()
  { }

  var (const std::string & n)
    : _mutex ()
    , _modified (false)
    , _size (0)
    , _name (n)
    , _description ()
  { }

  virtual ~var ()
  {
    clear ();
  }

  //  -1   - for simple variable
  // other - number of members in structure
  virtual int entries () const = 0;

  virtual size_t size () = 0;

  virtual void read (std::basic_istream<char> &) = 0;
  virtual void write (std::basic_ostream<char> &) = 0;

  virtual void loadAll () = 0;  // loads entire variable in memory

  virtual void clear () {}

  virtual bool modified () const;

  const std::string & name (const std::string &);
  const std::string & description (const std::string &);

  const std::string & name () const;
  const std::string & description () const;
};

class hub : public var
{
  typedef std::multimap<const std::string,
			var::upointer_type> container_type;

public:
  typedef hubcore::iterator                hubiter;
  typedef hubcore::key_type                key_type;
  typedef hubcore::value_type                value_type;
  typedef cBaseVarHandle                        data_type;
  
  typedef std::list< data_type >        tDList;
  typedef tDList::iterator                tDListIter;

protected:
        tDList olist;

        // An internal class. You should not attempt to use it directly
        struct get_modified {
                bool        operator () ( value_type & v ) const {
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

        void        append ( const data_type & );
        void        erase ( const key_type & );

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

}        /* namespace ObjectIO        */
#endif/* CROSS_OIO_FILEVAR_H        */
