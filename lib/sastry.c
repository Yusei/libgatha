#include "sastry.h"

GathaSastryData* gatha_sastry_data_new(GathaGame *g)
{
  GathaSastryData *d;

  d = (GathaSastryData*) malloc(sizeof(GathaSastryData));
  d->game = g;

  d->proba = gatha_game_pvect_new(g);
  assert(d->proba != NULL);

  d->b = 0.01;
  d->time = -1;
  d->max_time = -1;
  d->checkpoint_dir = NULL;
  d->save_interval = 1000;

  d->feedback_interval = 1000;
  d->feedback_func = NULL;
  d->feedback_data = NULL;

  d->convergence_func = NULL;
  d->convergence_data = NULL;

  return d;
}

void gatha_sastry_data_free(GathaSastryData *d)
{
    gatha_game_pvect_free(d->game, d->proba);
    free(d);
}

inline void sastry_update_proba(int player, int action,
				payoff_t payoff,
				GathaSastryData *data)
{
  int i, m;
  proba_t x;
  proba_t *proba;
  int c=0;

  m = data->game->n_strategies;
  proba = data->proba[player];

  /* check how many strategies have non-zero probability */
  for(i=0 ; i<m ; i++) {
    if((proba[i] > 0.0))
      c++;
  }
  assert(c > 0);

  if(c != 0) {
    /* how much should we add to the probability of
       choosing this action? */
    x = (data->b * payoff * (1.0-proba[action]));
    proba[action] += x;

    /* remove that amount from the other probabilities */
    x /= c;
    for(i=0 ; i<m ; i++) {
      if( i!=action &&  proba[i]>0.0) {
	proba[i] = proba[i] - x;
	if (proba[i] < 0.0) proba[i] = 0.0;
      }
    }
  }		

}

inline int sastry_draw(GathaSastryData *data, int player)
{
  proba_t result;
  proba_t b_sup;
  proba_t *T;
  int i, n, c;

  n = data->game->n_strategies;
  T = data->proba[player];

  // sometimes, due to rounding errors, the for loop
  // below may fail to pick a strategy, so we try it
  // 3 times.
  c = 0;
  while (c<3) {
    result = (proba_t)rand()/RAND_MAX;
    
    b_sup = T[0];
    for(i=0;i<n;i++) {
      if( result < b_sup ) {
	return i;
      }
      b_sup = b_sup + T[i+1];
    }
    c++;
  }

  assert(FALSE);
  return -1;
}

#define FILENAME_MAX_LENGTH 256

void sastry_save_checkpoint(GathaSastryData *data, int *actions, payoff_t *payoffs)
{
  char filename[FILENAME_MAX_LENGTH];
  FILE *f;
  int i, j, n, m;

  assert(data->checkpoint_dir != NULL);
  snprintf(filename, FILENAME_MAX_LENGTH, "%s/%.12d",
	   data->checkpoint_dir, data->time);
  f = fopen(filename, "w+");

  if (f == NULL) {
    perror("save_checkpoint");
    return;
  }

  // write the time
  fwrite(&(data->time), sizeof(int), 1, f);

  // write the probability vectors
  n = data->game->n_players;
  m = data->game->n_strategies;
  for(i=0 ; i<n; i++) {
    j = fwrite(data->proba[i], sizeof(proba_t), m, f);
    if (j < m) {
      fprintf(stderr, "%d/%d items\n", j, m);
      perror("write error");
      break;
    }
  }

  // write the actions
  j = fwrite(actions, sizeof(int), n, f);
  if (j < n) {
    fprintf(stderr, "%d/%d items\n", j, n);
    perror("write error");
  }
  
  // write the payoffs
  j = fwrite(payoffs, sizeof(payoff_t), n, f);
  if (j < n) {
    fprintf(stderr, "%d/%d items\n", j, n);
    perror("write error");
  }
  
  fclose(f);
}

void gatha_sastry_read_checkpoint(FILE *f, GathaSastryData *data, int *actions,
				  payoff_t *payoffs)
{
  int i, n, m, c;

  assert(f != NULL);
  assert(data != NULL);
  assert(data->proba != NULL);

  // read the time
  c = fread(&(data->time), sizeof(int), 1, f);
  assert(c == 1);

  n = data->game->n_players;
  m = data->game->n_strategies;

  // read the probability vectors
  for(i=0 ; i<n ; i++) {
    c = fread(data->proba[i], sizeof(proba_t), m, f);
    assert(c == m);
  }

  // if we provided a actions and a payoff arrays, read them too
  if (actions != NULL) {
    c = fread(actions, sizeof(int), n, f);
    assert(c == n);

    if (payoffs != NULL) {
      c = fread(payoffs, sizeof(payoff_t), n, f);
      assert(c == n);
    }
  }
}

void gatha_sastry(GathaSastryData *data) 
{
  int i, n, m;
  int *actions;
  payoff_t *payoffs;
  boolean stop;

  assert(data != NULL);
  assert(data->game != NULL);
  assert(data->proba != NULL);

  n = data->game->n_players;
  m = data->game->n_strategies;

  if (data->proba_init != NULL) {
    data->proba_init(data->game, data->proba);
  } else {
    gatha_game_pvect_uniformize(data->game, data->proba);
  }
  
  actions = (int*) calloc(n, sizeof(int));
  assert(actions != NULL);

  payoffs = (payoff_t*) calloc(n, sizeof(payoff_t));
  assert(payoffs != NULL);

  data->time = 0;
  stop = FALSE;
  while (stop == FALSE && (data->max_time == -1 ||
			   data->time < data->max_time)
	 )
    {
      if (data->time % data->save_interval == 0 && data->checkpoint_dir != NULL) {
	sastry_save_checkpoint(data, actions, payoffs);
      }   
      if (data->feedback_func != NULL && data->time % data->feedback_interval == 0) {
	data->feedback_func(data, actions, payoffs, data->feedback_data);
      }

      for(i=0 ; i<n ; i++) {
	actions[i] = sastry_draw(data, i);
      }
      
      data->game->payoff_func(data->game, actions, payoffs, 0);
      
      for(i=0 ; i<n ; i++) {
	sastry_update_proba(i, actions[i], payoffs[i], data);
      }
      gatha_game_pvect_normalize(data->game, data->proba);

      if (data->convergence_func != NULL) {
	stop = data->convergence_func(payoffs,
				      data->convergence_data);
      }

      data->time++;
    }
}
