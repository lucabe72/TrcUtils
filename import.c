#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <getopt.h>

#include "ftrace_read.h"
#include "jtrace_read.h"
#include "event_create.h"
#include "trace_write.h"
#include "pid_filter.h"

#define FTRACE  0
#define JTRACE  1

static int trace_type;
static const char *relevant_pids;

static unsigned int param(int argc, char *argv[])
{
  int c;

  while ((c = getopt(argc, argv, "jp:")) != -1)
    switch (c) {
      case 'j':
        trace_type = JTRACE;
        break;
      case 'p':
        relevant_pids = optarg; 
        break;
      default:
        exit(-1);
  }

  return 0;
}

#if 0
static void endAllTask(int time)
{
    unsigned int i;

    for (i = 0; i < count; i++) {
#if 0
        if (new_pids[i].state != 0) {	//1 - if it scheduled, it'll descheduled
            trc_force_desch(new_pids[i].pid, new_pids[i].cpu, time);
	} else {		//else it'll stoped
	    trc_force_deactivation(new_pids[i].pid, new_pids[i].cpu, time);
	}
#else
	evt_force_deactivation(new_pids[i].pid, new_pids[i].cpu, time);
#endif
    }
}
#endif

int main(int argc, char *argv[])
{
    FILE *f;
    //int start = 0, fc = 0, fo = 0, opt, i;
    long long int time = 0, res = 0, done = 0;

    param(argc, argv);
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);

        return -1;
    }

    createPidsFilter(relevant_pids);

    f = fopen(argv[argc-1], "r");
    if (f == NULL) {
        perror("Cannot open input file");

        return -1;
    }

    while (!done) {
        struct event *e;

        time = res + 1;
        switch (trace_type) {
            case FTRACE:
                res = ftrace_parse(f);
                break;
            case JTRACE:
                res = jtrace_read(f);
                break;
            default:
                fprintf(stderr, "Unknown trace type: this shouldn't have happened...\n");
                exit(-1);
        }
        while (e = evt_get()) {
          trc_write(e);
          free(e);
        }
        done = feof(f) || (res < 0);
    }

//    endAllTask(time);

    destroyPidsFilter();

    return 0;
}