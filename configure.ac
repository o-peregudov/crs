AC_PREREQ([2.63])
AC_INIT([crs],[2.0.0],[o.peregudov@gmail.com])

AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])

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

CRS_CURRENT=2
CRS_REVISION=0
CRS_AGE=0

CRS_LT_VERSION="${CRS_CURRENT}:${CRS_REVISION}:${CRS_AGE}"
AC_SUBST(CRS_LT_VERSION)

dnl Split VERSION into CRS_VERSION_MAJOR and CRS_VERSION_MINOR
dnl Follows AX_SPLIT_VERSION macro from AC-Archive
dnl Rhys Ulerich <rhys.ulerich@gmail.com>
AC_PROG_SED
CRS_MAJOR_VERSION=`echo "$VERSION" | $SED 's/\([[^.]][[^.]]*\).*/\1/'`
CRS_MINOR_VERSION=`echo "$VERSION" | $SED 's/[[^.]][[^.]]*.\([[^.]][[^.]]*\).*/\1/'`
AC_SUBST(CRS_MAJOR_VERSION)
AC_SUBST(CRS_MINOR_VERSION)

dnl CRS_CPPFLAGS=
dnl CRS_CFLAGS=
dnl CRS_LIBS=

AC_DEFINE_UNQUOTED(CONFIG_HOST_TYPE, "$host",[Host type from configure])
AC_DEFINE_UNQUOTED(CONFIG_BUILD_TYPE, "$build",[Build type from configure])
AC_DEFINE_UNQUOTED(LIBCRS_CONFIGURE_OPTIONS, "$ac_configure_args", [configure command line used to configure libcrs])

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

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([\
	arpa/inet.h	\
	fcntl.h	\
	netdb.h	\
	netinet/in.h\
	unistd.h	\
	sys/socket.h\
	syslog.h	\
	termios.h	\
	semaphore.h	\
	omp.h		])

dnl
dnl Check for integer types
dnl
if test x$ac_cv_header_inttypes_h = xyes; then
	HAVE_INTTYPES_H=1
else
	HAVE_INTTYPES_H=0
fi
AC_SUBST(HAVE_INTTYPES_H)

if test x$ac_cv_header_stdint_h = xyes; then
	HAVE_STDINT_H=1
else
	HAVE_STDINT_H=0
fi
AC_SUBST(HAVE_STDINT_H)

if test x$ac_cv_header_sys_types_h = xyes; then
	HAVE_SYS_TYPES_H=1
else
	HAVE_SYS_TYPES_H=0
fi
AC_SUBST(HAVE_SYS_TYPES_H)

dnl
dnl Checks for library functions.
dnl
AC_FUNC_ERROR_AT_LINE
AC_CHECK_FUNCS([\
	gethostbyname	\
	inet_ntoa		\
	socket		\
	strerror	])

dnl
dnl Checks for typedefs, structures, and compiler characteristics.
dnl
AC_C_INLINE
AC_C_RESTRICT
AC_TYPE_SIZE_T

dnl
dnl Check for OpenMP
dnl
if test x$ac_cv_header_omp_h = xyes; then
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
	if test x$have_openmp = xyes; then
		CFLAGS="$CFLAGS -fopenmp"
		CXXFLAGS="$CXXFLAGS -fopenmp"
		LIBS="$LIBS -lgomp"
		HAVE_OMP_H=1
	else
		HAVE_OMP_H=0
	fi
fi

dnl
dnl Check for GSL
dnl
AX_PATH_GSL([1.14],[have_libgsl=yes],[have_libgsl=no])
if test x$have_libgsl = xyes; then
	CFLAGS="$CFLAGS $GSL_CFLAGS"
	CPPFLAGS="$CPPFLAGS -DGSL_RANGE_CHECK_OFF -DHAVE_INLINE"
	CXXFLAGS="$CXXFLAGS $GSL_CFLAGS"
	LIBS="$LIBS $GSL_LIBS"
	HAVE_LIBGSL=1
