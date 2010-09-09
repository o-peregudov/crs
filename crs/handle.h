#ifndef CROSS_HANDLE_H
#define CROSS_HANDLE_H 1

//
//  HANDLE.H - handle classes and templates
//  Copyright (c) May 23, 2007 Oleg N. Peregudov <op@pochta.ru>
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
//	Dec 5, 2007 - DLL
//	Aug 27, 2010 - compartibility with C++0x locks
//

#include <crs/security.h>

namespace CrossClass {

struct CROSS_EXPORT handleCounter
{
	LockType lock;
	long nlinks;
	
	handleCounter ( const long nStartCounter = 0 )
		: lock()
		, nlinks( nStartCounter )
	{ }
};

//
// template class cHandle (unintrusive handle)
//
// Looks like a pointer to an instance of class Type. Class Type must not have any additional members.
// NOTE: Avoid creating handles to a static or automatic objects!
//
template <class Type> class cHandle
{
protected:
	Type * object;
	handleCounter * hcounter;
      
      void  addhandle ()
	{
		_LockIt exclusive_access ( hcounter->lock );
		++(hcounter->nlinks);
      }
      
	long  releasehandle ()
	{
		_LockIt exclusive_access ( hcounter->lock );
		return --(hcounter->nlinks);
      }
      
	void  unbind ()
	{
	      if( *this )
	      {
	            if( releasehandle() == 0 )
	            {
				delete hcounter;
				hcounter = 0;
	                  
	                  delete object;
	                  object = 0;
	            }
	      }
	}
      
	void	assign ( Type * const objectptr, handleCounter * const hcounterptr )
	{
		if( (object = objectptr) )
      	{
			hcounter = hcounterptr;
	            addhandle();
		}
	}
	
	void  assign ( const cHandle & o )
	{
	      if( object != o.object )
	      {
	            unbind();
			assign( o.object, o.hcounter );
	      }
	}
	
public:
      cHandle ( Type * ptr = 0 )
		: object( 0 )
		, hcounter( 0 )
	{
		bind( ptr );
	}
      
	cHandle ( const cHandle & o )
		: object( 0 )
		, hcounter( 0 )
	{
		assign( o );
	}
      
	cHandle ( Type * const objectptr, handleCounter * const hcounterptr )
		: object( objectptr )
		, hcounter( hcounterptr )
	{
		if( object && hcounter )
			addhandle();
		else
			object = 0, hcounter = 0;
	}
      
	~cHandle ()
	{
		unbind();
	}
      
      cHandle & operator = ( Type * ptr )
	{
		bind( ptr );
		return *this;
	}
      
	cHandle & operator = ( const cHandle & o )
	{
		assign( o );
		return *this;
	}
      
      void bind ( Type * ptr )
	{
	      if( object != ptr )
	      {
	            unbind();
	            if( ( object = ptr ) )
				hcounter = new handleCounter ( 1 );
	      }
	}
      
      Type * const get () const				{ return object; }
      Type * const operator -> () const               { return object; }
      Type & operator * () const                      { return *object; }
      
      bool operator ! () const                        { return (object == 0); }
      bool operator == ( const cHandle & o ) const	{ return (object == o.object); }
      bool operator != ( const cHandle & o ) const	{ return (object != o.object); }
      
      operator bool () const                          { return (object != 0); }
	
	template <class AnyType> operator cHandle<AnyType> () const
	{
		return cHandle<AnyType> ( dynamic_cast<AnyType*>( object ), hcounter );
	}
};

} // namespace CrossClass
#endif // CROSS_HANDLE_H
