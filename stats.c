#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include "event.h"
#include "event_list.h"
#include "trace_evt_handle.h"
#include "stats_utils.h"
#include "stats_events.h"

#define TIME_PERC 1000000

static unsigned int tolerance;
static int start_time, end_time;
static FILE *l;

static void stats_event_handle(const struct event *e)
{
  unsigned long int r, c, it, pdf;
  double cdf;
  static unsigned int time_back;

  switch (e->type) {
    case TASK_END:
      //Blocks
      r = response_time(e->task, e->time);
      pdf = pdf_response_time(e->task, r, tolerance);
      cdf = cdf_response_time(e->task, pdf);
      stats_print_int(l, e->time, e->task, RESPONSE_TIME, r, pdf, cdf);
      break;
    case TASK_DESCHEDULE:
      //Deschedule
      c = end_execution(e->task, e->time);
      if (c > 0) {
	pdf = pdf_executions(e->task, c, tolerance);
	cdf = cdf_executions(e->task, pdf);
	stats_print_int(l, e->time, e->task, EXECUTION_TIME, c, pdf, cdf);
      }
      break;
    case TASK_ARRIVAL:
      //Unblocks
      it = intervalls(e->task, e->time);
      if (it > 0) {
	pdf = pdf_intervalls(e->task, it, tolerance);
	cdf = cdf_intervalls(e->task, pdf);
	stats_print_int(l, e->time, e->task, INTERVALL_TIME, it, pdf, cdf);
      }
      break;
    case TASK_SCHEDULE:
      //Dispatched
      start_execution(e->task, e->time);
      break;
    case TASK_NAME:
      //Create
      create_task(e->task, e->time);
      break;
    default:
      fprintf(stderr, "Unknown event %d\n", e->type);
  }

  if (e->time - time_back >= TIME_PERC) {
    time_back = e->time;
    calculateCPUsUtil(l, TIME_PERC);
  }
}

int main(int argc, char *argv[])
{
  FILE *f;
  int res, done;
  char *fname;

  l = stdout;
  fname = argv[argc - 1];
  f = fopen(fname, "r");
  if (f == NULL) {
    perror(fname);

    return -1;
  }

  done = 0;
  while (!done) {
    res = trace_read_event(f, start_time, end_time);
    done = feof(f) || (res < 0);
  }

  done = 0;
  while (!done) {
    struct event *e;

    e = evt_get();
    if (e == NULL) {
      done = 1;
    } else {
      if (e->task > 0)
	stats_event_handle(e);
    }
  }

  return 0;
}