else
	HAVE_LIBGSL=0
fi
AC_SUBST(HAVE_LIBGSL)

dnl
dnl Check for GLIB
dnl
AM_PATH_GLIB_2_0([2.24.2],[have_glib_2_0=yes],[have_glib_2_0=no])
if test x$have_glib_2_0 = xyes; then
	CFLAGS="$CFLAGS $GLIB_CFLAGS"
	CXXFLAGS="$CXXFLAGS $GLIB_CFLAGS"
	LIBS="$LIBS $GLIB_LIBS"
	HAVE_GLIB_2_0=1
else
	HAVE_GLIB_2_0=0
fi
AC_SUBST(HAVE_GLIB_2_0)

CRS_CFLAGS="$CRS_CFLAGS -I$includedir"
CRS_LIBS="$CRS_LIBS -L$libdir -lcrs"
AC_SUBST(CRS_CFLAGS)
AC_SUBST(CRS_LIBS)

dnl
dnl Cygwin needs special define
dnl
case "$build_os" in
	cygwin*)
		CPPFLAGS="$CPPFLAGS -D__LARGE64_FILES"
		;;
	*)
		;;
esac

dnl
dnl Check for target OS
dnl
ac_cygwin_target=no
ac_mingw_target=no
ac_linux_target=no

ac_have_win32_api=no
ac_have_posix_api=no

ac_cv_winsock2_works=no

USE_WINSOCK2_API=0
case "$host_os" in
	cygwin*)
		ac_cygwin_target=yes
		ac_have_posix_api=yes
		ac_make_dll=yes
		;;
	mingw*)
		ac_mingw_target=yes
		ac_have_win32_api=yes
		ac_make_dll=yes
		
		dnl
		dnl Check for Winsock2 only on MinGW, on Cygwin we must use emulated BSD socket API
		dnl
		AC_MSG_CHECKING(for winsock2)
		AC_LANG_PUSH([C])
		save_LIBS="$LIBS"
		save_CFLAGS="$CFLAGS"
		LIBS="$LIBS -lws2_32"
		AC_LINK_IFELSE(
			[AC_LANG_PROGRAM(
				[[#include <winsock2.h>]],
				[[	socket(0,0,0);
					select(0,NULL,NULL,NULL,NULL);
					closesocket(0);
					gethostname(NULL,0);
				]])],
			[ac_cv_winsock2_works=yes],[ac_cv_winsock2_works=no])
		LIBS="$save_LIBS"
		AC_LANG_POP([C])
		AC_MSG_RESULT($ac_cv_have_winsock2)
		if test x$ac_cv_winsock2_works = xyes; then
			LIBS="$LIBS -lws2_32"
			CRS_LIBS="$CRS_LIBS -lws2_32"
			USE_WINSOCK2_API=1
		else
			AC_MSG_FAILURE([failed to link with winsock2])
		fi
		;;
	*)
		ac_linux_target=yes
		ac_have_posix_api=yes
		ac_make_dll=no
		;;
esac
AC_SUBST(USE_WINSOCK2_API)

AM_CONDITIONAL([CYGWIN_TARGET], [test x$ac_cygwin_target = xyes] )
AM_CONDITIONAL([MINGW_TARGET], [test x$ac_mingw_target = xyes] )
AM_CONDITIONAL([LINUX_TARGET], [test x$ac_linux_target = xyes] )

AM_CONDITIONAL([USE_WIN32_API], [test x$ac_have_win32_api = xyes] )
AM_CONDITIONAL([USE_POSIX_API], [test x$ac_have_posix_api = xyes] )
AM_CONDITIONAL([USE_WINSOCK2_API], [test x$ac_cv_winsock2_works = xyes] )

if test x$ac_have_posix_api = xyes; then
	AC_CHECK_LIB(rt, clock_gettime)
	
	dnl Verify that we have POSIX mutex
	AC_CHECK_FUNCS(pthread_mutex_lock)
	
	dnl Verify that we have POSIX spin locks
	AC_CHECK_FUNCS(pthread_spin_lock)
	
	HAVE_POSIX_API=1
else
	HAVE_POSIX_API=0
fi
AC_SUBST(HAVE_POSIX_API)
AM_CONDITIONAL([HAVE_PTHREAD_SPIN_LOCK], [test x$ac_cv_func_pthread_spin_lock = xyes])

if test x$ac_have_win32_api = xyes; then
	HAVE_WIN32_API=1
else
	HAVE_WIN32_API=0
fi
AC_SUBST(HAVE_WIN32_API)

dnl
dnl Whether we need to build only static library
dnl
libcrs_disable_shared=no
AC_ARG_ENABLE(shared,
	AS_HELP_STRING([--disable-shared],[By default libcrs is compiled
			as a shared library. This flag disables the behavior.]),
	[
		case	"${enableval}" in
			yes)	;;
			no)	libcrs_disable_shared=yes ;;
			*)	AC_MSG_ERROR(bad value ${enableval} for --disable-shared) ;;
		esac
	]
)
if test x$libcrs_disable_shared = xyes; then
	DEFINE_CROSS_EXPORT=0
	ac_make_dll=no
