#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "event.h"
#include "task_names.h"
#include "jtrace_write.h"

#define MAX_CPUS 8		//FIXME

#define JJOB_ARRIVAL		0
#define JTASK_EXECUTION		1
#define JTASK_PREEMPTION	2
#define JJOB_END		3
#define JTASK_CREATION		9
#define JDEADLINE_POSTPONING	4
#define JDEADLINE_SETTING	5
#define JDEADLINE_MISS		10

static unsigned long long int start_time;

static void jtrace_log_int(uint32_t v)
{
  uint32_t sw;
  sw = htonl(v);
  write(1, (char *) &sw, 4);
}

static void jtrace_common(int len, int type, int time, int task)
{
  jtrace_log_int(len + 3 * 4);
  jtrace_log_int(type);
  jtrace_log_int(time);
  jtrace_log_int(task);
  //fprintf(stdout, "%d %d %d %d\n", cpu, task, time, type);
}

void jtrace_dispatch(int lpid, int rpid, int cpu,
		     unsigned long long int time)
{
  int utime = time - start_time;
  jtrace_common(4, JTASK_PREEMPTION, utime, lpid);
  jtrace_log_int(cpu);
  jtrace_common(4, JTASK_EXECUTION, utime, rpid);
  jtrace_log_int(cpu);
}

void jtrace_activation(int pid, unsigned long long int time)
{
  int utime = time - start_time;
  jtrace_common(0, JJOB_ARRIVAL, utime, pid);
}

void jtrace_deactivation(int pid, int cpu, unsigned long long int time)
{
  int utime = time - start_time;
  jtrace_common(4, JJOB_END, utime, pid);
  jtrace_log_int(cpu);
}

void jtrace_creation(int pid, const char *name,
		     unsigned long long int time)
{
  int utime, len;

  utime = time - start_time;
  len = strlen(name);
  jtrace_common(len + 4, JTASK_CREATION, utime, pid);
  jtrace_log_int(len);
  write(1, name, len);
  //fprintf(stdout, "%d %s\n", len, name_pid);
}

void jtrace_start(unsigned long long int time)
{
  const char *header = "version 1.2";

  start_time = time;
  write(1, header, strlen(header) + 1);
}

void jtrace_write(struct event *e)
{
  int end_job[MAX_CPUS];
  static int inited;

  if (!inited) {
    inited = 1;
    jtrace_start(e->time);
  }
  switch (e->type) {
    case TASK_NAME:
      {
	const char *name = name_get(e->task, e->cpu);

	if (name) {
	  jtrace_creation(e->task, name, e->time);
	} else {
	  fprintf(stderr, "Unknown task %d %d!\n", e->task, e->cpu);
	}
      }
      break;
    case TASK_ARRIVAL:
      jtrace_activation(e->task, e->time);
      break;
    case TASK_SCHEDULE:
      jtrace_common(4, JTASK_EXECUTION, e->time - start_time, e->task);
      jtrace_log_int(e->cpu);
      break;
    case TASK_DESCHEDULE:
      if (!end_job[e->cpu]) {
	jtrace_common(4, JTASK_PREEMPTION, e->time - start_time, e->task);
	jtrace_log_int(e->cpu);
      }
      end_job[e->cpu] = 0;
      break;
    case TASK_END:
      jtrace_deactivation(e->task, e->cpu, e->time);
      end_job[e->cpu] = 1;
      break;
//#define TASK_DLINEPOST 4
//      break;
//#define TASK_DLINESET 5
//      break;
//#define TASK_WAIT 6
//      break;
//#define TASK_SIGNAL 7
//      break;
//#define TASK_IDLE 8
//      break;
    default:
      fprintf(stderr, "Error: unknown event %d!\n", e->type);
      exit(-1);

  }
}
