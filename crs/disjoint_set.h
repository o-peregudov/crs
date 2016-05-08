#ifndef CROSS_DISJOINT_SET_H_INCLUDED
#define CROSS_DISJOINT_SET_H_INCLUDED 1
/*
 *  crs/disjoint_set.h - disjoint set data structure (vector based)
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

#include <crs/libexport.h>
#include <vector>
#include <algorithm>

namespace CrossClass
{
  class disjoint_set
  {
    struct point_data_t
    {
      size_t parent;
      size_t rank;

      point_data_t ()
	: parent (static_cast<size_t> (-1))
	, rank (0)
      { }
    };

    std::vector<point_data_t> _cont;
    size_t _ngroups;

    const size_t & _get_parent (const size_t vx) const
    {
      return _cont[vx].parent;
    }

  public:
    explicit disjoint_set (const size_t sz = 0);

    void clear ();
    size_t make_set (const size_t vx);
    size_t find (const size_t vx);

    /* union operation */
    void join (const size_t vx, const size_t vy);

    size_t ngroups () const
    {
      return _ngroups;
    }
  };
} /* namespace CrossClass */
#endif /* CROSS_DISJOINT_SET_H_INCLUDED */
