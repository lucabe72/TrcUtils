/*
 * This is free software: see GPL.txt
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stats_utils.h"

struct trace {
  int name, blocked, cpus_util_size;
  long int start_execution;
  unsigned long int unblocks_time;
  struct record records[3];
  unsigned long int execution_total;
  unsigned long int execution_time;
  float *cpus_util;
  int p[3];
};

static struct trace *tasks;
static int size_tasks;

static int containsTask(int pid)
{
  int i;

  for (i = 0; i < size_tasks; i++) {
    if (tasks[i].name == pid) {
      return i;
    }
  }
  return -1;
}

static void utilisation_print(void *l, unsigned long int time, int task,
		       float cpu, float cpuw, float cpua)
{
  if (l == NULL) {
    return;
  }
  //fprintf(l, "%ld %d %s %d %f %f %f\n", time, task, getTaskName(task), cpu_stat, cpu, cpuw, cpua);
  fprintf(l, "%ld %d util %f %f %f\n", time, task, cpu, cpuw, cpua);
}

struct record *record_find(int pid)
{
  int i = containsTask(pid);

  if (i < 0) {
    return NULL;
  }

  return tasks[i].records;
}

void task_set_p(int pid, int type)
{
  int i = containsTask(pid);

  if (i < 0) {
    return;
  }

  tasks[i].p[type] = 1;
}

void task_unset_p(int pid, int type)
{
  int i = containsTask(pid);

  if (i < 0) {
    return;
  }

  tasks[i].p[type] = 0;
}

void start_execution(int pid, unsigned long int time)
{
  int i = containsTask(pid);

  if (i >= 0) {
    if (tasks[i].start_execution >= 0) {
      fprintf(stderr,
	      "Scheduling task %d with last scheduling time = %ld!!!\n",
	      pid, tasks[i].start_execution);
      exit(-1);
    }
    tasks[i].start_execution = time;
  }
}

unsigned long int end_execution(int pid, unsigned long int time)
{
  int i = containsTask(pid);
  unsigned long int result;

  if (i >= 0) {
    result = time - tasks[i].start_execution;
    tasks[i].start_execution = -1;
    tasks[i].execution_total += result;
    tasks[i].execution_time += result;

    if (tasks[i].blocked == 1) {
      result = tasks[i].execution_time;

      return result;
    }

    return 0;
  }

  abort();
  return -1;
}

unsigned long int intervalls(int pid, unsigned long int time)
{
  int i = containsTask(pid);
  unsigned long int result;

  if (i >= 0) {
    if (tasks[i].blocked == 0) {
      return 0;
    }
    result = time - tasks[i].unblocks_time;
    tasks[i].unblocks_time = time;
    tasks[i].execution_time = 0;
    tasks[i].blocked = 0;
    //first time 0, No interval
    return result;
  }

  abort();
  return -1;
}

unsigned long int response_time(int pid, unsigned long int time)
{
  int i = containsTask(pid);

  if (i >= 0) {
    tasks[i].blocked = 1;
    return time - tasks[i].unblocks_time;
  }

  abort();
  return -1;
}

void create_task(int pid, unsigned long int time)
{
  if (containsTask(pid) == -1) {

    tasks =
	(struct trace *) realloc(tasks,
				 ++size_tasks * sizeof(struct trace));
    tasks[size_tasks - 1].name = pid;

    tasks[size_tasks - 1].p[0] = 0;
    tasks[size_tasks - 1].p[1] = 0;
    tasks[size_tasks - 1].p[2] = 0;

    tasks[size_tasks - 1].unblocks_time = time;
    tasks[size_tasks - 1].records[0].size = 0;
    tasks[size_tasks - 1].records[0].entries = NULL;
    tasks[size_tasks - 1].records[1].size = 0;
    tasks[size_tasks - 1].records[1].entries = NULL;
    tasks[size_tasks - 1].records[2].size = 0;
    tasks[size_tasks - 1].records[2].entries = NULL;

    tasks[size_tasks - 1].start_execution = -1;

    tasks[size_tasks - 1].execution_total = 0;
    tasks[size_tasks - 1].execution_time = 0;
    tasks[size_tasks - 1].cpus_util = NULL;
    tasks[size_tasks - 1].cpus_util_size = 0;

    tasks[size_tasks - 1].blocked = 0;
  }
}

void calculateCPUsUtil(void *l, unsigned long int time)
{
  int i;

  for (i = 0; i < size_tasks; i++) {
    float t = tasks[i].execution_total, a = 0, w = 100;
    int j;

    tasks[i].execution_total = 0;
    t = (t / time) * 100.0;
    printf("Util %d (%d) = %f\n", i, tasks[i].name, t);
    tasks[i].cpus_util =
	(float *) realloc(tasks[i].cpus_util,
			  ++tasks[i].cpus_util_size * sizeof(float));
    (tasks[i].cpus_util)[tasks[i].cpus_util_size - 1] = t;

    for (j = 0; j < tasks[i].cpus_util_size; j++) {
      a += (tasks[i].cpus_util)[j];
      if ((tasks[i].cpus_util)[j] < w) {
	w = (tasks[i].cpus_util)[j];
      }
    }
    a /= tasks[i].cpus_util_size;
    //encod_stats_cpu(l, time, tasks[i].name, CPU_UTILIZ, t, w, a);
    utilisation_print(l, time, tasks[i].name, t, w, a);
  }
}


