#include "payoff_matrix.h"
#include "game.h"

#include <math.h>
#include <string.h>

GathaPayoffMatrix* gatha_payoff_matrix_new(int p, int s)
{
  GathaPayoffMatrix *m;
  long n;

  assert(p > 1 && s > 0);

  m = (GathaPayoffMatrix *) malloc(sizeof(GathaPayoffMatrix));
  if (m == NULL) return NULL;

  m->n_players = p;
  m->n_strategies = s;

  n = pow(s, p+1);
  assert(n > 0);
  m->payoffs = (payoff_t*) calloc(n, sizeof(payoff_t));
  assert(m->payoffs != NULL);

  m->max_payoff = 0;

  return m;
}

void gatha_payoff_matrix_free(GathaPayoffMatrix *m)
{
  assert(m != NULL);
  assert(m->payoffs != NULL);
  free(m->payoffs);
  free(m);
}

void gatha_payoff_matrix_compute_max_payoff(GathaPayoffMatrix *m)
{
  int i, n;
  n = pow(m->n_strategies, m->n_players+1);
  for(i=0 ; i<n ; i++) {
    if (m->max_payoff < m->payoffs[i])
      m->max_payoff = m->payoffs[i];
  }
}

void gatha_payoff_matrix_fprintf(GathaPayoffMatrix *m, FILE *f) {
  int r, c, v;
  fprintf(f, "%d players, %d strategies\n", m->n_players, m->n_strategies);
  if (m->n_players == 2) {
    for(r=0 ; r<m->n_strategies ; r++) {
      for(c=0 ; c<m->n_strategies ; c++) {
	for(v=0 ; v<m->n_players ; v++) {
	  fprintf(f, "%f ", gatha_payoff_matrix_get(m, v, r, c));
	}
	fprintf(f, ";");
      }
      fprintf(f, "\n");
    }
  }
}

void gatha_payoff_matrix_costs(GathaGame *g, int* actions, cost_t* costs,
			       int thread_id)
{
  int i, n;
  GathaPayoffMatrix *m;

  m = (GathaPayoffMatrix*) g->data;

  n = m->n_players;
  assert(n == 2);
  for(i=0 ; i<n ; i++) {
    costs[i] = m->max_payoff - gatha_payoff_matrix_get(m, i, actions[0], actions[1]);
  }
}

void gatha_payoff_matrix_payoffs(GathaGame *g, int* actions, payoff_t* payoffs,
				 int thread_id)
{
  int i, n;
  GathaPayoffMatrix *m;

  m = (GathaPayoffMatrix*) g->data;

  n = m->n_players;
  assert(n == 2);
  for(i=0 ; i<n ; i++) {
    payoffs[i] = gatha_payoff_matrix_get(m, i, actions[0], actions[1]);
  }
}

void gatha_payoff_matrix_set(GathaPayoffMatrix *m, int player, payoff_t value, ...)
{
  va_list arg;
  int i, v, n, s;
  // payoff matrix index
  int p;

  n = m->n_players;
  s = m->n_strategies;
  p = player;
  va_start(arg, value);
  for(i=0 ; i<n ; i++) {
    v = va_arg(arg, int);
    p += v * pow(s, i) * n;
  }

  m->payoffs[p] = value;

  gatha_payoff_matrix_compute_max_payoff(m);

  va_end(arg);
}

payoff_t gatha_payoff_matrix_get(GathaPayoffMatrix *m, int player,  ...)
{
  va_list arg;
  int i, v, n, s;
  // payoff matrix index
  int p;

  n = m->n_players;
  s = m->n_strategies;
  p = player;
  va_start(arg, player);
  for(i=0 ; i<n ; i++) {
    v = va_arg(arg, int);
    p += v * pow(s, i) * n;
  }

  va_end(arg);
  return m->payoffs[p];
}

GathaPayoffMatrix* gatha_payoff_matrix_2p_from_file(FILE *f)
{
  // number of players, strategies
  int p, s;
  // loop indices
  int i, j, k;
  // counter
  int c;
  int value;
  GathaPayoffMatrix *m = NULL;
  char buffer[1024];
  char *ptr, *token;
  char *ptr2, *token2;

  // number of players
  if (fscanf(f, "%d ", &p) != 1) goto error;
  // number of strategies
  if (fscanf(f, "%d ", &s) != 1) goto error;

  if (p != 2) {
    fprintf(stderr, "Too many players (%d)\n", p);
    return NULL;
  }

  m = gatha_payoff_matrix_new(p, s);

  // read the matrix, 2 dimensions, S rows, S columns,
  // P values in each cell
  for(i=0 ; i<s ; i++) {
    ptr = fgets(buffer, 1024, f);
    if (ptr == NULL) goto error;
    j = 0;
    // for each column
    token = strtok_r(buffer, ";", &ptr);
    while(token != NULL) {
      // for each value in a cell
      k = 0;
      token2 = strtok_r(token, ",", &ptr2);
      while(token2 != NULL) {
	c = sscanf(token2, "%d", &value);
	if (c != 1) goto error;
	gatha_payoff_matrix_set(m, k, (payoff_t)value, i, j);
	token2 = strtok_r(NULL, ",", &ptr2);
	k++;
      }
      if (k != p) goto error;
      // next column
      token = strtok_r(NULL, ";", &ptr);
      j++;
    }
    if (j != s) goto error;
  }

  goto ok;
 error:
  fprintf(stderr, "Could not load file\n"); 
  if (m != NULL) gatha_payoff_matrix_free(m);
  return NULL;
 ok:
  return m;
}
