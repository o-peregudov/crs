AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)

lib_LTLIBRARIES = libcrs.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA= $(top_builddir)/crs.pc

libcrs_la_LDFLAGS = -no-undefined -version-info $(CRS_LT_VERSION)

libcrs_la_SOURCES =		\
	crs-version.c		\
	CRCStuff.c		\
	disjoint_set.cpp

#	cmdLine.cpp		\
	parsing.cpp		\
	security.cpp		\
	timer.cpp		\
	bits/basic.ipchannel.cpp \
	bits/basic.netpoint.cpp	\
	oio/FileVar.cpp		\
	oio/iniFile.cpp		\
	math/interpolation.cpp	\
	math/peakCenter.cpp	\
	math/stddev.c		\
	math/spectrum.cpp	\
	tcp_peer.cpp		\
	simplenetpoint.cpp	\
	sc/basic.rs232port.cpp	\
	ude/packets.cpp		\
	ude/net_peer.cpp	\
	ude/netpoint.cpp	\
	ude/server.cpp		\
	msdc/data_classes.cpp	\
	msdc/pools.cpp		\
	msdc/ring_pool.cpp	\
	msdc/composite_core.cpp

#	ude/client.cpp		\
#	ude/pingtool.cpp

pkginclude_HEADERS =		\
	defsys.h		\
	libexport.h		\
	mutex.h			\
	callback.h		\
	crs-version.h		\
	CRCStuff.h		\
	heap.h			\
	interval_map.h		\
	disjoint_set.h

#	cmdLine.h		\
	handle.h		\
	parsing.h		\
	security.h		\
	thread.h		\
	timer.h			\
	socket.h		\
	tcp_peer.h		\
	netpoint.h		\
	simplenetpoint.h	\
	main			\
	myexcept.h		\
	regions.h		\
	charprintbuf.h		\
	condition_variable.hpp	\
	semaphore.hpp		\
	eveloop.h		\
	atomic_flag.h		\
	spinlock.h		\
	ipchannel.h

pkginclude_bitsdir = $(pkgincludedir)/bits
pkginclude_bits_HEADERS =
#	bits/basic.netpoint.h	\
	bits/hostsocket.hpp	\
	bits/basic.ipchannel.h

pkginclude_scdir = $(pkgincludedir)/sc
pkginclude_sc_HEADERS =
#	sc/basic.rs232port.h	\
	sc/rs232port.h		\
	sc/serialbyte.h		\
	sc/i41serial.h		\
	sc/twelvebyte.h

pkginclude_oiodir = $(pkgincludedir)/oio
pkginclude_oio_HEADERS =
#	oio/FileVar.h		\
	oio/iniFile.h

pkginclude_mathdir = $(pkgincludedir)/math
pkginclude_math_HEADERS =
#	math/interpolation.h	\
	math/peakCenter.h	\
	math/stddev.h		\
	math/unimath.h		\
	math/unimath.cc		\
	math/spectrum.h

pkginclude_udedir = $(pkgincludedir)/ude
pkginclude_ude_HEADERS =
#	ude/packets.h		\
	ude/data_core.h		\
	ude/net_peer.h		\
	ude/netpoint.h		\
	ude/server.h		\
	ude/client.h		\
	ude/pingtool.h

pkginclude_msdcdir = $(pkgincludedir)/msdc
pkginclude_msdc_HEADERS =
#	msdc/data_classes.h	\
	msdc/pools.h		\
	msdc/ring_pool.h	\
	msdc/composite_core.h

if PLATFORM_MINGW
libcrs_la_SOURCES +=		\
	bits/win32.mutex.cpp

#	bits/win32.thread.cpp	\
	bits/win32.cond.cpp	\
	bits/win32.semaphore.cpp \
	bits/win32.timer.cpp	\
	bits/win32.netpoint.cpp	\
	sc/win32.rs232port.cpp

pkginclude_bits_HEADERS +=	\
	bits/win32.mutex.h

#	bits/win32.thread.h	\
	bits/win32.cond.h	\
	bits/win32.semaphore.h	\
	bits/win32.spinlock.h	\
	bits/win32.timer.h	\
	bits/win32.netpoint.h

#pkginclude_sc_HEADERS +=	\
	sc/win32.rs232port.h
endif

if WINDOWS_RESOURCE
libcrs_la_SOURCES +=		\
	cross.res.rc

pkginclude_bits_HEADERS +=	\
	resource.h
endif

if PLATFORM_LINUX
libcrs_la_SOURCES +=		\
	bits/posix.mutex.cpp

#	bits/posix.thread.cpp	\
	bits/posix.cond.cpp	\
	sc/posix.rs232port.cpp	\
	bits/posix.semaphore.cpp \
	bits/posix.timer.cpp	\
	bits/posix.netpoint.cpp	\
	bits/posix.eveloop.cpp	\
	bits/posix.spinlock.cpp	\
	bits/posix.ipchannel.cpp

pkginclude_bits_HEADERS +=	\
	bits/posix.mutex.h

#	bits/posix.thread.h	\
	bits/posix.cond.h	\
	bits/posix.semaphore.h	\
	bits/posix.spinlock.h	\
	bits/posix.timer.h	\
	bits/posix.netpoint.h	\
	bits/posix.ipchannel.h

#pkginclude_sc_HEADERS +=	\
	sc/posix.rs232port.h
endif

if PLATFORM_CYGWIN
libcrs_la_SOURCES +=		\
	bits/posix.mutex.cpp

#	bits/posix.thread.cpp	\
	bits/posix.cond.cpp	\
	sc/posix.rs232port.cpp	\
	bits/posix.semaphore.cpp \
	bits/posix.timer.cpp	\
	bits/posix.netpoint.cpp	\
	bits/posix.eveloop.cpp	\
	bits/posix.spinlock.cpp	\
	bits/posix.ipchannel.cpp

pkginclude_bits_HEADERS +=	\
	bits/posix.mutex.h

#	bits/posix.thread.h	\
	bits/posix.cond.h	\
	bits/posix.semaphore.h	\
	bits/posix.spinlock.h	\
	bits/posix.timer.h	\
	bits/posix.netpoint.h	\
	bits/posix.ipchannel.h

#pkginclude_sc_HEADERS +=	\
	sc/posix.rs232port.h
endif

@CODE_COVERAGE_RULES@

.rc.lo: 
	$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --tag=RC --mode=compile $(RC) $(RCFLAGS) $< -o $@
