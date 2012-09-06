#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "event.h"
#include "period_detect.h"

#define TRANSF_ARITHMETIC 0
#define TRANSF_GEOMETRIC  1

#define ALLOCATION_HUNK 1024
#define MAX_TASKS 1024

struct task {
  int pid;
  int *wakeup_times;
  int events;
  int table_size;
};

static struct task tasks[MAX_TASKS];	//FIXME: Why MAX_TASKS? Make it configurable!

static struct task *task_find(int pid)
{
  int i;

  for (i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].pid == pid) {
      return &tasks[i];
    }
    if (tasks[i].pid == 0) {
      return NULL;
    }
  }

  return NULL;
}

static struct task *task_new(int pid)
{
  int i;

  for (i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].pid == 0) {
      tasks[i].pid = pid;

      return &tasks[i];
    }
  }

  return NULL;
}

static void task_reset(int i)
{
  tasks[i].events = 0;
}

static void compute(double val, double f, double *re, double *im)
{
  *re += cos(2 * M_PI * f * val / 1000000.0);
  *im += sin(2 * M_PI * f * val / 1000000.0);
}

static void transf_step(int *wakeup_times, int events, double f, double *re, double *im)
{
  int i;

  for (i = 0; i < events; i++) {
    compute(wakeup_times[i], f, re, im);
  }
}

static int period_detect(double *transf, const double freq_max, const double freq_min, const double freq_delta)
{
  int i;
  int max_i = 0;
  double avg = transf[0];
  int transf_size = (freq_max - freq_min) / freq_delta + 1;
  
  for (i = 1; i < transf_size; i++) {
    avg += transf[i];
    if (transf[i] > transf[max_i])
      max_i = i;
  }
  
  avg /= transf_size;
  if (avg < 0.5 * transf[max_i]) {
    double f = freq_min + freq_delta * max_i;
    int period = 1000000.0 / f;

    return period;
  }

  fprintf(stderr, "Aperiodic application detected");

  return 0;
}

static int period_detect_new(double *transf, const double freq_max, const double freq_min, const double freq_delta, double n_avg, int k_max, int type)
{
  const double freq_eps = 1.0;
  int i = 0;
  double max_acc = 0.0;
  double max_f = 0.0;
  int period = 0;
  double avg = 0.0;
  int transf_size = (freq_max - freq_min) / freq_delta + 1;

  for (i = 0; i < transf_size; i++) {
    avg += transf[i];
  }
  avg = avg / transf_size;

  for (i = 1; i < transf_size - 1; i++) {
    if ((transf[i - 1] < transf[i])  &&
        (transf[i] > transf[ i + 1]) &&
        (transf[i] > n_avg * avg)) {
      /* Local maximum detected: search for multiples */
      double acc = type == TRANSF_GEOMETRIC ? 1.0 : 0.0;
      double f_i = freq_min + freq_delta * i;
      double f_j = f_i;
      int k = 0;

      do {
        double g;

          for (g = f_j - freq_eps; g <= f_j + freq_eps; g += freq_delta) {
            int j = round((g - freq_min) / freq_delta);

            if (j >= 0 && j < transf_size) {
              fprintf(stderr, "Accumulating transform of frequency %g (%g) onto frequency %g",
                      f_j, transf[j], f_i);
            if (type == TRANSF_GEOMETRIC) {
              acc *= transf[j];
            } else {
              acc += transf[j];
            }
          }
        }
        f_j += f_i;
        ++k;
      } while (f_j <= freq_max && (k_max == 0 || k < k_max));
      if (type == TRANSF_GEOMETRIC) {
        acc = pow(acc, 1.0/k);
      } else {
        acc = acc / k;
      }
      fprintf(stderr, "acc=%g (max_acc=%g, type=%s)", acc, max_acc,
              type == TRANSF_GEOMETRIC ? "geometric" : "arithmetic");
      if (acc > max_acc) {
        max_acc = acc;
        max_f = freq_min + freq_delta * i;
        period = 1000000.0 / max_f;
        fprintf(stderr, "New f_max=%g, acc=%g", max_f, max_acc);
      }
    }
  }
  fprintf(stderr, "Returning period=%d (freq=%g)", period, max_f);
  
  return period;
}



int pdetect_event_handle(const struct event *e)
{
  if (e->type == TASK_ARRIVAL) {
    struct task *t;

    t = task_find(e->task);
    if (t == NULL) {
      t = task_new(e->task);
    }
    if (t == NULL) { // Cannot happen!!!
      return -1;
    }

    if (t->events == t->table_size) {
      t->table_size += ALLOCATION_HUNK;
      t->wakeup_times = realloc(t->wakeup_times, sizeof(int) * t->table_size);
    }
    t->wakeup_times[t->events++] = e->time;

    return e->time;
  }

  return 0;
}

void pdetect_reset(void)
{
  int i;

  for (i = 0; i < MAX_TASKS; i++) {
    task_reset(i);
  }
}

int pdetect_period(int pid)
{
  int i;
  struct task *t;
  int transf_size;
  int period;
  double *power;
  const double freq_min = 10.0;
  const double freq_max = 400.0;
  const double freq_delta = 0.1;

  t = task_find(pid);
  if (t == NULL) {
    return -1;
  }

  transf_size = (freq_max - freq_min) / freq_delta + 1;
  power = malloc(sizeof(double) * transf_size);
  if (power == NULL) {
    return -2;
  }

  for (i = 0; i < transf_size; i++) {
    double re = 0.0;
    double im = 0.0;
    double f = freq_min + i * freq_delta;

    transf_step(t->wakeup_times, t->events, f, &re, &im);
    power[i] = sqrt(re * re + im * im);
  }

  period = period_detect(power, freq_max, freq_min, freq_delta);

  return period;
}
