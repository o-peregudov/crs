#ifndef CRCSTUFF_H
#define CRCSTUFF_H 1
/*********************************************************************/
/* Module name: crcstuff.h                                           */
/* Date: 27 Dec 87                                                   */
/* Environment: Turbo C 1.0                                          */
/* Author: R. E. Faith                                               */
/* Notice: Public Domain: The following conditions apply to use:     */
/*         1) No fee shall be charged for distribution.              */
/*         2) Modifications may be made, but authorship information  */
/*            for all contributing authors shall be retained.        */
/*         3) This code may not be included as part of a commercial  */
/*            package.                                               */
/* This program is provided AS IS without any warranty, expressed or */
/* implied, including, but not limited to, fitness for a particular  */
/* purpose.                                                          */
/*********************************************************************/

#include <crs/libexport.h>
#if defined( __cplusplus )
extern "C" {
#endif

// CRC-CCITT is based on the polynomial x^16 + x^12 + x^5 + 1.  Bits
// are sent MSB to LSB.
extern unsigned short int get_crc_ccitt( unsigned short int, const char *, const int );

#if defined( __cplusplus )
};
#endif
#endif // CRCSTUFF_H
