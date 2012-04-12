#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stats_utils.h"


struct record {
    int times;
    unsigned long int time;
};

struct trace {
    int name, records_size_unblocks,
	records_size_response,
	executions_size, records_size_executions, blocks, cpus_util_size;
    char description[16];
    unsigned long int sched, number, pree, unblocks_time, start_execution,
	*executions;
    struct record *records_unblocks, *records_response,
	*records_executions;
    unsigned long int execution_total;
    float *cpus_util;
    int p_u, p_e, p_r;
};


static struct trace *tasks;
static int size_tasks;

static int containsTask(int pid)
{
    int i = 0;
    for (; i < size_tasks; i++) {
	if (tasks[i].name == pid) {
	    return i;
	}
    }
    return -1;
}

static struct record *updateRecord(unsigned long int time,
				   unsigned long int tollerance, int *size,
				   struct record *records)
{
    int i, found = -1;

    if (time != 0) {
        for (i = 0; i < *size && found == -1; i++) {
            if ((records[i].time + tollerance >= time) && (records[i].time <= time + tollerance)) {
                records[i].times++;
                found = 1;
            }
        }

        if (found == -1) {
            records = realloc(records, ++(*size) * sizeof(struct record));
            records[*size - 1].time = time;
            records[*size - 1].times = 1;
        }
    }

    return records;
}

static unsigned long int execution_time(int pid)
{
    int i, found = containsTask(pid);
    unsigned long int result = 0;

    for (i = 0; i < tasks[found].executions_size; i++) {
        result += (tasks[found].executions)[i];
    }
    free(tasks[found].executions);
    tasks[found].executions_size = 0;
    tasks[found].executions = NULL;

    return result;
}

static void encod_stats_cpu(void *l, unsigned long int time, int task, int cpu_stat, float cpu, float cpuw, float cpua)
{
    if (l == NULL) {
        return;
    }

    //fprintf(l, "%ld %d %s %d %f %f %f\n", time, task, getTaskName(task), cpu_stat, cpu, cpuw, cpua);
    fprintf(l, "%ld %d %d %f %f %f\n", time, task, cpu_stat, cpu, cpuw,
	    cpua);
}



void create_task(int pid, unsigned long int time)
{
    if (containsTask(pid) == -1) {

	tasks =
	    (struct trace *) realloc(tasks,
				     ++size_tasks * sizeof(struct trace));
	tasks[size_tasks - 1].name = pid;
	tasks[size_tasks - 1].number = 0;
	tasks[size_tasks - 1].sched = 0;
	tasks[size_tasks - 1].pree = 0;

	tasks[size_tasks - 1].p_u = 0;
	tasks[size_tasks - 1].p_e = 0;
	tasks[size_tasks - 1].p_r = 0;

	tasks[size_tasks - 1].unblocks_time = time;
	tasks[size_tasks - 1].records_size_unblocks = 0;
	tasks[size_tasks - 1].records_unblocks = NULL;

	tasks[size_tasks - 1].records_size_response = 0;
	tasks[size_tasks - 1].records_response = NULL;

	tasks[size_tasks - 1].start_execution = 0;
	tasks[size_tasks - 1].executions_size = 0;
	tasks[size_tasks - 1].executions = NULL;
	tasks[size_tasks - 1].records_executions = NULL;
	tasks[size_tasks - 1].records_size_executions = 0;

	tasks[size_tasks - 1].execution_total = 0;
	tasks[size_tasks - 1].cpus_util = NULL;
	tasks[size_tasks - 1].cpus_util_size = 0;

	tasks[size_tasks - 1].blocks = 0;
    }
}



unsigned long int response_time(int pid, unsigned long int time)
{
    int found = containsTask(pid);
    return time - tasks[found].unblocks_time;
}

unsigned long int pdf_response_time(int pid, unsigned long int time,
				    unsigned long int tollerance)
{
    int i, times = 0, found = containsTask(pid);
    unsigned long int higher = time;
    tasks[found].records_response =
	updateRecord(time, tollerance, &tasks[found].records_size_response,
		     tasks[found].records_response);

    if (tasks[found].records_size_response == 1) {
	tasks[found].p_r = 1;
    } else {
	tasks[found].p_r = 0;
    }

    for (i = 0; i < tasks[found].records_size_response; i++) {
	if ((tasks[found].records_response[i]).times >= times) {
	    times = (tasks[found].records_response[i]).times;
	    higher = (tasks[found].records_response[i]).time;
	}
    }

    return higher;
}

float cdf_response_time(int pid, unsigned long int time)
{
    int i, found = containsTask(pid);
    unsigned int commulative = 0, m = 0;

    for (i = 0; i < tasks[found].records_size_response; i++) {
	if ((tasks[found].records_response[i]).time < time) {
	    commulative += (tasks[found].records_response[i]).times;
	}
	m += (tasks[found].records_response[i]).times;
    }

    return (float) commulative / m;
}

