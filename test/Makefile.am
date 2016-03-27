ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)

LDADD = $(top_builddir)/crs/libcrs.la

AM_CFLAGS = @OPENMP_CFLAGS@
AM_CXXFLAGS = @OPENMP_CFLAGS@

TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(top_builddir)/test/.libs:$(top_builddir)/test:$$LD_LIBRARY_PATH

TESTS = 			\
	test_mutex		\
	test_heap		\
	test_interval_map

check_PROGRAMS =		\
	test_mutex		\
	test_heap		\
	test_interval_map

test_heap_SOURCES = test_heap.cpp
test_mutex_SOURCES = test_mutex.cpp
test_interval_map_SOURCES = test_interval_map.cpp

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
