lib_LIBRARIES = libcrs.a

INCLUDES = -I$(top_srcdir)

libcrs_a_SOURCES = CRCStuff.c cmdLine.cpp parsing.cpp security.cpp timer.cpp \
	oio/FileVar.cpp oio/iniFile.cpp \
	math/interpolation.cpp math/peakCenter.cpp math/stddev.c \
	bits/basic.netpoint.cpp simplenetpoint.cpp \
	sc/basic.rs232port.cpp
#	sc/rs232port.cpp

#libcrs_a_LDFLAGS = -version-info $(CRS_LT_VERSION)

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

if USE_WIN32API
libcrs_a_SOURCES += bits/win32.timer.cpp bits/win32.mutex.cpp bits/win32.thread.cpp \
	bits/win32.netpoint.cpp bits/win32.cond.cpp sc/win32.rs232port.cpp \
	bits/win32.semaphore.cpp
pkginclude_bits_HEADERS += bits/win32.timer.h bits/win32.mutex.h bits/win32.thread.h \
	bits/win32.netpoint.h bits/win32.cond.h bits/win32.semaphore.h
pkginclude_sc_HEADERS += sc/win32.rs232port.h
endif

if USE_POSIXAPI
libcrs_a_SOURCES += bits/posix.timer.cpp bits/posix.mutex.cpp bits/posix.thread.cpp \
	bits/posix.netpoint.cpp bits/posix.cond.cpp sc/posix.rs232port.cpp \
	bits/posix.semaphore.cpp
pkginclude_bits_HEADERS += bits/posix.timer.h bits/posix.mutex.h bits/posix.thread.h \
	bits/posix.netpoint.h bits/posix.cond.h bits/posix.semaphore.h
pkginclude_sc_HEADERS += sc/posix.rs232port.h
endif

if USE_CPP0XAPI
libcrs_a_SOURCES += bits/c++0x.timer.cpp
pkginclude_bits_HEADERS += bits/c++0x.timer.h
endif
