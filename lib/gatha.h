#ifndef _GATHA_H_
#define _GATHA_H_

#include "types.h"
#include "game.h"
#include "payoff_matrix.h"
#include "convergence.h"

/* algorithms for learning nash equilibria */
#include "sastry.h"
#include "mcb.h"
#include "sfp.h"

/* visualization using cairo */
#ifdef HAVE_CAIRO
# include "cairo_margin.h"
# include "cairo_payoff_chart.h"
# include "cairo_single_payoff_chart.h"
# include "cairo_pvect_timeline.h"
# include "cairo_pvect_array.h"
# include "cairo_timeline.h"
# include "cairo_save.h"
# include "cairo_report.h"
#endif

#define gatha_max(x, y) (((x) > (y)) ? (x) : (y))
#define gatha_maxify(x, y) ((x) = ((x) > (y)) ? (x) : (y))

#endif /* _GATHA_H_ */
