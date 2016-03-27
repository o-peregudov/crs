#ifndef CROSS_DISJOINT_SET_H
#define CROSS_DISJOINT_SET_H 1
/*
 *  crs/disjoint_set.h - disjoint set data structure (vector based)
 *  Copyright (c) 2013 Oleg N. Peregudov <o.peregudov@gmail.com>
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
 *	2013/09/22	added to library
 */

#include <vector>
#include <algorithm>

namespace CrossClass
{
  class disjoint_set
  {
  protected:
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
    disjoint_set (const size_t sz = 0);
    
    size_t MakeSet (const size_t vx);
    size_t Find (const size_t vx);
    void Union (const size_t vx, const size_t vy);
    
    void Clear ();
    size_t ngroups () const
    {
      return _ngroups;
    }
  };
  
}	/* namespace CrossClass		*/
#endif	/* CROSS_DISJOINT_SET_H		*/
