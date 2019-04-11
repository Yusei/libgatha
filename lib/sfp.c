#include "sfp.h"

#define FILENAME_MAX_LENGTH 256

GathaSfpData* gatha_sfp_data_new(GathaGame *g)
{
  GathaSfpData *d;
  int i;

  d = (GathaSfpData*) malloc(sizeof(GathaSfpData));
  d->game = g;

  d->proba = gatha_game_pvect_new(g);
  d->action_count = (int**) malloc(g->n_players * sizeof(int*));
  for(i=0 ; i<g->n_players ; i++) {
    d->action_count[i] = (int*) calloc(g->n_strategies, sizeof(int));
  }
  d->forbidden_actions = NULL;

  d->max_thread = 4;
  d->sampling_size = 100;
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

void gatha_sfp_data_free(GathaSfpData *d)
{
  int i;
    gatha_game_pvect_free(d->game, d->proba);
    for(i=0 ; i<d->game->n_players ; i++) {
      free(d->action_count[i]);
    }
    free(d->action_count);
    free(d);
}

static inline void sfp_update_proba(int player, int action,
				    payoff_t payoff,
				    GathaSfpData *data)
{
  int i, m, c;
  proba_t *proba;

  m = data->game->n_strategies;
  proba = data->proba[player];

  data->action_count[player][action]++;

  c = 0;
  for(i=0 ; i<m ; i++) {
    c += data->action_count[player][i];
  }

  if(c != 0) {
    for(i=0 ; i<m ; i++) {
      proba[i] = ((proba_t)data->action_count[player][i]) / c;
    }
  }		

}

static inline int sfp_draw(GathaSfpData *data, int player)
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

static inline int sfp_one_step(GathaSfpData *data, int player, int *actions,
			       payoff_t *payoffs, payoff_t *payoff_tmp,
			       int **sample,
			       int thread_id)
{
  int i, j, k, p, n, ss;
  int best_action;
  payoff_t best_payoff;
  payoff_t sum, total;

  best_payoff = 0.0;
  best_action = -1;
  total = 0.0;

  p = data->game->n_players;
  n = data->game->n_strategies;
  ss = data->sampling_size;

  /* compute the best answer with regards to the sample */
  for(i=0 ; i<n ; i++) {
    // TODO: add forbidden actions

    actions[player] = i;

    sum = 0.0;
    for(j=0 ; j<ss ; j++) {
      for(k=0 ; k<p ; k++) {
	if (k == player) continue;
	actions[k] = sample[j][k];
      }
      data->game->payoff_func(data->game, actions, payoffs, thread_id);
      //sum += log(1.0+payoffs[player]);
      sum += payoffs[player];
    }
    sum /= ss;
    //costs_tmp[i] = 1.0/sum;
    payoff_tmp[i] = sum;
    total += payoff_tmp[i];

    if (sum > best_payoff || best_action == -1) {
      best_action = actions[player];
      best_payoff = sum;
    }
  }

  return best_action;
}

void sfp_save_checkpoint(GathaSfpData *data, int *actions, payoff_t *payoffs)
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

void gatha_sfp(GathaSfpData *data) 
{
  int i, j, n, m, ss, c;
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

  int **sample;

  assert(data != NULL);
  assert(data->game != NULL);
  assert(data->proba != NULL);

  n = data->game->n_players;
  m = data->game->n_strategies;
  ss = data->sampling_size;

  if (data->proba_init != NULL) {
    data->proba_init(data->game, data->proba, data->action_count);
    printf(" Probabilities\n");
      for(i=0 ; i<n ; i++) {
      	printf("  Player %d: ", i);
      	for(j=0 ; j<m ; j++) {
      	  printf("%.2f ", data->proba[i][j]);
      	}
      	printf("\n");
      }
  } else {
    for(i=0 ; i<n ; i++) {
      c = 0;
      for(j=0 ; j<m ; j++) {
	if (data->forbidden_actions == NULL ||
	    data->forbidden_actions[i][j] == FALSE)
	  {
	    data->action_count[i][j] = 1;
	    c++;
	  }
      }
      for(j=0 ; j<m ; j++) {
	data->proba[i][j] = ((proba_t)data->action_count[i][j]) / c;
      }
    }
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
  sample = (int**) malloc(data->sampling_size * sizeof(int*));
  for(i=0 ; i<ss ; i++) {
    sample[i] = (int*) malloc(n*sizeof(int));
  }

  data->time = 0;
  stop = FALSE;
  while (stop == FALSE && (data->max_time == -1 ||
			   data->time < data->max_time)
	 )
    {
      if (data->time % data->save_interval == 0 && data->checkpoint_dir != NULL) {
	sfp_save_checkpoint(data, actions, payoffs);
      }

      if (data->feedback_func != NULL && data->time % data->feedback_interval == 0) {
	data->feedback_func(data, actions, payoffs, data->feedback_data);
      }
      
      /* draws a new sample */
      for(i=0 ; i<ss ; i++) {
	/* for each player, draw a strategy */
	for(j=0 ; j<n ; j++) {
	  sample[i][j] = sfp_draw(data, j);
	}
      }

      #pragma omp parallel for private(thread_id)
      for(i=0 ; i<n ; i++) {
	thread_id = omp_get_thread_num();
	actions[i] = sfp_one_step(data, i, actions_a[thread_id],
				  payoffs_a[thread_id],
				  payoffs_tmp[thread_id],
				  sample,
				  thread_id);
      }
      
      data->game->payoff_func(data->game, actions, payoffs, 0);
      
      for(i=0 ; i<n ; i++) {
	sfp_update_proba(i, actions[i], payoffs[i], data);
      }
      gatha_game_pvect_normalize(data->game, data->proba);

      if (data->convergence_func != NULL) {
	stop = data->convergence_func(payoffs,
				      data->convergence_data);
      }

      /* show the action counts */
      /* printf("Iteration %d\n", data->time); */
      /* printf(" Action counts\n"); */
      /* for(i=0 ; i<n ; i++) { */
      /* 	printf("  Player %d: ", i); */
      /* 	for(j=0 ; j<m ; j++) { */
      /* 	  printf("%2d ", data->action_count[i][j]); */
      /* 	} */
      /* 	printf("\n"); */
      /* } */
      /* printf(" Probabilities\n"); */
      /* for(i=0 ; i<n ; i++) { */
      /* 	printf("  Player %d: ", i); */
      /* 	for(j=0 ; j<m ; j++) { */
      /* 	  printf("%.2f ", data->proba[i][j]); */
      /* 	} */
      /* 	printf("\n") */;
      /* } */

      data->time++;
    }

  /* free the temp arrays we created */
  for(i=0 ; i<ss ; i++) {
    free(sample[i]);
  }
  free(sample);
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
