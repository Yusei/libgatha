#ifndef _GATHA_CAIRO_SINGLE_PAYOFF_CHART_H_
#define _GATHA_CAIRO_SINGLE_PAYOFF_CHART_H_

#include <glib.h>
#include <cairo.h>

#include "types.h"

typedef struct {
  GathaGame *game;
  int width, height;
  int interval;
  int count;
  payoff_t sum, min, max;
  boolean reset;
  GArray *series[3];

  boolean display_convergence_interval;
  payoff_t convergence_min, convergence_max;

  cairo_surface_t *surface;
  cairo_t *canvas;
} GathaSinglePayoffChart;

GathaSinglePayoffChart* gatha_single_payoff_chart_start(GathaGame *g,
							int width, int height);
void gatha_single_payoff_chart_add(GathaSinglePayoffChart *chart, payoff_t payoff);
void gatha_single_payoff_chart_end(GathaSinglePayoffChart *chart);
void gatha_single_payoff_chart_free(GathaSinglePayoffChart *chart);

#endif /* _GATHA_CAIRO_SINGLE_PAYOFF_CHART_H_ */
