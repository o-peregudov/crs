// (c) Mar 12, 2010 Oleg N. Peregudov
//	2011-Mar-19	new algorithms for peak measurement
#include <crs/math/peakCenter.h>
#include <crs/math/interpolation.h>
#include <crs/math/unimath.h>

#define NSIGMA    3

intDoublePair findMaximum ( const int nData, const double * y )
{
	intDoublePair result ( 0, y[ 0 ] );
	for( int i = 1; i < nData; ++i )
		if( result.second < y[ i ] )
		{
			result.first = i;
			result.second = y[ i ];
		}
	return result;
}

int	findLevelLeft ( const int nData, const double * y, const intDoublePair & maxPosition, const double & height )
{
	double level = maxPosition.second * height;
	if( UniMath::isZero( maxPosition.second ) )
		level = y[ maxPosition.first ] * height;
	for( int i = 0; i < maxPosition; ++i )
		if( level < y[ i ] )
			return i;
	return maxPosition.first;
}

int	findLevelRight ( const int nData, const double * y, const intDoublePair & maxPosition, const double & height )
{
	double level = maxPosition.second * height;
	if( UniMath::isZero( maxPosition.second ) )
		level = y[ maxPosition.first ] * height;
	for( int i = ( nData - 1 ); maxPosition < i; --i )
		if( level < y[ i ] )
			return i;
	return maxPosition.first;
}

void	peakCenter ( const int nData, const double * x, const double * y, peakCenterData & pcd )
{
	// find maximum
	pcd.peakMaximum = findMaximum( nData, y );
	
	// estimate peak width at 50%-height
	pcd.height50.height = 0.5;
	pcd.height50.idxSlope = measPeak( nData, y, pcd.peakMaximum, 0.5 );
	pcd.height50.width = ( x[ pcd.height50.idxSlope.second ] - x[ pcd.height50.idxSlope.first ] );
	pcd.height50.center = ( x[ pcd.height50.idxSlope.second ] + x[ pcd.height50.idxSlope.first ] ) / 2.0;
	pcd.height50.centroid = centroid( nData, x, y, 0.5 * pcd.peakMaximum.second );
	
	// estimate peak width at 10%-height
	pcd.height10.height = 0.1;
	pcd.height10.idxSlope = measPeak( nData, y, pcd.peakMaximum, pcd.height10.height );
	pcd.height10.width = ( x[ pcd.height10.idxSlope.second ] - x[ pcd.height10.idxSlope.first ] );
	pcd.height10.center = ( x[ pcd.height10.idxSlope.second ] + x[ pcd.height10.idxSlope.first ] ) / 2.0;
	pcd.height10.centroid = centroid( nData, x, y, pcd.height10.height * pcd.peakMaximum.second );
	
	// estimate peak width at 98%-height
	pcd.height98.height = 0.98;
	pcd.height98.idxSlope = measPeak( nData, y, pcd.peakMaximum, pcd.height98.height );
	pcd.height98.width = pcd.height10.width - ( x[ pcd.height98.idxSlope.second ] - x[ pcd.height98.idxSlope.first ] );
	pcd.height98.center = ( x[ pcd.height98.idxSlope.second ] + x[ pcd.height98.idxSlope.first ] ) / 2.0;
	pcd.height98.centroid = centroid( nData, x, y, pcd.height98.height * pcd.peakMaximum.second );
}

double centroid ( const int nData, const double * x, const double * y, const double & absoluteLevel )
{
	double sumY = 0, sumXY = 0;
      for( int i = 0; i < nData; ++i )
		if( absoluteLevel < y[ i ] )
	      {
			sumXY += y[ i ] * x[ i ];
			sumY += y[ i ];
	      }
	return ( sumXY /= sumY );
}

double background ( const int nData, const double * y )
{
	// prepare local copy of data for sorting
	double * ys = new double [ nData * sizeof( double ) ];
	memcpy( ys, y, nData * sizeof( double ) );
	
	// sort local copy of data
	double * pBegin = ys,
		 * pEnd = ( pBegin + nData );
	std::sort( pBegin, pEnd );
	
	// allocate histogram
	std::vector<unsigned int> histCounts;
	std::vector<double> histRanges;
	histRanges.push_back( ys[ 0 ] );
	histCounts.push_back( 1 );
	
	// will trace maximum counts in the fist half of the intensity range
	const int bgIdxLimit = nData / 2;
	unsigned int maxCounts = 0;
	unsigned int maxPosition = 0;
	
	// build up histogram and trace maximum counts for the first half
	// of the intensity range
	for( int i = 1; i < nData; ++i )
		if( UniMath::isZero( ys[ i ] - histRanges.back() ) )
			++(histCounts.back());
		else
		{
			if( ( i < bgIdxLimit ) && ( maxCounts < histCounts.back() ) )
			{
				maxCounts = histCounts.back();
				maxPosition = histCounts.size() - 1;
			}
			histRanges.push_back( ys[ i ] );
			histCounts.push_back( 1 );
		}
	
	double bgLevel = 0;
	if( maxPosition < ( histCounts.size() / 3 ) )
		bgLevel = histRanges[ maxPosition ];
	
	delete [] ys;
	return bgLevel;
}

