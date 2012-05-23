/*
 * This is free software: see GPL.txt
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "event.h"
#include "event_list.h"
#include "trace_evt_handle.h"
#include "stats_utils.h"
#include "stats_events.h"
#include "visual_gui.h"

#define TIME_PERC 1000000

static unsigned int tolerance;

static unsigned int opts_parse(int argc, char *argv[])
{
  int c;

  while ((c = getopt(argc, argv, "t:")) != -1)
    switch (c) {
      case 't':
	tolerance = atoi(optarg);
	break;
      default:
	exit(-1);
    }

  return optind;
}

static void stats_event_handle(const struct event *e)
{
  unsigned long int r, c, it;
  static unsigned int time_back;

  switch (e->type) {
    case TASK_END:
      //Blocks
      r = response_time(e->task, e->time);
      updateRT(e->task, r);
      break;
    case TASK_DESCHEDULE:
      //Deschedule
      c = end_execution(e->task, e->time);
      if (c > 0) {
        updateET(e->task, c);
      }
      break;
    case TASK_ARRIVAL:
      //Unblocks
      it = intervalls(e->task, e->time);
      if (it > 0) {
        updateI(e->task, it);
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
    //calculateCPUsUtil(l, TIME_PERC);
  }
}

int main(int argc, char *argv[])
{
  FILE *f;
  int res, done;
  char *fname;
  int first_parameter;

  first_parameter = opts_parse(argc, argv);
  fname = argv[first_parameter];
  f = fopen(fname, "r");
  if (f == NULL) {
    perror(fname);

    return -1;
  }

  initializeT(tolerance);
  done = 0;
  while (!done) {
    struct event *e;

    res = trace_read_event(f, 0, 0);
    while ((e = evt_get())) {
      if (e->task > 0)
	stats_event_handle(e);
      free(e);
    }
    done = feof(f) || (res < 0);
    if (suEzo() == 'q') done = 1;
  }

  exitT();

  return 0;
}
