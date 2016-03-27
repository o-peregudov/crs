/*
 *  SECURITY.H - RAII handle classes and templates
 *  Copyright (c) Aug 20, 2007 Oleg N. Peregudov <o.peregudov@gmail.com>
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
 *
 *	2007/11/21	new place for cross-compiling routines
 *	2007/12/05	DLL
 *	2010/08/30	cLocker compartibility with std::unique_lock from C++0x standard
 *	2010/12/01	translation for some comments
 */
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#endif

#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif

#include <crs/security.h>

namespace CrossClass {

cSaveStreamExceptions::cSaveStreamExceptions ( std::basic_ios<char, std::char_traits<char> > * s, std::ios_base::iostate def_state )
	: _stream( s )
{
	_state = s->exceptions();
	s->exceptions (def_state);
}

cSaveStreamExceptions::~cSaveStreamExceptions ()
{
	_stream->clear ();			/* clear error state	*/
	_stream->exceptions (_state);		/* restore exceptions	*/
}

cSaveIStreamPosition::cSaveIStreamPosition ( std::istream * s )
	: _stream( s )
	, _position( 0 )
{
	if (_stream)
		_position = _stream->tellg ();
}

cSaveIStreamPosition::~cSaveIStreamPosition ()
{
	try
	{
		if (_stream)
			_stream->seekg (_position);
	}
	catch (...)
	{
#if defined (DESTRUCTOR_EXCEPTIONS_ALLOWED)
		throw;
#endif
	}
}

cSaveOStreamPosition::cSaveOStreamPosition ( std::ostream * s )
	: _stream( s )
	, _position( 0 )
{
	if (_stream)
		_position = _stream->tellp ();
}

cSaveOStreamPosition::~cSaveOStreamPosition ()
{
	try
	{
		if (_stream)
			_stream->seekp (_position);
	}
	catch (...)
	{
#if defined (DESTRUCTOR_EXCEPTIONS_ALLOWED)
		throw;
#endif
	}
}

} // namespace CrossClass