unsigned long int pdf_executions(int pid, unsigned long int time,
				 unsigned long int tollerance)
{
    int i, times = 0, found = containsTask(pid);
    unsigned long int higher = time;
    tasks[found].records_executions =
	updateRecord(time, tollerance,
		     &tasks[found].records_size_executions,
		     tasks[found].records_executions);

    if (tasks[found].records_size_executions == 1) {
	tasks[found].p_e = 1;
    } else {
	tasks[found].p_e = 0;
    }

    for (i = 0; i < tasks[found].records_size_executions; i++) {
	if ((tasks[found].records_executions[i]).times >= times) {
	    times = (tasks[found].records_executions[i]).times;
	    higher = (tasks[found].records_executions[i]).time;
	}
    }

    return higher;
}

float cdf_executions(int pid, unsigned long int time)
{
    int i, found = containsTask(pid);
    unsigned int commulative = 0, m = 0;

    for (i = 0; i < tasks[found].records_size_executions; i++) {
	if ((tasks[found].records_executions[i]).time <= time) {
	    commulative += (tasks[found].records_executions[i]).times;
	}
	m += (tasks[found].records_executions[i]).times;
    }

    return (float) commulative / m;
}



void blocks_task(int pid)
{
    tasks[containsTask(pid)].blocks = 1;
}

void start_execution(int pid, unsigned long int time)
{
    int found = containsTask(pid);
    tasks[found].start_execution = time;
    tasks[found].sched++;
}

unsigned long int end_execution(int pid, unsigned long int time)
{
    int found = containsTask(pid);
    unsigned long int result;
    result = time - tasks[found].start_execution;

    if (result > 0) {
	tasks[found].execution_total += result;
    }

    tasks[found].executions = realloc(tasks[found].executions, ++tasks[found].executions_size * sizeof(int));
    tasks[found].executions[tasks[found].executions_size - 1] = result;
    if (tasks[found].blocks == 1) {
        result = execution_time(pid);
        tasks[containsTask(pid)].blocks = 0;

        return result;
    } else {
        tasks[found].pree++;
        return 0;
    }
}

unsigned long int intervalls(int pid, unsigned long int time)
{
    int found = containsTask(pid);
    unsigned long int result;
    result = time - tasks[found].unblocks_time;
    tasks[found].unblocks_time = time;
    tasks[found].number++;
    //first time 0, No interval
    return result;
}

float cdf_intervalls(int pid, unsigned long int time)
{
    int i, found = containsTask(pid);
    unsigned int commulative = 0, m = 0;

    for (i = 0; i < tasks[found].records_size_unblocks; i++) {
	if ((tasks[found].records_unblocks[i]).time < time) {
	    commulative += (tasks[found].records_unblocks[i]).times;
	}
	m += (tasks[found].records_unblocks[i]).times;
    }

    return (float) commulative / m;
}

unsigned long int pdf_intervalls(int pid, unsigned long int time,
				 unsigned long int tollerance)
{
    int i, times = 0, found = containsTask(pid);
    unsigned long int higher = time;
    tasks[found].records_unblocks =
	updateRecord(time, tollerance, &tasks[found].records_size_unblocks,
		     tasks[found].records_unblocks);

    if (tasks[found].records_size_unblocks == 1) {
	tasks[found].p_u = 1;
    } else {
	tasks[found].p_u = 0;
    }

    for (i = 0; i < tasks[found].records_size_unblocks; i++) {
	if ((tasks[found].records_unblocks[i]).times >= times) {
	    times = (tasks[found].records_unblocks[i]).times;
	    higher = (tasks[found].records_unblocks[i]).time;
	}
    }

    return higher;
}


void calculateCPUsUtil(void *l, unsigned long int time)
{
    int i, j;
    for (i = 0; i < size_tasks; i++) {
	float t = tasks[i].execution_total, a = 0, w = 100;
	tasks[i].execution_total = 0;
	t = (t / time) * 100.0;
	tasks[i].cpus_util =
	    (float *) realloc(tasks[i].cpus_util,
			      ++tasks[i].cpus_util_size * sizeof(float));
	(tasks[i].cpus_util)[tasks[i].cpus_util_size - 1] = t;

	for (j = 0; j < tasks[i].cpus_util_size; j++) {
	    a += (tasks[i].cpus_util)[j];
	    if ((tasks[i].cpus_util)[j] < w) {
		w = (tasks[i].cpus_util)[j];
	    }
	}
	a /= tasks[i].cpus_util_size;
	encod_stats_cpu(l, time, tasks[i].name, CPU_UTILIZ, t, w, a);
    }
}


void encod_stats_time(void *l, unsigned long int time, int task,
		      int kind_stat, unsigned long int time_i,
		      unsigned long int pdf, float cdf)
{
    if (l == NULL) {
        return;
    }

    //fprintf(l, "%ld %d %s %d %ld %ld %d\n", time, task, getTaskName(task), kind_stat, time_i, pdf, cdf);
    fprintf(l, "%ld %d %d %ld %ld %f\n", time, task, kind_stat, time_i,
	    pdf, cdf);
}
