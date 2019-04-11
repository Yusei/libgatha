#ifndef _GATHA_SASTRY_H_
#define _GATHA_SASTRY_H_

#include "types.h"
#include "game.h"

struct _gatha_sastry_data {
  /** The game used in this run of the Sastry algorithm. It contains the number
   * of players, of strategies, and the payoff function. */
  GathaGame *game;
  
  /** Current time of the simulation. This is the number of iterations in the main
   * loop. If gatha_sastry hasn't been run yet, `time' should be set to -1. */
  int time;

  /** Maximum time of the simulation. The main loop will never do more than `max_time'
   * iterations. If `max_time' is set to -1, then the limit is ignored and the loop
   * will run forever or until the convergence check callback returns true. */
  int max_time;

  /** Probability vector of the players. It is a N*M matrix where N is the number of
   * players and M the number of strategies. */
  proba_t **proba;

  /** Dampening parameter. See the paper by Sastry et al. It has to be more than 0.0
   * and should be less than 1.0. The smaller it is, the more iterations will be needed
   * before convergence, but the less likely the algorithm is to converge to something
   * that is not a Nash equilibrium. */
  proba_t b;

  /** Initialization of the probability vector. If it is set to NULL,
   * `gatha_sastry' will initialize the vector with uniform probability. */
  void (*proba_init)(GathaGame* g, proba_t **p);

  char* checkpoint_dir;
  int save_interval;

  /** Convergence check callback. Returns true if the algorithm convergd.
   * If NULL, there is no convergence check.
   */
  boolean (*convergence_func)(payoff_t*, void*);

  /** Data to be passed to the convergence check callback. */
  void *convergence_data;

  /** Feedback interval */
  int feedback_interval;

  /** Feedback callback */
  boolean (*feedback_func)(struct _gatha_sastry_data*, int *actions, payoff_t *payoffs,
			   void *data);

  void* feedback_data;
};

/** Creates a new GathaSastryData according suitable for
 *  a given game.
 */
extern GathaSastryData* gatha_sastry_data_new(GathaGame *g);

/** Frees the data. */
extern void gatha_sastry_data_free(GathaSastryData *d);

/** Run the Sastry algorithm.
 * @param data Game and parameters
 */
extern void gatha_sastry(GathaSastryData *data);

extern void gatha_sastry_read_checkpoint(FILE *f, GathaSastryData *data, int *actions,
					 payoff_t *payoff);

#endif /* _GATHA_SASTRY_H_ */
