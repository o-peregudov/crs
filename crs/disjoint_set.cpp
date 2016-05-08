/*
 *  crs/disjoint_set.cpp
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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <crs/disjoint_set.h>

namespace CrossClass
{
  disjoint_set::disjoint_set (const size_t sz)
    : _cont (sz, point_data_t ())
    , _ngroups (sz)
  {
  }

  size_t disjoint_set::make_set (const size_t vx)
  {
    return (_cont[vx].parent = vx);
  }

  size_t disjoint_set::find (const size_t vx)
  {
    size_t parent = _get_parent (vx);
    if (parent == static_cast<size_t> (-1))
      {
        parent = make_set (vx);
      }
    else
      {
        while (parent != _get_parent (parent))
          {
            parent = _get_parent (parent);
          }
        _cont[vx].parent = parent;
      }
    return parent;
  }

  void disjoint_set::join (const size_t vx, const size_t vy) //
  {
    size_t xroot = find (vx);
    size_t yroot = find (vy);

    if (xroot == yroot)
      {
        return;
      }

    if (_cont[xroot].rank < _cont[yroot].rank)
      {
        _cont[xroot].parent = yroot;
      }
    else if (_cont[yroot].rank < _cont[xroot].rank)
      {
        _cont[yroot].parent = xroot;
      }
    else
      {
        _cont[yroot].parent = xroot;
        ++(_cont[xroot].rank);
      }

    --_ngroups;
  }

  void disjoint_set::clear ()
  {
    _ngroups = _cont.size ();
    _cont.assign (_cont.size (), point_data_t ());
  }
} /* namespace CrossClass */
