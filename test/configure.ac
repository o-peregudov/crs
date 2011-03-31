dnl                                               -*- Autoconf -*-
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([libcrstest], [1.0.1], [op@pochta.ru])
AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

dnl Check for which system.
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
LT_INIT

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([\
	omp.h		])

if test $ac_cv_header_inttypes_h = yes; then
	HAVE_INTTYPES_H=1
else
	HAVE_INTTYPES_H=0
fi

if test $ac_cv_header_stdint_h = yes; then
	HAVE_STDINT_H=1
else
	HAVE_STDINT_H=0
fi

if test $ac_cv_header_sys_types_h = yes; then
	HAVE_SYS_TYPES_H=1
else
	HAVE_SYS_TYPES_H=0
fi

dnl Check for OpenMP
if test $ac_cv_header_omp_h = yes; then
	AC_MSG_CHECKING(whether -fopenmp switch usable)
	AC_LANG_PUSH([C])
	save_CFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS -fopenmp"
	AC_LINK_IFELSE(
		[AC_LANG_PROGRAM(
			[[#include <omp.h>]],
			[[#pragma omp parallel num_threads( 3 )
			{ omp_get_thread_num(); }]])],
		[have_openmp=yes],
		[have_openmp=no])
	CFLAGS="$save_CFLAGS"
	AC_LANG_POP([C])
	AC_MSG_RESULT($have_openmp)
	if test $have_openmp = yes; then
		CFLAGS="$CFLAGS -fopenmp"
		CXXFLAGS="$CXXFLAGS -fopenmp"
		HAVE_OMP_H=1
	else
		HAVE_OMP_H=0
	fi
fi

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_C_RESTRICT
AC_TYPE_SIZE_T

ac_cygwin_target=no
ac_mingw_target=no
ac_linux_target=no

ac_have_win32_api=no
ac_have_posix_api=no
ac_have_cpp0x_api=no

case "$host_os" in
	cygwin*)
		ac_have_posix_api=yes
		ac_cygwin_target=yes
		ac_make_dll=yes
		;;
	mingw*)
		ac_have_win32_api=yes
		ac_mingw_target=yes
		ac_make_dll=yes
		;;
	*)
		ac_have_posix_api=yes
		ac_linux_target=yes
		ac_make_dll=no
		;;
esac

case "$build_os" in
	cygwin*)
		CFLAGS="$CFLAGS -D__LARGE64_FILES"
		CXXFLAGS="$CXXFLAGS -D__LARGE64_FILES"
		;;
	mingw*)
		CFLAGS="$CFLAGS -mno-cygwin"
		CXXFLAGS="$CXXFLAGS -mno-cygwin"
		;;
	*)
		;;
esac

AM_CONDITIONAL([CYGWIN_TARGET], [test "$ac_cygwin_target" = yes] )
AM_CONDITIONAL([MINGW_TARGET], [test "$ac_mingw_target" = yes] )
AM_CONDITIONAL([LINUX_TARGET], [test "$ac_linux_target" = yes] )

AM_CONDITIONAL([USE_WIN32API], [test "$ac_have_win32_api" = yes] )
AM_CONDITIONAL([USE_POSIXAPI], [test "$ac_have_posix_api" = yes] )
AM_CONDITIONAL([USE_CPP0XAPI], [test "$ac_have_cpp0x_api" = yes] )

AC_DEFINE_UNQUOTED(CONFIG_HOST_TYPE, "$host",[Host type from configure])
AC_DEFINE_UNQUOTED(CONFIG_BUILD_TYPE, "$build",[Build type from configure])
AC_DEFINE_UNQUOTED(CONFIGURE_OPTIONS, "$ac_configure_args", [Configure command line used to configure cmsc])

if test $ac_make_dll = yes; then
	LT_INIT([win32-dll])
	LT_LANG([Windows Resource])
fi

if test $ac_have_win32_api = yes; then
	HAVE_WIN32_API=1
else
	HAVE_WIN32_API=0
fi

if test $ac_have_posix_api = yes; then
	HAVE_POSIX_API=1
else
	HAVE_POSIX_API=0
fi

if test $ac_have_cpp0x_api = yes; then
	HAVE_CPP0X_API=1
else
	HAVE_CPP0X_API=0
fi

# Check for needed libraries
AC_SEARCH_LIBS([gethostbyname],[nsl resolv bind])
AC_SEARCH_LIBS([bind],[socket])
AC_SEARCH_LIBS([pthread_mutex_init], [pthread])
AC_SEARCH_LIBS([clock_gettime], [rt])

dnl Check for libcrs
AC_MSG_CHECKING(for libcrs)
AC_LANG_PUSH([C++])
save_LIBS="$LIBS"
LIBS="$LIBS -lcrs"
AC_LINK_IFELSE(
	[AC_LANG_PROGRAM(
		[[#include <crs/mutex.h>]],
		[[CrossClass::cMutex mutex; mutex.lock(); mutex.unlock();]])],
	[have_libcrs=yes],
	[have_libcrs=no])
LIBS="$save_LIBS"
AC_LANG_POP([C++])
AC_MSG_RESULT($have_libcrs)
if test $have_libcrs = yes; then
	LIBS="$LIBS -lcrs"
else
	AC_MSG_FAILURE([failed to link with libcrs])
fi

dnl Make substitutions
AC_SUBST(HAVE_WIN32_API)
AC_SUBST(HAVE_POSIX_API)
AC_SUBST(HAVE_CPP0X_API)

AC_SUBST(HAVE_INTTYPES_H)
AC_SUBST(HAVE_STDINT_H)
AC_SUBST(HAVE_SYS_TYPES_H)
AC_SUBST(HAVE_OMP_H)

AC_CONFIG_FILES([Makefile src/Makefile twbyte/Makefile msdc/Makefile math/Makefile])
AC_OUTPUT

