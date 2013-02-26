#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "tasks.h"
#include "event.h"
#include "period_detect.h"

#define TRANSF_ARITHMETIC 0
#define TRANSF_GEOMETRIC  1

#define ALLOCATION_HUNK 1024
#define MAX_TASKS 1024

struct task_stats {
  int *wakeup_times;
  int events;
  int table_size;
};

static struct task_set *ts;

static struct task_stats *task_new(int pid)
{
  struct task_stats *t;

  t = malloc(sizeof(struct task_stats));
  if (t) {
    memset(t, 0, sizeof(struct task_stats));
    taskset_add_task(ts, pid, -1, t);
  }

  return t;
}

static void task_reset(struct task_stats *t)
{
  t->events = 0;
}

static void compute(double val, double f, double *re, double *im)
{
  *re += cos(2 * M_PI * f * val / 1000000.0);
  *im += sin(2 * M_PI * f * val / 1000000.0);
}

static void transf_step(int *wakeup_times, int events, double f, double *re, double *im)
{
  int i;

  *re = 0;
  *im = 0;
  for (i = 0; i < events; i++) {
    compute(wakeup_times[i], f, re, im);
  }
}

static int period_detect_old(double *transf, const double freq_max, const double freq_min, const double freq_delta)
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

  fprintf(stdout, "Aperiodic application detected\n");

  return 0;
}

static int blacklist(int i, int imax, int j, int d)
{
  int dd = (i + j) % (imax + j);

//printf("BL: %d %d %d |||| %d %d < %d\n", imax, ((i + j) / (imax + j) > 1), ((i + j) % (imax + j) < d), i + j, imax + j, d);
  return imax && ((i + j) / (imax + j) > 1) && ((dd < d) || (dd > (imax + j - d)));
//((max_i == 0) || (i / max_i > 1) || ((i % max_i) > (freq_eps / freq_delta))) &&
}

static int period_detect(double *transf, const double freq_max, const double freq_min, const double freq_delta)
{
  const double freq_eps = 1.0;
  int i = 0, max_i = 0;
  double max_acc = 0.0;
  double max_f = 0.0;
  int period = 0;
  double avg = 0.0;
  int transf_size = (freq_max - freq_min) / freq_delta + 1;
  /// Set to 0.0 to disable
  const double n_avg = 2.9;
  /// Set to 0 to disable
  const int k_max = 10;
  /// Set to either TRANSF_ARITHMETIC or TRANSF_GEOMETRIC
  const int type = TRANSF_GEOMETRIC;

  for (i = 0; i < transf_size; i++) {
    avg += transf[i];
  }
  avg = avg / transf_size;

  for (i = 1; i < transf_size / 2; i++) {
//printf("I: %d Imax: %d\n", i, max_i);
    if (!blacklist(i, max_i, /*freq_min / freq_delta*/ 50, /*freq_eps / freq_delta*/ 50) &&
        (transf[i - 1] < transf[i])  &&
        (transf[i] > transf[ i + 1]) &&
        (transf[i] > n_avg * avg)) {
      /* Local maximum detected: search for multiples */
      double acc = type == TRANSF_GEOMETRIC ? 1.0 : 0.0;
      double f_i = freq_min + freq_delta * i;
      double f_j = f_i;
      int k = 0;

//printf("Local Maximum at %lf %d\n", f_i, i);
      do {
        double g;

          for (g = f_j - freq_eps; g <= f_j + freq_eps; g += freq_delta) {
            int j = round((g - freq_min) / freq_delta);

            if (j >= 0 && j < transf_size) {
//              fprintf(stderr, "Accumulating transform of frequency %g (%g) onto frequency %g", f_j, transf[j], f_i);
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
//      fprintf(stderr, "acc=%g (max_acc=%g, type=%s)", acc, max_acc, type == TRANSF_GEOMETRIC ? "geometric" : "arithmetic");
      if (acc > max_acc) {
        max_acc = acc;
        max_f = freq_min + freq_delta * i;
        max_i = i;
        period = 1000000.0 / max_f;
//        fprintf(stderr, "New f_max=%g, acc=%g", max_f, max_acc);
      }
    }
  }
//  fprintf(stderr, "Returning period=%d (freq=%g)", period, max_f);
  
  return period;
}

static void transf_print(double *power, double freq_max, double freq_min, double freq_delta)
{
  FILE *file; static int id; char fname[16];
  int i;
  int size = (freq_max - freq_min) / freq_delta + 1;

  sprintf(fname, "t%d.txt", id++);
  file = fopen(fname, "w");
  for (i = 0; i < size; i++) {
    double f = freq_min + i * freq_delta;

    fprintf(file, "%f %f\n", f, power[i]);
  }
  fclose(file);
}

int pdetect_event_handle(const struct event *e)
{
  if (e->type == TASK_ARRIVAL) {
    struct task_stats *t;

    if (ts == NULL) {
      ts = taskset_init();
    }
//fprintf(stdout, "Inserting task %d w %d\n", e->task, e->time);
    t = taskset_find_task(ts, e->task, -1);
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
  unsigned int i = 0;
  struct task_stats *t;

  while ((t = taskset_nth_task(ts, i++, NULL, NULL))) { 
    task_reset(t);
  }
}

int pdetect_period(int pid)
{
  int i;
  struct task_stats *t;
  int transf_size;
  int period;
  double *power;
  const double freq_min = 5.0;
  const double freq_max = 200.0;
  const double freq_delta = 0.1;

  if (ts == NULL) {
    return -3;
  }

  t = taskset_find_task(ts, pid, -1);
  if (t == NULL) {
    return -1;
  }

  transf_size = (freq_max - freq_min) / freq_delta + 1;
  power = malloc(sizeof(double) * transf_size);
  if (power == NULL) {
    return -2;
  }

#ifdef DEBUG
  printf("Activations: ");
  for (i = 0; i < t->events; i++) {
    printf("%d ", t->wakeup_times[i]);
  }
  printf("\n");
#endif /* DEBUG */
  for (i = 0; i < transf_size; i++) {
    double re, im;
    double f = freq_min + i * freq_delta;

    transf_step(t->wakeup_times, t->events, f, &re, &im);
    power[i] = sqrt(re * re + im * im);
  }
#ifdef DEBUG
  transf_print(power, freq_max, freq_min, freq_delta);
#endif /* DEBUG */

  period = period_detect(power, freq_max, freq_min, freq_delta);
  free(power);

  return period;
}

int pid_get(int i)
{
  unsigned int pid;
  struct task_stats *t = taskset_nth_task(ts, i, &pid, NULL);

  if (t == NULL) {
    return -1;
  }
  if (t->events) {
    return pid;
  }

  return 0;
}
