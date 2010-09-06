// (c) Mar 12, 2010 Oleg N. Peregudov
#include <crs/math/peakCenter.h>
#include <crs/math/interpolation.h>
#include <iostream>

#define NSIGMA    3

intPair measPeak ( const int nData, const double * x, const double * y, const int maxPosition, const double & height )
{
	intPair slope ( 0, nData );
      double maxValue = y[ maxPosition ];
	
      // this value we'll search on slopes
      double slopeValue = maxValue * height;
	
	// scan left slope
	for( int i = maxPosition; i > 0; --i )
	{
		if( ( y[ i - 1 ] <= slopeValue ) && ( slopeValue <= y[ i ] ) )
		{
			slope.first = i;
			break;
		}
	}
	
      // scan right slope
      for( int i = maxPosition; i < ( nData - 1 ); ++i )
      {
            if( ( y[ i + 1 ] <= slopeValue ) && ( slopeValue <= y[ i ] ) )
            {
                  slope.second = i;
                  break;
            }
      }
	
	return slope;
}

int peakCenter ( const int nData, const double * x, const double * y, peakCenterData & pcd, const double & height )
{
      pcd.idxPeakCenter = 0;
      pcd.pcMaximum = y[ 0 ];
      
      // find maximum
      for( int i = 1; i < nData; ++i )
      {
            if( pcd.pcMaximum < y[ i ] )
            {
                  pcd.pcMaximum = y[ i ];
                  pcd.idxPeakCenter = i;
            }
      }
      
      intPair slope = measPeak( nData, x, y, pcd.idxPeakCenter, height );
      pcd.peakWidth = x[ slope.second ] - x[ slope.first ];
      pcd.desiredHeight = height;
      
      if( ( 1.0 + ( height - .5 ) ) == 1.0 )
      {
            pcd.fwhm = pcd.peakWidth;
            pcd.leftSlope = slope.first;
            pcd.rightSlope = slope.second;
      }
      else
      {
            slope = measPeak( nData, x, y, pcd.idxPeakCenter );
            pcd.fwhm = x[ slope.second ] - x[ slope.first ];
            pcd.leftSlope = slope.first;
            pcd.rightSlope = slope.second;
      }
      
      return ( pcd.idxPeakCenter = ( pcd.leftSlope + pcd.rightSlope ) / 2 );
}

int cutBackground ( const int nData, double * x, double * y, double & c0, double & c1, double & sumsq )
{
      int nLeft = 0, nRight = 0;
      double sum = 0;
	
	sumsq = 0;
      
      for( int i = 0; i < nData; ++i )
      {
            if( nLeft > 3 )
                  if( fabs( y[ i ] - sum / nLeft ) > NSIGMA * pow( fabs( ( sumsq - pow( sum, 2 ) / nLeft ) / ( nLeft - 1 ) ), .5 ) )
                        break;
            sum += y[ i ];
            sumsq += pow( y[ i ], 2 );
            ++nLeft;
      }
      
      sum = sumsq = 0;
      for( int i = nData; i; --i )
      {
            if( nRight > 3 )
                  if( fabs( y[ i - 1 ] - sum / nRight ) > NSIGMA * pow( fabs( ( sumsq - pow( sum, 2 ) / nRight ) / ( nRight - 1 ) ), .5 ) )
                        break;
            sum += y[ i - 1 ];
            sumsq += pow( y[ i - 1 ], 2 );
            ++nRight;
      }
      
      const int bgDataSize = nLeft + nRight;
      double * xbg = new double [ bgDataSize ],
		 * ybg = new double [ bgDataSize ];
      for( int i = 0; i < bgDataSize; ++i )
      {
            if( i < nLeft )
                  xbg[ i ] = x[ i ],
                  ybg[ i ] = y[ i ];
            else
                  xbg[ i ] = x[ nData - nRight + i - nLeft ],
                  ybg[ i ] = y[ nData - nRight + i - nLeft ];
      }
      
	Interpolation::line linearFit ( bgDataSize, xbg, ybg );
	delete [] ybg;
	delete [] xbg;
	c0 = linearFit[ 0 ];
	c1 = linearFit[ 1 ];
	sumsq = linearFit.rSum();
	
      #pragma omp parallel for
	for( int i = 0; i < nData; ++i )
            y[ i ] -= linearFit( x[ i ] );
	
	return bgDataSize;
}

