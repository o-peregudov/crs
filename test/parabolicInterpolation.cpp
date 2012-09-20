#include <crs/math/interpolation.h>
#include <iostream>

int main ()
{
	using namespace std;
	using namespace Interpolation;
	
	double x [ 101 ], y[ 101 ];
	for( size_t i = 0; i < sizeof( x )/sizeof( double ); ++i )
		x[ i ] = -50.0 + i;
	
	cout << "Testing parabola" << endl;
	parabola sampleParabola ( -13.0, 0.4, 1985 );
	cout << "Sample coefficients: ";
	for( size_t i = 0; i < ( sampleParabola.polyPower() + 1 ); ++i )
		cout << "cf[ " << i << " ]= " << sampleParabola[ i ] << ' ';
	cout << endl;
	
	for( size_t i = 0; i < sizeof( x )/sizeof( double ); ++i )
	{
		y[ i ] = sampleParabola( x[ i ] );
		if( ( i % 2 ) == 0 )
			y[ i ] += 0.0005;
		else
			y[ i ] -= 0.0005;
	}
	
	parabola testParabola ( x, y );
	cout << "Ip coefficients: ";
	for( size_t i = 0; i < ( testParabola.polyPower() + 1 ); ++i )
		cout << "cf[ " << i << " ]= " << testParabola[ i ] << ' ';
	cout << endl;
	
	cout << "Incremental regress coefficients:" << endl;
	testParabola.regressBuild( 10, x, y );
	for( size_t i = 10; i < sizeof( x )/sizeof( double ); ++i )
	{
		testParabola.regressAddPoint( x[ i ], y[ i ] );
		cout << "\tRegress coefficients: ";
		for( size_t c = 0; c < ( testParabola.polyPower() + 1 ); ++c )
			cout << "cf[ " << c << " ]= " << testParabola[ c ] << ' ';
		cout << endl;
	}
	
	testParabola.regressBuild( sizeof( x )/sizeof( double ), x, y );
	
	cout << "Regress coefficients: ";
	for( size_t i = 0; i < ( testParabola.polyPower() + 1 ); ++i )
		cout << "cf[ " << i << " ]= " << testParabola[ i ] << ' ';
	cout << endl;
	
	return 0;
}
