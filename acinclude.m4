dnl This encapsulates the nasty mess of headers we need to check when 
dnl checking types.
AC_DEFUN([LIBCRS_DEFAULT_INCLUDES],[[
/* What a mess.. many systems have added the (now standard) bit types
 * in their own ways, so we need to scan a wide variety of headers to
 * find them.
 * IMPORTANT: Keep crs/defsys.h syncronised with this list
 */
#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
#if HAVE_STDINT_H
#include <stdint.h>
#endif
#if HAVE_INTTYPES_H
#include <inttypes.h>
#endif
]])

dnl and this is for AC_CHECK_SIZEOF
AC_DEFUN([LIBCRS_DEFAULT_SIZEOF_INCLUDES],[
#include <stdio.h>
LIBCRS_DEFAULT_INCLUDES
])

