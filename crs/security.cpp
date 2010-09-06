// (c) Apr 17, 2008 Oleg N. Peregudov
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#endif

#include <crs/security.h>

namespace CrossClass {

cSaveStreamExceptions::cSaveStreamExceptions ( std::basic_ios<char, std::char_traits<char> > * s, std::ios_base::iostate def_state )
	: _stream( s )
{
	_state = s->exceptions();
      s->exceptions( def_state );
}

cSaveStreamExceptions::~cSaveStreamExceptions ()
{
	_stream->clear ();                  // clear error state
      _stream->exceptions ( _state );     // restore exceptions
}

cSaveIStreamPosition::cSaveIStreamPosition ( std::istream * s )
	: _stream( s )
	, _position( 0 )
{
	if( _stream )
		_position = _stream->tellg();
}

cSaveIStreamPosition::~cSaveIStreamPosition ()
{
	if( _stream )
		_stream->seekg( _position );
}

cSaveOStreamPosition::cSaveOStreamPosition ( std::ostream * s )
	: _stream( s )
	, _position( 0 )
{
	if( _stream )
		_position = _stream->tellp();
}

cSaveOStreamPosition::~cSaveOStreamPosition ()
{
	if( _stream )
		_stream->seekp( _position );
}

} // namespace CrossClass
