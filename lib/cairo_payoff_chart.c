#include <cairo.h>
#include <math.h>

#include "gatha.h"
#include "cairo_payoff_chart.h"

GathaPayoffChart* gatha_payoff_chart_start(GathaGame *g, int width, int height)
{
  GathaPayoffChart *chart;
  int i, n;
  cairo_rectangle_t bounds;

  assert(g != NULL);
  assert(width > 0);
  assert(height > 0);

  chart = (GathaPayoffChart*) malloc(sizeof(GathaPayoffChart));
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

  n = chart->game->n_players;
  chart->series = (GArray**) malloc(n * sizeof(GArray*));
  for(i=0 ; i<n ; i++) {
    chart->series[i] = g_array_sized_new(FALSE, FALSE, sizeof(payoff_t), 100);
  }

  chart->sums = (payoff_t*) calloc(n, sizeof(payoff_t));
  chart->count = 0;
  chart->interval = 1;

  chart->margins.xpadding = 0.0;
  chart->margins.ypadding = 0.0;
  chart->margins.xmargin = 0.0;
  chart->margins.ymargin = 0.0;

  return chart;
}

void gatha_payoff_chart_add(GathaPayoffChart *chart, payoff_t* payoffs)
{
  int i, n;
  payoff_t val;
  n = chart->game->n_players;
  for(i=0 ; i<n ; i++) {
    chart->sums[i] += payoffs[i];
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

static void y_legend(GathaPayoffChart *chart, payoff_t value, payoff_t min,
		     double xscale, double yscale)

{
  char tmp[16];
  double x, y;
  cairo_text_extents_t extents;
  cairo_t *canvas;

  canvas = chart->canvas;
  snprintf(tmp, 16, "%.3f", value);
  cairo_text_extents(canvas, tmp, &extents);
  x = (chart->margins.xmargin-extents.width);
  y = chart->height - (yscale*(value-min)) - chart->margins.ypadding;
  cairo_move_to(canvas, x, y + extents.height/2);
  cairo_show_text(canvas, tmp);
  x = chart->margins.xpadding + chart->margins.xmargin;
  cairo_arc(canvas, x, y, 2.0, 0, 2 * M_PI);
  cairo_fill(canvas);
}

static void y_right_legend(GathaPayoffChart *chart, payoff_t value, payoff_t min,
			   double xscale, double yscale)

{
  char tmp[16];
  double x, y;
  cairo_text_extents_t extents;
  cairo_t *canvas;

  canvas = chart->canvas;
  snprintf(tmp, 16, "%.3f", value);
  cairo_text_extents(canvas, tmp, &extents);

  x = chart->margins.xpadding + chart->margins.xmargin +
    xscale*(chart->series[0]->len-1) + 3.0;
  y = chart->height - (yscale*(value-min)) - chart->margins.ypadding;
  cairo_move_to(canvas, x, y + extents.height/2);
  cairo_show_text(canvas, tmp);

  x = chart->margins.xpadding + chart->margins.xmargin + xscale*(chart->series[0]->len-1);
  cairo_arc(canvas, x, y, 2.0, 0, 2 * M_PI);
  cairo_fill(canvas);
}

void gatha_payoff_chart_end(GathaPayoffChart *chart)
{
  payoff_t min, max, p;
  int i, j, n, m;
  double xscale, yscale, x, y, xpadding, ypadding;
  double xmargin, ymargin;
  cairo_t *canvas;
  cairo_text_extents_t extents;
  char tmp[16];

  canvas = chart->canvas;
  n = chart->game->n_players;

  /* if we have an unfinished interval, finish it */
  if (chart->count > 0) {
    for(i=0 ; i<n ; i++) {
      p = chart->sums[i]/chart->count;
      g_array_append_val(chart->series[i], p);
    }
  }
  m = chart->series[0]->len;

  /* find the min and max payoffs to be displayed */
  min = g_array_index(chart->series[0], payoff_t, 0);
  max = min;
  for(i=0 ; i<m ; i++) {
    for(j=0 ; j<n ; j++) {
      p = g_array_index(chart->series[j], payoff_t, i);
      if (p < min) min = p;
      if (p > max) max = p;
    }
  }

  gatha_maxify(chart->margins.xpadding, 10.0);
  gatha_maxify(chart->margins.ypadding, 10.0);

  /* find the space needed for the axis numbers */
  snprintf(tmp, 16, "%.3f", max);
  cairo_text_extents(canvas, tmp, &extents);
  gatha_maxify(chart->margins.xmargin, extents.width);

  snprintf(tmp, 16, "%.3f", min);
  cairo_text_extents(canvas, tmp, &extents);
  gatha_maxify(chart->margins.xmargin, extents.width);

  xpadding = chart->margins.xpadding;
  ypadding = chart->margins.ypadding;
  xmargin  = chart->margins.xmargin;
  ymargin  = chart->margins.ymargin;

  /* compute the scales */
  xscale = ((double)chart->width-2*xpadding-2*xmargin)/(m-1);
  yscale = ((double)chart->height-2*ypadding)/(max-min);

  /* for each series */ 
  for(j=0 ; j<n ; j++) {
    cairo_set_source_rgb(canvas, 0, 0, 0);
    x = xpadding + xmargin;
    y = chart->height - (yscale*(g_array_index(chart->series[j], payoff_t, 0)-min)
			 + ypadding);
    cairo_move_to(canvas, x, y);
    /* for each point */
    for(i=1 ; i<m ; i++) {
      x = xscale*i + xpadding + xmargin;
      y = chart->height - (yscale*(g_array_index(chart->series[j], payoff_t, i)-min) + ypadding);
      cairo_line_to(canvas, x, y);
    }
    cairo_stroke(canvas);
  }

  /* display the interesting y-axis values */
  /* - on the left: max, min, 0 and first values of each series */
  y_legend(chart, max, min, xscale, yscale);
  y_legend(chart, min, min, xscale, yscale);
  if (min < 0.0 && max > 0.0) {
    y_legend(chart, 0, min, xscale, yscale);
  }
  for(j=0 ; j<n ; j++) {
    // TODO: ensure labels don't overlap
    p = g_array_index(chart->series[j], payoff_t, 0);
    if (p != min && p != max && p != 0.0) {
      y_legend(chart, p, min, xscale, yscale);
    }
  }
  /* - on the right: last values of each series */
  for(j=0 ; j<n ; j++) {
    // TODO: ensure labels don't overlap
    p = g_array_index(chart->series[j], payoff_t, chart->series[j]->len-1);
    y_right_legend(chart, p, min, xscale, yscale);
  }
}

void gatha_payoff_chart_free(GathaPayoffChart *chart)
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
