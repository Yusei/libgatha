#ifndef _GATHA_CAIRO_PAYOFF_CHART_H_
#define _GATHA_CAIRO_PAYOFF_CHART_H_

#include <glib.h>
#include <cairo.h>

#include "types.h"

typedef struct {
  GathaGame *game;
  int width, height;
  int interval;
  int count;
  payoff_t *sums;
  GArray **series;

  cairo_surface_t *surface;
  cairo_t *canvas;

  GathaMargins margins;
} GathaPayoffChart;

GathaPayoffChart* gatha_payoff_chart_start(GathaGame *g, int width, int height);
void gatha_payoff_chart_add(GathaPayoffChart *chart, payoff_t* payoffs);
void gatha_payoff_chart_end(GathaPayoffChart *chart);
void gatha_payoff_chart_free(GathaPayoffChart *chart);

#endif /* _GATHA_CAIRO_PAYOFF_CHART_H_ */
