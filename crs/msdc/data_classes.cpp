//
// data_classes.cpp: general mass spectrum representation data base
// (c) Nov 19, 2007 Oleg N. Peregudov
//	12/07/2010	included into libcrs
//
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4351 )
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

} // namespace msdc
