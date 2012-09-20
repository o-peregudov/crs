ACLOCAL_AMFLAGS = -I m4
INCLUDES = -I$(top_srcdir) -I$(top_builddir)
LDADD = $(top_builddir)/crs/libcrs.la

check_PROGRAMS = parabolicInterpolation handle measurePeak condition \
	ev-loop ring-pool server client i41-loop

server_SOURCES = server.cpp
client_SOURCES = client.cpp
handle_SOURCES = handle.cpp
ring_pool_SOURCES = ring-pool.cpp
condition_SOURCES = condition.cpp
measurePeak_SOURCES = measurePeak.cpp
parabolicInterpolation_SOURCES = parabolicInterpolation.cpp

ev_loop_SOURCES = ev-loop.cpp


i41_loop_SOURCES = i41-loop.cpp
if WINDOWS_RESOURCE
i41_loop_SOURCES += loop.rc
endif

.rc.lo: 
	$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --tag=RC --mode=compile $(RC) $(RCFLAGS) $< -o $@

