#ifndef CRS_HEAP_H_INCLUDED
#define CRS_HEAP_H_INCLUDED 1
/*
 *  crs/heap.h - heap container (vector based)
 *  Copyright (c) 2012-2016 Oleg N. Peregudov <o.peregudov@gmail.com>
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
#include <vector>

namespace crs
{
  template <class Type, class Comp>
  class heap : protected std::vector<Type>
  {
  protected:
    Comp _comp;

    size_t _ancestor_index (const size_t node_index) const;
    size_t _descendant_index (const size_t node_index) const;

    size_t _bubleup_heap (size_t node_index);
    void _bubledown_heap (size_t node_index, const size_t sz);

  public:
    using std::vector<Type>::size;
    using std::vector<Type>::empty;
    using std::vector<Type>::clear;
    using std::vector<Type>::reserve;
    using std::vector<Type>::capacity;

    heap (const size_t sz = 0);
    heap (const size_t sz, const Comp & cmp);

    size_t insert (const Type & val);
    void remove (const size_t node_index);

    const Type & at (const size_t node_index) const;
    const Type & peek () const;
    Type get ();

    void swap (heap & h);
  };

  /*
   *
   * members of class heap <>
   *
   */
  template <class Type, class Comp>
  heap<Type, Comp>::heap (const size_t sz)
    : std::vector<Type> ()
    , _comp ()
  {
    reserve (sz);
  }

  template <class Type, class Comp>
  heap<Type, Comp>::heap (const size_t sz, const Comp & cmp)
    : std::vector<Type> ()
    , _comp (cmp)
  {
    reserve (sz);
  }

  template <class Type, class Comp>
  size_t
  heap<Type, Comp>::_ancestor_index (const size_t node_index) const
  {
    if (node_index == 0)
      {
	return 0;
      }

    if (node_index & static_cast<const size_t> (1))
      {
	return (node_index >> 1);
      }

    return ((node_index - 1) >> 1);
  }

  template <class Type, class Comp>
  size_t
  heap<Type, Comp>::_descendant_index (const size_t node_index) const
  {
    return ((node_index << 1) + 1);
  }

  template <class Type, class Comp>
  size_t
  heap<Type, Comp>::_bubleup_heap (size_t node_index)
  {
    size_t ancr_index = _ancestor_index (node_index);
    while ((ancr_index < node_index) &&
	   _comp ((*this)[node_index], (*this)[ancr_index]))
      {
	std::swap ((*this)[node_index], (*this)[ancr_index]);
	node_index = ancr_index;
	ancr_index = _ancestor_index (node_index);
      }
    return node_index;
  }

  template <class Type, class Comp>
  void
  heap<Type, Comp>::_bubledown_heap (size_t node_index, const size_t sz)
  {
    size_t desc_index = _descendant_index (node_index);
    while (desc_index < sz)
      {
	if (((desc_index + 1) < sz) &&
	    _comp ((*this)[desc_index + 1], (*this)[desc_index]))
	  {
	    ++desc_index;
	  }

	if (_comp ((*this)[desc_index], (*this)[node_index]))
	  {
	    std::swap ((*this)[node_index], (*this)[desc_index]);
	    node_index = desc_index;
	    desc_index = _descendant_index (node_index);
	  }
	else
	  {
	    break;
	  }
      }
  }

  template <class Type, class Comp>
  size_t
  heap<Type, Comp>::insert (const Type & val)
  {
    std::vector<Type>::push_back (val);
    return _bubleup_heap (size () - 1);
  }

  template <class Type, class Comp>
  void
  heap<Type, Comp>::remove (const size_t node_index)
  {
    if ((node_index < size ()) && (node_index != (size () - 1)))
      {
	std::swap ((*this)[node_index], (*this)[size () - 1]);
	_bubleup_heap (node_index);
	_bubledown_heap (node_index, size () - 1);
      }
    std::vector<Type>::pop_back ();
  }

  template <class Type, class Comp>
  const Type &
  heap<Type, Comp>::peek () const
  {
    return std::vector<Type>::front ();
  }

  template <class Type, class Comp>
  Type
  heap<Type, Comp>::get ()
  {
    Type r = peek ();
    if (1 < size ())
      {
	std::swap ((*this)[0], (*this)[size () - 1]);
	_bubledown_heap (0, size () - 1);
      }
    std::vector<Type>::pop_back ();
    return r;
  }

  template <class Type, class Comp>
  const Type &
  heap<Type, Comp>::at (const size_t node_index) const
  {
    return std::vector<Type>::operator [] (node_index);
  }

  template <class Type, class Comp>
  void
  heap<Type, Comp>::swap (heap<Type, Comp> & h)
  {
    std::vector<Type>::swap (h);
  }
} /* namespace crs */
#endif /* CRS_HEAP_H_INCLUDED */
