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

    utime = time - start_time;
    evt_store(TASK_NAME, utime, pid, cpu);
    name_register(pid, cpu, name);
}

void evt_start(unsigned long long int time)
{
    start_time = time;
}