else
	DEFINE_CROSS_EXPORT=1
fi
AC_SUBST(DEFINE_CROSS_EXPORT)

AM_CONDITIONAL([WINDOWS_RESOURCE], [test x$ac_make_dll = xyes] )
if test x$ac_make_dll = xyes; then
	LT_INIT([win32-dll])
	LT_LANG([Windows Resource])
	CROSS_MAKE_DLL=1
else
	CROSS_MAKE_DLL=0
fi
AC_SUBST(CROSS_MAKE_DLL)

dnl
dnl Check whether we have to disable epoll () support
dnl
disable_epoll=no
AC_ARG_ENABLE(epoll, AS_HELP_STRING([--disable-epoll],[Disable Linux epoll(2) support.]),
	[
		case "$enableval" in
			yes)
				;;
			no)
				AC_MSG_WARN([Forcing epoll() to be disabled])
				disable_epoll=yes
				;;
		esac
	])

dnl
dnl Check for epoll () on POSIX systems
dnl
USE_EPOLL=0
if test x$ac_have_posix_api = xyes && test x$disable_epoll = xno; then
	dnl
	dnl auto-detect and verify sys/epoll.h header and library present and working
	dnl (some parts were borrowed from the squid-cache project)
	dnl
	
	dnl Check for epoll_create
	AC_CHECK_FUNCS(epoll_create)
	
	dnl epoll requires sys/epoll.h
	AC_CHECK_HEADERS([sys/epoll.h])
	
	dnl Verify that epoll really works
	if test x$ac_cv_func_epoll_create = xyes; then
		AC_CACHE_CHECK(if epoll works, ac_cv_epoll_works,
			AC_RUN_IFELSE([AC_LANG_SOURCE(
			[[	#include <sys/epoll.h>
				#include <unistd.h>
				int main(int argc, char **argv)
				{
					int fd = epoll_create (256);
					if (fd < 0)
						return 1;
					else
						close (fd);
					return 0;
				}
			]]
		)],[ac_cv_epoll_works=yes],[ac_cv_epoll_works=no],[:]))
	fi
	
	if test x$ac_cv_epoll_works = xyes; then
		USE_EPOLL=1
	fi
else
	ac_cv_epoll_works=no
fi
dnl AC_SUBST(USE_EPOLL)

