#ifndef _GATHA_CONVERGENCE_H_
#define _GATHA_CONVERGENCE_H_

#include "types.h"

struct _gatha_interval_data {
  int size;
  int time;
  int n_players;
  payoff_t interval;
  payoff_t **payoffs;
};

/** Creates a new GathaIntervalData object.
 * @param g The game we're playing (used for getting the number of players)
 * @param interval Size of the payoff interval
 * @param size Size of the time window (in iterations)
 */
GathaIntervalData* gatha_interval_data_new(GathaGame *g, payoff_t interval,
					  int size);

/** Resets the GathaIntervalData object to default values.
 */
void gatha_interval_data_reset(GathaIntervalData *d);

/** Frees a GathaIntervalData object. */
void gatha_interval_data_free(GathaIntervalData *d);

/** Checks for convergence within an interval. Assumes we reached
    convergence if the payoffs stayed T iterations in an interval of
    size S. N and T are stored in a GathaIntervalData object.
 * @param payoffs The current interation payoff array,
 * containing dp->n_players elements.
 * @param dp A pointer to a GathaIntervalData object.
 * @returns TRUE if the algorithm converged, FALSE otherwise.
 */
boolean gatha_interval_check(payoff_t *payoffs, void *dp);

#endif /* _GATHA_CONVERGENCE_H_ */
