#ifndef _GATHA_PAYOFF_MATRIX_H_
#define _GATHA_PAYOFF_MATRIX_H_

#include "types.h"

#include <stdarg.h>

/** Normal Form game. The payoffs for all possible choices of strategies are
 *  stored in a matrix of dimension n_players, that contains n_strategies^(n_players+1)
 *  elements. */
struct _gatha_payoff_matrix {

  /** Number of players in the game. This is also the dimension of the matrix. */
  int n_players;

  /** Number of strategies for each player. It is assumed that all players have the
   same number of strategies. */
  int n_strategies;

  /** Payoffs for all possible choices of strategies. The array should not
   * be accessed directly, but through accessors like gatha_payoff_matrix_get and _set.
   * \see gatha_payoff_matrix_get
   * \see gatha_payoff_matrix_set
   */
  payoff_t *payoffs;

  /** Maximum payoff in the matrix. It is used, for example, for turning
   * a gain into a cost when needed: the cost is then max_payoff-actual_payoff. */
  payoff_t max_payoff;
} ;

/** Creates a GathaPayoffMatrix. The `payoffs' array contains ns^(np+1) elements and
 * is initialized with 0 for each player.
 * @param np Number of players.
 * @param ns Number of strategies for each player.
 */
extern GathaPayoffMatrix* gatha_payoff_matrix_new(int np, int ns);

/** Frees a GathaPayoffMatrix. The `payoffs' array is freed first. */
extern void gatha_payoff_matrix_free(GathaPayoffMatrix *m);

/** Prints the matrix in a file descriptor. */
extern void gatha_payoff_matrix_fprintf(GathaPayoffMatrix *m, FILE *f);

/** Retrieves the players' costs for a choice of strategies.
 * 
 * \todo So far, it only works with two-player games. Make it work for
 * any number of players.
 *
 * @param g The game
 * @param actions The integer array containing the strategy choices for the players.
 * @param[out] costs The cost_t array where the costs will be stored.
 */
extern void gatha_payoff_matrix_costs(GathaGame *g, int* actions,
				      cost_t* costs, int trhead_id);

/** Retrieves the players' payoffs for a choice of strategies.
 * 
 * \todo So far, it only works with two-player games. Make it work for
 * any number of players.
 *
 * @param g The game
 * @param actions The integer array containing the strategy choices for the players.
 * @param[out] payoffs The payoff_t array where the payoffs will be stored.
 */
extern void gatha_payoff_matrix_payoffs(GathaGame *g, int* actions,
					payoff_t* payoffs, int thread_id);

/** Sets the payoff of a player, given a set of strategy choices.
 * \see gatha_payoff_matrix_get explains how the variable length list of
 * strategies is used.
 * @param m The game matrix
 * @param player The player
 * @param value The new payoff value
 * @param ... The strategy choices, given as a variable length list of
 * integers.
 */
extern void gatha_payoff_matrix_set(GathaPayoffMatrix *m, int player,
				    payoff_t value, ...);

/** Retrieves the payoff of a player, given a set of strategy choices.
 * \remark For a two-player game with 5 strategies, the payoff for player 1,
 * when 1 plays 3 and 2 plays 4 is given by: \c gatha_payoff_matrix(m, 1, 3, 4);
 * \remark For a 3-player game with 5 strategies, the payoff for player 1,
 * when 1 plays 3, 2 plays 4 and 3 plays 1 is given by:
 * \c gatha_payoff_matrix(m, 1, 3, 4, 1);
 * @param m The game matrix
 * @param player The player
 * @param ... The strategy choices, given as a variable length list of
 * integers.
 */
extern payoff_t gatha_payoff_matrix_get(GathaPayoffMatrix *m, int player,  ...);

/** Reads a two-player game from a text file.
 * File format:
 *
 * line 1:        number of players (p)
 *
 * line 2:        number of strategies (s)
 *
 * line 3..(3+p): s elements containing p payoffs, separated by ';'
 * the payoffs are separated by ','
 *
 * @param f The file descriptor
 */
extern GathaPayoffMatrix* gatha_payoff_matrix_2p_from_file(FILE *f);

#endif /* _GATHA_PAYOFF_MATRIX_H_ */
