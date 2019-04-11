#ifndef _GATHA_CAIRO_MARGIN_H_
#define _GATHA_CAIRO_MARGIN_H_

typedef struct {
  double xpadding, ypadding;
  double xmargin, ymargin;
} GathaMargins;

extern GathaMargins* gatha_margins_new();

#define gatha_margins_free(m) (free(m))

extern void gatha_margins_copy(GathaMargins *dest, GathaMargins *orig);

#endif
