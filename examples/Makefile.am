exampledir=.
example_PROGRAMS=mcb sastry sfp
AM_CFLAGS = ../lib/.libs/libgatha.la -I ../lib/
AM_LDFLAGS = -fopenmp ../lib/.libs/libgatha.la
mcb_SOURCES = mcb.c
sastry_SOURCES = sastry.c
sfp_SOURCES = sfp.c
if CAIRO
visualization_SOURCES = visualization.c
visualization_mcb_SOURCES = visualization_mcb.c
visualization_sfp_SOURCES = visualization_sfp.c
example_PROGRAMS += visualization visualization_mcb visualization_sfp
AM_LDFLAGS += $(CAIRO_LIBS) $(GLIB_LIBS)
AM_CFLAGS += $(CAIRO_CFLAGS) $(GLIB_CFLAGS)
endif