dnl
dnl Check for poll () on POSIX systems
dnl
USE_POLL=0
if test x$ac_have_posix_api = xyes; then
	dnl
	dnl auto-detect and verify poll.h header and library present and working
	dnl
	
	dnl Check for poll
	AC_CHECK_FUNCS(poll)
	
	dnl epoll requires poll.h
	AC_CHECK_HEADERS([poll.h])
	
	dnl Verify that poll really works
	if test x$ac_cv_func_poll = xyes; then
		AC_CACHE_CHECK(if poll works, ac_cv_poll_works,
			AC_RUN_IFELSE([AC_LANG_SOURCE(
			[[	#include <poll.h>
				int main(int argc, char **argv)
				{
					return poll (0, 0, 1);
				}
			]]
		)],[ac_cv_poll_works=yes],[ac_cv_poll_works=no],[:]))
	fi
	
	if test x$ac_cv_poll_works = xyes; then
		USE_POLL=1
	fi
fi
dnl AC_SUBST(USE_POLL)

dnl
dnl Check for select () on POSIX systems
dnl
USE_SELECT=0
if test x$ac_have_posix_api = xyes; then
	dnl
	dnl auto-detect and verify sys/select.h header and library present and working
	dnl
	
	dnl Check for select
	AC_CHECK_FUNCS(select)
	
	dnl select requires sys/select.h
	AC_CHECK_HEADERS([sys/select.h sys/time.h sys/types.h unistd.h])
	
	dnl Verify that select really works
	if test x$ac_cv_func_select = xyes; then
		AC_CACHE_CHECK(if select works, ac_cv_select_works,
		AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
int main(int argc, char **argv)
{
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 10;
	return select (0, 0, 0, 0, &tv);
}
	]])],[ac_cv_select_works=yes],[ac_cv_select_works=no],[:]))
	fi
	
	if test x$ac_cv_select_works = xyes; then
		USE_SELECT=1
	fi
fi
dnl AC_SUBST(USE_SELECT)

if test x$ac_cv_winsock2_works = xyes; then
	SELECT_TYPE="winsock2"
	AC_DEFINE(WINSOCK2_IO_MULTIPLEXING,1,[Define to 1 if winsock2 should be used for I/O multiplexing.])
elif test x$ac_cv_epoll_works = xyes; then
	SELECT_TYPE="epoll"
	AC_DEFINE(EPOLL_IO_MULTIPLEXING,1,[Define to 1 if epoll should be used for I/O multiplexing.])
elif test x$ac_cv_poll_works = xyes; then
	SELECT_TYPE="poll"
	AC_DEFINE(POLL_IO_MULTIPLEXING,1,[Define to 1 if poll should be used for I/O multiplexing.])
else
	SELECT_TYPE="select"
	AC_DEFINE(SELECT_IO_MULTIPLEXING_POSIX,1,[Define to 1 if POSIX select should be used for I/O multiplexing.])
fi
AC_DEFINE_UNQUOTED(SELECT_TYPE,"$SELECT_TYPE",[Define to inform about available I/O multiplexing method.])
AC_MSG_NOTICE([Using ${SELECT_TYPE} for the internal I/O loops.])

dnl
dnl Enable STL's <chrono>
dnl
AC_ARG_ENABLE(chrono, AS_HELP_STRING([--disable-chrono],[Disable C++11 <chrono> support.]),
	[
		case "$enableval" in
			yes)
				disable_chrono="no"
				;;
			no)
				AC_MSG_WARN([Forcing C++11 <chrono> to be disabled])
				disable_chrono="yes"
				;;
		esac
	])

dnl
dnl auto-detect and verify <chrono> header present and really works
dnl
AC_LANG_PUSH([C++])
AC_CACHE_CHECK(whether <chrono> is usable, ac_cv_std_chrono_works,
	AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <chrono>
int main (int argc, char **argv)
{
	long sum = 0;
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime = std::chrono::high_resolution_clock::now ();
	for (long i = 0; i < 1000; sum += i++);
	std::chrono::high_resolution_clock::duration usExeTime = std::chrono::high_resolution_clock::now () - startTime;
	return 0;
}
	]])],[ac_cv_std_chrono_works=yes],[ac_cv_std_chrono_works=no],[:]))
AC_LANG_POP([C++])
if test x$disable_chrono = xyes ; then
	USE_CXX11_CHRONO=0
