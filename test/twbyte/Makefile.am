bin_PROGRAMS = loop

INCLUDES = -I$(top_srcdir)

loop_SOURCES = loop.cpp
loop_DEPENDENCIES =
loop_LDADD =
BUILT_SOURCES =
CLEANFILES =

if CYGWIN_TARGET
loop_SOURCES += loop.res.rc
endif

if MINGW_TARGET
loop_SOURCES += loop.res.rc
endif

.rc.o:
	$(RC) $(RCFLAGS) $(DEFAULT_INCLUDES) $< -o $@
