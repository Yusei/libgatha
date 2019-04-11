#ifndef _GATHA_CAIRO_TIMELINE_H_
#define _GATHA_CAIRO_TIMELINE_H_

#include <glib.h>
#include <cairo.h>

#include "types.h"

typedef enum {
  GATHA_TIMELINE_TICKS_NONE,
  GATHA_TIMELINE_TICKS_AUTO,
  GATHA_TIMELINE_TICKS_USER,
} GathaTimelineTicks;

typedef struct {
  int width, height;
  int size;
  int ticks_interval, ticks_type;

  cairo_surface_t *surface;
  cairo_t *canvas;

  GathaMargins margins;
} GathaTimeline;

GathaTimeline* gatha_timeline_start(int width, int height);
void gatha_timeline_set_size(GathaTimeline *chart, int time);
void gatha_timeline_end(GathaTimeline *chart);
void gatha_timeline_free(GathaTimeline *chart);

#endif /* _GATHA_CAIRO_TIMELINE_H_ */