elif test x$ac_cv_std_chrono_works = xyes ; then
	USE_CXX11_CHRONO=1
else
	USE_CXX11_CHRONO=0
fi
AC_SUBST(USE_CXX11_CHRONO)

dnl
dnl auto-detect and verify <mutex> header present and really works
dnl
AC_LANG_PUSH([C++])
AC_CACHE_CHECK(whether <mutex> is usable, ac_cv_std_mutex_works,
	AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <mutex>
int main (int argc, char **argv)
{
	std::mutex mutex;
	std::unique_lock<std::mutex> lock (mutex);
	if (lock)
		return 0;
	else
		return 1;
}
	]])],[ac_cv_std_mutex_works=yes],[ac_cv_std_mutex_works=no],[:]))
AC_LANG_POP([C++])
if test x$ac_cv_std_mutex_works = xyes ; then
	USE_CXX11_MUTEX=1
	
	dnl
	dnl auto-detect and verify <condition_variable> header present and really works
	dnl
	AC_LANG_PUSH([C++])
	AC_CACHE_CHECK(whether <condition_variable> is usable, ac_cv_std_condition_variable_works,
		AC_RUN_IFELSE([AC_LANG_SOURCE([[
	#include <mutex>
	#include <condition_variable>
	int main (int argc, char **argv)
	{
		std::mutex mutex;
		std::condition_variable condition;
		bool flag (false);
		std::unique_lock<std::mutex> lock (mutex);
		flag = true;
		condition.notify_one ();
		return 0;
	}
		]])],[ac_cv_std_condition_variable_works=yes],[ac_cv_std_condition_variable_works=no],[:]))
	AC_LANG_POP([C++])
	if test x$ac_cv_std_condition_variable_works = xyes ; then
		USE_CXX11_CONDITION_VARIABLE=1
	else
		USE_CXX11_CONDITION_VARIABLE=0
	fi
else
	USE_CXX11_MUTEX=0
	USE_CXX11_CONDITION_VARIABLE=0
fi
AC_SUBST(USE_CXX11_MUTEX)
AC_SUBST(USE_CXX11_CONDITION_VARIABLE)

dnl
dnl auto-detect and verify <atomic> header present and really works
dnl
AC_LANG_PUSH([C++])
AC_CACHE_CHECK(whether <atomic> is usable, ac_cv_std_atomic_works,
	AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <atomic>
int main (int argc, char **argv)
{
	std::atomic<bool> flag_bool (false);
	if (flag_bool.is_lock_free ())
		return 0;
	else
		return 1;
}
	]])],[ac_cv_std_atomic_works=yes],[ac_cv_std_atomic_works=no],[:]))
AC_LANG_POP([C++])
if test $ac_cv_std_atomic_works = xyes ; then
	USE_CXX11_ATOMIC=1
else
	USE_CXX11_ATOMIC=0
fi
AC_SUBST(USE_CXX11_ATOMIC)

dnl
dnl Check for types
dnl
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

AC_CHECK_SIZEOF(void*)

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
AC_SUBST(SIZE8)
AC_SUBST(USIZE8)
AC_SUBST(SIZE16)
AC_SUBST(USIZE16)
AC_SUBST(SIZE32)
AC_SUBST(USIZE32)
AC_SUBST(SIZE64)

AC_ARG_ENABLE(destructor-exceptions,
[  --disable-destructor-exceptions  Don't throw exceptions from destructors.
  --enable-destructor-exceptions   Throw all exceptions (even from destructors).])
if test x$enable_destructor_exceptions != xno; then
	AC_DEFINE(DESTRUCTOR_EXCEPTIONS_ALLOWED,1,[Define to 1 if you want to get all exceptions (even from destructors).])
fi

AC_CONFIG_FILES([	Makefile \
			crs.pc \
			crs/Makefile \
			crs/defsys.h \
			crs/crs-version.h \
			test/Makefile \
])
AC_OUTPUT
