#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include "event.h"
#include "event_list.h"
#include "trace_evt_handle.h"
#include "text_out.h"
#include "xfig_out.h"
#include "trace_write.h"
#include "jtrace_write.h"

#define MAX_EVENTS 10000

/* FIXME! */
#define OUTPUT_DUMP 1
#define OUTPUT_INFO 2
#define OUTPUT_TRACE 3
#define OUTPUT_RTSIM 4

static int output_type;
static int start_time, end_time;

static void help(void)
{
  fprintf(stdout, "Usage:\n");
  fprintf(stdout, "trc2fig <options> file\n\n");

  fprintf(stdout, "Options:\n");
  fprintf(stdout, "-C n\tMaximum number of CPU\n");
  fprintf(stdout, "-E n\tMaximum number of events\n");
  fprintf(stdout, "-S n\tMaximum number of servers\n");
  fprintf(stdout, "-s t\tStart time\n");
  fprintf(stdout, "-e t\tEnd Time\n");
}

static unsigned int param(int argc, char *argv[])
{
  int c;

  while ((c = getopt(argc, argv, "s:e:idtj")) != -1) {
    switch (c) {
      case 's':
	start_time = atoi(optarg);
	break;
      case 'e':
	end_time = atoi(optarg);
	break;
      case 'i':
	output_type = OUTPUT_INFO;
	break;
      case 'd':
	output_type = OUTPUT_DUMP;
	break;
      case 't':
	output_type = OUTPUT_TRACE;
	break;
      case 'j':
	output_type = OUTPUT_RTSIM;
	break;
      default:
	help();
	exit(2);
    }
  }

  return optind;
}

int main(int argc, char *argv[])
{
  FILE *f;
  unsigned int j, z, cpus;
  int done, i, last_server, last_server_tot = 0, first_arg;
  int step, scale;
  char *fname;
  struct event_trace *t;

  first_arg = param(argc, argv);

  if (first_arg >= argc) {
    fprintf(stderr, "Usage: %s [options] <input file>\n", argv[0]);
  }

  fname = argv[first_arg];
  f = fopen(fname, "r");
  if (f == NULL) {
    perror(fname);

    return -1;
  }

  done = 0;
  while (!done) {
    int res;

    res = trace_read_event(f, start_time, end_time);
    done = feof(f) || (res < 0);
  }


  step_compute(last_time(), &step, &scale);
  fprintf(stderr, "Step: %d\tScale: %d\n", step, scale);

  switch (output_type) {
    case OUTPUT_DUMP:
      done = 0;
      i = 0;
      while (!done) {
	struct event *e;

	e = evt_get();
	if (e == NULL) {
	  done = 1;
	} else {
	  printf("[%d - %d]\t", i++, e->cpu);
	  trace_dump_event(e);
	}
      }
      break;
    case OUTPUT_TRACE:
      done = 0;
      while (!done) {
	struct event *e;

	e = evt_get();
	if (e == NULL) {
	  done = 1;
	} else {
	  trc_write(e);
	}
      }
      break;
    case OUTPUT_RTSIM:
      done = 0;
      while (!done) {
	struct event *e;

	e = evt_get();
	if (e == NULL) {
	  done = 1;
	} else {
	  jtrace_write(e);
	}
      }
      break;
    case OUTPUT_INFO:
      t = trace_export(&cpus);

      for (j = 0; j < cpus; j++) {
	if (servers(j) > 1) {
	  trace_info(t[j].ev, t[j].last_event, servers(j));
	}
      }
      break;
    default:
      fprintf(stderr, "Printing...\n");
      fprintf(stderr,
	      "If you don't see a process: its beahviour is a sequence of events too far from time 0.\n");

      header_out();
      t = trace_export(&cpus);
      for (j = 0, z = 0; j < cpus; j++) {
	last_server = servers(j);
	if (last_server > 1) {	//last_server == 0 is idle
	  fprintf(stderr, "CPU %d - %d\n", z, last_server);
	  yax_draw(last_server, last_server_tot, z);
	  cpu_name(z, last_server_tot);

	  for (i = 0; i < last_server; i++) {
	    fprintf(stderr, "%d/%d \t%s\n", i, last_server - 1,
		    srv_name(srv_id(i, j), j));
	    ax_draw(start_time, last_time(), step, scale, i,
		    (i == (last_server - 1)), last_server_tot, z);
	    task_plot(t[j].ev, t[j].last_event, scale, srv_id(i, j), i,
		      last_server_tot, z, j, start_time);
	  }
	  last_server_tot += last_server;
	  z++;
	}
      }
  }

  return 0;
}
