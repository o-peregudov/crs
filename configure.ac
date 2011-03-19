AC_PREREQ([2.63])
AC_INIT([crs],[0.8.0],[op@pochta.ru])
AM_INIT_AUTOMAKE

dnl  Here are a set of rules to help you update your library version information:
dnl 1. Start with version information of '0:0:0' for each libtool library.
dnl 2. Update the version information only immediately before a public release of your soft-
dnl    ware. More frequent updates are unnecessary, and only guarantee that the current
dnl    interface number gets larger faster.
dnl 3. If the library source code has changed at all since the last update, then increment
dnl    revision ('c:r:a' becomes 'c:r + 1:a').
dnl 4. If any interfaces have been added, removed, or changed since the last update, increment
dnl    current, and set revision to 0.
dnl 5. If any interfaces have been added since the last public release, then increment age.
dnl 6. If any interfaces have been removed since the last public release, then set age to 0.

CRS_CURRENT=1
CRS_REVISION=0
CRS_AGE=0

dnl Check for which system.
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
LT_INIT

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

libcrs_disable_shared=no
dnl AC_MSG_CHECKING(whether to disable shared library)
AC_ARG_ENABLE(shared,
	AS_HELP_STRING([--disable-shared],[By default libcrs is compiled
			as a shared library. This flag disables the behavior]),
	[
		case	"${enableval}" in
			yes)	;;
			no)	libcrs_disable_shared=yes ;;
			*)	AC_MSG_ERROR(bad value ${enableval} for --disable-shared) ;;
		esac
dnl		AC_MSG_RESULT([$libcrs_disable_shared])
	]
)

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([\
	arpa/inet.h	\
	fcntl.h	\
	netdb.h	\
	netinet/in.h\
	sys/socket.h\
	sys/select.h\
	poll.h	\
	syslog.h	\
	termios.h	])

dnl AC_CHECK_HEADERS(	[inttypes.h],	[HAVE_INTTYPES_H=1],	[HAVE_INTTYPES_H=0] )
dnl AC_CHECK_HEADERS(	[stdint.h],		[HAVE_STDINT_H=1],	[HAVE_STDINT_H=0]	)
dnl AC_CHECK_HEADERS(	[sys/types.h],	[HAVE_SYS_TYPES_H=1],	[HAVE_SYS_TYPES_H=0] )

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

dnl Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_CHECK_FUNCS([\
	gethostbyname	\
	inet_ntoa		\
	memmove		\
	memcpy		\
	memset		\
	select		\
	poll			\
	socket		\
	strerror	])

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

ac_make_dll=no
ac_have_winsock=no

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
		
		dnl
		dnl Check for Winsock only on MinGW, on Cygwin we must use emulated BSD socket API
		dnl
		AC_MSG_CHECKING(for winsock)
		save_LIBS="$LIBS"
		for curlib in ws2_32 wsock32; do
			LIBS="$LIBS -l$curlib"
			AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <winsock.h>]], [[
					socket(0,0,0);
					select(0,NULL,NULL,NULL,NULL);
					closesocket(0);
					gethostname(NULL,0);
					]])],[ac_have_winsock=yes],[ac_have_winsock=no])
			if test $ac_have_winsock = yes; then
				ac_cv_func_select='yes'
				if test $curlib = ws2_32; then
					ac_have_winsock=winsock2
				fi
				break
			fi
			LIBS="$save_LIBS"
		done
			AC_MSG_RESULT($ac_have_winsock)
		if test $ac_have_winsock = winsock2; then
			AC_CHECK_HEADERS(winsock2.h)
		else
			AC_CHECK_HEADERS(winsock.h)
		fi
		;;
	*)
		ac_have_posix_api=yes
		ac_linux_target=yes
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

AM_CONDITIONAL([USE_WINSOCK2], [test "$ac_have_winsock" = yes] )

AC_DEFINE_UNQUOTED(CONFIG_HOST_TYPE, "$host",[Host type from configure])
AC_DEFINE_UNQUOTED(CONFIG_BUILD_TYPE, "$build",[Build type from configure])
AC_DEFINE_UNQUOTED(LIBCRS_CONFIGURE_OPTIONS, "$ac_configure_args", [configure command line used to configure libcrs])

AM_CONDITIONAL([WINDOWS_RESOURCE], [test "$ac_make_dll" = yes] )

if test $libcrs_disable_shared = yes; then
	DEFINE_CROSS_EXPORT=1
	ac_make_dll=no
else
	DEFINE_CROSS_EXPORT=0
fi

if test $ac_make_dll = yes; then
	LT_INIT([win32-dll])
	LT_LANG([Windows Resource])
fi

if test $ac_have_posix_api = yes; then
	AC_CHECK_LIB(pthread, pthread_mutex_init)
	AC_CHECK_LIB(rt, clock_gettime)
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