void	fltMovingAverage ( const int nData, const double * x, const double * y, double * yo, const int nFilterSize )
{
	int nPoints = abs( nFilterSize );
	if( nPoints < 3 )
		nPoints = 3;
	const int HALFPOINTS = ( nPoints - 1 ) / 2;
	
	#pragma omp parallel for if ( nData > 2000 )
	for( int i = HALFPOINTS; i < (nData-HALFPOINTS); ++i )
	{
		yo[ i ] = y[ i ];
		for( int j = 1; j < (HALFPOINTS+1); ++j )
			yo[ i ] += y[ i - j ] + y[ i + j ];
		yo[ i ] /= ( 2.0 * HALFPOINTS + 1.0 );
	}
	memcpy( yo, y, HALFPOINTS * sizeof( double ) );
	memcpy( yo + nData - HALFPOINTS, y + nData - HALFPOINTS, HALFPOINTS * sizeof( double ) );
}

void	fltSavitzkyGolay ( const int nData, const double * x, const double * y, double * yo, const int nFilterSize )
// Convolutes smoothing according to Savitzky & Golay (Anal. Chem., 1964)
{
	static const double constant [] = {
		 17.0,  12.0,  -3.0,     0,     0,     0,     0,     0,     0,      0,      0,      0,      0,	  35.0,	// 5
		  7.0,   6.0,   3.0,  -2.0,     0,     0,     0,     0,     0,      0,      0,      0,      0,	  21.0,	// 7
		 59.0,  54.0,  39.0,  14.0, -21.0,     0,     0,     0,     0,      0,      0,      0,      0,	 231.0,	// 9
		 89.0,  84.0,  69.0,  44.0,   9.0, -36.0,     0,     0,     0,      0,      0,      0,      0,	 429.0,	// 11
		 25.0,  24.0,  21.0,  16.0,   9.0,   0.0, -11.0,     0,     0,      0,      0,      0,      0,	 143.0,	// 13
		167.0, 162.0, 147.0, 122.0,  87.0,  42.0, -13.0, -78.0,     0,      0,      0,      0,      0,	1105.0,	// 15
		 43.0,  42.0,  39.0,  34.0,  27.0,  18.0,   7.0,  -6.0, -21.0,      0,      0,      0,      0,	 323.0,	// 17
		269.0, 264.0, 249.0, 224.0, 189.0, 144.0,  89.0,  24.0, -51.0, -136.0,      0,      0,      0,	2261.0,	// 19
		329.0, 324.0, 309.0, 284.0, 249.0, 204.0, 149.0,  84.0,   9.0,  -76.0, -171.0,      0,      0,	3059.0,	// 21
		 79.0,  78.0,  75.0,  70.0,  63.0,  54.0,  43.0,  30.0,  15.0,   -2.0,  -21.0,  -42.0,      0,	8059.0,	// 23
		467.0, 462.0, 447.0, 422.0, 387.0, 322.0, 287.0, 222.0, 147.0,   62.0,  -33.0, -138.0, -253.0,	5175.0	// 25
	};
	
	const int HALFPOINTS = ( nFilterSize - 1 ) / 2;
	int nConstantRow = ( nFilterSize - 5 ) / 2;
	if( ( nConstantRow < 0 ) || ( nConstantRow > 10 ) )
		nConstantRow = 1;
	if( nConstantRow == 9 )
		// there is a bug in 23-point width kernel
		nConstantRow = 8;
	
	#pragma omp parallel for if( nData > 2000 )
	for( int i = HALFPOINTS; i < (nData-HALFPOINTS); ++i )
	{
		yo[ i ] = constant[ nConstantRow * 14 ] * y[ i ];
		for( int j = 1; j < (HALFPOINTS+1); ++j )
			yo[ i ] += constant[ nConstantRow * 14 + j ] * ( y[ i - j ] + y[ i + j ] );
		yo[ i ] /= constant[ nConstantRow * 14 + 13 ];
	}
	memcpy( yo, y, HALFPOINTS * sizeof( double ) );
	memcpy( yo + nData - HALFPOINTS, y + nData - HALFPOINTS, HALFPOINTS * sizeof( double ) );
}

