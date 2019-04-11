#ifndef _GATHA_GAME_H_
#define _GATHA_GAME_H_

#include "types.h"

struct _gatha_game {
  int n_players;
  int n_strategies;
  void (*payoff_func)(GathaGame *g, int *actions,
		      payoff_t *payoffs, int thread_id);
  void *data;
} ;

extern GathaGame* gatha_game_new(int np, int ns);
extern GathaGame* gatha_game_from_matrix(GathaPayoffMatrix *m);
extern void gatha_game_free(GathaGame *g);

extern proba_t** gatha_game_pvect_new(GathaGame *g);
extern void gatha_game_pvect_fprintf(GathaGame *g, proba_t **proba, FILE *f);
extern void gatha_game_pvect_free(GathaGame *g, proba_t **proba);
extern void gatha_game_pvect_normalize(GathaGame *g, proba_t **proba);
extern void gatha_game_pvect_uniformize(GathaGame *g, proba_t **proba);

#endif /* _GATHA_GAME_H_ */
