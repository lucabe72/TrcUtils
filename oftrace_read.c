/*
 * This is free software: see GPL.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>

#include "tasks.h"
#include "event_create.h"
#include "pid_filter.h"
#include "oftrace_read.h"

#define MAX_CPUS 8

static int *cpu_events;
static int *last_pid_run;	// for each cpu
static int forced_deactivation_pid = -1;


static unsigned long long int time_convert(unsigned long s,
					   unsigned long us)
{
  return ((s * 1000000) + us);
}

static int create(long long int time, int pid, const char *name, int cpu)
{
  static struct task_set *ts;

  if (ts == NULL) {
    ts = taskset_init();
  }

  if (taskset_find_task(ts, pid, cpu) == NULL) {
    //create
    taskset_add_task(ts, pid, cpu, (void *)1);

    evt_creation(pid, name, cpu, time);
    evt_activation(pid, cpu, time);

    return 0;
  } else {
    return 1;
  }
}

static void trace(unsigned long long int time, int cpid, char *taskc, int opid, char *tasko,
                  char *sched, char cstate, char ostate, int cpul,
                  int cpur)
{
  int c;

  if (cpid != 0 && cpu_events[cpul] == 0 && strcmp(sched, " ==> ") == 0) {
    cpu_events[cpul] = 1;
    create(time, cpid, taskc, cpul);

    last_pid_run[cpul] = cpid;      //Last pid scheduled
    evt_dispatch(0, cpid, cpul, time);
  }

  create(time, cpid, taskc, cpul);
  c = create(time, opid, tasko, cpur);

  if (strcmp(sched, "   + ") == 0 && c == 1) {
    //wake up
    if (forced_deactivation_pid != opid) {
      evt_activation(opid, cpur, time);
    } else {
      forced_deactivation_pid = -1;
    }
  }
  if (cstate != 'R' && strcmp(sched, "   + ") != 0) {
    //sleep
    evt_deactivation(cpid, cpur, time);
  }

  if (strcmp(sched, " ==> ") == 0) {
    if ((last_pid_run[cpul] != -1 && last_pid_run[cpul] != cpid)) { //Check between the new schedulation and the old
      evt_deactivation(last_pid_run[cpur], cpur, time);   //Stop: last_pid_run
      evt_dispatch(last_pid_run[cpur], cpid, cpul, time); //Dispatch: last_pid_run > cpid
      evt_activation(last_pid_run[cpur], cpur, time);     //Wake up: last_pid_run reactivated
      forced_deactivation_pid = last_pid_run[cpur];       //trace the last_pid_run reactivated
    }
    //dispatch
    if (cpid == 0) {
      cpu_events[cpul] = 1;
    }
    last_pid_run[cpul] = opid;      //Last pid scheduled
    evt_dispatch(cpid, opid, cpul, time);
  }
}

long long int oftrace_parse(FILE * f)
{
  char line[79];
  char *res;
  long long int current_time;


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
    char taskc[24], to, tasko[24], cstate, ostate, sched[6];
    unsigned int cpul, cpur, cprio, oprio, opid, cpid;
    unsigned long s, us;
    int i = 0;
    static int start;

    if (line[0] != '#') {
      sscanf(line, "%[^[][%d]%lu.%lu:%d:%d:%c%[^[][%d]%d:%d:%c",
                    taskc, &cpul, &s, &us, &cpid, &cprio, &cstate,
                    sched, &cpur, &opid, &oprio, &ostate);

      current_time = time_convert(s, us);
      //unknown or variable the length                
      while ((to = fgetc(f)) != '\n') {       //last task name (the other)
        tasko[i++] = to;
      }
      tasko[i] = '\0';

      cpid = filterPid(cpid);
      opid = filterPid(opid);

      if (start == 0) {
        start = 1;
        evt_start(current_time);

        for (i = 0; i < MAX_CPUS; i++) {
	  create(current_time, 0, "idle\0", i);
	  cpu_events[i] = 0;
	  evt_initialize(0, i, current_time);
        }

        if (cpid != 0 && cpu_events[cpul] == 0) {
          cpu_events[cpul] = 1;
          create(current_time, cpid, taskc, cpul);

          last_pid_run[cpul] = cpid;      //Last pid scheduled
          evt_dispatch(0, cpid, cpul, current_time);
        }
      }

      trace(current_time, cpid, taskc, opid, tasko, sched, cstate, ostate, cpul, cpur);
    } else {
      current_time = 0;
    }
  } else {
    current_time = -1;
    free(cpu_events);
    free(last_pid_run);
  }

  return current_time;
}

