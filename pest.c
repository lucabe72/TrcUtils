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

static int analysis_period = 100;

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

int main(int argc, char *argv[])
{
  FILE *f;
  int res, done, told;
  char *fname;
  int first_parameter, pid;

  first_parameter = opts_parse(argc, argv);
  if (first_parameter < argc - 1) {
    fname = argv[first_parameter];
    f = fopen(fname, "r");
    if (f == NULL) {
      perror(fname);

      return -1;
    }
    pid = atoi(argv[first_parameter + 1]);
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
        int p;

printf("Computing... %d %d | %d\n", t, told, t - told);

        told = t;
        p = pdetect_period(pid);
        pdetect_reset();
        printf("%d Estimated period: %d\n", t, p);
      }
      free(e);
    }
    done = feof(f) || (res < 0);
  }

  return 0;
}
