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

#if defined( HAVE_CONFIG_H )
#  include "config.h"
#endif

#include "crs/oio/FileVar.h"

namespace ObjectIO
{
  bool var::modified () const
  {
    lock_type guard (_mutex);
    return _modified;
  }

  const	std::string & var::name (const std::string & n)
  {
    lock_type guard (_mutex);
    return _name = n;
  }

  const	std::string & var::description (const std::string & d)
  {
    lock_type guard (_mutex);
    return _description = d;
  }
  
  const	std::string & var::name () const
  {
    lock_type guard (_mutex);
    return _name;
  }

  const	std::string & var::description () const
  {
    lock_type guard (_mutex);
    return _description;
  }

  void hub::clear ()
  {
    lock_type guard (_mutex);
    hubcore::clear ();
    olist.clear();
  }

  bool hub::modified ()
  {
    get_modified f;
    lock_type guard (_mutex);
    _modified = false;
    for( hubiter first = begin(); first != end(); first++ )
      if( ( _modified |= f( *first ) ) )
	break;
    return _modified;
  }

void	BaseHub::loadAll ()
{
	CrossClass::_LockIt lock ( _mutex );
	std::for_each( olist.begin(), olist.end(), get_all() );
	_modified = 0;
}

void	BaseHub::append ( const data_type & v )
{
	CrossClass::_LockIt lock ( _mutex );
	insert( value_type( v->name(), v ) );
	olist.push_back( v );
	_modified = true;
}

void	BaseHub::erase ( const key_type & v )
{
	CrossClass::_LockIt lock ( _mutex );
	hubiter p1 ( find( v ) );
	if( p1 != end() )
	{
		tDListIter p2 ( std::find( olist.begin(), olist.end(), p1->second ) );
		hubcore::erase( p1 );
		if( p2 != olist.end() )
			olist.erase( p2 );
		_modified = true;
	}
}

} /* namespace ObjectIO	*/
