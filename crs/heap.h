#ifndef CROSS_HEAP_H
#define CROSS_HEAP_H 1
/*
 *  crs/heap.h - heap container (vector based)
 *  Copyright (c) 2012 Oleg N. Peregudov <o.peregudov@gmail.com>
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
 *	2012-12-22	added to library
 */

#include <vector>
#include <algorithm>

namespace CrossClass {

template <class Type, class Compare = std::less<Type> >
class heap
{
protected:
	std::vector<Type>	_container;
	Compare		_comp;
	
	size_t num_nodes ( const size_t level ) const
	{
		return (1 << level);
	}
	
	size_t level_index ( const size_t level ) const
	{
		return (num_nodes (level) - 1);
	}
	
	size_t num_levels ( size_t num_items ) const
	{
		size_t result = 0;
		while (num_items > 0)
		{
			++result;
			if (num_items > 1)
				num_items >>= 1;
			else
				break;
		}
		return result;
	}
	
	size_t ancestor_index ( const size_t node_index ) const
	{
		if (node_index == 0)
			return 0;
		if (node_index & static_cast<const size_t> (1))
			return (node_index >> 1);
		else
			return ((node_index - 1) >> 1);
	}
	
	size_t descendant_index ( const size_t node_index ) const
	{
		return ((node_index << 1) + 1);
	}
	
	size_t invariant ( size_t node_index )
	{
		size_t neighbour_index = ancestor_index (node_index);
		while ((node_index > 0) && (_comp (_container [node_index], _container [neighbour_index])))
		{
			std::swap (_container [node_index], _container [neighbour_index]);
			node_index = neighbour_index;
			neighbour_index = ancestor_index (node_index);
		}
		
		neighbour_index = descendant_index (node_index);
		while (neighbour_index < _container.size ())
		{
			if ((neighbour_index + 1) < _container.size ())
			{
				if (_comp (_container [neighbour_index + 1], _container [neighbour_index]))
					++neighbour_index;
			}
			if (_comp (_container [neighbour_index], _container [node_index]))
			{
				std::swap (_container [node_index], _container [neighbour_index]);
				node_index = neighbour_index;
				neighbour_index = descendant_index (node_index);
			}
			else
				break;
		}
		
		return node_index;
	}
	
public:
	heap () : _container (), _comp () {}
	
	void reserve ( const size_t sz )	{ _container.reserve (sz); }
	
	bool empty () const			{ return _container.empty (); }
	size_t size () const			{ return _container.size (); }
	size_t capacity () const		{ return _container.capacity (); }
	
	const Type & peek () const		{ return _container.front (); }
	
	Type get ()
	{
		Type r = peek ();
		if (_container.size () == 1)
			_container.pop_back ();
		else
		{
			std::swap (_container.front (), _container.back ());
			_container.pop_back ();
			invariant (0);
		}
		return r;
	}
	
	size_t insert ( const Type value )
	{
		_container.push_back (value);
		return invariant (_container.size () - 1);
	}
};

}	/* namespace CrossClass	*/
#endif/* CROSS_HEAP_H		*/
