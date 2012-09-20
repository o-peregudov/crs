#ifndef STDDEV_H_INCLUDED
#define STDDEV_H_INCLUDED 1

#include <crs/libexport.h>

// allow access to functions for C++ applications as well
#ifdef __cplusplus
extern "C"
{
#endif

#pragma pack(push, 1)
typedef struct
{
      unsigned long n;
      double sum;
      double sumsq;
}  stddev_data;
#pragma pack(pop)

extern void CROSS_EXPORT		init_stddev_data ( stddev_data * data );
extern double CROSS_EXPORT		mean ( const stddev_data * data );
extern double CROSS_EXPORT		variance ( const stddev_data * data );
extern double CROSS_EXPORT		deviation ( const stddev_data * data );
extern unsigned long CROSS_EXPORT	addPoint ( stddev_data * data, const double x );

#ifdef __cplusplus
} // extern "C"
#endif

#endif // STDDEV_H_INCLUDED
