#ifndef _GATHA_CAIRO_REPORT_H_
#define _GATHA_CAIRO_REPORT_H_

#include <glib.h>
#include <cairo.h>

#include "types.h"

typedef struct {
  GathaGame *game;
  double width, height, y_unit;

  GathaCairoSave *cs;

  GathaPayoffChart *payoffs;
  GathaPvectTimeline **timelines;
  GathaPvectArray *first_array;
  GathaPvectArray *last_array;

  GathaTimeline *timeline;

  double timeline_height;
  char *timeline_title_format;

  double arrays_size;
  double pvect_timeline_size;
  double payoffs_size;
} GathaReport;

GathaReport* gatha_report_start(char* filename, GathaGame *g, double width, double height,
				boolean show_arrays);
void gatha_report_add_payoffs(GathaReport *r, payoff_t *payoffs);
void gatha_report_add_pvect(GathaReport *r, proba_t** pvect);
void gatha_report_end(GathaReport *r);
void gatha_report_free(GathaReport *r);

#endif /* _GATHA_CAIRO_REPORT_H_ */
