#include "game.h"
#include "payoff_matrix.h"

GathaGame* gatha_game_new(int np, int ns)
{
  GathaGame *g;
  assert(np > 1 && ns > 0);
  g = (GathaGame*) malloc(sizeof(GathaGame));
  if (g == NULL) return g;

  g->n_players = np;
  g->n_strategies = ns;
  g->payoff_func = NULL;
  g->data = NULL;
  return g;
}

void gatha_game_free(GathaGame *g)
{
  free(g);
}

GathaGame* gatha_game_from_matrix(GathaPayoffMatrix *m)
{
  GathaGame *g;
  assert(m != NULL);
  g = gatha_game_new(m->n_players, m->n_strategies);
  g->payoff_func = gatha_payoff_matrix_payoffs;
  g->data = m;
  return g;
}

proba_t** gatha_game_pvect_new(GathaGame *g)
{
  int n, m, i;
  proba_t **proba;

  n = g->n_players;
  m = g->n_strategies;

  proba = (proba_t**) malloc(n * sizeof(proba_t*));
  for(i=0 ; i<n ; i++) {
    proba[i] = (proba_t*)malloc(m*sizeof(proba_t));
  }

  return proba;
}

void gatha_game_pvect_free(GathaGame *g, proba_t **proba)
{
  int n, i;

  n = g->n_players;

  for(i=0 ; i<n ; i++) {
    free(proba[i]);
  }
  free(proba);
}

void gatha_game_pvect_fprintf(GathaGame *g, proba_t **proba, FILE *f)
{
  int i, j, n, m;
  n = g->n_players;
  m = g->n_strategies;
  for(i=0 ; i<n ; i++) {
    for(j=0 ; j<m ; j++) {
      fprintf(f, "%.3f ", proba[i][j]);
    }
    fprintf(f, "\n");
  }
  fprintf(f, "\n");
}

void gatha_game_pvect_normalize(GathaGame *g, proba_t **proba)
{
  int i, n, m;
  proba_t sum=0;
  proba_t *p;
  int player;

  n = g->n_players;
  m = g->n_strategies;

  for(player=0 ; player<n ; player++) {
    p = proba[player];
    sum=0;

    for(i=0;i<m;i++) {
      sum += p[i];
    }
    for(i=0;i<m;i++) {
      p[i] /= sum;
    }
  }
}

void gatha_game_pvect_uniformize(GathaGame *g, proba_t **proba)
{
  int i, j, n, m;
  n = g->n_players;
  m = g->n_strategies;
  for(i=0;i<n;i++) {
    for(j=0;j<m;j++) {
      proba[i][j] = 1.0/m;
    }
  }
}