void estimateGuess ( const int nData, const double * x, const double * y, peakCenterData & pcd )
{
	pcd.centroid = 0;
	peakCenter( nData, x, y, pcd );
	double sum = 0, level = pcd.desiredHeight * pcd.pcMaximum;
      for( int i = 0; i < nData; ++i )
		if( y[ i ] > level )
	      {
			pcd.centroid += y[ i ] * x[ i ];
			sum += y[ i ];
	      }
	pcd.centroid /= sum;
}

void medSmooth ( const int nData, double * x, double * y )
{
	for( int i = 1; i < ( nData - 1 ); ++i )
	{
		if( ( y[ i - 1 ] <= y[ i ] ) && ( y[ i ] <= y[ i + 1 ] )
             || ( y[ i + 1 ] <= y[ i ] ) && ( y[ i ] <= y[ i - 1 ] ) )
			continue;
		else
			y[ i ] = ( y[ i - 1 ] + y[ i + 1 ] ) / 2.0;
	}
}

void SavitzkyGolaySmooth ( const int nData, const double * x, const double * y, double * yo, const int nPoints )
// Convolutes smoothing according to Savitzky & Golay (Anal. Chem., 1964)
{
	static const double constant [] = {
		 17.0,  12.0,  -3.0,     0,     0,     0,     0,     0,     0,      0,      0,	  35.0,	// 5
		  7.0,   6.0,   3.0,  -2.0,     0,     0,     0,     0,     0,      0,      0,	  21.0,	// 7
		 59.0,  54.0,  39.0,  14.0, -21.0,     0,     0,     0,     0,      0,      0,	 231.0,	// 9
		 89.0,  84.0,  69.0,  44.0,   9.0, -36.0,     0,     0,     0,      0,      0,	 429.0,	// 11
		 25.0,  24.0,  21.0,  16.0,   9.0,   0.0, -11.0,     0,     0,      0,      0,	 143.0,	// 13
		167.0, 162.0, 147.0, 122.0,  87.0,  42.0, -13.0, -78.0,     0,      0,      0,	1105.0,	// 15
		 43.0,  42.0,  39.0,  34.0,  27.0,  18.0,   7.0,  -6.0, -21.0,      0,      0,	 323.0,	// 17
		269.0, 264.0, 249.0, 224.0, 189.0, 144.0,  89.0,  24.0, -51.0, -136.0,      0,	2261.0,	// 19
		329.0, 324.0, 309.0, 284.0, 249.0, 204.0, 149.0,  84.0,   9.0,  -76.0, -171.0,	3059.0	// 21
	};
	
	const int HALFPOINTS = ( nPoints - 1 ) / 2;
	int nConstantRow = ( nPoints - 5 ) / 2;
	if( ( nConstantRow < 0 ) || ( nConstantRow > 8 ) )
		nConstantRow = 1;
	
	#pragma omp parallel for
	for( int i = (HALFPOINTS+1); i < (nData-HALFPOINTS); ++i )
	{
		yo[ i ] = constant[ nConstantRow * 12 ] * y[ i ];
		for( int j = 1; j < (HALFPOINTS+1); ++j )
			yo[ i ] += constant[ nConstantRow * 12 + j ] * ( y[ i - j ] + y[ i + j ] );
		yo[ i ] /= constant[ nConstantRow * 12 + 11 ];
	}
}

void SavitzkyGolaySmooth ( const int nData, const double * x, double * y, const int nPoints )
{
	double * yo = new double [ nData ];
	SavitzkyGolaySmooth ( nData, x, y, yo, nPoints );
	memcpy( y, yo, nData * sizeof( double ) );
	delete [] yo;
}

