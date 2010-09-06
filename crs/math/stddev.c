#include <math.h>
#include <crs/math/stddev.h>

void init_stddev_data ( stddev_data * data )
{
	data->n = 0;
	data->sum = data->sumsq = 0;
}

double mean ( const stddev_data * data )
{
	if( data->n > 0 )
            return data->sum / data->n;
      else
            return 0;
}

double variance ( const stddev_data * data )
{
      if( data->n > 1 )
            return ( data->sumsq - pow( data->sum, 2 ) / data->n ) / ( data->n - 1 );
      else
            return 0;
}

double deviation ( const stddev_data * data )
{
      return pow( variance( data ), .5 );
}

unsigned long addPoint ( stddev_data * data, const double x )
{
      data->sum += x;
      data->sumsq += pow( x, 2 );
      return ++(data->n);
}
