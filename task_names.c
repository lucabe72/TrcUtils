/*
 * This is free software: see GPL.txt
 */
#include <stdlib.h>
#include <string.h>

#include "task_names.h"

#define MAX_TASKS 1024

struct task {
  char *name;
  int cpu;
  int pid;
};

static struct task task[MAX_TASKS];

void name_register(int pid, int cpu, const char *name)
{
  int i;

  for (i = 0; i < MAX_TASKS; i++) {
    if (task[i].name == NULL) {
      task[i].name = strdup(name);
      task[i].pid = pid;
      task[i].cpu = cpu;

      return;
    }
  }
}

const char *name_get(int pid, int cpu)
{
  int i;

  for (i = 0; i < MAX_TASKS; i++) {
    if (task[i].name && (task[i].cpu == cpu) && (task[i].pid == pid)) {
      return task[i].name;
    }
  }

  return NULL;
}

int task_ith(int i, int cpu)
{
  int j, cnt = 0;

  for (j = 0; j < MAX_TASKS; j++) {
    if (task[j].name && (task[j].cpu == cpu)) {
      if (cnt++ == i) {
	return task[j].pid;
      }
    }
  }

  return -1;
}
