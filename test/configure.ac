#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([libcrs-test], [1.0], [op@pochta.ru])

AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])

# Check for which system.
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

# Checks for typedefs, structures, and compiler characteristics.
case "$build_os" in
	cygwin*)
		CFLAGS="$CFLAGS -D__LARGE64_FILES"
		CXXFLAGS="$CXXFLAGS -D__LARGE64_FILES"
		allow_win32api=yes
		allow_posixapi=yes
		allow_cpp0xapi=no
		;;
	mingw*)
		CFLAGS="$CFLAGS -DCROSS_EXPORT= -mno-cygwin"
		CXXFLAGS="$CXXFLAGS -DCROSS_EXPORT= -mno-cygwin"
		allow_win32api=yes
		allow_posixapi=no
		allow_cpp0xapi=no
		;;
	*)
		allow_win32api=no
		allow_posixapi=yes
		allow_cpp0xapi=no
		;;
esac

AM_CONDITIONAL([USE_WIN32API], [test "$allow_win32api" = yes] )
AM_CONDITIONAL([USE_POSIXAPI], [test "$allow_posixapi" = yes] )
AM_CONDITIONAL([USE_CPP0XAPI], [test "$allow_cpp0xapi" = yes] )

AC_DEFINE_UNQUOTED(CONFIG_HOST_TYPE, "$host",[Host type from configure])
AC_DEFINE_UNQUOTED(CONFIG_BUILD_TYPE, "$build",[Build type from configure])
AC_DEFINE_UNQUOTED(CONFIGURE_OPTIONS, "$ac_configure_args", [configure command line used to configure tests])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

# Check for needed libraries
AC_SEARCH_LIBS([gethostbyname],[nsl resolv bind])
AC_SEARCH_LIBS([bind],[socket])

# Checks for libraries.
AC_CHECK_LIB(crs, get_crc_ccitt)
AC_CHECK_LIB(pthread, pthread_mutex_init)
AC_CHECK_LIB(rt, clock_gettime)

# Check for Winsock only on MinGW, on Cygwin we must use emulated BSD socket API
case "$host_os" in
	mingw*)
		AC_MSG_CHECKING(for winsock)
		save_LIBS="$LIBS"
		for curlib in ws2_32 wsock32; do
			LIBS="$LIBS -l$curlib"
			AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <winsock.h>]], [[
					socket(0,0,0);
					select(0,NULL,NULL,NULL,NULL);
					closesocket(0);
					gethostname(NULL,0);
					]])],[have_winsock=yes],[have_winsock=no])
			
			if test $have_winsock = yes; then
				ac_cv_func_select='yes'
				if test $curlib = ws2_32; then
					have_winsock=winsock2
				fi
				break
			fi
			LIBS="$save_LIBS"
		done
			AC_MSG_RESULT($have_winsock)
		if test $have_winsock = winsock2; then
			AC_CHECK_HEADERS(winsock2.h)
		else
			AC_CHECK_HEADERS(winsock.h)
		fi
	;;
esac

# Checks for header files.
AC_CHECK_HEADERS([\
      stdint.h    \
      inttypes.h])

# Checks for typedefs, structures, and compiler characteristics.
dnl 8 bit integers - int8_t
dnl if this is defined we trust it to be 8 bits
AC_CHECK_TYPE(int8_t,[
  AC_CHECK_SIZEOF(int8_t,,LIBCRS_DEFAULT_SIZEOF_INCLUDES)
  AC_DEFINE(HAVE_INT8_T,1,[int8_t is defined in system headers])
  ],,LIBCRS_DEFAULT_INCLUDES)

dnl fallback #1
AC_CHECK_TYPE(char,[
  AC_CHECK_SIZEOF(char,,LIBCRS_DEFAULT_SIZEOF_INCLUDES)
  AC_DEFINE(HAVE_CHAR,1,[char is defined in system headers])
  ],,LIBCRS_DEFAULT_INCLUDES)

dnl fallback #1
dnl if this is defined we trust it to be 8 bits
AC_CHECK_TYPE(uint8_t,[
  AC_CHECK_SIZEOF(uint8_t,,LIBCRS_DEFAULT_SIZEOF_INCLUDES)
  AC_DEFINE(HAVE_UINT8_T,1,[uint8_t is defined in system headers])
  ],,LIBCRS_DEFAULT_INCLUDES)

