#include "gatha.h"
#include "cairo_margin.h"

GathaMargins* gatha_margins_new()
{
  GathaMargins *m;
  m = (GathaMargins*) malloc(sizeof(GathaMargins));
  m->xpadding = 0.0;
  m->ypadding = 0.0;
  m->xmargin = 0.0;
  m->ymargin = 0.0;
  return m;
}

void gatha_margins_copy(GathaMargins *dest, GathaMargins *orig)
{
  dest->xmargin = orig->xmargin;
  dest->ymargin = orig->ymargin;
  dest->xpadding = orig->xpadding;
  dest->ypadding = orig->ypadding;
}
