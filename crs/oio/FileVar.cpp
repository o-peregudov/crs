//
//  This file is a part of Object I/O library
//
//  FileVar.cpp - base OIO classes and templates
//  Copyright (C) Nov 16, 2004 Oleg N. Peregudov
//
//  WWW:    http://op.pochta.ru
//  E-Mail: op@pochta.ru
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
//	May 22, 2007 - uniform locks & handles
//	Oct 17, 2007 - ANSI C++ compartibility
//	Dec 6, 2007 - new project name & DLL
//	Jan 24, 2008 - no throw specificator for read\write members
//	Apr 17, 2008 - suppress some warning of VC compiler
//	Sep 8, 2010 - C++0x compartible locks
//

#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#endif

#include <crs/oio/FileVar.h>

namespace ObjectIO {

bool	BaseVar::modified () const
{
	CrossClass::_LockIt lock ( _mutex );
	return _modified;
}

const	std::string & BaseVar::name ( const std::string & nn )
{
	CrossClass::_LockIt lock ( _mutex );
	return _name = nn;
}

const	std::string & BaseVar::description ( const std::string & nd )
{
	CrossClass::_LockIt lock ( _mutex );
	return _description = nd;
}

const	std::string & BaseVar::name () const
{
	CrossClass::_LockIt lock ( _mutex );
	return _name;
}

const	std::string & BaseVar::description () const
{
	CrossClass::_LockIt lock ( _mutex );
	return _description;
}

void	BaseHub::clear ()
{
	CrossClass::_LockIt lock ( _mutex );
	hubcore::clear();
	olist.clear();
}

bool	BaseHub::modified ()
{
	get_modified f;
	CrossClass::_LockIt lock ( _mutex );
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

} // namespace ObjectIO
