AM_CFLAGS = -I$(top_srcdir)/include --include=config.h @CHECK_CFLAGS@

TESTS = shadow thread try volatile with
check_PROGRAMS = shadow thread try volatile with

thread_CFLAGS = -D_GNU_SOURCE -lpthread $(AM_CFLAGS)

LDADD = $(top_builddir)/src/libec.la @CHECK_LIBS@

