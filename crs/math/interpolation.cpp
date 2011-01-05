// (c) Jul 24, 2009 Oleg N. Peregudov
//	12/13/2010	default constructor for line
#include <crs/math/interpolation.h>

namespace Interpolation {

//
// members of abstract polynom class
//
polynom::polynom ( const unsigned int p )
	: power( p + 1 )
	, cf( new double [ power ] )
	, resSum( 0 )
	, b00( 0 )	// number of points for regression
{
}

polynom::polynom ( const polynom & p )
	: power( p.power )
	, cf( new double [ power ] )
	, resSum( p.resSum )
	, b00( p.b00 )
{
	memcpy( cf, p.cf, power * sizeof( double ) );
}

polynom::~polynom ()
{
	delete [] cf;
}

void polynom::constructOther ( const unsigned int p )
{
	delete [] cf;
	power = p + 1;
	cf = new double [ power ];
}

double polynom::func ( const double & x ) const
{
	double result ( 0 );
	for( unsigned int i = 0; i < power; ++i )
		result += cf[ i ] * pow( x, static_cast<int>( i ) );
	return result;
}

double polynom::deriv ( const double & x ) const
{
	double result ( 0 );
	for( unsigned int i = 1; i < power; ++i )
		result += cf[ i ] * i * pow( x, static_cast<int>( i - 1 ) );
	return result;
}

const double & polynom::sumOfSquaredResiduals ( const unsigned int nPoints, const double * x, const double * y )
{
	resSum = 0;
	for( unsigned int i = 0; i < nPoints; ++i )
		resSum += pow( y[ i ] - func( x[ i ] ), 2 );
	return resSum;
}

//
// members of class line
//
line::line ( )
	: polynom( 1 )
	, c0( 0.0 )
	, c1( 1.0 )
	, b01( 0 )
	, b02( 0 )
	, s( 0 )
	, q( 0 )
{
}

line::line ( const std::pair<double, double> & coefficient )
	: polynom( 1 )
	, c0( 0 )
	, c1( 0 )
	, b01( 0 )
	, b02( 0 )
	, s( 0 )
	, q( 0 )
{
	cf[ 0 ] = coefficient.first;
	cf[ 1 ] = coefficient.second;
}

line::line ( const double * x, const double * y )
	: polynom( 1 )
	, c0( 0 )
	, c1( 0 )
	, b01( 0 )
	, b02( 0 )
	, s( 0 )
	, q( 0 )
{
	ipBuild( x, y );
}

line::line ( const unsigned int nPoints, const double * x, const double * y )
	: polynom( 1 )
	, c0( 0 )
	, c1( 0 )
	, b01( 0 )
	, b02( 0 )
	, s( 0 )
	, q( 0 )
{
	regressBuild( nPoints, x, y );
}

line::line ( const line & l )
	: polynom( l )
	, c0( l.c0 )
	, c1( l.c1 )
	, b01( l.b01 )
	, b02( l.b02 )
	, s( l.s )
	, q( l.q )
{
}

line::~line ()
{

}

void line::ipBuild ( const double * x, const double * y )
{
	cf[ 1 ] = ( y[ 0 ] - y[ 1 ] ) / ( x[ 0 ] - x[ 1 ] );
	cf[ 0 ] = y[ 1 ] - cf[ 1 ] * x[ 1 ];
}

void line::regressCalcCoefficients ()
{
	s = c1 * b00 - c0 * b01;
	q = b02 * b00 - b01 * b01;
	
	cf[ 1 ] = s / q;
	cf[ 0 ] = ( c0 - cf[ 1 ] * b01 ) / b00;
}

void line::regressAddPoint ( const double & x, const double & y )
{
	++b00;
	b01 += x;
	b02 += x * x;
	c0 += y;
	c1 += x * y;
	regressCalcCoefficients();
}

const double & line::regressBuild ( const unsigned int nPoints, const double * x, const double * y )
{
	b00 = nPoints;
	c0 = c1 = b01 = b02 = 0;
	for( unsigned int i = 0; i < nPoints; ++i )
	{
		b01 += x[ i ];
		b02 += pow( x[ i ], 2 );
		c0 += y[ i ];
		c1 += x[ i ] * y[ i ];
	}
	regressCalcCoefficients();
	return sumOfSquaredResiduals( nPoints, x, y );
}

//
// members of class parabola
//
parabola::parabola ( const double & a, const double & b, const double & c )
	: line( std::pair<double, double>( c, b ) )
	, c2( 0 )
	, b12( 0 )
	, b22( 0 )
{
	constructOther( 2 );
	cf[ 2 ] = a;
	cf[ 1 ] = b;
	cf[ 0 ] = c;
}

parabola::parabola ( const double * x, const double * y )
	: line( std::pair<double, double>( 0.0, 1.0 ) )
	, c2( 0 )
	, b12( 0 )
	, b22( 0 )
{
	constructOther( 2 );
	ipBuild( x, y );
}

parabola::parabola ( const unsigned int nPoints, const double * x, const double * y )
	: line( std::pair<double, double>( 0.0, 1.0 ) )
	, c2( 0 )
	, b12( 0 )
	, b22( 0 )
{
	constructOther( 2 );
	regressBuild( nPoints, x, y );
}

parabola::parabola ( const parabola & p )
	: line( p )
	, c2( p.c2 )
	, b12( p.b12 )
	, b22( p.b22 )
{
}

parabola::~parabola ()
{

}

void parabola::ipBuild ( const double * x, const double * y )
{
	double o = x[ 0 ] + x[ 1 ],
		 p = pow( x[ 2 ], 2 ),
		 q = x[ 2 ] - p / o,
		 r = pow( x[ 0 ], 2 ),
		 s = ( y[ 0 ] - y[ 1 ] ) / ( r - pow( x[ 1 ], 2 ) ),
		 t = y[ 2 ] - s * p;
	
	cf[ 1 ] = ( y[ 0 ] - s * r - t ) / ( x[ 0 ] - r / o - q );
	cf[ 0 ] = t - cf[ 1 ] * q;
	cf[ 2 ] = s - cf[ 1 ] / o;
}

const double & parabola::regressBuild ( const unsigned int nPoints, const double * x, const double * y )
{
	double x2;
	b00 = nPoints;
	c0 = c1 = c2 = 0;
	b01 = b02 = b12 = b22 = 0;
	for( unsigned int i = 0; i < nPoints; ++i )
	{
		b01 += x[ i ];
		b02 += ( x2 = pow( x[ i ], 2 ) );
		c0 += y[ i ];
		c1 += x[ i ] * y[ i ];
		
		b12 += pow( x[ i ], 3 );
		b22 += pow( x[ i ], 4 );
		c2 += x2 * y[ i ];
	}
	regressCalcCoefficients();
	return sumOfSquaredResiduals( nPoints, x, y );
}

void parabola::incrementCoefficients ()
{
	double p = b12 * b00 - b01 * b02,
		 r = p / q;
	
	cf[ 2 ] = ( c2 * b00 - c0 * b02 - s * r ) / ( b22 * b00 - b02 * b02 - p * r );
	cf[ 1 ] -= cf[ 2 ] * r;
	cf[ 0 ] -= cf[ 2 ] * b02 / b00;
}

void parabola::regressCalcCoefficients ()
{
	line::regressCalcCoefficients();
	incrementCoefficients();
}

void parabola::regressAddPoint ( const double & x, const double & y )
{
	line::regressAddPoint( x, y );
	b12 += pow( x, 3 );
	b22 += pow( x, 4 );
	c2 += pow( x, 2 ) * y;
	incrementCoefficients();
}

} // namespace Interpolation
