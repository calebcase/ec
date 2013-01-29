AM_CFLAGS = -I$(top_srcdir)/include --include=config.h @CHECK_CFLAGS@

TESTS = shadow thread try volatile with
check_PROGRAMS = shadow thread try volatile with

thread_CFLAGS = -lpthread $(AM_CFLAGS)

volatile_CFLAGS = $(AM_CFLAGS) -O2

LDADD = $(top_builddir)/src/libec.la -lpthread @CHECK_LIBS@
