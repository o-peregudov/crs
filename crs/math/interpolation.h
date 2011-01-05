#ifndef INTERPOLATION_H_INCLUDED
#define INTERPOLATION_H_INCLUDED 1
// (c) Jul 24, 2009 Oleg N. Peregudov
//	12/13/2010	default contructor for line

#include <cmath>
#include <cstring>
#include <algorithm>
#include <crs/libexport.h>

namespace Interpolation {

class CROSS_EXPORT polynom
{
	polynom ();
	
protected:
	unsigned int power;
	double * cf;
	double resSum;
	size_t b00;
	
	polynom ( const unsigned int p );
	polynom ( const polynom & );
	virtual ~polynom ();
	
	void constructOther ( const unsigned int p );
	
public:
	virtual double func ( const double & x ) const;
	virtual double deriv ( const double & x ) const;
	
	const double & sumOfSquaredResiduals ( const unsigned int nPoints, const double * x, const double * y );
	const double & rSum () const
	{
		return resSum;
	}
	
	unsigned int polyPower () const
	{
		return power;
	}
	
	virtual void ipBuild ( const double * x, const double * y ) = 0;
	virtual const double & regressBuild ( const unsigned int nPoints, const double * x, const double * y ) = 0;
	virtual void regressAddPoint ( const double & x, const double & y ) = 0;
};

class CROSS_EXPORT line : public polynom
{
protected:
	double c0, c1, b01, b02, s, q;
	void regressCalcCoefficients ();
	
public:
	line ();
	line ( const std::pair<double, double> & coefficient );
	line ( const double * x, const double * y );
	line ( const unsigned int nPoints, const double * x, const double * y );
	line ( const line & );
	
	virtual ~line ();
	
	void ipBuild ( const double * x, const double * y );
	const double & regressBuild ( const unsigned int nPoints, const double * x, const double * y );
	void regressAddPoint ( const double & x, const double & y );
	
	double operator () ( const double & x ) const {
		return func( x );
	}
	
	double dF ( const double & x ) const {
		return deriv( x );
	}
	
	const double & operator [] ( const unsigned int nCoeff ) const {
		return cf[ nCoeff ];
	}
};

class CROSS_EXPORT parabola : public line
{
protected:
	double c2, b12, b22;
	void incrementCoefficients ();
	void regressCalcCoefficients ();
	
public:
	parabola ( const double & a, const double & b, const double & c );
	parabola ( const double * x, const double * y );
	parabola ( const unsigned int nPoints, const double * x, const double * y );
	parabola ( const parabola & );
	
	virtual ~parabola ();
	
	void ipBuild ( const double * x, const double * y );
	const double & regressBuild ( const unsigned int nPoints, const double * x, const double * y );
	void regressAddPoint ( const double & x, const double & y );
	
	double operator () ( const double & x ) const {
		return func( x );
	}
	
	double dF ( const double & x ) const {
		return deriv( x );
	}
	
	const double & operator [] ( const unsigned int nCoeff ) const {
		return cf[ nCoeff ];
	}
};

} // namespace Interpolation
#endif // INTERPOLATION_H_INCLUDED
