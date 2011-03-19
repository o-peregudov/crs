#ifndef REGIONS_H_INCLUDED
#define REGIONS_H_INCLUDED 1
//
//  REGIONS.H - Regions Tracer (map based)
//  Copyright (c) Sep 14, 2005 Oleg N. Peregudov <op@pochta.ru>
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

#include <map>
#include <limits>

namespace Regions {

//
// class Tracer (concrete)
//
// класс служит для хранения и отслеживания различных диапазонов или
// областей на вещественной оси
//
// хранит только точку начала области (окончание области - начало следующей
// по возрастанию координаты)
//
// вообще-то это контейнер на база std::map
//
template <typename _FLT, typename Type, bool useCache = true>
class Tracer : protected std::map<_FLT, Type>
{
      typedef std::map<_FLT, Type> cCoreContainer;
      
public:
      typedef typename cCoreContainer::value_type value_type;
      typedef typename cCoreContainer::size_type size_type;
      typedef typename cCoreContainer::iterator iterator;
      typedef typename cCoreContainer::key_type key_type;
      
private:
      iterator iRecent, iNext;
      
      bool  inCache ( const key_type & k ) {
		return ( ( iRecent != iNext ) && ( iRecent->first <= k ) &&
			 ( ( iNext == end() ) || ( iNext->first > k ) ) );
	}
      
	void  clearCache () {
		iRecent = iNext = begin();
	}
      
protected:
      Tracer ( const value_type & defaultvalue, int )
		: cCoreContainer()
		, iRecent()
		, iNext()
      {
            insert( defaultvalue );
            clearCache();
      }
      
public:
      cCoreContainer::begin;
      cCoreContainer::end;
      
      Tracer ( const Type & defaultvalue )
		: cCoreContainer()
		, iRecent()
		, iNext()
      {
            insert( value_type( -std::numeric_limits< key_type >::infinity(), defaultvalue ) );
            clearCache();
      }
      
      ~Tracer () {
		cCoreContainer::clear();
	}
      
      bool  empty () {
		return ( cCoreContainer::size() == 1 );
	}
      
      bool  append ( const key_type & k, const Type & v ) {
	      std::pair<iterator, bool> insert_result ( insert( value_type( k, v ) ) );
	      if( insert_result.second ) clearCache();
	      return insert_result.second;
	}
	
      bool  remove ( const key_type & k ) {
		iterator cp ( select( k ) );
		if( cp != end() )
		{
	            clearCache();
			erase( cp );
			return true;
		}
		return false;
	}
      
      void  clear () {
		cCoreContainer::erase( ++begin(), end() );
		clearCache();
	}
      
      void  erase ( iterator q ) {
		if( q != begin() )
		{
	            clearCache();
			cCoreContainer::erase( q );
		}
	}
      
	void  erase ( iterator q1, iterator q2 ) {
		if( q1 == begin() )
	            cCoreContainer::erase( ++q1, q2 );
	      else
			cCoreContainer::erase( q1, q2 );
		clearCache();
	}
      
      size_type erase ( const key_type & k ) {
		if( k == -std::numeric_limits< key_type >::infinity() )
	            return 0;
	      else
			clearCache();
		return cCoreContainer::erase( k );
	}
      
	iterator select ( const key_type & k ) {
	      if( !useCache || !inCache( k ) )
	      {
			iRecent = end();
			if( (--iRecent)->first > k )
			{
	                  iRecent = lower_bound( k );
				if( iRecent->first != k ) --iRecent;
			}
			++(iNext = iRecent);
		}
		return iRecent;
	}
      
      iterator recent () {
		return iRecent;
	}
};

template <typename _FLT>
struct pair : std::pair<_FLT, _FLT>
{
      pair () : std::pair<_FLT, _FLT> () {}
      pair ( const _FLT & v ) : std::pair<_FLT, _FLT> ( v, v ) {}
      pair ( const _FLT & f, const _FLT & s ) : std::pair<_FLT, _FLT> ( f, s ) {}
      
      pair & operator = ( const _FLT & v )
      {
            std::pair<_FLT, _FLT>::first = std::pair<_FLT, _FLT>::second = v;
            return *this;
      }
      
      pair  operator - () const
	{
		return pair( -std::pair<_FLT, _FLT>::first, -std::pair<_FLT, _FLT>::second );
	}
};

} // namespace Regions //

namespace std {

template <typename _FLT> inline
bool  operator < ( const Regions::pair<_FLT> & a, const Regions::pair<_FLT> & b )
{
      return ( a.first < b.first ) ||
             ( ( a.first == b.first ) && ( a.second > b.second ) );
}

} // namespace std //
#endif // REGIONS_H_INCLUDED
