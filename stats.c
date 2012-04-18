#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include "event.h"
#include "event_list.h"
#include "trace_evt_handle.h"
#include "stats_utils.h"

#define TIME_PERC 1000000

int main(int argc, char *argv[])
{
    FILE *f;
    int res, done;
    int start_time = 0, end_time = 0;
    char *fname;

    unsigned int tolerance = 0, time_back;
    FILE *l = NULL;


l = fopen("test.txt", "w");

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

    time_back = 0;
    done = 0;
    while(!done) {
      struct event *e;
      unsigned long int instant, pdf;
      double cdf;

      e = evt_get();

      switch (e->type) {
        case TASK_END:
          //Blocks
          instant = response_time(e->task, e->time);
          pdf = pdf_response_time(e->task, instant, tolerance);
          cdf = cdf_response_time(e->task, pdf);
          encod_stats_time(l, e->time, e->task, RESPONSE_TIME, instant, pdf, cdf);
          blocks_task(e->task);
          break;
        case TASK_DESCHEDULE:
          //Deschedule
          instant = end_execution(e->task, e->time);
          if (instant != 0) {
            pdf = pdf_executions(e->task, instant, tolerance);
            cdf = cdf_executions(e->task, pdf);
            encod_stats_time(l, e->time, e->task, EXECUTION_TIME, instant, pdf, cdf);
          }
          break;
        case TASK_ARRIVAL:
          //Unblocks
          instant = intervalls(e->task, e->time);
          if (instant == 0) {	//first time no interval
            encod_stats_time(l, e->time, e->task, INTERVALL_TIME, 0, 0, 0);
          } else {
            pdf = pdf_intervalls(e->task, instant, tolerance);
            cdf = cdf_intervalls(e->task, pdf);
            encod_stats_time(l, e->time, e->task, INTERVALL_TIME, instant, pdf, cdf);
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
      }

      if (e->time - time_back >= TIME_PERC) {
        time_back = e->time;
        calculateCPUsUtil(l, TIME_PERC);
      }
    }

    return 0;
}
