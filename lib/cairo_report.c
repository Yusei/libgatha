#include <cairo.h>
#include <math.h>

#include "gatha.h"
#include "cairo_report.h"

GathaReport* gatha_report_start(char *filename, GathaGame *g, double width, double height,
				boolean show_arrays)
{
  GathaReport *r;
  GathaPvectTimeline *tl;
  int i;
  double h, sum;

  assert(g != NULL);
  assert(width > 0);
  assert(height > 0);

  r = (GathaReport*) malloc(sizeof(GathaReport));
  r->width = width;
  r->height = height;
  r->game = g;

  r->cs = gatha_cairo_save_new_svg(filename, width, height);
  cairo_set_source_rgb(r->cs->canvas, 1, 1, 1);
  cairo_paint(r->cs->canvas);

  if (!show_arrays) {
    r->arrays_size = 0.0;
  } else if (g->n_players < 3) {
    r->arrays_size = 1.0;
  } else if (g->n_players < 6) {
    r->arrays_size = 2.0;
  }  else if (g->n_players < 12) {
    r->arrays_size = 3.0;
  } else {
    r->arrays_size = 4.0;
  }
  r->pvect_timeline_size = 1.0;
  r->payoffs_size = 3.0;

  r->timeline_height = 50;
  sum = r->arrays_size + r->pvect_timeline_size*g->n_players + r->payoffs_size;
  h = (height-r->timeline_height)/sum;
  r->y_unit = h;

  r->timeline_title_format = NULL;
  r->timelines = (GathaPvectTimeline**) malloc(sizeof(GathaPvectTimeline*)*g->n_players);
  for(i=0 ; i<g->n_players ; i++) {
    r->timelines[i] = gatha_pvect_timeline_start(g, width, r->pvect_timeline_size*h);
    r->timelines[i]->interval = 10;
  }

  r->payoffs = gatha_payoff_chart_start(g, width, r->payoffs_size*h);
  r->payoffs->interval = 10;

  if (show_arrays) {
    sum = r->arrays_size*h;
    r->first_array = gatha_pvect_array_start(g, sum, sum);
    r->last_array = gatha_pvect_array_start(g, sum, sum);
  } else {
    r->first_array = NULL;
    r->last_array = NULL;
  }

  r->timeline = gatha_timeline_start(width, r->timeline_height);
  r->timeline->ticks_type = GATHA_TIMELINE_TICKS_AUTO;

  return r;
}

void gatha_report_add_payoffs(GathaReport *r, payoff_t *payoffs)
{
  gatha_payoff_chart_add(r->payoffs, payoffs);
}

void gatha_report_add_pvect(GathaReport *r, proba_t** pvect)
{
  int i,n;
  n = r->game->n_players;
  for(i=0 ; i<n ; i++) {
    gatha_pvect_timeline_add(r->timelines[i], pvect[i]);
  }
}

void gatha_report_end(GathaReport *r)
{
  int i, j, m, n, len;
  double x, y;
  proba_t **pvect;
  char tmp[256];

  n = r->game->n_players;
  m = r->game->n_strategies;

  /* payoff chart (start with this, as its xmargins are bigger than the
     other ones and must be computed first) */
  x = 0.0;
  y = (r->arrays_size + r->pvect_timeline_size*r->game->n_players)*r->y_unit;
  gatha_payoff_chart_end(r->payoffs);
  gatha_cairo_save_add_at(r->cs, r->payoffs->surface, x, y);

  /* timeline */
  x = 0.0;
  y = (r->arrays_size + r->payoffs_size +
       r->pvect_timeline_size*r->game->n_players)*r->y_unit;
  if (r->timeline->size <= 0) {
    i = r->payoffs->series[0]->len*r->payoffs->interval;
    gatha_timeline_set_size(r->timeline, i);
  }
  gatha_margins_copy(&(r->timeline->margins), &(r->payoffs->margins));
  gatha_timeline_end(r->timeline);
  gatha_cairo_save_add_at(r->cs, r->timeline->surface, x, y);

  /* pvect arrays on top*/
  if (r->first_array != NULL && r->last_array != NULL) {
    x = 0.0;
    y = 0.0;
    pvect = gatha_game_pvect_new(r->game);

    for(i=0 ; i<n ; i++) {
      for(j=0 ; j<m ; j++) {
	pvect[i][j] = g_array_index(r->timelines[i]->series[j], proba_t, 0);
      }
    }
    gatha_pvect_array_add(r->first_array, pvect);
    //gatha_margins_copy(&(r->first_array->margins), &(r->payoffs->margins));
    gatha_pvect_array_end(r->first_array);
    x = r->payoffs->margins.xpadding + r->payoffs->margins.xmargin - r->first_array->margins.xmargin;
    gatha_cairo_save_add_at(r->cs, r->first_array->surface, x, y);

    len = r->timelines[0]->series[0]->len;
    for(i=0 ; i<n ; i++) {
      for(j=0 ; j<m ; j++) {
	gatha_margins_copy(&(r->timelines[i]->margins), &(r->payoffs->margins));
	pvect[i][j] = g_array_index(r->timelines[i]->series[j], proba_t, len-1);
      }
    }
    gatha_pvect_array_add(r->last_array, pvect);
    gatha_pvect_array_end(r->last_array);
    x = r->width - r->last_array->width - r->payoffs->margins.xmargin;
    gatha_cairo_save_add_at(r->cs, r->last_array->surface, x, y);

    gatha_game_pvect_free(r->game, pvect);
  }

  /* pvect timelines */
  x = 0.0;
  y = r->arrays_size*r->y_unit;
  for(i=0 ; i<n ; i++) {
    if (r->timeline_title_format != NULL) {
      snprintf(tmp, 256, r->timeline_title_format, i);
      r->timelines[i]->title = tmp;
    }
    gatha_pvect_timeline_end(r->timelines[i]);
    gatha_cairo_save_add_at(r->cs, r->timelines[i]->surface, x, y);
    y += r->pvect_timeline_size*r->y_unit;
  }


}

void gatha_report_free(GathaReport *r)
{
  int i,n;
  n = r->game->n_players;
  for(i=0 ; i<n ; i++) {
    gatha_pvect_timeline_free(r->timelines[i]);
  }
  free(r->timelines);

  gatha_payoff_chart_free(r->payoffs);
  gatha_cairo_save_free(r->cs);

  gatha_pvect_array_free(r->first_array);
  gatha_pvect_array_free(r->last_array);

  free(r);
}
