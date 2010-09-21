AC_PREREQ([2.63])
AC_INIT([crs],[1.0],[op@pochta.ru])
AM_INIT_AUTOMAKE

# Check for which system.
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
LT_INIT

AC_CONFIG_HEADERS([config.h])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h netdb.h netinet/in.h sys/socket.h syslog.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_C_RESTRICT
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_CHECK_FUNCS([clock_gettime gethostbyname inet_ntoa memmove memcpy memset modf pow select poll socket sqrt strpbrk strerror])

# Checks for typedefs, structures, and compiler characteristics.
case "$build_os" in
	cygwin*)
		CFLAGS="$CFLAGS -D__LARGE64_FILES"
		CXXFLAGS="$CXXFLAGS -D__LARGE64_FILES"
		allow_win32api=no
		allow_posixapi=yes
		allow_cpp0xapi=no
		;;
	mingw*)
		CFLAGS="$CFLAGS -D__LARGE64_FILES -DCROSS_EXPORT= -mno-cygwin"
		CXXFLAGS="$CXXFLAGS -D__LARGE64_FILES -DCROSS_EXPORT= -mno-cygwin"
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

CRS_CURRENT=1
CRS_REVISION=0
CRS_AGE=5

CRS_LT_VERSION="${CRS_CURRENT}:${CRS_REVISION}:${CRS_AGE}"
AC_SUBST(CRS_LT_VERSION)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([
 Makefile
 crs/Makefile
 crs/ude/Makefile
])
AC_OUTPUT
