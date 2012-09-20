#ifndef CROSS_UDE_DATA_CODE_H_INCLUDED
#define CROSS_UDE_DATA_CODE_H_INCLUDED 1

/*
 *  crs/ude/data_core.h - General Hardware-to-PC interface ( base classes )
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
 *	2012/09/16	generalized data containers that is used for UDE device
 *			management
 */

#include <algorithm>
#include <vector>
#include <map>

namespace ude {

template <class element_type>
struct pointer_destroy_predicate
{
	bool	pointer_is_non_null;
	
	pointer_destroy_predicate ( element_type * e )
		: pointer_is_non_null (e != 0)
	{ }
	
	pointer_destroy_predicate ( const pointer_destroy_predicate & o )
		: pointer_is_non_null (o.pointer_is_non_null)
	{ }
	
	operator bool () const
	{
		return pointer_is_non_null;
	}
};

/*
 * template class element_list
 *	+ stores pointers to elements
 *	+ can add element
 * 	+ can remove particular element
 *	+ performs very simple memory management for elements
 *	+ can do some action with elements
 */

template <class element_type, class destroy_predicate = pointer_destroy_predicate<element_type> >
class element_list
{
	element_list ( const element_list & );
	element_list & operator = ( const element_list & );
	
protected:
	typedef std::vector<element_type *>		container_type;
	typedef typename container_type::iterator	container_iterator;
	
	struct element_destroyer
	{
		void operator () ( element_type * e ) const
		{
			if (destroy_predicate (e))
				delete e;
		}
	};
	
	container_type	_container;
	element_destroyer	_destroy_element;
	
public:
	element_list ()
		: _container ()
		, _destroy_element ()
	{ }
	
	~element_list ()
	{
		std::for_each (_container.begin (), _container.end (), _destroy_element);
	}
	
	bool	empty () const
	{
		return _container.empty ();
	}
	
	void	add ( element_type * element )
	{
		if (element != 0)
			_container.push_back (element);
	}
	
	void	remove ( element_type * element )
	{
		if (_container.empty ())
			return;
		
		container_iterator l = _container.end ();
		for (container_iterator i = _container.begin (); i != l; ++i)
		{
			if (*i == element)
			{
				--l;
				std::swap (*i, *l);
				_destroy_element (*l);
				if (i == l)
					break;
			}
		}
		
		/* shrink vector	*/
		_container.erase (l, _container.end ());
	}
	
	/*
	 * class some_action must have function call member
	 * 	bool operator () (element_type * e)
	 * and if it returns true for the particular element than this element
	 * will be removed from the list and (depending on the result of
	 * destroy_predicate) destroyed
	 */
	template <class some_action>
	void	for_each ( some_action action )
	{
		if (_container.empty ())
			return;
		
		container_iterator l = _container.end ();
		for (container_iterator i = _container.begin (); i != l; ++i)
		{
			if (action (*i))
			{
				--l;
				std::swap (*i, *l);
				_destroy_element (*l);
				if (i == l)
					break;
			}
		}
		
		/* shrink vector	*/
		_container.erase (l, _container.end ());
	}
};

/*
 * template class element_map
 *	+ stores several element_lists
 *	+ can add element
 * 	+ can remove particular element
 *	+ performs very simple memory management for elements
 *	+ can do some action with elements
 */

template <class key_type, class element_type, class destroy_predicate = pointer_destroy_predicate<element_type> >
class element_map
{
	element_map ( const element_map & );
	element_map & operator = ( const element_map & );
	
protected:
	typedef element_list<element_type, destroy_predicate>	elist;
	typedef std::pair<const key_type, elist *>		epair;
	typedef std::map<key_type, elist *>				container_type;
	typedef typename container_type::iterator			container_iterator;
	
	struct element_list_destroyer
	{
		void operator () ( epair & p ) const
		{
			delete p.second;
		}
	};
	
	template <class some_action>
	struct element_list_action
	{
		some_action action;
		
		element_list_action ( some_action act )
			: action (act)
		{ }
		
		void operator () ( epair & p )
		{
			p.second->for_each (action);
		}
	};
	
	container_type		_container;
	element_list_destroyer	_destroy_list;
	
public:
	element_map ()
		: _container ()
		, _destroy_list ()
	{ }
	
	~element_map ()
	{
		std::for_each (_container.begin (), _container.end (), _destroy_list);
	}
	
	void	add ( const key_type domain, element_type * element )
	{
		if (element != 0)
		{
			std::pair<container_iterator, bool> insert_result = _container.insert (epair (domain, 0));
			if (insert_result.second)
				(insert_result.first)->second = new elist ();
			(insert_result.first)->second->add (element);
		}
	}
	
	void	remove ( const key_type domain, element_type * element )
	{
		container_iterator domain_iterator = _container.find (domain);
		if (domain_iterator != _container.end ())
			domain_iterator->second->remove (element);
	}
	
	/*
	 * see description of element_list::for_each
	 */
	template <class some_action>
	void	for_each ( const key_type domain, some_action action )
	{
		container_iterator domain_iterator = _container.find (domain);
		if (domain_iterator != _container.end ())
			domain_iterator->second->for_each (action);
	}
	
	template <class some_action>
	void	for_each ( some_action action )
	{
		std::for_each (_container.begin (), _container.end (), element_list_action<some_action> (action));
	}
};

}	/* namespace ude				*/
#endif/* CROSS_UDE_DATA_CODE_H_INCLUDED	*/
