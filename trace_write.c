#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "event.h"
#include "trace_write.h"
#include "task_names.h"

static unsigned long long int start_time;

static void trace_log_int(uint32_t v)
{
  uint32_t sw;
  sw = htonl(v);
  fwrite(&sw, 4, 1, stdout);
}

static void trace_common(int type, int time, int task, int cpu)
{
  trace_log_int(type);
  trace_log_int(time);
  trace_log_int(task);
  trace_log_int(cpu);
  //fprintf(stdout, "%d %d %d %d\n", cpu, task, time, type);
}

void trc_dispatch(int lpid, int rpid, int cpu, unsigned long long int time)
{
  int utime = time - start_time;
  trace_common(TASK_DESCHEDULE, utime, lpid, cpu);
  trace_common(TASK_SCHEDULE, utime, rpid, cpu);
}

void trc_force_desch(int pid, int cpu, unsigned long long int time)
{
  int utime = time - start_time;
  trace_common(TASK_FORCE_DESCHEDULE, utime, pid, cpu);
}

void trc_initialize(int pid, int cpu, unsigned long long int time)
{
  int utime = time - start_time;
  trace_common(TASK_SCHEDULE, utime, pid, cpu);
}

void trc_activation(int pid, int cpu, unsigned long long int time)
{
  int utime = time - start_time;
  trace_common(TASK_ARRIVAL, utime, pid, cpu);
}

void trc_deactivation(int pid, int cpu, unsigned long long int time)
{
  int utime = time - start_time;
  trace_common(TASK_END, utime, pid, cpu);
}

void trc_force_deactivation(int pid, int cpu, unsigned long long int time)
{
  int utime = time - start_time;
  trace_common(TASK_FORCE_END, utime, pid, cpu);
}

void trc_creation(int pid, const char *name, int cpu,
		  unsigned long long int time)
{
  int utime, len;

  utime = time - start_time;
  trace_common(TASK_NAME, utime, pid, cpu);
  len = strlen(name);
  trace_log_int(len);
  fwrite(name, len, 1, stdout);
  //fprintf(stdout, "%d %s\n", len, name_pid);
}

void trc_start(unsigned long long int time)
{
  start_time = time;
}

void trc_write(struct event *e)
{
  if (e->type != TASK_NAME) {
    trace_common(e->type, e->time, e->task, e->cpu);
  } else {
    const char *name = name_get(e->task, e->cpu);

    if (name) {
      trc_creation(e->task, name, e->cpu, e->time);
    } else {
      fprintf(stderr, "Unknown task %d %d!\n", e->task, e->cpu);
    }
  }
#if 0				//FIXME: handle deadline events!
case TASK_DLINEPOST:
  trace_write_int(e->old_dl);
case TASK_DLINESET:
  trace_write_int(e->new_dl);
#endif
}
