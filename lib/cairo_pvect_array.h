#ifndef _GATHA_CAIRO_PVECT_ARRAY_H_
#define _GATHA_CAIRO_PVECT_ARRAY_H_

#include <glib.h>
#include <cairo.h>

#include "types.h"

typedef struct {
  GathaGame *game;
  int width, height;
  int count;
  proba_t **proba;

  cairo_surface_t *surface;
  cairo_t *canvas;

  GathaMargins margins;
} GathaPvectArray;

GathaPvectArray* gatha_pvect_array_start(GathaGame *g, int width, int height);
void gatha_pvect_array_add(GathaPvectArray *chart, proba_t** pvect);
void gatha_pvect_array_end(GathaPvectArray *chart);
void gatha_pvect_array_free(GathaPvectArray *chart);

#endif /* _GATHA_CAIRO_PVECT_ARRAY_H_ */
