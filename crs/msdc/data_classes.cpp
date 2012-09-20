/*
 *  General mass spectrum presentation database
 *
 *  crs/msdc/data_classes.cpp
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

#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif

#include <crs/msdc/data_classes.h>
#include <iomanip>

namespace msdc {

cIntensityPoint::cIntensityPoint( const size_t nData, const basic_ipoint::time_type & tm )
	: bLocalStor( true )
	, nByteSize( getByteSize( nData ) )
	, pStor( new char [ nByteSize ] )
{
	memset( pStor, 0, nByteSize );
	ip->time = tm;
}

cIntensityPoint::cIntensityPoint( const cIntensityPoint & o )
	: bLocalStor( true )
	, nByteSize( o.nByteSize )
	, pStor( new char [ nByteSize ] )
{
	memcpy( pStor, o.pStor, nByteSize );
}

cIntensityPoint & cIntensityPoint::linkWith ( const cIntensityPoint & o )
{
	if( this != &o )
	{
		if( o.bLocalStor )
		{
			if( bLocalStor )
			{
				if( nByteSize != o.nByteSize )
				{
					delete [] pStor;
					nByteSize = o.nByteSize;
					pStor = new char [ nByteSize ];
				}
			}
			else
			{
				nByteSize = o.nByteSize;
				pStor = new char [ nByteSize ];
				bLocalStor = true;
			}
			memcpy( pStor, o.pStor, nByteSize );
		}
		else
		{
			if( bLocalStor )
				delete [] pStor;
			bLocalStor = o.bLocalStor;
			nByteSize = o.nByteSize;
			pStor = o.pStor;
		}
	}
	return *this;
}

cIntensityPoint & cIntensityPoint::operator = ( const cIntensityPoint & o )
{
	if( this != &o )
	{
		if( bLocalStor && ( nByteSize != o.nByteSize ) )
		{
			delete [] pStor;
			nByteSize = o.nByteSize;
			pStor = new char [ nByteSize ];
		}
		memcpy( pStor, o.pStor, ( nByteSize < o.nByteSize ) ? nByteSize : o.nByteSize );
	}
	return *this;
}

} /* namespace msdc	*/
