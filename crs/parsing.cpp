//
//  PARSING.CPP
//  Copyright (c) Mar 20, 2007 Oleg N. Peregudov
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
//	Nov 21, 2007 - new place for cross-compiling routines
//	Feb 7, 2008 - tokens now contains only letters and numbers
//	Mar 10, 2008 - table processing: tab delimited to fixed width columns
//	Apr 7, 2008 - scan_token from string bug fixed
//	May 6, 2008 - full stream parser - tab2fixed
//	Aug 26, 2008 - predicate versions of scan_token
//
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#endif

#include <crs/parsing.h>
#include <crs/security.h>

#include <sstream>
#include <deque>

#define DOUBLE_L  '"'

namespace parsing {

using std::string;
using std::istream;
using std::fstream;
using std::streampos;
using CrossClass::cSaveStreamExceptions;
using CrossClass::cSaveIStreamPosition;

bool scan_token_space_predicate::check ( const char ch )
{
	return ( isspace( ch ) != 0 );
}

std::string scan_token ( std::istream & stream )
{
	scan_token_space_predicate isSpace;
	return predicate_scan_token( stream, isSpace );
}

std::string scan_token ( std::string & string )
{
	scan_token_space_predicate isSpace;
	return predicate_scan_token( string, isSpace );
}

string predicate_scan_token ( string & s, scan_token_predicate & prd )
{
	size_t i = 0, idxFrom;
	
	// eat whitespaces ...
	while( ( i < s.size() ) && ( isspace( s[ i ] ) ) ) ++i;
	idxFrom = i;
	
	for( bool longString = false; i < s.size(); ++i )
	{
		if( s[ i ] == DOUBLE_L )
		{
			if( longString )
				break;
			else
			{
				longString = true;
				++idxFrom;
			}
		}
		else if( longString )
			continue;
		else if( prd.check( s[ i ] ) )
			break;
	}
	
	string result = s.substr( idxFrom, i - idxFrom );
	s.erase( 0, i );
	return result;
}

string predicate_scan_token ( istream & is, scan_token_predicate & prd )
{
	char ch = '\0';
	string res = "";
	
	do {  // eat whitespaces ...
		ch = is.get();
	} while ( isspace( ch ) );
	
	for( bool longString = false;; ch = is.get() )
	{
		if( ch == DOUBLE_L )
		{
			if( longString )
				break;
			else
				longString = true;
		}
		else if( longString )
			res += ch;
		else if( prd.check( ch ) )
		{
			if( res.size() )
				is.putback( ch );
			else
				res += ch;
			break;
		}
		else
			res += ch;
	}
	
	return res;
}

std::streampos search_forward ( std::fstream & _bs, const std::string & sample )
{
	size_t match ( 0 );
	cSaveIStreamPosition ip ( &_bs );
	streampos result ( static_cast<streampos>( -1 ) );
	cSaveStreamExceptions se ( &_bs, std::ios_base::goodbit );
	
	for( int ch( EOF ); match < sample.length(); match++ )
	{
		ch = _bs.get();
		if( ch == EOF )
			break;
		else if( ch != sample[match] )
			match = 0;
	}
	if( match == sample.length() )
		result = _bs.tellg() - static_cast<streampos>( match );
	
	_bs.clear();
	
	return result;
}

std::string setCenter ( const std::string & str, const size_t nWidth, const char fillChar )
// place the text in the middle of the field
{
	if( nWidth < str.size() )
		return str;
	size_t nRestSpaces = nWidth - str.size();
	std::string result ( nRestSpaces / 2, fillChar );
	result += str;
	return ( result += std::string( nWidth - result.size(), fillChar ) );
}

struct defaultMeasure
{
	typedef size_t	size_type;
	typedef char	char_type;
	
	size_type operator () ( const std::basic_string<char_type> & s )
	{
		return s.size();
	}
	
	std::basic_string<char_type> convert ( std::basic_string<char_type> & s )
	{
		return s.substr( 0, s.find( '\r' ) );
	}
};

std::string tab2fixed ( const std::string & srcTable, const size_t nExtraSpaces, const char lineChar )
// convert the table to text-printer friendly form (columns of fixed width)
{
	defaultMeasure dm;
	std::deque< size_t > colWidth;
	std::deque< std::deque< std::basic_string< char > > > table;
	std::basic_istringstream< char > mainStream ( srcTable );
	readAndMeasureTable( dm, mainStream, table, colWidth );
	
	size_t nFullWidth = 0;
	std::basic_ostringstream< char > outStream;
	for( size_t nCol = 0; nCol < colWidth.size(); ++nCol )
	{
		colWidth[ nCol ] += nExtraSpaces;
		nFullWidth += colWidth[ nCol ];
		outStream << setCenter( table[ 0 ][ nCol ], colWidth[ nCol ] );
	}
	
	outStream	<< std::endl
			<< std::string( nFullWidth, lineChar )
			<< std::endl;
	
	for( size_t nRow = 1; nRow < table.size(); ++nRow )
	{
		for( size_t nCol = 0; nCol < table[ nRow ].size(); ++nCol )
			outStream << setCenter( table[ nRow ][ nCol ], colWidth[ nCol ] );
		outStream << std::endl;
	}
	
	return outStream.str();
}

std::ostream & tab2fixed ( std::istream & mainStream, std::ostream & outStream, const size_t tabSize, const size_t nExtraSpaces, const char lineChar )
{
	std::string lineBuffer, fieldBuffer;
	std::basic_istringstream<char> lineStream;
	while( mainStream.good() )
	{
		std::getline( mainStream, lineBuffer );
		if( mainStream.eof() && ( lineBuffer == "" ) )
			break;
		
		if( lineBuffer[ 0 ] == '\v' )
		{
			// read, parse and out the table
			std::getline( mainStream, lineBuffer, '\v' );
			outStream << tab2fixed( lineBuffer, nExtraSpaces, lineChar );
		}
		else
		{
			lineStream.clear();
			lineStream.str( lineBuffer );
			while( lineStream.good() )
			{
				std::getline( lineStream, fieldBuffer, '\t' );
				outStream << fieldBuffer;
				if( !lineStream.eof() )
					outStream << std::string( tabSize, '\x20' );
			}
			outStream << std::endl;
		}
	}
	return outStream;
}

//
// add suffix to numeral (makes '1st', '2nd', '3rd' etc.)
//
std::string smart_numeral ( const unsigned long num )
{
      if( num )
      {
		std::basic_ostringstream<char> stream;
		stream << num;
            switch( ( (num % 100 < 10) || (num % 100 > 20) ) ? ( num % 10 ) : num )
            {
            case  1:
                  stream << "st";
                  break;
            
		case  2:
                  stream << "nd";
                  break;
            
		case  3:
                  stream << "rd";
                  break;
            
		default:
                  stream << "th";
            }
            
            return stream.str();
      }
      
      return "zero";
}

} // namespace parsing
