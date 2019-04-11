lib_LTLIBRARIES = libgatha.la
libgatha_la_SOURCES = game.c payoff_matrix.c sastry.c mcb.c convergence.c sfp.c
libgatha_la_LDFLAGS = -version-info 0:0:0 
libgatha_la_CFLAGS = -fopenmp -Wall 
libgatha_includedir=$(includedir)/gatha/
nobase_libgatha_include_HEADERS = gatha.h types.h sastry.h game.h mcb.h \
	convergence.h sfp.h
if CAIRO
libgatha_la_SOURCES += cairo_payoff_chart.c cairo_single_payoff_chart.c \
	cairo_pvect_timeline.c cairo_pvect_array.c cairo_save.c cairo_report.c \
	cairo_margin.c cairo_timeline.c
libgatha_la_LDFLAGS += $(CAIRO_LIBS) $(GLIB_LIBS)
libgatha_la_CFLAGS += $(CAIRO_CFLAGS) $(GLIB_CFLAGS)
nobase_libgatha_include_HEADERS += cairo_payoff_chart.h cairo_single_payoff_chart.h \
	cairo_pvect_timeline.h cairo_pvect_array.h cairo_save.h cairo_report.h \
	cairo_margin.h cairo_timeline.h
endif