dnl 16 bit integers - int16_t
dnl if this is defined we trust it to be 16 bits
AC_CHECK_TYPE(int16_t,[
  AC_CHECK_SIZEOF(int16_t,,LIBCRS_DEFAULT_SIZEOF_INCLUDES)
  AC_DEFINE(HAVE_INT16_T,1,[int16_t is defined in system headers])
  ],,LIBCRS_DEFAULT_INCLUDES)

dnl fallback #1
AC_CHECK_TYPE(short,[
  AC_CHECK_SIZEOF(short,,LIBCRS_DEFAULT_SIZEOF_INCLUDES)
  AC_DEFINE(HAVE_SHORT,1,[short is defined in system headers])
  ],,LIBCRS_DEFAULT_INCLUDES)

dnl fallback #2
AC_CHECK_TYPE(int,[
  AC_CHECK_SIZEOF(int,,LIBCRS_DEFAULT_SIZEOF_INCLUDES)
  AC_DEFINE(HAVE_INT,1,[int is defined in system headers])
  ],,LIBCRS_DEFAULT_INCLUDES)

dnl fallback #1
dnl if this is defined we trust it to be 16 bits
AC_CHECK_TYPE(uint16_t,[
  AC_CHECK_SIZEOF(uint16_t,,LIBCRS_DEFAULT_SIZEOF_INCLUDES)
  AC_DEFINE(HAVE_UINT16_T,1,[uint16_t is defined in system headers])
  ],,LIBCRS_DEFAULT_INCLUDES)

dnl 32 bit signed int - int32_t
dnl if this is defined we trust it to be 32 bits
AC_CHECK_TYPE(int32_t,[
  AC_CHECK_SIZEOF(int32_t,,LIBCRS_DEFAULT_SIZEOF_INCLUDES)
  AC_DEFINE(HAVE_INT32_T,1,[int32_t is defined in system headers])
  ],,LIBCRS_DEFAULT_INCLUDES)

dnl fallback #1
AC_CHECK_TYPE(long,[
  AC_CHECK_SIZEOF(long,,LIBCRS_DEFAULT_SIZEOF_INCLUDES)
  AC_DEFINE(HAVE_LONG,1,[long is defined in system headers])
  ],,LIBCRS_DEFAULT_INCLUDES)

dnl fallback #1
dnl if this is defined we trust it to be 32 bits
AC_CHECK_TYPE(uint32_t,[
  AC_CHECK_SIZEOF(uint32_t,,LIBCRS_DEFAULT_SIZEOF_INCLUDES)
  AC_DEFINE(HAVE_UINT32_T,1,[uint32_t is defined in system headers])
  ],,LIBCRS_DEFAULT_INCLUDES)

dnl 64 bit signed - int64_t
dnl if this is defind we trust it to be 64 bits
AC_CHECK_TYPE(int64_t,[
  AC_CHECK_SIZEOF(int64_t,,LIBCRS_DEFAULT_SIZEOF_INCLUDES)
  AC_DEFINE(HAVE_INT64_T,1,[int64_t is defined in system headers])
  ],,LIBCRS_DEFAULT_INCLUDES)

dnl fallback #1
dnl if this is defind we trust it to be 64 bits
AC_CHECK_TYPE(__int64,[
  AC_CHECK_SIZEOF(__int64,,LIBCRS_DEFAULT_SIZEOF_INCLUDES)
  AC_DEFINE(HAVE___INT64,1,[__int64 is defined in system headers])
  ],,LIBCRS_DEFAULT_INCLUDES)

dnl fallback #2
AC_CHECK_TYPE(long long,[
  AC_CHECK_SIZEOF(long long,,LIBCRS_DEFAULT_SIZEOF_INCLUDES)
  AC_DEFINE(HAVE_LONG_LONG,1,[long long is defined in system headers])
  ],,LIBCRS_DEFAULT_INCLUDES)

dnl fallback #1
dnl if this is defind we trust it to be 64 bits
AC_CHECK_TYPE(uint64_t,[
  AC_CHECK_SIZEOF(uint64_t,,LIBCRS_DEFAULT_SIZEOF_INCLUDES)
  AC_DEFINE(HAVE_UINT64_T,1,[uint64_t is defined in system headers])
  ],,LIBCRS_DEFAULT_INCLUDES)

# Checks for library functions.

AC_CONFIG_FILES([Makefile twbyte/Makefile ude/Makefile])
AC_OUTPUT

