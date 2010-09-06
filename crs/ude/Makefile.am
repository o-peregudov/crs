lib_LIBRARIES = libcrsude.a

INCLUDES = -I$(top_srcdir)
SUBLIBS = ../libcrs.a

libcrsude_a_SOURCES = basics.cpp netpoint.cpp devman.cpp devmanstub.cpp server.cpp
libcrsude_a_LIBADD = $(SUBLIBS)

pkginclude_udedir = $(pkgincludedir)/ude
pkginclude_ude_HEADERS = packets.h basics.h netpoint.h devman.h devmanstub.h server.h
