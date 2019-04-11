#include <cairo.h>
#include <math.h>

#include "gatha.h"
#include "cairo_timeline.h"

GathaTimeline* gatha_timeline_start(int width, int height)
{
  GathaTimeline *chart;
  cairo_rectangle_t bounds;

  assert(width > 0);
  assert(height > 0);

  chart = (GathaTimeline*) malloc(sizeof(GathaTimeline));
  chart->width = width;
  chart->height = height;

  bounds.x = 0;
  bounds.y = 0;
  bounds.width = width;
  bounds.height = height;
  chart->surface = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA, &bounds);
  chart->canvas = cairo_create(chart->surface);

  chart->size = 0;
  chart->ticks_interval = 0;
  chart->ticks_type = GATHA_TIMELINE_TICKS_AUTO;

  chart->margins.xpadding = 0.0;
  chart->margins.ypadding = 0.0;
  chart->margins.xmargin = 0.0;
  chart->margins.ymargin = 0.0;

  return chart;
}

void gatha_timeline_set_size(GathaTimeline *chart, int time)
{
  chart->size = time;
}

void gatha_timeline_end(GathaTimeline *chart)
{
  int i, n;
  double xscale, yscale, x, y, xpadding, ypadding;
  double xmargin, ymargin;
  int d;
  cairo_t *canvas;
  cairo_text_extents_t extents;
  char tmp[16];

  n = chart->size;
  canvas = chart->canvas;

  gatha_maxify(chart->margins.xpadding, 10.0);
  gatha_maxify(chart->margins.ypadding, 10.0);

  /* find the space needed for the axis numbers */
  if (chart->ticks_type != GATHA_TIMELINE_TICKS_NONE) {
    snprintf(tmp, 16, "%d", 0);
    cairo_text_extents(canvas, tmp, &extents);
    gatha_maxify(chart->margins.ymargin, extents.height);
    gatha_maxify(chart->margins.xmargin, extents.width/2.0 + 3.0);

    snprintf(tmp, 16, "%d", n);
    cairo_text_extents(canvas, tmp, &extents);
    gatha_maxify(chart->margins.ymargin, extents.height);
    gatha_maxify(chart->margins.xmargin, extents.width/2.0 + 3.0);
  }

  xpadding = chart->margins.xpadding;
  ypadding = chart->margins.ypadding;
  xmargin  = chart->margins.xmargin;
  ymargin  = chart->margins.ymargin;

  /* if needed, compute the ticks interval */
  if (chart->ticks_type == GATHA_TIMELINE_TICKS_AUTO) {
    d = log10(n);
    chart->ticks_interval = pow(10,d);
  }

  /* compute the scales */
  xscale = ((double)chart->width-2*xpadding-2*xmargin)/chart->size;

  /* draw */
  cairo_set_source_rgb(canvas, 0, 0, 0);
  cairo_move_to(canvas, xpadding+xmargin, ypadding+5.0);
  cairo_line_to(canvas, chart->width-xpadding-xmargin, ypadding+5.0);
  cairo_stroke(canvas);

  if (chart->ticks_interval > 1) {
    for(i=0 ; i<n ; i += chart->ticks_interval) {
      x = xscale*i + xpadding + xmargin;
      y = ypadding;
      cairo_move_to(canvas, x, y);
      cairo_line_to(canvas, x, y+10.0);
      cairo_stroke(canvas);
      snprintf(tmp, 16, "%d", i);
      cairo_text_extents(canvas, tmp, &extents);
      x -= extents.width / 2.0;
      cairo_move_to(canvas, x, y+13.0+extents.height);
      cairo_show_text(canvas, tmp);
    }
    x = xscale*n + xpadding + xmargin;
    y = ypadding;
    cairo_move_to(canvas, x, y);
    cairo_line_to(canvas, x, y+10.0);
    cairo_stroke(canvas);
    snprintf(tmp, 16, "%d", i);
    cairo_text_extents(canvas, tmp, &extents);
    x -= extents.width / 2.0;
    cairo_move_to(canvas, x, y+13.0+extents.height);
    cairo_show_text(canvas, tmp);
  }
}

void gatha_timeline_free(GathaTimeline *chart)
{
  cairo_destroy(chart->canvas);
  cairo_surface_destroy(chart->surface);
  free(chart);
}
