#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "trace_read.h"
#include "event.h"
#include "event_list.h"
#include "trace_evt_handle.h"

struct server {
    char *name;
    int id;
    int cpu;
};

#define TASKS       100
#define MAX_EVENTS 100000
#define MAX_CPUS 8 //FIXME!
#define MAX_SERVERS (TASKS * MAX_CPUS)


static struct server srv[MAX_SERVERS];
static int last_server[MAX_CPUS];
static int max;

static int srv_find(struct server s[], int id, int cpu)
{
    int i = 0;

    for (; i < MAX_SERVERS; i++) {
	if (s[i].id == id && s[i].cpu == cpu && s[i].name != NULL) {
	    return i;
	}
    }

    return -1;
}

int trace_read_event(void *h, int start, int end)
{
    int type, time, task, cpu, res, old_dl = 0, new_dl = 0;
    static struct server priv_srv[MAX_SERVERS];
    static int last_priv_server;
    FILE *f = h;

    res = trace_common(f, &type, &time, &task, &cpu);
    if (res < 0) {
        return res;
    }

    if (cpu < 0 || cpu > MAX_CPUS) {
	return -1;
    }
//    if (upc->cpus < (unsigned int)cpu) {
//        upc->cpus = cpu;
//    }

//When the trace has finished I want to stop all the tasks running
    if (type == TASK_FORCE_END) {
	type = TASK_END;
    } else if (type == TASK_FORCE_DESCHEDULE) {
	type = TASK_DESCHEDULE;
    }

    if ((end != 0) && (time > end)) {
	return 0;
    }

    switch (type) {
    case TASK_END:
    case TASK_DESCHEDULE:
	if (time >= start) {
	    int sid = srv_find(srv, task, cpu);

	    if (sid >= 0) {
                evt_store(type, time, task, cpu);
	    }
	}
	break;
    case TASK_ARRIVAL:
    case TASK_SCHEDULE:
	if (time >= start) {
            evt_store(type, time, task, cpu);
	    if (srv_find(srv, task, cpu) < 0) {
		int sid;

		sid = srv_find(priv_srv, task, cpu);
		if (sid < 0) {
		    fprintf(stderr,
			    "[%d] Error: cannot find task %d %d\n", time, task, cpu);

		    return -3;
		}

		srv[last_server[cpu] + (cpu * TASKS)].name =
		    priv_srv[sid].name;
		srv[last_server[cpu] + (cpu * TASKS)].id = task;
		srv[last_server[cpu] + (cpu * TASKS)].cpu = cpu;

		last_server[cpu]++;
	    }
	}
	break;
    case TASK_NAME:
	priv_srv[last_priv_server].name = task_name(f);
	priv_srv[last_priv_server].id = task;
	priv_srv[last_priv_server].cpu = cpu;

	//fprintf(stderr, "cpu %d - %s\n", e->cpu, priv_srv[last_priv_server].name);

	last_priv_server++;

	time = start;
        evt_store(type, time, task, cpu);
	break;
    case TASK_DLINEPOST:
	old_dl = task_dline(f);
    case TASK_DLINESET:
	new_dl = task_dline(f);
	if (time >= start) {
	    if (srv_find(srv, task, cpu) < 0) {
              return -5;
	    }
	}
        evt_store_dl(type, time, task, cpu, old_dl, new_dl);
	break;
    default:
	fprintf(stderr, "Strange event type %d\n", type);
	return -4;
    }
    if (new_dl > max) {
      max = new_dl;
    }
    if (time > max) {
      max = time;
    }

    return 0;
}

int last_time(void)
{
    return max;
}

const char *srv_name(int task, int cpu)
{
    int i;

    i = srv_find(srv, task, cpu);
    if (i >= 0) {
        return srv[i].name;
    }

    return NULL;
}

int srv_id(int i, int cpu)
{
    return srv[i + (cpu * TASKS)].id;
}

int servers(int cpu)
{
    return last_server[cpu];
}

