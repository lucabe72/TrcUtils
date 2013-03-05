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
  int *periods;
  int n_samples;
};

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

static void period_add(struct task_stats *s, int period)
{
  int *tmp = s->periods;

  s->periods = realloc(s->periods, sizeof(int) * (s->n_samples + 1));
  if (s->periods == NULL) {
    s->periods = tmp;
    return;
  }
  s->periods[s->n_samples] = period;
  s->n_samples++;
}

static void do_period_estimation(unsigned int pid)
{
  int p;
  struct task_stats *s;

  if (ts == NULL) {
    ts = taskset_init();
  }

  p = pdetect_period(pid);
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

  while ((p = taskset_nth_task(ts, i++, &pid, NULL))) {
    int j;
    double sum, sum2;

    sum = 0; sum2 = 0;
    for (j = 0; j < p->n_samples; j++) {
      if (p->periods[j] == 0) {
        sum = -1;
        j = p->n_samples;
      } else {
        sum += p->periods[j];
        sum2 += (double)p->periods[j] * (double)p->periods[j];
      }
    }
    if (sum > 0) {
      printf("Task %d is periodic: %lf %lf\n", pid, sum / p->n_samples, sum2 / (p->n_samples - 1) - (sum / p->n_samples) * (sum / (p->n_samples - 1)));
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

  print_results();

  return 0;
}
