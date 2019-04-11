#include "gatha.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv)
{
  int seed, i, c;
  GathaSastryData *data;
  GathaPayoffMatrix *mat;
  GathaGame tmp;
  GathaIntervalData *id;
  boolean do_log;
  FILE *f;
  double d;

  /* default values */
  do_log = FALSE;
  mat = NULL;
  seed = time(NULL);

  tmp.n_players = 2;
  tmp.n_strategies = 2;
  data = gatha_sastry_data_new(&tmp);
  data->b = 0.1;

  /* options */
  while ((c = getopt(argc, argv, "i:b:s:mLI:")) != -1) {
    switch (c) {
    case 'b':
      d = atof(optarg);
      if (d > 0.0 && d <= 1.0) {
	data->b = d;
      } else {
	fprintf(stderr, "-b ignored: b value should be in ]0.0, 1.0]\n");
      }
      break;
    case 's':
      i = atoi(optarg);
      if (i >= 0) {
	seed = i;
      } else {
	fprintf(stderr, "-s ignored: seed value must be a positive integer\n");
      }
      break;
    case 'i':
      i = atoi(optarg);
      if (i >= 0) {
	data->save_interval = i;
      } else {
	fprintf(stderr, "-i ignored: interval should be a positive integer\n");
      }
      break;
    case 'L':
      do_log = FALSE;
      break;
    case 'I':
      i = atoi(optarg);
      if (i >= 0) {
	data->max_time = i;
      } else {
	fprintf(stderr, "valeur de max d'iterations ignoree\n");
      }
      break;
    default:
      abort();
    }
  }

  srand(seed);

  if (optind < argc) {
    f = fopen(argv[optind], "r");
    mat = gatha_payoff_matrix_2p_from_file(f);
    gatha_payoff_matrix_fprintf(mat, stdout);
    data->game = gatha_game_from_matrix(mat);
    id = gatha_interval_data_new(data->game, 0.1, 50);
    data->convergence_func = gatha_interval_check;
    data->convergence_data = id;
    fclose(f);
  } else {
    perror("no game");
  }

  assert(data->game != NULL);
  assert(data->game->n_players > 1);
  assert(data->game->n_strategies > 1);
  assert(data->game->payoff_func != NULL);

  gatha_sastry(data);

  printf("last iteration: %d\n", data->time);
  int n = data->game->n_players;
  int m = data->game->n_strategies;
  int j;
  for(i=0 ; i<n ; i++) {
    for(j=0 ; j<m ; j++) {
      printf("%.3f ", data->proba[i][j]);
    }
    printf("\n");
  }

  gatha_sastry_data_free(data);
  gatha_interval_data_free(id);
}