if test $ac_have_winsock = winsock2; then
	HAVE_WINSOCK2_H=1
else
	HAVE_WINSOCK2_H=0
fi

dnl Check for types
AC_CHECK_SIZEOF(char)
AC_CHECK_SIZEOF(int8_t)
AC_CHECK_SIZEOF(uint8_t)
AC_CHECK_SIZEOF(u_int8_t)

AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int16_t)
AC_CHECK_SIZEOF(uint16_t)
AC_CHECK_SIZEOF(u_int16_t)

AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(int32_t)
AC_CHECK_SIZEOF(uint32_t)
AC_CHECK_SIZEOF(u_int32_t)

AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(int64_t)
AC_CHECK_SIZEOF(__int64)

case 1 in
	$ac_cv_sizeof_char) SIZE8="char";;
	$ac_cv_sizeof_int8_t) SIZE8="int8_t";;
esac

case 1 in
	$ac_cv_sizeof_char) USIZE8="unsigned char";;
	$ac_cv_sizeof_uint8_t) USIZE8="uint8_t";;
	$ac_cv_sizeof_u_int8_t) USIZE8="u_int8_t";;
esac

case 2 in
	$ac_cv_sizeof_short) SIZE16="short";;
	$ac_cv_sizeof_int) SIZE16="int";;
	$ac_cv_sizeof_int16_t) SIZE16="int16_t";;
esac

case 2 in
	$ac_cv_sizeof_short) USIZE16="unsigned short";;
	$ac_cv_sizeof_int) USIZE16="unsigned int";;
	$ac_cv_sizeof_u_int16_t) USIZE16="u_int16_t";;
	$ac_cv_sizeof_uint16_t) USIZE16="uint16_t";;
esac

case 4 in
	$ac_cv_sizeof_short) SIZE32="short";;
	$ac_cv_sizeof_int) SIZE32="int";;
	$ac_cv_sizeof_long) SIZE32="long";;
	$ac_cv_sizeof_int32_t) SIZE32="int32_t";;
esac

case 4 in
	$ac_cv_sizeof_short) USIZE32="unsigned short";;
	$ac_cv_sizeof_int) USIZE32="unsigned int";;
	$ac_cv_sizeof_long) USIZE32="unsigned long";;
	$ac_cv_sizeof_u_int32_t) USIZE32="u_int32_t";;
	$ac_cv_sizeof_uint32_t) USIZE32="uint32_t";;
esac

case 8 in
	$ac_cv_sizeof_int) SIZE64="int";;
	$ac_cv_sizeof_long) SIZE64="long";;
	$ac_cv_sizeof_long_long) SIZE64="long long";;
	$ac_cv_sizeof_int64_t) SIZE64="int64_t";;
	$ac_cv_sizeof___int64) SIZE64="__int64";;
esac

if test -z "$SIZE8"; then
	AC_MSG_ERROR(No 8 bit type found on this platform!)
fi
if test -z "$USIZE8"; then
	AC_MSG_ERROR(No unsigned 8 bit type found on this platform!)
fi
if test -z "$SIZE16"; then
	AC_MSG_ERROR(No 16 bit type found on this platform!)
fi
if test -z "$USIZE16"; then
	AC_MSG_ERROR(No unsigned 16 bit type found on this platform!)
fi
if test -z "$SIZE32"; then
	AC_MSG_ERROR(No 32 bit type found on this platform!)
fi
if test -z "$USIZE32"; then
	AC_MSG_ERROR(No unsigned 32 bit type found on this platform!)
fi
if test -z "$SIZE64"; then
	AC_MSG_ERROR(No 64 bit type found on this platform!)
fi

dnl Make substitutions
AC_SUBST(HAVE_WIN32_API)
AC_SUBST(HAVE_POSIX_API)
AC_SUBST(HAVE_CPP0X_API)

AC_SUBST(HAVE_INTTYPES_H)
AC_SUBST(HAVE_STDINT_H)
AC_SUBST(HAVE_SYS_TYPES_H)
AC_SUBST(HAVE_WINSOCK2_H)

AC_SUBST(DEFINE_CROSS_EXPORT)

AC_SUBST(SIZE8)
AC_SUBST(USIZE8)
AC_SUBST(SIZE16)
AC_SUBST(USIZE16)
AC_SUBST(SIZE32)
AC_SUBST(USIZE32)
AC_SUBST(SIZE64)

CRS_LT_VERSION="${CRS_CURRENT}:${CRS_REVISION}:${CRS_AGE}"
AC_SUBST(CRS_LT_VERSION)

AC_CONFIG_FILES([Makefile crs/Makefile crs/defsys.h])
AC_OUTPUT
