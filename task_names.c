/*
 * This is free software: see GPL.txt
 */
#include <stdlib.h>
#include <string.h>

#include "tasks.h"
#include "task_names.h"

static struct task_set *ts;

static void task_name_init(void)
{
  ts = taskset_init();
}

static void task_name_check(void)
{
  if (ts == NULL) {
    task_name_init();
  }
}

void name_register(int pid, int cpu, const char *name)
{
  task_name_check();
  taskset_add_task(ts, pid, cpu, strdup(name));
}

const char *name_get(int pid, int cpu)
{
  task_name_check();
  return taskset_find_task(ts, pid, cpu);
}

int task_ith(unsigned int i, int cpu)
{
  unsigned int j = 0, cnt = 0, done = 0;

  task_name_check();
  while (!done) {
    unsigned int pid;
    int task_cpu;
    const char *c = taskset_nth_task(ts, j, &pid, &task_cpu);

    if (c) {
      if (task_cpu == cpu) {
        if (cnt++ == i) {
	  return pid;
        }
      }
      j++;
    } else {
      done = 1;
    }
  }

  return -1;
}
