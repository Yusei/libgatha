#ifndef _GATHA_CAIRO_SAVE_H_
#define _GATHA_CAIRO_SAVE_H_

#include <glib.h>
#include <cairo.h>

#include "types.h"

typedef enum {
  CSF_UNKNOWN,
  CSF_PNG,
  CSF_SVG,
  CSF_PDF
} CairoSaveFormat;

typedef struct {
  int width, height;
  char* filename;
  CairoSaveFormat format;
  cairo_surface_t *surface;
  cairo_t *canvas;
} GathaCairoSave;

#ifdef CAIRO_HAS_PNG_FUNCTIONS
GathaCairoSave* gatha_cairo_save_new_png(char* filename, int width, int height);
#endif

#ifdef CAIRO_HAS_SVG_SURFACE
GathaCairoSave* gatha_cairo_save_new_svg(char* filename, int width, int height);
#endif

#ifdef CAIRO_HAS_PDF_SURFACE
GathaCairoSave* gatha_cairo_save_new_pdf(char* filename, int width, int height);
#endif

void gatha_cairo_save_add_at(GathaCairoSave *cs, cairo_surface_t *surface,
			     double x, double y);
static inline void gatha_cairo_save_add(GathaCairoSave *cs, cairo_surface_t *surface)
{
  gatha_cairo_save_add_at(cs, surface, 0.0, 0.0);
}

void gatha_cairo_save_free(GathaCairoSave *cs);

#endif /* _GATHA_CAIRO_SAVE_H_ */
