#include <cairo.h>
#include <math.h>

#include "gatha.h"
#include "cairo_pvect_timeline.h"

GathaPvectTimeline* gatha_pvect_timeline_start(GathaGame *g, int width, int height)
{
  GathaPvectTimeline*chart;
  int i, n;
  cairo_rectangle_t bounds;

  assert(g != NULL);
  assert(width > 0);
  assert(height > 0);

  chart = (GathaPvectTimeline*) malloc(sizeof(GathaPvectTimeline));
  chart->width = width;
  chart->height = height;
  chart->game = g;

  bounds.x = 0;
  bounds.y = 0;
  bounds.width = width;
  bounds.height = height;
  chart->surface = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA, &bounds);
  //chart->surface = cairo_svg_surface_create("", width, height);
  chart->canvas = cairo_create(chart->surface);
  cairo_set_source_rgb(chart->canvas, 1, 1, 1);
  cairo_paint(chart->canvas);

  n = chart->game->n_strategies;
  chart->series = (GArray**) malloc(n * sizeof(GArray*));
  for(i=0 ; i<n ; i++) {
    chart->series[i] = g_array_sized_new(FALSE, FALSE, sizeof(proba_t), 100);
  }

  chart->sums = (proba_t*) calloc(n, sizeof(proba_t));
  chart->count = 0;
  chart->interval = 1;

  chart->title = NULL;

  chart->margins.xpadding = 0.0;
  chart->margins.ypadding = 0.0;
  chart->margins.xmargin = 0.0;
  chart->margins.ymargin = 0.0;

  return chart;
}

void gatha_pvect_timeline_add(GathaPvectTimeline *chart, proba_t* pvect)
{
  int i, n;
  proba_t val;
  n = chart->game->n_strategies;
  for(i=0 ; i<n ; i++) {
    chart->sums[i] += pvect[i];
  }
  chart->count++;
  if (chart->count == chart->interval) {
    chart->count = 0;
    for(i=0 ; i<n ; i++) {
      val = chart->sums[i]/chart->interval;
      g_array_append_val(chart->series[i], val);
      chart->sums[i] = 0;
    }
  }
}

void gatha_pvect_timeline_end(GathaPvectTimeline *chart)
{
  int i, j, n, m;
  proba_t p;
  cairo_t *canvas;
  cairo_text_extents_t extents;
  double xscale, yscale, x, y, xpadding, ypadding, xmargin, ymargin;
  double legend_point_diameter;
  int label_frequency;
  char tmp[16];
  boolean compute_margins;

  canvas = chart->canvas;
  n = chart->game->n_strategies;
  m = chart->series[0]->len;

  /* if we have an unfinished interval, finish it */
  if (chart->count > 0) {
    for(i=0 ; i<n ; i++) {
      p = chart->sums[i]/chart->count;
      g_array_append_val(chart->series[i], p);
    }
  }
 
  /* compute the scales */
  gatha_maxify(chart->margins.xpadding, 10.0);
  gatha_maxify(chart->margins.ypadding, 5.0);

  /* vertical margin used for the title */
  if (chart->title != NULL) {
    cairo_text_extents(canvas, chart->title, &extents);
    gatha_maxify(chart->margins.ymargin, extents.height);
  }
  /* horizontal margin used for the y-axis ticks and labels */
  snprintf(tmp, 16, "%d", n);
  cairo_text_extents(canvas, tmp, &extents);
  gatha_maxify(chart->margins.xmargin, extents.width + 9.0);

  xpadding = chart->margins.xpadding;
  ypadding = chart->margins.ypadding;
  ymargin  = chart->margins.ymargin;

  yscale = ((double)chart->height-2*ypadding-ymargin)/n;

  legend_point_diameter = yscale/15.0;
  if (legend_point_diameter > 3.0) legend_point_diameter = 3.0;

  snprintf(tmp, 16, "%d", n-1);
  cairo_text_extents(canvas, tmp, &extents);
  gatha_maxify(chart->margins.xmargin, extents.width + 9.0);
  xmargin = chart->margins.xmargin;

  xscale = ((double)chart->width-2*xpadding-2*xmargin)/(m);
  
  label_frequency = 0;
  for(i=1 ; i<=10 ; i++) {
    if (yscale*i >= extents.height + 2.0) {
      label_frequency = i;
      break;
    }
  }

  /* title */
  if (chart->title != NULL) {
    x = xpadding + xmargin - 3.0;
    y = ypadding;
    cairo_set_source_rgb(canvas, 0, 0, 0);
    cairo_text_extents(canvas, chart->title, &extents);
    cairo_move_to(canvas, x, y+extents.height/2);
    cairo_show_text(canvas, chart->title);
  }

  /* y-axis */
  cairo_set_source_rgb(canvas, 0, 0, 0);
  x = xpadding + xmargin-3.0;
  y = ypadding + ymargin;
  cairo_move_to(canvas, x, y);
  cairo_line_to(canvas, x, yscale*n+ypadding+ymargin);
  cairo_stroke(canvas);

  /* for each series */
  for(j=0 ; j<n ; j++) {
    x = xpadding + xmargin-0.5;
    y = (yscale*j) + ypadding + ymargin;

    if (j%label_frequency == 0) {
      cairo_set_source_rgb(canvas, 0, 0, 0);
      cairo_arc(canvas, x-2.5, y+0.5*yscale, 2.0, 0, 2 * M_PI);
      cairo_fill(canvas);

      snprintf(tmp, 16, "%d", j);
      cairo_text_extents(canvas, tmp, &extents);
      x = (xmargin-extents.width);

      cairo_move_to(canvas, x, y + 0.5*yscale + extents.height/2);
      cairo_show_text(canvas, tmp);
    }

    x = xpadding + xmargin-0.5;
    cairo_move_to(canvas, x, y);

    /* for each point */
    for(i=0 ; i<m ; i++) {
      p = 1.0-g_array_index(chart->series[j], proba_t, i);
      x = xscale*i + xpadding + xmargin;
      cairo_set_source_rgb(canvas, p, p, p);
      cairo_rectangle(canvas, x, y, xscale+1.0, yscale);
      cairo_fill(canvas);
    }

    /* if one point has a majority, display its number for easy reading */
    if (p < 0.5) {
      cairo_set_source_rgb(canvas, 0, 0, 0);
      x += xscale+1.0 + 6.0;
      cairo_arc(canvas, x-2.5, y+0.5*yscale, 2.0, 0, 2 * M_PI);
      cairo_fill(canvas);

      snprintf(tmp, 16, "%d", j);
      cairo_text_extents(canvas, tmp, &extents);
      x += 3.0;
      cairo_move_to(canvas, x, y + 0.5*yscale + extents.height/2);
      cairo_show_text(canvas, tmp);
    }
  }
}

void gatha_pvect_timeline_free(GathaPvectTimeline *chart)
{
  int i;
  cairo_destroy(chart->canvas);
  cairo_surface_destroy(chart->surface);

  for(i=0 ; i<chart->game->n_players ; i++) {
    g_array_free(chart->series[i], TRUE);
  }
  free(chart->series);
  free(chart->sums);
  free(chart);
}
