lib_LTLIBRARIES = libcrs.la

INCLUDES = -I$(top_srcdir)

libcrs_la_SOURCES = CRCStuff.c cmdLine.cpp parsing.cpp security.cpp timer.cpp \
	oio/FileVar.cpp oio/iniFile.cpp \
	math/interpolation.cpp math/peakCenter.cpp math/stddev.c \
	bits/basic.netpoint.cpp simplenetpoint.cpp \
	sc/basic.rs232port.cpp \
	ude/packets.cpp ude/netpoint.cpp ude/server.cpp ude/client.cpp ude/pingtool.cpp \
	msdc/data_classes.cpp msdc/pools.cpp msdc/ring_pool.cpp msdc/composite_core.cpp

libcrs_la_LDFLAGS = -no-undefined -version-info $(CRS_LT_VERSION)

pkginclude_HEADERS = cmdLine.h mutex.h handle.h parsing.h security.h \
	thread.h timer.h socket.h netpoint.h simplenetpoint.h main myexcept.h \
	regions.h mtsqueue.h charprintbuf.h CRCStuff.h libexport.h defsys.h \
	condition_variable.hpp semaphore.hpp callback.h

pkginclude_bitsdir = $(pkgincludedir)/bits
pkginclude_bits_HEADERS = bits/basic.netpoint.h bits/hostsocket.hpp

pkginclude_scdir = $(pkgincludedir)/sc
pkginclude_sc_HEADERS = sc/basic.rs232port.h sc/rs232port.h sc/serialbyte.h sc/i41serial.h sc/twelvebyte.h

pkginclude_oiodir = $(pkgincludedir)/oio
pkginclude_oio_HEADERS = oio/FileVar.h oio/iniFile.h

pkginclude_mathdir = $(pkgincludedir)/math
pkginclude_math_HEADERS = math/interpolation.h math/peakCenter.h math/stddev.h math/unimath.h math/unimath.cc

pkginclude_udedir = $(pkgincludedir)/ude
pkginclude_ude_HEADERS = ude/packets.h ude/netpoint.h ude/server.h ude/client.h ude/pingtool.h

pkginclude_msdcdir = $(pkgincludedir)/msdc
pkginclude_msdc_HEADERS = msdc/data_classes.h msdc/pools.h msdc/ring_pool.h msdc/composite_core.h

if USE_WIN32API
libcrs_la_SOURCES += bits/win32.timer.cpp bits/win32.mutex.cpp bits/win32.thread.cpp \
	bits/win32.cond.cpp bits/win32.semaphore.cpp \
	sc/win32.rs232port.cpp bits/win32.netpoint.cpp

pkginclude_bits_HEADERS += bits/win32.timer.h bits/win32.mutex.h bits/win32.thread.h \
	bits/win32.cond.h bits/win32.semaphore.h bits/win32.netpoint.h

pkginclude_sc_HEADERS += sc/win32.rs232port.h

libcrs_la_SOURCES += cross.res.rc
pkginclude_bits_HEADERS += resource.h
endif

if USE_POSIXAPI
libcrs_la_SOURCES += bits/posix.timer.cpp bits/posix.mutex.cpp bits/posix.thread.cpp \
	bits/posix.cond.cpp bits/posix.semaphore.cpp \
	sc/posix.rs232port.cpp bits/posix.netpoint.cpp

pkginclude_bits_HEADERS += bits/posix.timer.h bits/posix.mutex.h bits/posix.thread.h \
	bits/posix.cond.h bits/posix.semaphore.h bits/posix.netpoint.h

pkginclude_sc_HEADERS += sc/posix.rs232port.h
endif

if USE_CPP0XAPI
libcrs_la_SOURCES += bits/c++0x.timer.cpp
pkginclude_bits_HEADERS += bits/c++0x.timer.h
endif

if CYGWIN_TARGET
libcrs_la_SOURCES += bits/win32.timer.cpp bits/win32.mutex.cpp bits/win32.thread.cpp \
	bits/win32.cond.cpp bits/win32.semaphore.cpp
pkginclude_bits_HEADERS += bits/win32.timer.h bits/win32.mutex.h bits/win32.thread.h \
	bits/win32.cond.h bits/win32.semaphore.h 

libcrs_la_SOURCES += cross.res.rc
pkginclude_bits_HEADERS += resource.h
endif

.rc.lo: 
	$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --tag=RC --mode=compile $(RC) $(RCFLAGS) $< -o $@ 
