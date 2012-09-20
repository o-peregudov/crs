#ifndef CROSS_MSDC_DATA_CLASSES_H
#define CROSS_MSDC_DATA_CLASSES_H 1
/*
 *  General mass spectrum presentation database
 *
 *  crs/msdc/data_classes.h
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
 *	2010/12/07	included into libcrs
 */

#include <cstring>
#include <crs/libexport.h>

namespace msdc {

#pragma pack(push,1)
struct basic_ipoint
{
	typedef double	time_type;
	typedef void *	link_type;
	typedef double	data_type;
	
	time_type	time;
	link_type	link;
	data_type	intensity[ 1 ];
	
	basic_ipoint ()
		: time( 0 )
		, link( 0 )
		, intensity( )
	{ }
	
	bool	operator < ( const basic_ipoint & o ) const
	{
		return ( time < o.time );
	}
};
#pragma pack(pop)

/*
 * class cIntensityPoint
 *
 *	variable size basic_ipoint with internal memory management
 */
class CROSS_EXPORT cIntensityPoint
{
protected:
	bool	 bLocalStor;
	size_t nByteSize;
	union
	{
		char * pStor;
		basic_ipoint * ip;
	};
	
public:
	static size_t getByteSize ( const size_t nData )
	{
		return ( sizeof( basic_ipoint::data_type ) * ( nData - 1 ) + sizeof( basic_ipoint ) );
	}
	
	static size_t getDataSize ( const size_t nByteSize )
	{
		return ( ( nByteSize - sizeof( basic_ipoint ) ) / sizeof( basic_ipoint::data_type ) + 1 );
	}
	
public:
	cIntensityPoint()
		: bLocalStor( true )
		, nByteSize( 0 )
		, pStor( 0 )
	{ }
	
	cIntensityPoint( const size_t nData, char * pExtStor )
		: bLocalStor( false )
		, nByteSize( getByteSize( nData ) )
		, pStor( pExtStor )
	{ }
	
	cIntensityPoint( const size_t nData, const basic_ipoint::time_type & tm = 0 );
	cIntensityPoint( const cIntensityPoint & o );
	
	~cIntensityPoint ()
	{
		if( bLocalStor && ( nByteSize != 0 ) )
			delete [] pStor;
	}
	
	void	allocate ( const size_t nData, char * pExtStor )
	{
		bLocalStor = false;
		nByteSize = getByteSize( nData );
		pStor = pExtStor;
	}
	
	cIntensityPoint & operator = ( const cIntensityPoint & o );
	cIntensityPoint & linkWith ( const cIntensityPoint & o );
	
	basic_ipoint::data_type & operator [] ( const size_t idx )
	{
		return ip->intensity[ idx ];
	}
	
	const	basic_ipoint::data_type & operator [] ( const size_t idx ) const
	{
		return ip->intensity[ idx ];
	}
	
	basic_ipoint * operator -> ()
	{
		return ip;
	}
	
	const	basic_ipoint * operator -> () const
	{
		return ip;
	}
	
	bool	operator < ( const cIntensityPoint & o ) const
	{
		return ( (*ip) < (*o.ip) );
	}
	
	operator bool () const
	{
		return bLocalStor;
	}
	
	size_t size () const
	{
		return getDataSize( nByteSize );
	}
};

/*
 * class cFieldState
 */
struct cFieldState
{
	typedef unsigned short int	pilot_type;
	typedef double			mass_type;
	
	pilot_type	npilot;
	mass_type	mass;
	
	cFieldState ()
		: npilot( 0 )
		, mass( 1 )
	{ }
	
	cFieldState ( const pilot_type & p, const mass_type & m )
		: npilot( p )
		, mass( m )
	{ }
	
	bool	operator < ( const cFieldState & o ) const {
		// ascending order by mass and pilot number
		return ( mass < o.mass ) || (( mass == o.mass ) && ( npilot < o.npilot ));
	}
	
	bool	operator > ( const cFieldState & o ) const {
		// ascending order by mass and pilot number
		return ( mass > o.mass ) || (( mass == o.mass ) && ( npilot > o.npilot ));
	}
	
	bool	operator != ( const cFieldState & o ) const {
		return ( npilot != o.npilot ) || ( mass != o.mass );
	}
};

}	/* namespace msdc			*/
#endif/* CROSS_MSDC_DATA_CLASSES_H	*/
