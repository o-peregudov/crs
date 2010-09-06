#ifndef PARSING_H_INCLUDED
#define PARSING_H_INCLUDED 1
//
//  PARSING.H
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
//	Dec 5, 2007 - DLL
//	Mar 10, 2008 - tab2col member
//	Apr 15, 2008 - table parser
//	May 6, 2008 - full stream parser - tab2fixed
//	Aug 26, 2008 - predicate versions of scan_token
//	Sep 19, 2008 - smart_numerals
//

#include <fstream>
#include <sstream>
#include <deque>
#include <crs/libexport.h>

namespace parsing {

struct CROSS_EXPORT scan_token_predicate
{
	virtual bool check ( const char ch ) = 0;
};

struct CROSS_EXPORT scan_token_space_predicate : scan_token_predicate
{
	virtual bool check ( const char ch );
	//	return isspace( ch );
};

extern CROSS_EXPORT std::string predicate_scan_token ( std::string &, scan_token_predicate & prd );
extern CROSS_EXPORT std::string predicate_scan_token ( std::istream &, scan_token_predicate & prd );

extern CROSS_EXPORT std::string scan_token ( std::istream & );
extern CROSS_EXPORT std::string scan_token ( std::string & );

extern CROSS_EXPORT std::streampos search_forward ( std::fstream &, const std::string & );

extern CROSS_EXPORT std::string setCenter ( const std::string & str, const size_t nWidth, const char fillChar = '\x20' );
extern CROSS_EXPORT std::string tab2fixed ( const std::string & table, const size_t nExtraSpaces = 2, const char lineChar = '-' );

extern CROSS_EXPORT std::ostream & tab2fixed ( std::istream & mainStream, std::ostream & outStream, const size_t tabSize = 6, const size_t nExtraSpaces = 2, const char lineChar = '-' );

template <typename measure>
void	readAndMeasureTable (
	measure & baseMetter,
	std::basic_istream<char> & mainStream,
	std::deque< std::deque< std::basic_string< typename measure::char_type > > > & table,
	std::deque< typename measure::size_type > & colWidth )
{
	std::basic_string<char> lineBuffer, cell;
	std::basic_istringstream<char> lineStream;
	typename measure::size_type cellWidth;
	
	while( mainStream.good() )
	{
		std::getline( mainStream, lineBuffer );
		if( mainStream.eof() && ( lineBuffer == "" ) )
			break;
		
		table.push_back( std::deque< std::basic_string< typename measure::char_type > > () );
		
		lineStream.clear();
		lineStream.str( lineBuffer );
		while( lineStream.good() )
		{
			std::getline( lineStream, cell, '\t' );
			if( lineStream.eof() && ( cell == "" ) )
				break;
			
			table.back().push_back( baseMetter.convert( cell ) );
			if( colWidth.size() < table.back().size() )
				colWidth.resize( table.back().size() );
			
			cellWidth = baseMetter( table.back().back() );
			if( colWidth[ table.back().size() - 1 ] < cellWidth )
				colWidth[ table.back().size() - 1 ] = cellWidth;
		}
	}
}

//
// adds suffix to numeral (makes '1st', '2nd', '3rd' etc.)
//
extern CROSS_EXPORT std::string smart_numeral ( const unsigned long num );

} // namespace parsing
#endif // PARSING_H_INCLUDED
