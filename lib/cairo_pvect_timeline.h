#ifndef _GATHA_CAIRO_PVECT_TIMELINE_H_
#define _GATHA_CAIRO_PVECT_TIMELINE_H_

#include <glib.h>
#include <cairo.h>

#include "types.h"

typedef struct {
  GathaGame *game;
  int width, height;
  int interval;
  int count;
  proba_t *sums;
  GArray **series;

  cairo_surface_t *surface;
  cairo_t *canvas;

  char *title;

  GathaMargins margins;
} GathaPvectTimeline;

GathaPvectTimeline* gatha_pvect_timeline_start(GathaGame *g, int width, int height);
void gatha_pvect_timeline_add(GathaPvectTimeline *chart, proba_t* pvect);
void gatha_pvect_timeline_end(GathaPvectTimeline *chart);
void gatha_pvect_timeline_free(GathaPvectTimeline *chart);

#endif /* _GATHA_CAIRO_PVECT_TIMELINE_H_ */
