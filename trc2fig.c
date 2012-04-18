#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include "event.h"
#include "event_list.h"
#include "trace_evt_handle.h"
#include "text_out.h"
#include "xfig_out.h"

#define MAX_EVENTS 10000

/* FIXME! */
#define OUTPUT_DUMP 1
#define OUTPUT_INFO 2
#define OUTPUT_TRACE 3

static int output_type;

static void help(void)
{
    fprintf(stdout, "Usage:\n");
    fprintf(stdout, "trc2fig <options> file\n\n");

    fprintf(stdout, "Options:\n");
    fprintf(stdout, "-C n\tMaximum number of CPU\n");
    fprintf(stdout, "-E n\tMaximum number of events\n");
    fprintf(stdout, "-S n\tMaximum number of servers\n");
    fprintf(stdout, "-s t\tStart time\n");
    fprintf(stdout, "-e t\tEnd Time\n");
}

static unsigned int param(int argc, char *argv[], int *start_time, int *end_time)
{
    int c;
    while ((c = getopt(argc, argv, "C:E:S:s:e:id")) != -1)
	switch (c) {
	case 'C':
	    break;
	case 'E':
	    break;
	case 'S':
	    break;
	case 's':
	    *start_time = atoi(optarg);
	    break;
	case 'e':
	    *end_time = atoi(optarg);
	    break;
        case 'i':
            output_type = OUTPUT_INFO;
	    break;
        case 'd':
            output_type = OUTPUT_DUMP;
            break;
	default:
	    help();
	    exit(2);
	}
   return 0;
}

/*
static unsigned long int maxLastEvent(struct cpu *upc)
{
    unsigned long int max = 0;
    int i;
    struct trace *trc;

    for (i = 0; i < MAX_CPUS; i++) {
	    trc = &upc->trc[i];
	    if (trc->last_event > max) {
		max = trc->last_event;
	    }
	}
    return max;
}
*/

int main(int argc, char *argv[])
{
    FILE *f;
    unsigned int j, z, cpus;
    int done, i, last_server, last_server_tot = 0;
    int step, scale, start_time = 0, end_time = 0;
    char *fname;
    struct event_trace *t;

    param(argc, argv,&start_time,&end_time);

    fname = argv[argc-1];
    f = fopen(fname, "r");
    if (f == NULL) {
	perror(fname);

	return -1;
    }

    done = 0;
    while (!done) {
        int res;

	res = trace_read_event(f, start_time, end_time);
	//done = feof(f) || (maxLastEvent(upc) >= MAX_EVENTS)
	done = feof(f) || (res < 0);
    }


    step_compute(last_time(), &step, &scale);
    fprintf(stderr, "Step: %d\tScale: %d\n", step, scale);

    switch (output_type) {
        case OUTPUT_DUMP:
            done = 0;
            i = 0;
            while(!done) {
                struct event *e;

                e = evt_get();
                if (e == NULL) {
                    done = 1;
                } else {
                    printf("[%d - %d]\t", i++, e->cpu);
                    trace_dump_event(e, servers(e->cpu));
                }
            }
            break;
        case OUTPUT_TRACE:
            done = 0;
            while(!done) {
                struct event *e;

                e = evt_get();
                if (e == NULL) {
                    done = 1;
                } else {
                    trace_write_event(e, servers(e->cpu));
                }
            }
            break;
        case OUTPUT_INFO:
            t = trace_export(&cpus);

            for (j = 0; j < cpus; j++) {
                if (servers(j) > 1) {
                    trace_info(t[j].ev, t[j].last_event, servers(j));
                }
            }
            break;
        default:
            fprintf(stderr, "Printing...\n");
            fprintf(stderr,
	            "If you don't see a process: its beahviour is a sequence of events too far from time 0.\n");

            header_out();
            t = trace_export(&cpus);
            for (j = 0, z = 0; j < cpus; j++) {
                last_server = servers(j);
                if (last_server > 1) {	//last_server == 0 is idle
                    fprintf(stderr, "CPU %d\n", z);
                    yax_draw(last_server, last_server_tot, z);
                    cpu_name(z, last_server_tot);

                    for (i = 0; i < last_server; i++) {
                        //fprintf(stderr, "\t%s\n", srv_name(i, j, last_server));
                        fprintf(stderr, "\t%s\n", srv_name(i, j));
                        ax_draw(start_time, last_time(), step, scale, i, (i == (last_server - 1)), last_server_tot, z);
                        //task_plot(trac->ev, trac->last_event, scale, srv_id(i, j, last_server), i, last_server_tot, z, j, last_server,start_time);
                        task_plot(t[j].ev, t[j].last_event, scale, srv_id(i, j), i, last_server_tot, z, j, start_time);
	            }
	            last_server_tot += last_server + 1;
	            z++;
                }
            }
    }

    return 0;
}
