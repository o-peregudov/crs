#include <crs/math/interpolation.h>
#include <iostream>

int main ()
{
	using namespace std;
	using namespace Interpolation;
	
	double x [ 81 ], y[ 81 ];
	
	cout << "x series:" << endl;
	for( size_t i = 0; i < sizeof( x )/sizeof( double ); ++i )
	{
		x[ i ] = -40.0 + i;
		cout << x[ i ] << ' ';
	}
	cout << endl;
	
	cout << "Testing line" << endl;
	line sampleLine ( -4, 12 );
	
	for( size_t i = 0; i < sizeof( x )/sizeof( double ); ++i )
	{
		y[ i ] = sampleLine( x[ i ] );
		if( ( i % 2 ) == 0 )
			y[ i ] += 0.005;
		else
			y[ i ] -= 0.005;
	}
	
	line testLine ( x, y );
	cout << "Ip coefficients: ";
	for( size_t i = 0; i < testLine.polyPower(); ++i )
		cout << "cf[ " << i << " ]= " << testLine[ i ] << ' ';
	cout << endl;
	
	cout << "Incremental regress coefficients:" << endl;
	testLine.regressBuild( 10, x, y );
	for( size_t i = 10; i < sizeof( x )/sizeof( double ); ++i )
	{
		testLine.regressAddPoint( x[ i ], y[ i ] );
		cout << "\tRegress coefficients: ";
		for( size_t j = 0; j < testLine.polyPower(); ++j )
			cout << "cf[ " << j << " ]= " << testLine[ j ] << ' ';
		cout << endl;
	}
	
	cout << "Regress coefficients: ";
	testLine.regressBuild( sizeof( x )/sizeof( double ), x, y );
	for( size_t i = 0; i < testLine.polyPower(); ++i )
		cout << "cf[ " << i << " ]= " << testLine[ i ] << ' ';
	cout << endl;
	
	return 0;
}
