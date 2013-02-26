/*
 * This is free software: see GPL.txt
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tasks.h"
#include "stats_events.h"

struct task_stats {
  int blocked, cpus_util_size;
  long int start_execution;
  unsigned long int unblocks_time;
  struct record *records[3];
  unsigned long int execution_total;
  unsigned long int execution_time;
  float *cpus_util;
  int p[3];
};

static struct task_set *ts;

static void utilisation_print(void *l, unsigned long int time, int task,
		       float cpu, float cpuw, float cpua)
{
  if (l == NULL) {
    return;
  }
  //fprintf(l, "%ld %d %s %d %f %f %f\n", time, task, getTaskName(task), cpu_stat, cpu, cpuw, cpua);
  fprintf(l, "%ld %d util %f %f %f\n", time, task, cpu, cpuw, cpua);
}

struct record **record_find(int pid)
{
  struct task_stats *stat = taskset_find_task(ts, pid, -1);

  if (stat == NULL) {
    return NULL;
  }

  return stat->records;
}

void task_set_p(int pid, int type)
{
  struct task_stats *stat = taskset_find_task(ts, pid, -1);

  if (stat == NULL) {
    return;
  }

  stat->p[type] = 1;
}

void task_unset_p(int pid, int type)
{
  struct task_stats *stat = taskset_find_task(ts, pid, -1);

  if (stat == NULL) {
    return;
  }

  stat->p[type] = 0;
}

void start_execution(int pid, unsigned long int time)
{
  struct task_stats *stat = taskset_find_task(ts, pid, -1);

  if (stat) {
    if (stat->start_execution >= 0) {
      fprintf(stderr,
	      "Scheduling task %d with last scheduling time = %ld!!!\n",
	      pid, stat->start_execution);
      exit(-1);
    }
    stat->start_execution = time;
  }
}

unsigned long int end_execution(int pid, unsigned long int time)
{
  struct task_stats *stat = taskset_find_task(ts, pid, -1);
  unsigned long int result;

  if (stat) {
    result = time - stat->start_execution;
    stat->start_execution = -1;
    stat->execution_total += result;
    stat->execution_time += result;

    if (stat->blocked == 1) {
      result = stat->execution_time;

      return result;
    }

    return 0;
  }

  abort();
  return -1;
}

unsigned long int intervalls(int pid, unsigned long int time)
{
  struct task_stats *stat = taskset_find_task(ts, pid, -1);
  unsigned long int result;

  if (stat) {
    if (stat->blocked == 0) {
      return 0;
    }
    result = time - stat->unblocks_time;
    stat->unblocks_time = time;
    stat->execution_time = 0;
    stat->blocked = 0;
    //first time 0, No interval
    return result;
  }

  abort();
  return -1;
}

unsigned long int response_time(int pid, unsigned long int time)
{
  struct task_stats *stat = taskset_find_task(ts, pid, -1);

  if (stat) {
    stat->blocked = 1;
    return time - stat->unblocks_time;
  }

  abort();
  return -1;
}

int create_task(int pid, unsigned long int time)
{
  struct task_stats *stat = taskset_find_task(ts, pid, -1);

  if (stat == NULL) {
    stat = malloc(sizeof(struct task_stats));
    if (stat == NULL) {
      return -1;
    }
    stat->p[0] = 0;
    stat->p[1] = 0;
    stat->p[2] = 0;

    stat->unblocks_time = time;
    stat->records[0] = NULL;
    stat->records[1] = NULL;
    stat->records[2] = NULL;

    stat->start_execution = -1;

    stat->execution_total = 0;
    stat->execution_time = 0;
    stat->cpus_util = NULL;
    stat->cpus_util_size = 0;

    stat->blocked = 0;
    taskset_add_task(ts, pid, -1, stat);
  }

  return 0;
}

void calculateCPUsUtil(void *l, unsigned long int time)
{
  int i = 0;
  int done = 0;

  while (!done) {
    unsigned int pid;
    struct task_stats *stat = taskset_nth_task(ts, i, &pid, NULL);

    if (stat) {
      float t = stat->execution_total, a = 0, w = 100;
      int j;

      stat->execution_total = 0;
      t = (t / time) * 100.0;
      printf("Util %d (%d) = %f\n", i, pid, t);
      stat->cpus_util =
	(float *) realloc(stat->cpus_util,
			  ++stat->cpus_util_size * sizeof(float));
      stat->cpus_util[stat->cpus_util_size - 1] = t;

      for (j = 0; j < stat->cpus_util_size; j++) {
        a += (stat->cpus_util)[j];
        if ((stat->cpus_util)[j] < w) {
          w = (stat->cpus_util)[j];
        }
      }
      a /= stat->cpus_util_size;
    //encod_stats_cpu(l, time, tasks[i].name, CPU_UTILIZ, t, w, a);
      utilisation_print(l, time, pid, t, w, a);
      i++;
    } else {
      done = 1;
    }
  }
}

void stats_init(void)
{
  ts = taskset_init();
}
