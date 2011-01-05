bin_PROGRAMS = loop

INCLUDES = -I$(top_srcdir)

loop_SOURCES = loop.cpp

if USE_WIN32API
BUILT_SOURCES = loop.coff
CLEANFILES = loop.coff
loop_DEPENDENCIES = loop.rc
loop_LDADD = loop.coff
endif

loop.coff : loop.rc
	windres --input=$? --output=$@ --output-format=coff
