#include <stdlib.h>

#include "tasks.h"

struct task {
  unsigned int pid;
  int cpu;
  void *attributes;
};

struct task_set {
  struct task *tasks;
  unsigned int task_n;
};

struct task_set *taskset_init(void)
{
  struct task_set *p;

  p = malloc(sizeof(struct task_set));
  if (p == NULL) {
    return NULL;
  }

  p->task_n = 0;
  p->tasks = NULL;

  return p;
}

int taskset_add_task(struct task_set *p, unsigned int pid, int cpu, void *attr)
{
  struct task *old_tasks = p->tasks;

  p->tasks = realloc(p->tasks, sizeof(struct task) * (p->task_n + 1));
  if (p->tasks == NULL) {
    p->tasks = old_tasks;

    return -1;
  }

  p->tasks[p->task_n].pid = pid;
  p->tasks[p->task_n].cpu = cpu;
  p->tasks[p->task_n].attributes = attr;

  p->task_n++;

  return p->task_n;
}

void *taskset_find_task(const struct task_set *p, unsigned int pid, int cpu)
{
  unsigned int i;

  for (i = 0; i < p->task_n; i++) {
    if ((p->tasks[i].cpu == cpu) && (p->tasks[i].pid == pid)) {
      return p->tasks[i].attributes;
    }
  }

  return NULL;
}

void *taskset_nth_task(const struct task_set *p, unsigned int i, unsigned int *pid, int *cpu)
{
  if (i < p->task_n) {
    if (pid) {
      *pid = p->tasks[i].pid;
    }
    if (cpu) {
      *cpu = p->tasks[i].cpu;
    }

    return p->tasks[i].attributes;
  }

  return NULL;
}
