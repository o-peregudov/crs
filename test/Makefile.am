AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)

LDADD = $(top_builddir)/crs/libcrs.la

AM_TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(top_builddir)/test/.libs:$(top_builddir)/test:$$LD_LIBRARY_PATH
AM_DEFAULT_SOURCE_EXT = .cpp

@CODE_COVERAGE_RULES@

TESTS =	test_mutex		\
	test_heap		\
	test_interval_map

check_PROGRAMS =		\
	test_mutex		\
	test_heap		\
	test_interval_map

#handle_SOURCES = handle.cpp
#condition_SOURCES = condition.cpp
#server_SOURCES = server.cpp
#client_SOURCES = client.cpp
#ring_pool_SOURCES = ring-pool.cpp
#measurePeak_SOURCES = measurePeak.cpp
#parabolicInterpolation_SOURCES = parabolicInterpolation.cpp
#ev_loop_SOURCES = ev-loop.cpp

#i41_loop_SOURCES = i41-loop.cpp
#if WINDOWS_RESOURCE
#i41_loop_SOURCES += loop.rc
#endif

.rc.lo: 
	$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --tag=RC --mode=compile $(RC) $(RCFLAGS) $< -o $@
