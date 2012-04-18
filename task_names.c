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
  int i = 0;

  while (i < MAX_TASKS) {
    if (task[i].name == NULL) {
      task[i].name = strdup(name);
      task[i].pid = pid;
      task[i].cpu = cpu;

      return;
    }
    i++;
  }
}

const char *name_get(int pid, int cpu)
{
  int i = 0;

  while (i < MAX_TASKS) {
    if (task[i].name && (task[i].cpu == cpu) && (task[i].pid == pid)) {
      return task[i].name;
    }
    i++;
  }

  return NULL;
}
