#include <cairo.h>
#include <math.h>

#include "gatha.h"
#include "cairo_save.h"

inline GathaCairoSave* gatha_cairo_save_new(char* filename, int width, int height)
{
  GathaCairoSave* cs;
  cs = (GathaCairoSave*) malloc(sizeof(GathaCairoSave));
  cs->filename = filename;
  cs->width = width;
  cs->height = height;
  cs->format = CSF_UNKNOWN;
  return cs;
}

#ifdef CAIRO_HAS_PNG_FUNCTIONS
GathaCairoSave* gatha_cairo_save_new_png(char* filename, int width, int height)
{
  GathaCairoSave* cs = gatha_cairo_save_new(filename, width, height);
  cs->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  cs->canvas = cairo_create(cs->surface);
  cs->format = CSF_PNG;
  return cs;
}
#endif

#ifdef CAIRO_HAS_SVG_SURFACE
GathaCairoSave* gatha_cairo_save_new_svg(char* filename, int width, int height)
{
  GathaCairoSave* cs = gatha_cairo_save_new(filename, width, height);
  cs->surface = cairo_svg_surface_create(filename, width, height);
  cs->canvas = cairo_create(cs->surface);
  cs->format = CSF_SVG;
  return cs;
}
#endif

#ifdef CAIRO_HAS_PDF_SURFACE
GathaCairoSave* gatha_cairo_save_new_pdf(char* filename, int width, int height)
{
  GathaCairoSave* cs = gatha_cairo_save_new(filename, width, height);
  cs->surface = cairo_pdf_surface_create(filename, width, height);
  cs->canvas = cairo_create(cs->surface);
  cs->format = CSF_PDF;
  return cs;
}
#endif

void gatha_cairo_save_add_at(GathaCairoSave *cs, cairo_surface_t *surface,
			     double x, double y)
{
  cairo_set_source_surface(cs->canvas, surface, x, y);
  cairo_paint(cs->canvas);
  if (cs->format == CSF_PNG) {
    cairo_surface_write_to_png(cs->surface, cs->filename);
  }
}

void gatha_cairo_save_free(GathaCairoSave *cs)
{
  cairo_destroy(cs->canvas);
  cairo_surface_destroy(cs->surface);
  free(cs);
}
