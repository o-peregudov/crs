bin_PROGRAMS = loop
INCLUDES = -I$(top_srcdir)
loop_SOURCES = loop.cpp

if CYGWIN_TARGET
loop_SOURCES += loop.res.rc
endif

if MINGW_TARGET
loop_SOURCES += loop.res.rc
endif

.rc.o:
	$(RC) $(RCFLAGS) $(DEFAULT_INCLUDES) $< -o $@
