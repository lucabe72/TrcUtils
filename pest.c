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
#include "tasks.h"

struct task_stats {
  double avg;
  double m2;
  unsigned int samples;
};

static int print_all;
static int print_stats = 1;
static int analysis_period = 200000;
static struct task_set *ts;

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

  while ((c = getopt(argc, argv, "P:vxh")) != -1)
    switch (c) {
      case 'P':
	analysis_period = atoi(optarg);
	break;
      case 'v':
        print_all = 1;
        break;
      case 'x':
        print_stats = 0;
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

void period_add(struct task_stats *s, int p)
{
  double delta;

  if (s->avg < 0) return;
  if (p == 0) {
    if (s->avg) s->avg = -1;

    return;
  }

  s->samples++;
  delta = (p - s->avg);
  s->avg = s->avg + delta / s->samples;
  s->m2  = s->m2  + delta * (p - s->avg);
}

static void do_period_estimation(unsigned int pid)
{
  int p;
  struct task_stats *s;

  if (ts == NULL) {
    ts = taskset_init();
  }

  p = pdetect_period(pid);
  if (print_all) printf("%d Estimated period: %d\n", pid, p);
  s = taskset_find_task(ts, pid, -1);
  if (s == NULL) {
    if (p == 0) {
      /* If the task is not periodic and has never been detected as periodic, skip it! */
      return;
    }
    s = malloc(sizeof(struct task_stats));
    if (s == NULL) {
      perror("MAlloc failed!\n");
      return;
    }
    memset(s, 0, sizeof(struct task_stats));
    taskset_add_task(ts, pid, -1, s);
  }
  period_add(s, p);
  //printf("%d Estimated period: %d\n", pid, p);
}

static void print_results(void)
{
  unsigned int i = 0, pid;
  struct task_stats *p;

  if (ts == NULL) {
    printf("The taskset structure does not exist...\n");
    printf("Maybe do_period_estimation() has never been called?\n");
    printf("(the analysis_period is shorter than the trace length)\n");

    return;
  }
  while ((p = taskset_nth_task(ts, i++, &pid, NULL))) {
    if (p->avg > 0) {
      double variance;

      variance = p->m2 / (p->samples - 1);
      printf("Task %d is periodic: %lf %lf\n", pid, p->avg, variance);
    }
  }
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

  if (print_stats) print_results();

  return 0;
}
