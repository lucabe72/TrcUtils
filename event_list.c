#include <stdlib.h>
 
#include "event.h"
#include "event_list.h"

struct node {
  struct event e;
  struct node *next;
};

static struct node *last;
static struct node *first;

void evt_store_dl(int type, int time, int pid, int cpu, int old_dl, int new_dl)
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


