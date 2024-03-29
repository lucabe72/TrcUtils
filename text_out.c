/*
 * This is free software: see GPL.txt
 */
#include <stdio.h>

#include "event.h"
#include "trace_evt_handle.h"
#include "text_out.h"

void trace_dump_event(struct event *e)
{
  const char *name;

  switch (e->type) {
    case TASK_DESCHEDULE:
      //printf("Server %d descheduled at time %"PRIu64"\n", e->task, e->time);
      printf("Server %d descheduled at time %d\n", e->task, e->time);
      break;
    case TASK_SCHEDULE:
      //printf("Server %d dispatched at time %"PRIu64"\n", e->task, e->time);
      printf("Server %d dispatched at time %d\n", e->task, e->time);
      break;
    case TASK_ARRIVAL:
      //printf("Server %d unblocks at time %"PRIu64"\n", e->task, e->time);
      printf("Server %d unblocks at time %d\n", e->task, e->time);
      break;
    case TASK_END:
      //printf("Server %d blocks at time %"PRIu64"\n", e->task, e->time);
      printf("Server %d blocks at time %d\n", e->task, e->time);
      break;
    case TASK_NAME:
      name = srv_name(e->task, e->cpu);
      if (name == NULL) {
	name = "Unknown";
      }
      //printf("Server %d (%s) created at time %"PRIu64"\n", e->task, name, e->time);
      printf("Server %d (%s) created at time %d\n", e->task, name,
	     e->time);
      break;
    case TASK_DLINEPOST:
      //printf("Deadline of server %d postponed from %"PRIu64" to %"PRIu64" at time %"PRIu64"\n",
      //            e->task, e->old_dl, e->new_dl, e->time);
      printf("Deadline of server %d postponed from %d to %d at time %d\n",
	     e->task, e->old_dl, e->new_dl, e->time);
      break;
    case TASK_DLINESET:
      //printf("New deadline %"PRIu64" assigned to server %d at time %"PRIu64"\n",
      //            e->new_dl, e->task, e->time);
      printf("New deadline %d assigned to server %d at time %d\n",
	     e->new_dl, e->task, e->time);
      break;
    default:
      fprintf(stderr, "Strange event type %d\n", e->type);
      break;
  }
}

void trace_info(struct event *ev, unsigned int last_event,
		unsigned int last_server)
{
  unsigned int i, j;
  int first, last;

  printf("CPU %d\n", ev[0].cpu);
  printf("\tNumber of events: %u\n", last_event);
  //printf("First event time: %"PRIu64"\n", trc->ev[0].time);
  //printf("Last event time: %"PRIu64"\n", trc->ev[trc->last_event].time);
  printf("\tFirst event time: %d\n", ev[0].time);
  printf("\tLast event time: %d\n", ev[last_event - 1].time);
  printf("\tNumber of servers: %u\n\n", last_server);
  for (i = 0; i < last_server; i++) {
    printf("\tServer %d: %s\n", i,
	   srv_name(srv_id(i, ev[0].cpu), ev[0].cpu));
    first = -1;
    last = -1;
    for (j = 0; j < last_event; j++) {
      if (ev[j].task == srv_id(i, ev[0].cpu)) {
	last = j;
	if (first == -1) {
	  first = j;
	}
      }
    }
    //printf("\tFirst event (%d) at time %"PRIu64"\n", first, trc->ev[first].time);
    //printf("\tLast event (%d) at time %"PRIu64"\n", last, trc->ev[last].time);
    if (first >= 0 && last >= 0) {
      printf("\t\tFirst event (%d) at time %d\n", first, ev[first].time);
      printf("\t\tLast event (%d) at time %d\n", last, ev[last].time);
    } else {
      printf("\t\tNo events: first=%d last=%d\n", first, last);
    }
  }
}