// full preprocessing; returns spectrum intensity histogram 
void	estimateGuess ( const int nData, const double * x, const double * y, double * yo, peakCenterData & pcd, double & bgLevel, std::vector<intDoublePair> & hist, const int nFilterSize )
{
	//
	// estimate background level
	//
	
	// prepare local copy of data for sorting
	memcpy( yo, y, nData * sizeof( double ) );
	
	// sort local copy of data
	double * pBegin = yo,
		 * pEnd = ( pBegin + nData );
	std::sort( pBegin, pEnd );
	
	// initialize histogram
	hist.clear();
	hist.push_back( intDoublePair( 1, yo[ 0 ] ) );
	
	// will trace maximum counts in the fist half of the intensity range
	const int bgIdxLimit = nData / 2;
	unsigned int maxCounts = 0;
	unsigned int maxPosition = 0;
	
	// build up histogram and trace maximum counts for the first half
	// of the intensity range
	for( int i = 1; i < nData; ++i )
		if( UniMath::isZero( yo[ i ] - hist.back().second ) )
			++(hist.back().first);
		else
		{
			if( ( i < bgIdxLimit ) && ( maxCounts < hist.back().first ) )
			{
				maxCounts = hist.back().first;
				maxPosition = hist.size() - 1;
			}
			hist.push_back( intDoublePair( 1, yo[ i ] ) );
		}
	
	if( maxPosition < ( hist.size() / 3 ) )
		bgLevel = hist[ maxPosition ].second;
	else
		bgLevel = 0;
	
	//
	// smooth raw data, compensate background and find maximum smoothed
	// intensity
	//
	int nPoints = abs( nFilterSize );
	if( nPoints < 3 )
		nPoints = 3;
	const int HALFPOINTS = ( nPoints - 1 ) / 2;
	
	pcd.peakMaximum.first = 0;
	for( int i = 0; i < HALFPOINTS; ++i )
	{
		yo[ i ] = y[ i ] - bgLevel;
		if( yo[ pcd.peakMaximum.first ] < yo[ i ] )
			pcd.peakMaximum.first = i;
		
		yo[ nData - i - 1 ] = y[ nData - i - 1 ] - bgLevel;
		if( yo[ pcd.peakMaximum.first ] < yo[ nData - i - 1 ] )
			pcd.peakMaximum.first = nData - i - 1;
	}
	
	int & maxPos = pcd.peakMaximum.first;
	#pragma omp parallel for if ( nData > 2000 ), shared ( maxPos )
	for( int i = HALFPOINTS; i < (nData-HALFPOINTS); ++i )
	{
		yo[ i ] = y[ i ];
		for( int j = 1; j < (HALFPOINTS+1); ++j )
			yo[ i ] += y[ i - j ] + y[ i + j ];
		yo[ i ] = yo[ i ] / ( 2.0 * HALFPOINTS + 1.0 ) - bgLevel;
		if( yo[ maxPos ] < yo[ i ] )
			maxPos = i;
	}
	pcd.peakMaximum.second = yo[ pcd.peakMaximum.first ];
	
	//
	// do measure the peak
	//
	// estimate peak width at 50%-height
      pcd.height50.height = 0.50 * pcd.peakMaximum.second;
	pcd.height50.idxSlope = intPair( pcd.peakMaximum, pcd.peakMaximum );
	pcd.height50.centroid = 0;
      
	// estimate peak width at 10%-height
      pcd.height10.height = 0.10 * pcd.peakMaximum.second;
	pcd.height10.idxSlope = intPair( pcd.peakMaximum, pcd.peakMaximum );
	pcd.height10.centroid = 0;
      
	// estimate peak width at 98%-height
      pcd.height98.height = 0.98 * pcd.peakMaximum.second;
	pcd.height10.idxSlope = intPair( pcd.peakMaximum, pcd.peakMaximum );
	pcd.height98.centroid = 0;
	
	// left slope
	for( int i = pcd.peakMaximum; -1 < i; --i )
	{
		if( pcd.height98.height < yo[ i ] )
			pcd.height98.idxSlope.first = i;
		
		if( pcd.height50.height < yo[ i ] )
			pcd.height50.idxSlope.first = i;
		
		if( pcd.height10.height < yo[ i ] )
			pcd.height10.idxSlope.first = i;
	}
	
	// right slope
	for( int i = pcd.peakMaximum; i < nData; ++i )
	{
		if( pcd.height98.height < yo[ i ] )
			pcd.height98.idxSlope.second = i;
		
		if( pcd.height50.height < yo[ i ] )
			pcd.height50.idxSlope.second = i;
		
		if( pcd.height10.height < yo[ i ] )
			pcd.height10.idxSlope.second = i;
	}
	
	// calculate rest parameters for 50%-height
	pcd.height50.width = ( x[ pcd.height50.idxSlope.second ] - x[ pcd.height50.idxSlope.first ] );
	pcd.height50.center = ( x[ pcd.height50.idxSlope.second ] + x[ pcd.height50.idxSlope.first ] ) / 2.0;
	
	// calculate rest parameters for 10%-height
	pcd.height10.width = ( x[ pcd.height10.idxSlope.second ] - x[ pcd.height10.idxSlope.first ] );
	pcd.height10.center = ( x[ pcd.height10.idxSlope.second ] + x[ pcd.height10.idxSlope.first ] ) / 2.0;
	
	// calculate rest parameters for 98%-height
	pcd.height98.width = pcd.height10.width - ( x[ pcd.height98.idxSlope.second ] - x[ pcd.height98.idxSlope.first ] );
	pcd.height98.center = ( x[ pcd.height98.idxSlope.second ] + x[ pcd.height98.idxSlope.first ] ) / 2.0;
}

