/*
 * This is free software: see GPL.txt
 */
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
static int do_pmf;
static FILE *l;

static void help(const char *name)
{
  fprintf(stdout, "Usage:\n");
  fprintf(stdout, "%s [options] <input file>\n\n", name);

  fprintf(stdout, "Options:\n");
  fprintf(stdout, "-f <file> \tOutput file\n");
  fprintf(stdout, "-p \tGenerate PFMs as output\n");
  fprintf(stdout, "-s t\tStart time\n");
  fprintf(stdout, "-e t\tEnd time\n");
  exit(-1);
}

static unsigned int opts_parse(int argc, char *argv[])
{
  int c;

  while ((c = getopt(argc, argv, "p:s:e:f:")) != -1)
    switch (c) {
      case 'f':
	l = fopen(optarg, "w");
	break;
      case 'p':
	do_pmf = atoi(optarg);
	break;
      case 's':
	start_time = atoi(optarg);
	break;
      case 'e':
	end_time = atoi(optarg);
	break;
      default:
	help(argv[0]);
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
      pdf_response_time(e->task, r, tolerance);
      stats_print_int(l, e->time, e->task, RESPONSE_TIME, r);
      break;
    case TASK_DESCHEDULE:
      //Deschedule
      c = end_execution(e->task, e->time);
      if (c > 0) {
	pdf_executions(e->task, c, tolerance);
	stats_print_int(l, e->time, e->task, EXECUTION_TIME, c);
      }
      break;
    case TASK_ARRIVAL:
      //Unblocks
      it = intervalls(e->task, e->time);
      if (it > 0) {
	pdf_intervalls(e->task, it, tolerance);
	stats_print_int(l, e->time, e->task, INTERVALL_TIME, it);
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

static void stats_pmf_out(int pid)
{
  char fname[32];
  FILE *f;

  sprintf(fname, "%d-exec.txt", pid);
  f = fopen(fname, "w");
  pmf_write(f, pid, 2);
  fclose(f);
  sprintf(fname, "%d-interarrival.txt", pid);
  f = fopen(fname, "w");
  pmf_write(f, pid, 0);
  fclose(f);
  sprintf(fname, "%d-response.txt", pid);
  f = fopen(fname, "w");
  pmf_write(f, pid, 1);
  fclose(f);
}

int main(int argc, char *argv[])
{
  FILE *f;
  int res, done;
  char *fname;
  int first_parameter;

  first_parameter = opts_parse(argc, argv);
  if (!l) {
    l = stdout;
  }
  if (first_parameter < argc) {
    fname = argv[first_parameter];
    f = fopen(fname, "r");
    if (f == NULL) {
      perror(fname);

      return -1;
    }
  } else {
    f = stdin;
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

  if (do_pmf) {
    stats_pmf_out(do_pmf);
  }

  return 0;
}
