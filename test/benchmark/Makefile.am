AM_CFLAGS = -I$(top_srcdir)/include --include=config.h

check_PROGRAMS = speed speed-try speed-try-throw speed-with speed-with-on-x speed-try-throw-with-on-x size

speed_try_SOURCES = speed.c
speed_try_CFLAGS = -DDO_TRY $(AM_CFLAGS)

speed_try_throw_SOURCES = speed.c
speed_try_throw_CFLAGS = -DDO_TRY -DDO_THROW $(AM_CFLAGS)

speed_with_SOURCES = speed.c
speed_with_CFLAGS = -DDO_WITH $(AM_CFLAGS)

speed_with_on_x_SOURCES = speed.c
speed_with_on_x_CFLAGS = -DDO_WITH_ON_X $(AM_CFLAGS)

speed_try_throw_with_on_x_SOURCES = speed.c
speed_try_throw_with_on_x_CFLAGS = -DDO_TRY -DDO_THROW -DDO_WITH_ON_X $(AM_CFLAGS)

size_CFLAGS = $(AM_CFLAGS) -O0

LDADD = $(top_builddir)/src/libec.la
