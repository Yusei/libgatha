#include "mcb.h"

GathaMcbData* gatha_mcb_data_new(GathaGame *g)
{
  GathaMcbData *d;

  d = (GathaMcbData*) malloc(sizeof(GathaMcbData));
  d->game = g;

  d->proba = gatha_game_pvect_new(g);

  d->max_thread = 4;
  d->n_sim = 100;
  d->b = 0.01;
  d->time = -1;
  d->max_time = -1;
  d->checkpoint_dir = NULL;
  d->save_interval = 1000;

  d->feedback_interval = 10;
  d->feedback_func = NULL;
  d->feedback_data = NULL;

  d->convergence_func = NULL;
  d->convergence_data = NULL;

  return d;
}

void gatha_mcb_data_free(GathaMcbData *d)
{
    gatha_game_pvect_free(d->game, d->proba);
    free(d);
}

static inline void mcb_update_proba(int player, int action,
				    payoff_t payoff,
				    GathaMcbData *data)
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

static inline int mcb_draw(GathaMcbData *data, int player)
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
  }

  assert(FALSE);
  return -1;
}


static inline int mcb_one_step(GathaMcbData *data, int player, int *actions, payoff_t *payoffs,
			payoff_t *payoff_tmp, int thread_id)
{
  int i, j, k, p, n;
  int best_action;
  payoff_t best_payoff;
  payoff_t sum, total;
  proba_t rnd;
  payoff_t incr;

  best_payoff = 0.0;
  best_action = -1;
  total = 0.0;

  p = data->game->n_players;
  n = data->game->n_strategies;
  for(i=0 ; i<n ; i++) {
    if (data->proba[player][i] == 0.0) {
      payoff_tmp[i] = 0.0;
      continue;
    }
    actions[player] = i;

    /* runs data->n_sim simulations for this action and computes
       the average payoff */
    sum = 0.0;
    for(j=0 ; j<data->n_sim ; j++) {
      for(k=0 ; k<p ; k++) {
	if (k == player) continue;
	actions[k] = mcb_draw(data, k);
      }
      data->game->payoff_func(data->game, actions, payoffs, thread_id);
      //sum += log(1.0+payoffs[player]);
      sum += payoffs[player];
    }
    sum /= data->n_sim;
    //costs_tmp[i] = 1.0/sum;
    payoff_tmp[i] = sum;
    total += payoff_tmp[i];

    if (sum > best_payoff || best_action == -1) {
      best_action = actions[player];
      best_payoff = sum;
    }
  }

  assert(best_action != -1);
  return best_action;

  /* finds the 'best' solution, and increases its probability
     to speed up the process */
  payoff_t diff = payoff_tmp[best_action] * 9.0;
  total += diff;
  payoff_tmp[best_action] += diff;

  /* picks a strategy with a probability proportional to its average payoff */
  rnd = (proba_t)rand()/RAND_MAX;
  rnd *= total;
  incr = 0.0;
  for(i=0 ; i<n ; i++) {
    incr += payoff_tmp[i];
    if (rnd < incr && !(data->proba[player][i] == 0.0)) {
      return i;
    }
  }

  return -1;
}

#define FILENAME_MAX_LENGTH 256

void mcb_save_checkpoint(GathaMcbData *data, int *actions, payoff_t *payoffs)
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

void gatha_mcb(GathaMcbData *data) 
{
  int i, n, m;
  int *actions;
  payoff_t *payoffs;
  boolean stop;
  int thread_id;

  /* payoff arrays, one for each thread */
  payoff_t **payoffs_a;
  /* payoff tmp arrays, one for each thread */
  payoff_t **payoffs_tmp;
  /* action arrays, one for each thread */
  int **actions_a;

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
  
  actions = (int*) calloc(n,sizeof(int));
  assert(actions != NULL);

  payoffs = (payoff_t*) calloc(n, sizeof(payoff_t));
  assert(payoffs != NULL);

  /* initialize the action and costs temp arrays used in the different
     threads */
  actions_a = (int**)malloc(data->max_thread*sizeof(int*));
  payoffs_a = (payoff_t**)malloc(data->max_thread*sizeof(payoff_t*));
  payoffs_tmp = (payoff_t**)malloc(data->max_thread*sizeof(payoff_t*));
  for(i=0 ; i<data->max_thread ; i++) {
    actions_a[i] = (int*)malloc(n*sizeof(int));
    payoffs_a[i] = (payoff_t*)malloc(n*sizeof(payoff_t));
    payoffs_tmp[i] = (payoff_t*)malloc(m*sizeof(payoff_t));
  }

  data->time = 0;
  stop = FALSE;
  while (stop == FALSE && (data->max_time == -1 ||
			   data->time < data->max_time)
	 )
    {
      if (data->time % data->save_interval == 0 && data->checkpoint_dir != NULL) {
	mcb_save_checkpoint(data, actions, payoffs);
      }

      if (data->feedback_func != NULL && data->time % data->feedback_interval == 0) {
	data->feedback_func(data, actions, payoffs, data->feedback_data);
      }

      #pragma omp parallel for private(thread_id)
      for(i=0 ; i<n ; i++) {
	thread_id = omp_get_thread_num();
	actions[i] = mcb_one_step(data, i, actions_a[thread_id],
				  payoffs_a[thread_id],
				  payoffs_tmp[thread_id],
				  thread_id);
      }
      
      data->game->payoff_func(data->game, actions, payoffs, 0);
      
      for(i=0 ; i<n ; i++) {
	mcb_update_proba(i, actions[i], payoffs[i], data);
      }
      gatha_game_pvect_normalize(data->game, data->proba);

      if (data->convergence_func != NULL) {
	stop = data->convergence_func(payoffs,
				      data->convergence_data);
      }


      data->time++;
    }

  /* free the temp arrays we created */
  for(i=0 ; i<data->max_thread ; i++) {
    free(actions_a[i]);
    free(payoffs_a[i]);
    free(payoffs_tmp[i]);
  }
  free(actions_a);
  free(payoffs_a);
  free(payoffs_tmp);
  free(actions);
  free(payoffs);
}

void gatha_mcb_read_checkpoint(FILE *f, GathaMcbData *data, int *actions,
				  payoff_t *payoffs)
{
  int i, n, m, c;

  assert(f != NULL);
  assert(data != NULL);
  assert(data->proba == NULL);

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
