#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "trace_read.h"
#include "event.h"
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

struct cpu *cpus_alloc(void)
{
    struct cpu *upc;
    int i;

    upc = malloc(sizeof(struct cpu));
    if (upc == NULL) {
	perror("Malloc(upc)");

        return NULL;
    }

    upc->cpus = 0;
    upc->trc = malloc(sizeof(struct trace) * MAX_CPUS);
    if (upc->trc == NULL) {
	perror("Malloc(trc)");
	free(upc);

	return NULL;
    }

    for (i = 0; i < MAX_CPUS; i++) {
	upc->trc[i].ev = malloc(sizeof(struct event) * MAX_EVENTS);

	if (upc->trc[i].ev == NULL) {
	    perror("Malloc(ev)");
	    free(upc);
	    free(upc->trc);

	    return NULL;
	}

	memset(upc->trc[i].ev, 0, sizeof(struct event) * MAX_EVENTS);
	upc->trc[i].last_event = 0;
	upc->trc[i].last_server = 0;
    }

    return upc;
}



int srv_find(struct server s[], int id, int cpu)
{
    int i = 0;

    for (; i < MAX_SERVERS; i++) {
	if (s[i].id == id && s[i].cpu == cpu && s[i].name != NULL) {
	    return i;
	}
    }

    return -1;
}

int trace_read_event(void *h, struct cpu *upc, int start, int end)
{
    int type, time, task, cpu, i, res;
    struct event *e;
    struct trace *trc;
    static struct server priv_srv[MAX_SERVERS];
    static int last_priv_server;
    FILE *f = h;

    res = trace_common(f, &type, &time, &task, &cpu);
    if (res < 0) {
        return res;
    }

    if (cpu < 0 || cpu > MAX_CPUS) {
	for (i = 0; i < MAX_CPUS; i++) {
	    trc = &upc->trc[i];
	    if (trc->last_event > 0) {
		trc->last_event--;
	    }
	}
	return -1;
    }
    if (upc->cpus < (unsigned int)cpu) {
        upc->cpus = cpu;
    }

//When the trace has finished I want to stop all the tasks running
    if (type == TASK_FORCE_END) {
	type = TASK_END;
    } else if (type == TASK_FORCE_DESCHEDULE) {
	type = TASK_DESCHEDULE;
    }

    trc = &upc->trc[cpu];
    if (trc->last_event == MAX_EVENTS) {
        return -2;
    }

    e = &trc->ev[trc->last_event];

    e->type = type;
    e->time = time;
    e->task = task;
    e->cpu = cpu;

    switch (e->type) {
    case TASK_END:
    case TASK_DESCHEDULE:
	if (e->time >= start) {
	    int sid = srv_find(srv, e->task, e->cpu);

	    if (sid >= 0) {
		trc->last_event++;
	    }
	}
	break;
    case TASK_ARRIVAL:
    case TASK_SCHEDULE:
	if (e->time >= start) {
	    trc->last_event++;
	    if (srv_find(srv, e->task, e->cpu) < 0) {
		int sid;

		sid = srv_find(priv_srv, e->task, e->cpu);
		if (sid < 0) {
		    fprintf(stderr,
			    "[%ld - %d] Error: cannot find task %d %d\n",
			    trc->last_event, e->time, e->task, e->cpu);

		    return --trc->last_event;
		}

		srv[trc->last_server + (e->cpu * TASKS)].name =
		    priv_srv[sid].name;
		srv[trc->last_server + (e->cpu * TASKS)].id = e->task;
		srv[trc->last_server + (e->cpu * TASKS)].cpu = e->cpu;

		trc->last_server++;
	    }
	}
	break;
    case TASK_NAME:
	priv_srv[last_priv_server].name = task_name(f);
	priv_srv[last_priv_server].id = e->task;
	priv_srv[last_priv_server].cpu = e->cpu;

	//fprintf(stderr, "cpu %d - %s\n", e->cpu, priv_srv[last_priv_server].name);

	last_priv_server++;

	e->time = start;
	trc->last_event++;
	break;
    case TASK_DLINEPOST:
	e->old_dl = task_dline(f);
    case TASK_DLINESET:
	e->new_dl = task_dline(f);
	if (e->time >= start) {
	    if (srv_find(srv, e->task, e->cpu) >= 0) {
		trc->last_event++;
	    }
	}
	break;
    default:
	fprintf(stderr, "[%ld] Strange event type %d\n", trc->last_event,
		e->type);
	trc->last_event--;
	return -1;
    }
    if ((end != 0) && (e->time > end)) {
	trc->last_event--;
    }

    return trc->last_event;
}

int last_time(struct cpu *upc)
{
    unsigned int i, j;
    int max = 0;
    struct trace *trc;
    struct event *e;

    for (j = 0; j < MAX_CPUS; j++) {
	trc = &upc->trc[j];
	e = trc->ev;
	for (i = 0; i < trc->last_event; i++) {
	    if (e[i].new_dl > max) {
		max = e[i].new_dl;
	    }
	    if (e[i].time > max) {
		max = e[i].time;
	    }
	}
    }

    return max;
}

const char *srv_name(int i, int cpu)
{
    return srv[i + (cpu * TASKS)].name;
}

int srv_id(int i, int cpu)
{
    return srv[i + (cpu * TASKS)].id;
}

int servers(struct trace *trc)
{
    return trc->last_server;
}

