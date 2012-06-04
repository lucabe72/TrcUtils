/*
 * This is free software: see GPL.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>

#include "event_create.h"
#include "pid_filter.h"
#include "l4trace_read.h"
#include "event.h"

#define MAX_CPUS 8

struct new_pid {
  int pid;
  int cpu;
};

static unsigned int count;
static struct new_pid *new_pids;
static int *cpu_events;
static int *last_pid_run;	// for each cpu
static unsigned int previous_pid=0;

#define dprintf(...)
//#define dprintf printf

static int create(unsigned long long int time, int pid, const char *name,
		  int cpu)
{
  unsigned int i, found = 0, existing = 0;

//if (time != current_time) abort();
  for (i = 0; i < count; i++) {
    if (pid == new_pids[i].pid) {
      existing = 1;
      if (cpu == new_pids[i].cpu) {
	found = 1;
	break;
      }
    }
  }
  if (found == 0) {
    //create
    new_pids =
	(struct new_pid *) realloc(new_pids,
				   ++count * sizeof(struct new_pid));
    new_pids[count - 1].pid = pid;
    new_pids[count - 1].cpu = cpu;

    evt_creation(pid, name, cpu, time);
    if (!existing)
      evt_activation(pid, cpu, time);

    return 0;
  } else {
    return 1;
  }
}
/*
static void attributes_parse(const char *attributes, char *name,
			     unsigned int *curr_pid, unsigned int *cpu,
			     unsigned int *prev_pid, char *prev_state)
{
  int done = 0;
  const char *p = attributes;
  char attr[32], val[32];

  while (!done) {
    while ((*p != 0) && (*p != ' '))
      p++;
    if (*p == 0) {
      done = 1;
    } else {
      p++;
      sscanf(p, "%[^=]=%s", attr, val);
      dprintf("Attr: %s = %s\n", attr, val);
      if ((strcmp(attr, "pid") == 0) || (strcmp(attr, "next_pid") == 0)) {
	*curr_pid = atoi(val);
      }
      if ((strcmp(attr, "comm") == 0) || (strcmp(attr, "next_comm") == 0)) {
	strcpy(name, val);
      }
      if ((strcmp(attr, "target_cpu") == 0)) {
	*cpu = atoi(val);
      }
      if ((strcmp(attr, "prev_pid") == 0)) {
	*prev_pid = atoi(val);
      }
      if ((strcmp(attr, "prev_state") == 0)) {
	*prev_state = val[0];
      }
    }
  }
}
*/
static void trace(unsigned long long int time, int current_pid,
		  const char *name, int cpu, char *event)/*,
		  const char *prev_name, int prev_pid)*/
{
  if (strcmp(event,"00000009")==0) {
      create(time, current_pid, name, cpu);
    evt_activation(current_pid, cpu, time);
  } else  
  if (strcmp(event,"00000000")==0) {
    evt_activation(current_pid, cpu, time);
  } else 
  if (strcmp(event,"00000001")==0) {
    evt_dispatch(0,current_pid, cpu, time);
  } else 
  if (strcmp(event,"00000002")==0) {
    evt_dispatch(current_pid,0, cpu, time);
  } else
  if (strcmp(event,"00000003")==0) {
    evt_deactivation(current_pid, cpu, time);
  }
  else {
    dprintf("Unknown event %s",event);
  } 
  
/*
  if (event==9) {
    create(time, current_pid, name, cpu);
    evt_activation(current_pid, cpu, time);
  }
  if (strcmp(event, "sched_switch:") == 0) {
    dprintf("Switch %d -> %d\n", prev_pid, current_pid);
    create(time, current_pid, name, cpu);
    create(time, prev_pid, prev_name, cpu);
    if ((last_pid_run[cpu] != -1) && (last_pid_run[cpu] != prev_pid)) {
      fprintf(stderr, "[%d]Error! %d != %d... Correcting\n", cpu,
	      last_pid_run[cpu], prev_pid);
      prev_pid = last_pid_run[cpu];
    }
    //dispatch
    if (current_pid == 0) {
      cpu_events[cpu] = 1;
    }
    last_pid_run[cpu] = current_pid;	//Last pid scheduled
    if (prev_state != 'R') {
      evt_deactivation(prev_pid, cpu, time);
    }
    evt_dispatch(prev_pid, current_pid, cpu, time);
  }*/
}

long long int l4trace_parse(FILE * f)
{
  char line[256];
  char *res;
  long long int current_time = -1;


  if (cpu_events == NULL) {
    cpu_events = (int *) malloc(MAX_CPUS * sizeof(int));
  }
  if (last_pid_run == NULL) {
    int i;

    last_pid_run = (int *) malloc(MAX_CPUS * sizeof(int));
    for (i = 0; i < MAX_CPUS; i++) {
      last_pid_run[i] = -1;
    }
  }

  res = fgets(line, sizeof(line), f);
  if (res) {
    //char taskc[24], to, tasko[24], cstate, ostate, sched[6];
    char taskc[24];
    char event[32];
    //unsigned int cpul, cpur, cprio, oprio, opid, cpid;
    unsigned int cpu, current_pid;
    static int start;
    unsigned long us;
    int i = 0;
    current_time = 0;
    if (line[0] != '#') {
      cpu=1;
      //sscanf(line, "%[^[][%d]%lu.%lu:%s%[^!]",
      sscanf(line, "%s %s %lu",
	     taskc, event, &us);
      fprintf(stderr,"Task %s at %lu on CPU %d does %s\n", taskc, us,
	      cpu, event);
      current_time=us;
      if (strcmp(taskc,"????")!=0)
         sscanf(taskc, "%x", &current_pid);
      else current_pid=0;
      
      if (start == 0) {
	start = 1;
	evt_start(current_time);
	for (i = 0; i < MAX_CPUS; i++) {
	  create(current_time, 0, "idle\0", i);
	  cpu_events[i] = 0;
	  evt_initialize(0, i, current_time);
	}
      }

      if ((current_pid != 0) && (cpu_events[cpu] == 0)) {
	cpu_events[cpu] = 1;
	create(current_time, current_pid, taskc, cpu);

	last_pid_run[cpu] = current_pid;	//Last pid scheduled
	evt_dispatch(0, current_pid, cpu, current_time);
      }
      dprintf("%s", line);
      trace(current_time, current_pid, taskc, cpu, event);
//, taskc,previous_pid);
      previous_pid=current_pid;
    }
  } else {
    free(new_pids);
    free(cpu_events);
    free(last_pid_run);
  }
  return current_time;
}
