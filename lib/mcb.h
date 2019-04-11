#ifndef _GATHA_MCB_H_
#define _GATHA_MCB_H_

#include "types.h"
#include "game.h"

struct _gatha_mcb_data {
  /** The game used in this run of the MCB algorithm. It contains the number
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
   * `gatha_mcb' will initialize the vector with uniform probability. */
  void (*proba_init)(GathaGame* g, proba_t **p);

  char* checkpoint_dir;
  int save_interval;

  /** Convergence check callback. Returns true if the algorithm convergd.
   * If NULL, there is no convergence check.
   */
  boolean (*convergence_func)(payoff_t*, void*);

  /** Data to be passed to the convergence check callback. */
  void *convergence_data;

  /** Number of simulations each player does when it chooses its strategy. */
  int n_sim;

  /** Maximum number of threads to start */
  int max_thread;

  /** Feedback interval */
  int feedback_interval;

  /** Feedback callback */
  boolean (*feedback_func)(struct _gatha_mcb_data*, int *actions, payoff_t *payoffs,
			   void* data);

  void* feedback_data;
};

extern GathaMcbData* gatha_mcb_data_new(GathaGame *g);

/** Frees the data. */
extern void gatha_mcb_data_free(GathaMcbData *d);

/** Run the MCB algorithm.
 * @param data Game and parameters
 */
extern void gatha_mcb(GathaMcbData *data);

extern void gatha_mcb_read_checkpoint(FILE *f, GathaMcbData *data, int *actions,
				      payoff_t *payoffs);

#endif /* _GATHA_MC_B_ */
