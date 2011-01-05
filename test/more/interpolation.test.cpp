#include <crs/math/interpolation.h>
#include <iostream>

int main ()
{
	double x [] = {
		99.365,
		197.816,
		296.345
	};
	
	double y [] = {
		93.054621,
		185.101965,
		277.149309
	};
	
	double z [] = {
		98.984172,
		196.961067,
		294.937962
	};
	
	using namespace Interpolation;
	parabola testParabola ( x, z );
	std::cout.precision( 8 );
	double px = 174.0162;
	double xm = testParabola( 174.761 ),
		 d = px - xm;
	std::cout << xm << " ; " << d << " ; " << d / px * 1e6 << std::endl;
	
	return 0;
}
