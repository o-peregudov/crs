#ifndef CRS_INTERVAL_MAP_H_INCLUDED
#define CRS_INTERVAL_MAP_H_INCLUDED 1
/*
 *  crs/interval_map.h
 *  Copyright (c) 2013-2016 Oleg N. Peregudov <o.peregudov@gmail.com>
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

/*
  interval_map<K,V> is a data structure that efficiently associates
  intervals of keys of type K with values of type V.

  Each key-value-pair (k,v) in the map means that the value v is
  associated to the interval from k (including) to the next key
  (excluding) in map.

  Example: the map (0,'A'), (3,'B'), (5,'A') represents the mapping
  0 -> 'A'
  1 -> 'A'
  2 -> 'A'
  3 -> 'B'
  4 -> 'B'
  5 -> 'A'
  6 -> 'A'
  7 -> 'A'
  ... all the way to numeric_limits<key>::max()

  The representation in map is canonical, that is, consecutive map
  entries doesn't have the same value:
  ..., (0,'A'), (3,'A'), ... is not allowed.

  Initially, the whole range of K is associated with a given initial
  value, passed to the constructor.

  Key type K
  - besides being copyable and assignable, is less-than comparable via
  operator< ;
  - is bounded below, with the lowest value being
  std::numeric_limits<K>::min();
  - does not implement any other operations, in particular no equality
  comparison or arithmetic operators.

  Value type V
  - besides being copyable and assignable, is equality-comparable via
  operator== ;
  - does not implement any other operations.
*/

#include <map>
#include <limits>

namespace crs
{
  template<class K, class V>
  class interval_map
  {
    typedef std::map<K, V>                    container_type;
    typedef typename container_type::iterator iterator;

    container_type _map;

  public:
    /*
     * Constructor associates whole range of K with val
     * by inserting (K_min, val) into the map
     */
    interval_map ( const V & val)
    {
      _map.insert (_map.begin (),
                   std::make_pair (std::numeric_limits<K>::min (),
                                   val));
    }

    /*
     * Assign value val to interval [keyBegin, keyEnd).
     *
     * Overwrite previous values in this interval. Do not change
     * values outside this interval.
     * Conforming to the C++ Standard Library conventions,
     * the interval includes keyBegin, but excludes keyEnd.
     * If !( keyBegin < keyEnd ), this designates an empty interval,
     * and assign do nothing.
     */
    bool assign (const K & keyBegin, const K & keyEnd, const V & val)
    {
      if (!(keyBegin < keyEnd))
        {
          return false;
        }

      iterator it;
      std::pair<iterator, bool> right = _map.insert (std::make_pair (keyEnd, val));
      if (right.second == true)
        {
          --(it = right.first);
          if (!(it->second == right.first->second))
            {
              right.first->second = it->second;
            }
        }

      std::pair<iterator, bool> left = _map.insert (std::make_pair (keyBegin, val));
      if (left.second == false)
        {
          left.first->second = val;
        }

      it = left.first;
      if (it != _map.begin ())
        {
          if (!((--it)->second == left.first->second))
            {
              it = left.first;
              ++left.first;
            }
        }
      else
        {
          ++left.first;
        }

      if ((right.first != _map.end ()) &&
          (it->second == right.first->second))
        {
          ++right.first;
        }

      _map.erase (left.first, right.first);
      return true;
    }

    /*
     * Look-up of the value associated with key
     */
    const V & operator[] (const K & key ) const
    {
      return ( --_map.upper_bound (key) )->second;
    }

    size_t size () const
    {
      return _map.size ();
    }
  };
} /* namespace crs */
#endif /* CRS_INTERVAL_MAP_H_INCLUDED */
