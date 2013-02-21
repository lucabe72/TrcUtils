/*
 * This is free software: see GPL.txt
 */
#include <stdlib.h>

#include "event.h"
#include "event_list.h"

#define MAX_CPUS 8		//FIXME!

struct node {
  struct event e;
  struct node *next;
};

static struct node *last;
static struct node *first;

void evt_store_dl(int type, int time, int pid, int cpu, int old_dl,
		  int new_dl)
{
  struct node *p;

  p = malloc(sizeof(struct node));
  if (p == NULL) {
    return;
  }

  p->e.type = type;
  p->e.time = time;
  p->e.task = pid;
  p->e.cpu = cpu;
  p->e.old_dl = old_dl;
  p->e.new_dl = new_dl;

  p->next = NULL;
  if (last == NULL) {
    first = p;
  } else {
    last->next = p;
  }
  last = p;
}

void evt_store(int type, int time, int pid, int cpu)
{
  evt_store_dl(type, time, pid, cpu, 0, 0);
}

struct event *evt_get(void)
{
  if (first) {
    struct event *e;

    e = &first->e;
    first = first->next;
    if (first == NULL) {
      last = NULL;
    }

    return e;
  }

  return NULL;
}

struct event_trace *trace_export(unsigned int *cpus)
{
  struct event_trace *t;
  struct node *p = first;
  unsigned int n[MAX_CPUS] = { 0 };
  unsigned int i;

  *cpus = 0;
  while (p) {
    if ((unsigned int) p->e.cpu > *cpus) {
      *cpus = p->e.cpu;
    }
    n[p->e.cpu]++;
    p = p->next;
  }
  *cpus = *cpus + 1;

  t = malloc(sizeof(struct event_trace) * *cpus);
  if (t == NULL) {
    return NULL;
  }
  for (i = 0; i < *cpus; i++) {
    t[i].ev = malloc(sizeof(struct event) * n[i]);
    t[i].last_event = 0;
  }

  p = first;
  while (p) {
    struct event *e = &p->e;

    t[e->cpu].ev[t[e->cpu].last_event++] = *e;
    p = p->next;
    free(e);
  }
  first = NULL;
  last = NULL;

  return t;
}
