#include "convergence.h"
#include "game.h"

GathaIntervalData* gatha_interval_data_new(GathaGame *g, payoff_t interval,
					  int size)
{
  GathaIntervalData *d;
  int i, n;

  assert(g != NULL);

  d = (GathaIntervalData*) malloc(sizeof(GathaIntervalData));
  assert(d != NULL);

  d->interval = interval;
  d->size = size;
  d->time = 0;

  d->payoffs = (payoff_t**) calloc(size, sizeof(payoff_t*));
  assert(d->payoffs != NULL);

  n = g->n_players;
  d->n_players = n;
  for(i=0 ; i<size ; i++) {
    d->payoffs[i] = (payoff_t*) calloc(n, sizeof(payoff_t));
    assert(d != NULL);
  }

  return d;
}

void gatha_interval_data_reset(GathaIntervalData *d)
{
  int i, j, n;
  assert(d != NULL);
  n = d->n_players;
  for(i=0 ; i<d->size ; i++) {
    for(j=0 ; j<n ; j++) {
      d->payoffs[i][j] = 0;
    }
  }

  d->time = 0;
}

void gatha_interval_data_free(GathaIntervalData *d)
{
  int i, n;
  assert(d != NULL);
  n = d->n_players;
  for(i=0 ; i<d->size ; i++) {
    free(d->payoffs[i]);
  }
  free(d->payoffs);
  free(d);
}

boolean gatha_interval_check(payoff_t *payoffs, void *dp)
{
  int c_stable = 0;
  payoff_t min, max;
  int i, j, x, n;
  GathaIntervalData *d;

  assert(dp != NULL);
  assert(payoffs != NULL);

  d = (GathaIntervalData*) dp;

  n = d->n_players;

  /* copy the payoffs to the relevant column x */
  x = d->time % d->size;
  for(i=0 ; i<n ; i++) {
    d->payoffs[x][i] = payoffs[i];
  }

  if (d->time > d->size) {
    for(i=0 ; i<n ; i++) {
      min = d->payoffs[0][i];
      max = d->payoffs[0][i];
      for(j=0 ; j<d->size ; j++) {
	if(d->payoffs[j][i] < min)
	  min = d->payoffs[j][i];
	if(d->payoffs[j][i] > max)
	  max = d->payoffs[j][i];
      }
      if((max-min) <= d->interval) {
	c_stable++;
      }
    }
  }
  d->time++;

  return (c_stable == n);
}
