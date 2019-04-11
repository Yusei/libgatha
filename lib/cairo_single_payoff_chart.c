#include <cairo.h>
#include <math.h>

#include "gatha.h"
#include "cairo_single_payoff_chart.h"

GathaSinglePayoffChart* gatha_single_payoff_chart_start(GathaGame *g, 
							int width, int height)
{
  GathaSinglePayoffChart *chart;
  cairo_rectangle_t bounds;

  assert(g != NULL);
  assert(width > 0);
  assert(height > 0);

  chart = (GathaSinglePayoffChart*) malloc(sizeof(GathaSinglePayoffChart));
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

  chart->series[0] = g_array_sized_new(FALSE, FALSE, sizeof(payoff_t), 100);
  chart->series[1] = g_array_sized_new(FALSE, FALSE, sizeof(payoff_t), 100);
  chart->series[2] = g_array_sized_new(FALSE, FALSE, sizeof(payoff_t), 100);

  chart->display_convergence_interval = FALSE;
  chart->convergence_min = 0;
  chart->convergence_max = 0;

  chart->reset = TRUE;
  chart->sum = 0;
  chart->min = 0;
  chart->max = 0;

  chart->count = 0;
  chart->interval = 1;

  return chart;
}

void gatha_single_payoff_chart_add(GathaSinglePayoffChart *chart, payoff_t payoff)
{
  payoff_t val;

  if (chart->reset) {
    chart->reset = FALSE;
    chart->min = payoff;
    chart->max = payoff;
    chart->sum = payoff;
  } else {
    chart->sum += payoff;
    if (payoff < chart->min) chart->min = payoff;
    if (payoff > chart->max) chart->max = payoff;
  }

  chart->count++;
  if (chart->count == chart->interval) {
    chart->count = 0;
    chart->reset = TRUE;
    val = chart->sum/chart->interval;
    g_array_append_val(chart->series[0], chart->min);
    g_array_append_val(chart->series[1], val);
    g_array_append_val(chart->series[2], chart->max);
  }
}

static void y_legend(GathaSinglePayoffChart *chart, payoff_t value, payoff_t min, double xpadding,
	      double ypadding, double xmargin, double xscale, double yscale)

{
  char tmp[16];
  double x, y;
  cairo_text_extents_t extents;
  cairo_t *canvas;

  canvas = chart->canvas;
  snprintf(tmp, 16, "%.3f", value);
  cairo_text_extents(canvas, tmp, &extents);
  x = (xmargin-extents.width);
  y = chart->height - (yscale*(value-min)) - ypadding;
  cairo_move_to(canvas, x, y + extents.height/2);
  cairo_show_text(canvas, tmp);
  x = xpadding + xmargin;
  cairo_arc(canvas, x, y, 2.0, 0, 2 * M_PI);
  cairo_fill(canvas);
}

static void y_right_legend(GathaSinglePayoffChart *chart, payoff_t value, payoff_t min, double xpadding,
	      double ypadding, double xmargin, double xscale, double yscale)

{
  char tmp[16];
  double x, y;
  cairo_text_extents_t extents;
  cairo_t *canvas;

  canvas = chart->canvas;
  snprintf(tmp, 16, "%.3f", value);
  cairo_text_extents(canvas, tmp, &extents);
  x = xpadding + xmargin + xscale*(chart->series[0]->len-1) + xpadding;
  y = chart->height - (yscale*(value-min)) - ypadding;
  cairo_move_to(canvas, x, y + extents.height/2);
  cairo_show_text(canvas, tmp);
  x = xpadding + xmargin + xscale*(chart->series[0]->len-1);
  cairo_arc(canvas, x, y, 2.0, 0, 2 * M_PI);
  cairo_fill(canvas);
}

