#include <stdio.h>

#include "event.h"
#include "event_list.h"
#include "task_names.h"
#include "event_create.h"

static unsigned long long int start_time;

void evt_dispatch(int lpid, int rpid, int cpu, unsigned long long int time)
{
    int utime = time - start_time;

    evt_store(TASK_DESCHEDULE, utime, lpid, cpu);
    evt_store(TASK_SCHEDULE, utime, rpid, cpu);
}

void evt_force_desch(int pid, int cpu, unsigned long long int time)
{
    int utime = time - start_time;

    evt_store(TASK_FORCE_DESCHEDULE, utime, pid, cpu);
}

void evt_initialize(int pid, int cpu, unsigned long long int time)
{
    int utime = time - start_time;

    evt_store(TASK_SCHEDULE, utime, pid, cpu);
}

void evt_activation(int pid, int cpu, unsigned long long int time)
{
    int utime = time - start_time;

    evt_store(TASK_ARRIVAL, utime, pid, cpu);
}

void evt_deactivation(int pid, int cpu, unsigned long long int time)
{
    int utime = time - start_time;

    evt_store(TASK_END, utime, pid, cpu);
}

void evt_force_deactivation(int pid, int cpu, unsigned long long int time)
{
    int utime = time - start_time;

    evt_store(TASK_FORCE_END, utime, pid, cpu);
}

void evt_creation(int pid, const char *name, int cpu,
		  unsigned long long int time)
{
    int utime;
    int i, j;
    char name_pid[25], name_support[16];

    for (i = 0, j = 0; j < 15 && name[i] != '\0'; i++) {
	if (name[i] != ' ') {
	    name_support[j++] = name[i];
	}
    }
    name_support[j] = '\0';

    sprintf(name_pid, "%s*%d", name_support, pid);
    //fprintf(stderr, "cpu %d - %s\n", cpu, name_pid);

    utime = time - start_time;
    evt_store(TASK_NAME, utime, pid, cpu);
    name_register(pid, cpu, name_pid);
}

void evt_start(unsigned long long int time)
{
    start_time = time;
}
