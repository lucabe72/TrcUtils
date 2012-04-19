#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stats_utils.h"

struct record {		//FIXME!!!
  int size;
  struct record_entry *entries;
};

struct trace {
    int name, executions_size, blocks, cpus_util_size;
    char description[16];
    unsigned long int sched, number, pree, unblocks_time, start_execution,
	*executions;
    struct record records[3];
    unsigned long int execution_total;
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

static void encod_stats_cpu(void *l, unsigned long int time, int task, int cpu_stat, float cpu, float cpuw, float cpua)
{
  if (l == NULL) {
    return;
  }

    //fprintf(l, "%ld %d %s %d %f %f %f\n", time, task, getTaskName(task), cpu_stat, cpu, cpuw, cpua);
  fprintf(l, "%ld %d %d %f %f %f\n", time, task, cpu_stat, cpu, cpuw, cpua);
}


static unsigned long int execution_time(int pid)
{
    int i, found = containsTask(pid);
    unsigned long int result = 0;

    for (i = 0; i < tasks[found].executions_size; i++) {
        result += (tasks[found].executions)[i];
    }
    free(tasks[found].executions);
    tasks[found].executions_size = 0;
    tasks[found].executions = NULL;

    return result;
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
    int found = containsTask(pid);
    tasks[found].start_execution = time;
    tasks[found].sched++;
}

unsigned long int end_execution(int pid, unsigned long int time)
{
    int found = containsTask(pid);
    unsigned long int result;
    result = time - tasks[found].start_execution;

    if (result > 0) {
	tasks[found].execution_total += result;
    }

    tasks[found].executions = realloc(tasks[found].executions, ++tasks[found].executions_size * sizeof(int));
    tasks[found].executions[tasks[found].executions_size - 1] = result;
    if (tasks[found].blocks == 1) {
        result = execution_time(pid);
        tasks[containsTask(pid)].blocks = 0;

        return result;
    } else {
        tasks[found].pree++;
        return 0;
    }
}

unsigned long int intervalls(int pid, unsigned long int time)
{
    int found = containsTask(pid);
    unsigned long int result;
    result = time - tasks[found].unblocks_time;
    tasks[found].unblocks_time = time;
    tasks[found].number++;
    //first time 0, No interval
    return result;
}

void blocks_task(int pid)
{
    tasks[containsTask(pid)].blocks = 1;
}

unsigned long int response_time(int pid, unsigned long int time)
{
    int found = containsTask(pid);
    return time - tasks[found].unblocks_time;
}

void create_task(int pid, unsigned long int time)
{
    if (containsTask(pid) == -1) {

	tasks =
	    (struct trace *) realloc(tasks,
				     ++size_tasks * sizeof(struct trace));
	tasks[size_tasks - 1].name = pid;
	tasks[size_tasks - 1].number = 0;
	tasks[size_tasks - 1].sched = 0;
	tasks[size_tasks - 1].pree = 0;

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

	tasks[size_tasks - 1].start_execution = 0;
	tasks[size_tasks - 1].executions_size = 0;
	tasks[size_tasks - 1].executions = NULL;

	tasks[size_tasks - 1].execution_total = 0;
	tasks[size_tasks - 1].cpus_util = NULL;
	tasks[size_tasks - 1].cpus_util_size = 0;

	tasks[size_tasks - 1].blocks = 0;
    }
}

void calculateCPUsUtil(void *l, unsigned long int time)
{
    int i, j;
    for (i = 0; i < size_tasks; i++) {
	float t = tasks[i].execution_total, a = 0, w = 100;
	tasks[i].execution_total = 0;
	t = (t / time) * 100.0;
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
	encod_stats_cpu(l, time, tasks[i].name, CPU_UTILIZ, t, w, a);
    }
}