void gatha_single_payoff_chart_end(GathaSinglePayoffChart *chart)
{
  payoff_t min, max, p;
  int i, j, n, m;
  double xscale, yscale, x, y, xpadding, ypadding;
  cairo_t *canvas;
  cairo_text_extents_t extents;
  char tmp[16];
  double xmargin;

  /* if we have an unfinished interval, finish it */
  if (chart->count > 0) {
    p = chart->sum/chart->count;
    g_array_append_val(chart->series[0], chart->min);
    g_array_append_val(chart->series[1], p);
    g_array_append_val(chart->series[2], chart->max);
  }

  canvas = chart->canvas;
  n = 3;
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

  /* find the space needed for the axis numbers */
  snprintf(tmp, 16, "%f", max);
  cairo_text_extents(canvas, tmp, &extents);
  xmargin = extents.width;
  snprintf(tmp, 16, "%f", min);
  cairo_text_extents(canvas, tmp, &extents);
  if (xmargin < extents.width) xmargin = extents.width;

  /* compute the scales */
  xpadding = 10;
  ypadding = 10;
  xscale = ((double)chart->width-2*xpadding-2*xmargin)/(m-1);
  yscale = ((double)chart->height-2*ypadding)/(max-min);

  /* min-max gray background */
  cairo_set_source_rgb(canvas, 0.8, 0.8, 0.8);
  x = xpadding + xmargin;
  y = chart->height - (yscale*(g_array_index(chart->series[0], payoff_t, 0)-min) + ypadding);
  cairo_move_to(canvas, x, y);
  for(i=1 ; i<m ; i++) {
    x = xscale*i + xpadding + xmargin;
    y = chart->height - (yscale*(g_array_index(chart->series[0], payoff_t, i)-min) + ypadding);
    cairo_line_to(canvas, x, y);
  }
  for(i=m-1 ; i>=0 ; i--) {
    x = xscale*i + xpadding + xmargin;
    y = chart->height - (yscale*(g_array_index(chart->series[2], payoff_t, i)-min) + ypadding);
    cairo_line_to(canvas, x, y);
  }
  x = xpadding + xmargin;
  y = chart->height - (yscale*(g_array_index(chart->series[0], payoff_t, 0)-min) + ypadding);
  cairo_line_to(canvas, x, y);
  cairo_fill(canvas);

  /* mean line */
  cairo_set_source_rgb(canvas, 0, 0, 0);
  x = xpadding + xmargin;
  y = chart->height - (yscale*(g_array_index(chart->series[1], payoff_t, 0)-min) + ypadding);
  cairo_move_to(canvas, x, y);

  for(i=1 ; i<m ; i++) {
    x = xscale*i + xpadding + xmargin;
    y = chart->height - (yscale*(g_array_index(chart->series[1], payoff_t, i)-min) + ypadding);
    cairo_line_to(canvas, x, y);
  }
  cairo_stroke(canvas);

  /* display the interesting y-axis values */
  /* - on the left: max, min, 0 and first values of each series */
  y_legend(chart, max, min, xpadding, ypadding, xmargin, xscale, yscale);
  y_legend(chart, min, min, xpadding, ypadding, xmargin, xscale, yscale);
  if (min < 0.0 && max > 0.0) {
    y_legend(chart, 0, min, xpadding, ypadding, xmargin, xscale, yscale);
  }
  for(j=0 ; j<n ; j++) {
    // TODO: ensure labels don't overlap
    p = g_array_index(chart->series[j], payoff_t, 0);
    if (p != min && p != max && p != 0.0) {
      y_legend(chart, p, min, xpadding, ypadding, xmargin, xscale, yscale);
    }
  }
  /* - on the right: last values of each series */
  for(j=0 ; j<n ; j++) {
    // TODO: ensure labels don't overlap
    p = g_array_index(chart->series[j], payoff_t, chart->series[j]->len-1);
    y_right_legend(chart, p, min, xpadding, ypadding, xmargin, xscale, yscale);
  }
}

void gatha_single_payoff_chart_free(GathaSinglePayoffChart *chart)
{
  int i;
  cairo_destroy(chart->canvas);
  cairo_surface_destroy(chart->surface);

  for(i=0 ; i<chart->game->n_players ; i++) {
    g_array_free(chart->series[i], TRUE);
  }
  free(chart);
}
