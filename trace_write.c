#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "event.h"
#include "trace_write.h"

static unsigned long long int start_time;

static void trace_log_int(uint32_t v)
{
    uint32_t sw;
    sw = htonl(v);
    write(1, (char *) &sw, 4);
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
    int utime;
    int len, i, j;
    char name_pid[25], name_support[16];

    for (i = 0, j = 0; j < 15 && name[i] != '\0'; i++) {
	if (name[i] != ' ') {
	    name_support[j++] = name[i];
	}
    }
    name_support[j] = '\0';

    utime = time - start_time;
    trace_common(TASK_NAME, utime, pid, cpu);
    sprintf(name_pid, "%s*%d", name_support, pid);
    //fprintf(stderr, "cpu %d - %s\n", cpu, name_pid);
    len = strlen(name_pid);
    trace_log_int(len);
    write(1, name_pid, len);
    //fprintf(stdout, "%d %s\n", len, name_pid);
}

void trc_start(unsigned long long int time)
{
    start_time = time;
}

