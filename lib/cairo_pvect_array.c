#include <cairo.h>
#include <math.h>

#include "gatha.h"
#include "cairo_pvect_array.h"

GathaPvectArray* gatha_pvect_array_start(GathaGame *g, int width, int height)
{
  GathaPvectArray *chart;
  int i, j, n, m;
  cairo_rectangle_t bounds;

  assert(g != NULL);
  assert(width > 0);
  assert(height > 0);

  chart = (GathaPvectArray*) malloc(sizeof(GathaPvectArray));
  chart->width = width;
  chart->height = height;
  chart->game = g;

  bounds.x = 0;
  bounds.y = 0;
  bounds.width = width;
  bounds.height = height;
  chart->surface = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA, &bounds);
  chart->canvas = cairo_create(chart->surface);
  cairo_set_source_rgb(chart->canvas, 1, 1, 1);
  cairo_paint(chart->canvas);

  chart->proba = gatha_game_pvect_new(g);
  n = g->n_players;
  m = g->n_strategies;
  for(i=0 ; i<n ; i++) {
    for(j=0 ; j<m ; j++) {
      chart->proba[i][j] = 0.0;
    }
  }

  chart->count = 0;

  chart->margins.xpadding = 0.0;
  chart->margins.ypadding = 0.0;
  chart->margins.xmargin = 0.0;
  chart->margins.ymargin = 0.0;

  return chart;
}

void gatha_pvect_array_add(GathaPvectArray *chart, proba_t** pvect)
{
  int i, j, n, m;
  payoff_t val;
  n = chart->game->n_players;
  m = chart->game->n_strategies;
  for(i=0 ; i<n ; i++) {
    for(j=0 ; j<m ; j++) {
      chart->proba[i][j] += pvect[i][j];
    }
  }
  chart->count++;
}

void gatha_pvect_array_end(GathaPvectArray *chart)
{
  int i, j, n, m, b;
  proba_t p;
  cairo_t *canvas;
  cairo_text_extents_t extents;
  double xscale, yscale, x, y, xpadding, ypadding, xmargin, ymargin;
  double legend_point_diameter;
  int label_frequency;
  char tmp[16];
 
  canvas = chart->canvas;
  n = chart->game->n_players;
  m = chart->game->n_strategies;

  /* compute the average value for the cells */
  if (chart->count > 1) {
    for(i=0 ; i<n ; i++) {
      for(j=0 ; j<m ; j++) {
	chart->proba[i][j] = chart->proba[i][j] / chart->count;
      }
    }
  }

  /* compute the scales */
  gatha_maxify(chart->margins.xpadding, 10.0);
  gatha_maxify(chart->margins.ypadding, 10.0);

  /* horizontal margin used for the y-axis ticks and labels */
  snprintf(tmp, 16, "%d", m);
  cairo_text_extents(canvas, tmp, &extents);
  gatha_maxify(chart->margins.xmargin, extents.width);

  /* vertical margin used for the y-axis ticks and labels */
  snprintf(tmp, 16, "%d", n);
  cairo_text_extents(canvas, tmp, &extents);
  gatha_maxify(chart->margins.ymargin, extents.width);

  xpadding = chart->margins.xpadding;
  ypadding = chart->margins.ypadding;
  xmargin  = chart->margins.xmargin;
  ymargin  = chart->margins.ymargin;

  b = (m > n) ? m : n;
  xscale = ((double)chart->width  -2*xpadding-xmargin)/b;
  yscale = ((double)chart->height -2*ypadding-ymargin)/b;

  label_frequency = 0;
  for(i=1 ; i<=10 ; i++) {
    if (yscale*i >= extents.height + 2.0) {
      label_frequency = i;
      break;
    }
  }

  /* show the data */
  /* for each player (row) */
  for(i=0 ; i<n ; i++) {
    y = (yscale*i) + ypadding + ymargin;



    /* for each strategy (column) */
    for(j=0 ; j<m ; j++) {
      x = xpadding + xmargin + xscale*j;
      p = 1.0-chart->proba[i][j];
      cairo_set_source_rgb(canvas, p, p, p);
      cairo_rectangle(canvas, x, y, xscale, yscale);
      cairo_fill(canvas);
    }
  }

  /* show the labels */
  cairo_set_source_rgb(canvas, 0, 0, 0);

  /* rows */
  for(i=0 ; i<n ; i++) {
    y = (yscale*i) + ypadding + ymargin;

    if (i%label_frequency == 0) {
      snprintf(tmp, 16, "%d", i);
      cairo_text_extents(canvas, tmp, &extents);
      x = (xpadding+xmargin-extents.width-5.0);

      cairo_move_to(canvas, x, y + 0.5*yscale + extents.height/2);
      cairo_show_text(canvas, tmp);
    }
  }

  /* columns */
  for(i=0 ; i<m ; i++) {
    y = ypadding + ymargin-3.0;

    if (i%label_frequency == 0) {
      snprintf(tmp, 16, "%d", i);
      cairo_text_extents(canvas, tmp, &extents);
      x = (xscale*i+xpadding+xmargin);

      cairo_move_to(canvas, x+0.5*xscale-extents.width/2, y);
      cairo_show_text(canvas, tmp);
    }
  }

  /* show the grid */
  p = 0.0;
  cairo_set_source_rgb(canvas, p, p, p);
  cairo_set_line_width(canvas, 0.1);
  for(i=0 ; i<=n ; i++) {
    y = (yscale*i) + ypadding + ymargin;

    x = xmargin + xpadding;
    cairo_move_to(canvas, x, y);
    cairo_line_to(canvas, x+m*xscale, y);
    cairo_stroke(canvas);

    for(j=0 ; j<=m ; j++) {
      x = xpadding + xmargin + xscale*j;
      y = ymargin + ypadding;
      cairo_move_to(canvas, x, y);
      cairo_line_to(canvas, x, y+n*yscale);
      cairo_stroke(canvas);
    }
  }
}

void gatha_pvect_array_free(GathaPvectArray *chart)
{
  cairo_destroy(chart->canvas);
  cairo_surface_destroy(chart->surface);

  gatha_game_pvect_free(chart->game, chart->proba);

  free(chart);
}
