/*
 * This is free software: see GPL.txt
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "event_list.h"
#include "trace_evt_handle.h"
#include "period_detect.h"

static int analysis_period = 200000;

static void help(const char *name)
{
  fprintf(stdout, "Usage:\n");
  fprintf(stdout, "%s [options] <input file> <pid>\n\n", name);

  fprintf(stdout, "-h t\tHelp\n");
  exit(-1);
}

static unsigned int opts_parse(int argc, char *argv[])
{
  int c;

  while ((c = getopt(argc, argv, "P:h")) != -1)
    switch (c) {
      case 'P':
	analysis_period = atoi(optarg);
	break;
      case 'h':
	help(argv[0]);
	break;
      default:
        fprintf(stderr, "Unknown option %c\n", c);
        exit(-1);
    }

  return optind;
}

static void do_period_estimation(unsigned int pid)
{
  int p;

  p = pdetect_period(pid);
  printf("%d Estimated period: %d\n", pid, p);
}

int main(int argc, char *argv[])
{
  FILE *f;
  int res, done, told;
  char *fname;
  int first_parameter, pid;

  first_parameter = opts_parse(argc, argv);
  if (first_parameter < argc) {
    fname = argv[first_parameter];
    f = fopen(fname, "r");
    if (f == NULL) {
      perror(fname);

      return -1;
    }
    if (first_parameter < argc - 1) {
      pid = atoi(argv[first_parameter + 1]);
    } else {
      pid = 0;
    }
  } else {
    help(argv[0]);
  }

  done = 0; told = 0;
  while (!done) {
    struct event *e;

    res = trace_read_event(f, 0, 0);
    while ((e = evt_get())) {
      int t;

      t = pdetect_event_handle(e);
      if (t - told > analysis_period) {
        told = t;
        if (pid) {
          do_period_estimation(pid);
        } else {
          int j, todo;

          todo = 1;
          j = 0;
          while(todo >= 0) {
            todo = pid_get(j++);
            if (todo > 0) {
              do_period_estimation(todo);
            }
          }
        }
        pdetect_reset();
      }
      free(e);
    }
    done = feof(f) || (res < 0);
  }

  return 0;
}
