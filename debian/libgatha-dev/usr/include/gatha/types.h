#ifndef _GATHA_TYPES_H_
#define _GATHA_TYPES_H_

#include "../config.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/* --- basic types --- */

typedef double payoff_t;
typedef double cost_t;
typedef float  proba_t;

typedef int    boolean;
#define TRUE 1
#define FALSE 0

/* --- structured types */

typedef struct _gatha_game GathaGame;
typedef struct _gatha_payoff_matrix GathaPayoffMatrix;
typedef struct _gatha_sastry_data GathaSastryData;
typedef struct _gatha_mcb_data GathaMcbData;
 
#endif /* _GATHA_TYPES_H_ */
