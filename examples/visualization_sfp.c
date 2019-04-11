#include "gatha.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
  GathaPayoffChart *payoff;
  GathaSinglePayoffChart *single_payoff;
  GathaPvectTimeline *pvect_timeline;
  GathaPvectTimeline *pvect_timeline2;
  GathaPvectArray *pvect;
  GathaReport *report;
} MyData;

boolean add_to_chart(GathaSfpData *sfpdata, int *actions, payoff_t *payoffs,
		     void* data)
{
  MyData *d = (MyData*) data;

  gatha_payoff_chart_add(d->payoff, payoffs);
  gatha_single_payoff_chart_add(d->single_payoff, payoffs[0]);
  gatha_pvect_timeline_add(d->pvect_timeline, sfpdata->proba[0]);
  gatha_pvect_timeline_add(d->pvect_timeline2, sfpdata->proba[1]);

  gatha_report_add_payoffs(d->report, payoffs);
  gatha_report_add_pvect(d->report, sfpdata->proba);

  if (sfpdata->time % 10 == 0) printf("%d\n", sfpdata->time);

  return TRUE;
}

void init_probas_randomly(GathaGame* g, proba_t **p, int **counts)
{
  int i, j, m, n, c;

  n = g->n_players;
  m = g->n_strategies;

  for(i=0 ; i<n ; i++) {
    c = 0;
    for(j=0 ; j<m ; j++) {
      counts[i][j] = (int) ((((double)rand())/RAND_MAX) * 20) + 1;
      c += counts[i][j];
      printf("%d\n", counts[i][j]);
    }

    for(j=0 ; j<m ; j++) {
      p[i][j] = ((double)counts[i][j]) / c;
    }
  }
}

int main(int argc, char **argv)
{
  FILE *f;
  GathaSfpData *data;
  GathaPayoffMatrix *mat;
  GathaGame tmp;
  GathaIntervalData *id;
  GathaCairoSave *cs;
  MyData d;
  int i;

  srand(time(NULL));
  mat = NULL;

  tmp.n_players = 2;
  tmp.n_strategies = 3;
  data = gatha_sfp_data_new(&tmp);

  f = fopen(argv[1], "r");
  mat = gatha_payoff_matrix_2p_from_file(f);
  gatha_payoff_matrix_fprintf(mat, stdout);
  data->game = gatha_game_from_matrix(mat);
  id = gatha_interval_data_new(data->game, 0.1, 50);
  data->convergence_func = gatha_interval_check;
  data->convergence_data = id;
  data->max_time = 1000;
  data->proba_init = init_probas_randomly;
  fclose(f);

  d.payoff = gatha_payoff_chart_start(data->game, 1200, 400);
  d.payoff->interval = 100;

  d.single_payoff = gatha_single_payoff_chart_start(data->game, 1200, 400);
  d.single_payoff->interval = 100;

  d.pvect_timeline = gatha_pvect_timeline_start(data->game, 1200, 200);
  d.pvect_timeline->interval = 100;

  d.pvect_timeline2 = gatha_pvect_timeline_start(data->game, 1200, 200);
  d.pvect_timeline2->interval = 100;

  d.pvect = gatha_pvect_array_start(data->game, 200, 200);

  d.report = gatha_report_start("report.svg", data->game, 800, 400, TRUE);
  d.report->timeline_title_format = "Joueur %d";
  d.report->payoffs->interval = 20;
  for(i=0 ; i<data->game->n_players ; i++) {
    d.report->timelines[i]->interval = 10;
  }

  data->feedback_func = add_to_chart;
  data->feedback_interval = 1;
  data->feedback_data = &d;

  gatha_sfp(data);

  gatha_payoff_chart_end(d.payoff);
  gatha_single_payoff_chart_end(d.single_payoff);
  gatha_pvect_timeline_end(d.pvect_timeline);
  gatha_pvect_timeline_end(d.pvect_timeline2);

  gatha_pvect_array_add(d.pvect, data->proba);
  gatha_pvect_array_end(d.pvect);

  gatha_report_end(d.report);

  cs = gatha_cairo_save_new_png("all_payoffs.png", 1200, 400);
  gatha_cairo_save_add(cs, d.payoff->surface);
  gatha_cairo_save_free(cs);

  cs = gatha_cairo_save_new_png("one_payoff.png", 1200, 400);
  gatha_cairo_save_add(cs, d.single_payoff->surface);
  gatha_cairo_save_free(cs);

  cs = gatha_cairo_save_new_png("pvect_timeline.png", 1200, 400);
  gatha_cairo_save_add(cs, d.pvect_timeline->surface);
  gatha_cairo_save_add_at(cs, d.pvect_timeline2->surface, 0, 200);
  gatha_cairo_save_free(cs);

  cs = gatha_cairo_save_new_png("pvect_array.png", 200, 200);
  gatha_cairo_save_add(cs, d.pvect->surface);
  gatha_cairo_save_free(cs);

  gatha_single_payoff_chart_free(d.single_payoff);
  gatha_pvect_array_free(d.pvect);
  gatha_pvect_timeline_free(d.pvect_timeline);
  gatha_payoff_chart_free(d.payoff);
  gatha_report_free(d.report);

  gatha_sfp_data_free(data);
  gatha_interval_data_free(id);
}
