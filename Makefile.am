ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)

AM_CFLAGS = @OPENMP_CFLAGS@
AM_CXXFLAGS = @OPENMP_CFLAGS@

SUBDIRS = crs test

@CODE_COVERAGE_RULES@

clean-local:
	-find . -name "*.gcda" -o -name "*.gcov" -o -name "*.gcno" -delete
