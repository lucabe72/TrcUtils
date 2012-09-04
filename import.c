/*
 * This is free software: see GPL.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <getopt.h>

#include "trace_read.h"
#include "ftrace_read.h"
#include "oftrace_read.h"
#include "jtrace_read.h"
#include "l4trace_read.h"
#include "event_create.h"
#include "trace_write.h"
#include "pid_filter.h"

#define FTRACE  0
#define JTRACE  1
#define OFTRACE 2
#define L4TRACE 3
#define TRCUTILS 4
#define XTRACE 5

static int trace_type;
static const char *relevant_pids;

static unsigned int param(int argc, char *argv[])
{
  int c;

  while ((c = getopt(argc, argv, "xtojfp:")) != -1)
    switch (c) {
      case 'x':
	trace_type = XTRACE;
	break;
      case 't':
	trace_type = TRCUTILS;
	break;
      case 'o':
	trace_type = OFTRACE;
	break;
      case 'j':
	trace_type = JTRACE;
	break;
      case 'f':
	trace_type = L4TRACE;
	break;
      case 'p':
	relevant_pids = optarg;
	break;
      default:
	exit(-1);
    }

  return optind;
}

#if 0
static void endAllTask(int time)
{
  unsigned int i;

  for (i = 0; i < count; i++) {
#if 0
    if (new_pids[i].state != 0) {	//1 - if it scheduled, it'll descheduled
      trc_force_desch(new_pids[i].pid, new_pids[i].cpu, time);
    } else {			//else it'll stoped
      trc_force_deactivation(new_pids[i].pid, new_pids[i].cpu, time);
    }
#else
    evt_force_deactivation(new_pids[i].pid, new_pids[i].cpu, time);
#endif
  }
}
#endif

int main(int argc, char *argv[])
{
  FILE *f;
  //int start = 0, fc = 0, fo = 0, opt, i;
  long long int time = 0, res = 0, done = 0;
  int first_param;

  first_param = param(argc, argv);
  if (argc - first_param < 1) {
    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);

    return -1;
  }

  createPidsFilter(relevant_pids);

  if (strcmp(argv[first_param], "-")) {
    f = fopen(argv[first_param], "r");
  } else {
    f = stdin;
  }
  if (f == NULL) {
    perror("Cannot open input file");

    return -1;
  }

  while (!done) {
    struct event *e;

    time = res + 1;
    switch (trace_type) {
      case FTRACE:
	res = ftrace_parse(f);
	break;
      case OFTRACE:
	res = oftrace_parse(f);
	break;
      case JTRACE:
	res = jtrace_read(f);
	break;
      case L4TRACE:
        res = l4trace_parse(f);
	break;
      case TRCUTILS:
	res = trace_read(f, 0);
	break;
      case XTRACE:
	res = trace_read(f, 1);
	break;
      default:
	fprintf(stderr,
		"Unknown trace type: this shouldn't have happened...\n");
	exit(-1);
    }
    while (e = evt_get()) {
      trc_write(e);
      free(e);
    }
    done = feof(f) || (res < 0);
  }

//    endAllTask(time);

  destroyPidsFilter();

  return 0;
}
