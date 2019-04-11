#ifndef _GATHA_SFP_H_
#define _GATHA_SFP_H_

#include "types.h"
#include "game.h"

struct _gatha_sfp_data {
  /** The game used in this run of the SFP algorithm. It contains the number
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

  int **action_count;

  boolean **forbidden_actions;

  /** Initialization of the probability vector. If it is set to NULL,
   * `gatha_mcb' will initialize the vector with uniform probability. */
  void (*proba_init)(GathaGame* g, proba_t **p, int **counts);

  char* checkpoint_dir;
  int save_interval;

  /** Convergence check callback. Returns true if the algorithm convergd.
   * If NULL, there is no convergence check.
   */
  boolean (*convergence_func)(payoff_t*, void*);

  /** Data to be passed to the convergence check callback. */
  void *convergence_data;

  /** Sampling size */
  int sampling_size;

  /** Maximum number of threads to start */
  int max_thread;

  /** Feedback interval */
  int feedback_interval;

  /** Feedback callback */
  boolean (*feedback_func)(struct _gatha_sfp_data*,
			   int *actions, payoff_t *payoffs,
			   void* data);

  void* feedback_data;
};

extern GathaSfpData* gatha_sfp_data_new(GathaGame *g);

/** Frees the data. */
extern void gatha_sfp_data_free(GathaSfpData *d);

/** Run the SFP algorithm.
 * @param data Game and parameters
 */
extern void gatha_sfp(GathaSfpData *data);

extern void gatha_sfp_read_checkpoint(FILE *f, GathaSfpData *data, int *actions,
				      payoff_t *payoffs);

#endif /* _GATHA_SFP_H_ */
