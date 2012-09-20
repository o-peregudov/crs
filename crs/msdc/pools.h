#ifndef CROSS_MSDC_POOLS_H
#define CROSS_MSDC_POOLS_H 1
/*
 *  General mass spectrum presentation database
 *
 *  crs/msdc/pools.h
 *  Copyright (c) 2007-2012 Oleg N. Peregudov <o.peregudov@gmail.com>
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
 *	2007/11/30	general mass spectrum representation data base
 *	2010/09/07	conforming libcrs v1.0.x
 *	2010/09/28	non standard temporary file names in Win32
 *	2010/12/07	included into libcrs
 */

#include <crs/security.h>
#include <crs/msdc/data_classes.h>
#include <fstream>
#include <string>

namespace msdc {

//
// class basic_pool (abstract)
//
class CROSS_EXPORT basic_pool
{
protected:
	const size_t nRecordSize,
		 	 nByteRecordSize,
			 nFullSize;
	size_t	 nCount;
	
public:
	typedef cIntensityPoint record;
	
	basic_pool ( const size_t nData, const size_t nPoolSize )
		: nRecordSize( nData )
		, nByteRecordSize( record::getByteSize( nData ) )
		, nFullSize( nPoolSize )
		, nCount( 0 )
	{ }
	
	basic_pool ( const basic_pool & o )
		: nRecordSize( o.nRecordSize )
		, nByteRecordSize( o.nByteRecordSize )
		, nFullSize( o.nFullSize )
		, nCount( 0 )
	{ }
	
	virtual ~basic_pool () { }
	
	record * allocateRecord ( const size_t ) const
	{
		return new record( nRecordSize );
	}
	
	virtual void clear ();
	virtual void get ( const size_t, record & ) const = 0;
	virtual bool push ( const record & ) = 0;
};

// forward definitions
class CROSS_EXPORT file_pool;

//
// class mem_pool ( concrete )
//
class CROSS_EXPORT mem_pool : public basic_pool
{
private:
	mem_pool ();
	mem_pool ( const mem_pool & );
	mem_pool & operator = ( const mem_pool & );
	
	friend class file_pool;
	
protected:
	char *		pBigStor;
	cIntensityPoint * vContainer;
	
public:
	mem_pool ( const size_t nData, const size_t nPoolSize );
	virtual ~mem_pool ();
	
	virtual void get ( const size_t, record & ) const;
	virtual bool push ( const record & r );
};

//
// class file_pool ( concrete )
//
class CROSS_EXPORT file_pool : public basic_pool
{
private:
	file_pool ();
	file_pool ( const file_pool & );
	file_pool & operator = ( const file_pool & );
	
protected:
	mutable CrossClass::LockType _mutex;
	mutable std::fstream _stream;
	std::string _path;
	
	void _create_file ();
	
public:
	file_pool ( const size_t nData, const size_t nPoolSize, const std::string & path = "" );
	file_pool ( const mem_pool & mp, const std::string & path = "" );
	
	virtual ~file_pool ();
	
	void assign ( const mem_pool & mp );
	
	virtual void get ( const size_t, record & ) const;
	virtual bool push ( const record & r );
};

}	/* namespace msdc		*/
#endif/* CROSS_MSDC_POOLS_H	*/
