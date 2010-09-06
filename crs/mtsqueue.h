#ifndef CROSS_MTSQUEUE_H_INCLUDED
#define CROSS_MTSQUEUE_H_INCLUDED 1
//
//  MTSQUEUE.H - Multi-thread safe queue (deque based)
//  Copyright (c) Jun 1, 2007 Oleg N. Peregudov <op@pochta.ru>
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
//	Nov 21, 2007 - new place for cross-compiling routines
//	Jan 16, 2009 - push is now returns nothing (void)
//

#include <deque>
#include <crs/security.h>

namespace CrossClass {

//
// class cMTSQueue (concrete)
// provides multi-thread safe push and pop operations
// main idea: perform all memory-dependent operations within one transaction
//
template <typename Type> class cMTSQueue
{
public:
      typedef typename std::deque<Type>                container_type;
      typedef typename container_type::value_type      value_type;
      typedef typename container_type::size_type       size_type;
      
protected:
	mutable CrossClass::LockType _central_lock;
      container_type _container;
      size_type _cursor;
      
public:
      cMTSQueue ()
      	: _central_lock()
      	, _container( )
      	, _cursor( 0 )
      { }
      
      bool empty () const
	{
		CrossClass::_LockIt exclusive_access ( _central_lock, true );
            return _cursor == _container.size();
      }
      
      size_type size () const
	{
		CrossClass::_LockIt exclusive_access ( _central_lock, true );
            return ( _container.size() - _cursor );
      }
      
      value_type & front ()
	{
		CrossClass::_LockIt exclusive_access ( _central_lock, true );
            return _container[ _cursor ];
      }
      
      value_type & back ()
	{
		CrossClass::_LockIt exclusive_access ( _central_lock, true );
            return _container.back();
      }
      
      const value_type & front () const	
	{
		CrossClass::_LockIt exclusive_access ( _central_lock, true );
            return _container[ _cursor ];
      }
      
      const value_type & back () const
	{
		CrossClass::_LockIt exclusive_access ( _central_lock, true );
            return _container.back();
      }
      
      void  pop ()
	{
		if( !empty() )
		{
			CrossClass::_LockIt exclusive_access ( _central_lock, true );
                  ++_cursor;
		}
      }
	
	void  push ( const value_type & x )
	{
		CrossClass::_LockIt exclusive_access ( _central_lock, true );
		_container.push_back( x );
		if( _cursor )
		{
			_container.pop_front();
			--_cursor;
		}
	}
};

} // namespace CrossClass
#endif // CROSS_MTSQUEUE_H_INCLUDED
