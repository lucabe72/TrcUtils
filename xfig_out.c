#include <stdlib.h>
#include <stdio.h>

#include "event.h"
#include "trace_evt_handle.h"
#include "xfig_out.h"

int steps[] = { 5, 10, 25, 50, 100, 200, 250, 500, 0 };

#define MINSTEP 225

#define MAXT 20000

#define XBORDER 225
#define YBORDER 225
#define YSIZE   750
#define YBOX 225

#define XOFF 450
#define YAX 1800
#define XAX XOFF + XBORDER

#define YSPACE 1000

void step_compute(int max, int *step, int *scale)
{
    int j;

    *step = steps[0];
    j = 1;

    if (MAXT > max + XBORDER) {
	*scale = MAXT / (max + XBORDER);
    } else {
	*scale = 1;
    }
    while ((*step * *scale < MINSTEP) && steps[j] != 0) {
	*step = steps[j++];
    }
}

void ax_draw(unsigned long long int min, unsigned long long int max, int step, int scale, int n,
	     int label, int ntot, int y0)
{
    int max_scale, j;

    max_scale = (max - min + XBORDER) / step;

    y0 = (y0 * ((ntot - 1) * YSPACE + YAX + YBORDER));

    /* Draw the X ax with scale */
    printf("2 1 0 1 0 7 50 0 -1 0.000 0 0 -1 1 0 2\n");
    printf("0 0 1.00 60.00 120.00\n");
    printf("\t%d %d %d %d\n", XOFF , YSPACE * (n + 1) + YAX + y0,
	   XAX + ((int) max - (int) min + XBORDER) * scale,
	   YSPACE * (n + 1) + YAX + y0);

    /* Draw the X ax... */
    printf("2 1 0 1 0 7 50 0 -1 0.000 0 0 -1 1 0 2\n");
    printf("0 0 1.00 60.00 120.00\n");
    printf("\t%d %d %d %d\n", XOFF , YSPACE * n + YAX + y0,
	   XAX + ((int) max - (int) min + XBORDER) * scale, YSPACE * n + YAX + y0);


    for (j = 1; j < max_scale; j++) {
	printf("2 1 0 1 0 7 50 0 -1 0.000 0 0 -1 0 0 2\n");
	printf("\t%d %d %d %d\n", XAX + (j * step) * scale,
	       YSPACE * n + YAX + YBORDER / 2 + y0,
	       XAX + (j * step) * scale, YSPACE * n + YAX + y0);
	if (label) {
	    printf("2 1 0 1 0 7 50 0 -1 0.000 0 0 -1 0 0 2\n");
	    printf("\t%d %d %d %d\n", XAX + (j * step) * scale,
		   YSPACE * (n + 1) + YAX + YBORDER / 2 + y0,
		   XAX + (j * step) * scale, YSPACE * (n + 1) + YAX + y0);
	    printf("4 0 0 50 0 0 12 4.7124 4 135 345 ");
	    printf("%d %d ", XAX + (j * step) * scale - 60,
		   YSPACE * (n + 1) + YAX + YBORDER + y0);
	    printf("%d\\001\n", j * step);
	} else {
	}
    }
}

void label_plot(int n, const char *name, int y0)
{
    printf("4 2 0 50 0 0 12 0.0 4 135 345 ");
    printf("%d %d ", XAX, YSPACE * n - (int) (0.2 * YSPACE) + YAX + y0);
    printf("%s\\001\n", name);
}

void header_out(void)
{
    printf("#FIG 3.2\nLandscape\nCenter\nMetric\nA4\n100.00\nSingle\n-2\n1200 2\n");
}

void cpu_name(int cpu, int npidstot)
{
    char name[64];

    sprintf(name, "CPU %d", cpu);
    int y0 = cpu * ((npidstot - 1) * YSPACE + YAX + YBORDER);

    printf("4 2 0 50 0 0 12 0.0 4 135 345 ");
    printf("%d %d ", XAX, YSPACE + y0);
    printf("%s\\001\n", name);
}

void yax_draw(int npids, int npidstot, int y0)
{
    /* Draw the Y ax... */
    y0 *= (npidstot - 1) * YSPACE + YAX + YBORDER;
    printf("2 1 0 1 0 7 50 0 -1 0.000 0 0 -1 1 0 2\n");
    printf("0 0 1.00 60.00 120.00\n");
    printf("\t%d %d %d %d\n", XAX, npids * YSPACE + YAX + YBORDER + y0,
	   XAX, YAX - YSIZE + y0);
}

static void job_plot(int start, int stop, int col, int n, int scale,
		     int y0)
{
    /* 15 is 75% filled (area fill goes from 0 to 20) */
    /* Depth 100: in bkg respect to all other objs... */
    printf("2 2 0 1 0 7 100 0 %d 0.000 0 0 -1 0 0 5\n", col * 20 / 100);
    printf("\t%d %d ", XAX + (int) start * scale, YSPACE * n + YAX + y0);
    printf("%d %d ", XAX + (int) start * scale,
	   YSPACE * n + YAX - YBOX + y0);
    printf("%d %d ", XAX + (int) stop * scale,
	   YSPACE * n + YAX - YBOX + y0);
    printf("%d %d ", XAX + (int) stop * scale, YSPACE * n + YAX + y0);
    printf("%d %d\n", XAX + (int) start * scale, YSPACE * n + YAX + y0);
}

static void r_plot(int t, int n, int scale, int y0)
{
    printf("2 1 0 1 0 7 50 0 -1 0.000 0 0 -1 0 1 2\n");
    printf("1 1 1.00 60.00 120.00\n");
    printf("\t%d %d ", XAX + (int) t * scale, YSPACE * n + YAX + y0);
    printf("%d %d\n", XAX + (int) t * scale,
	   (YSPACE * n + YAX + 2 * YBOX) + y0);
}

static void d_plot(int t, int n, int scale, int y0)
{
    printf("2 1 0 1 0 7 50 0 -1 0.000 0 0 -1 0 1 2\n");
    printf("1 1 1.00 60.00 120.00\n");	/* Arrow type: closed triangle */
    printf("\t%d %d ", XAX + (int) t * scale, (YSPACE * n + YAX) + y0);
    printf("%d %d\n", XAX + (int) t * scale,
	   (YSPACE * n + YAX - 2 * YBOX) + y0);
}

static void arc_plot(int t1, int t2, int n, int scale, int y0)
{
    printf("3 0 2 1 0 7 50 0 -1 0.000 0 0 1 3\n");
    printf("1 1 1.00 60.00 120.00\n");
    printf("\t%d %d ", XAX + (int) t1 * scale,
	   (YSPACE * n + YAX - (int) (2.2 * YBOX)) + y0);
    printf("%d %d ", XAX + (int) (t1 + t2) / 2 * scale,
	   (YSPACE * n + YAX - (int) (2.5 * YBOX)) + y0);
    printf("%d %d\n", XAX + (int) t2 * scale,
	   (YSPACE * n + YAX - (int) (2.2 * YBOX)) + y0);
    printf("1 1 1\n");
}

void task_plot(struct event ev[], int i, int scale, int id, int tid,
	       int ntot, int y0, int cpu, int min)
{
    int j;
    int start, stop, colour;

    y0 *= (ntot - 1) * YSPACE + YAX + YBORDER;

    colour = 75;		/* 75 % */
    start = -1;
    stop = 0;

    for (j = 0; j < i; j++) {
	if (ev[j].task == id) {
	    switch (ev[j].type) {
	    case TASK_IDLE:
	    case TASK_SIGNAL:
	    case TASK_WAIT:
		break;
	    case TASK_NAME:
		//label_plot(tid, srv_name(tid, cpu, ls), y0);
		label_plot(tid, srv_name(id, cpu), y0);
		break;
	    case TASK_DLINEPOST:
		arc_plot(ev[j].new_dl, ev[j].old_dl, tid, scale, y0);
	    case TASK_DLINESET:
		d_plot(ev[j].new_dl, tid, scale, y0);
		break;
	    case TASK_ARRIVAL:
		r_plot(ev[j].time-min, tid, scale, y0);
		break;
	    case TASK_END:
		colour = 25 + (50 - (colour - 25));	/* Switch between 75 and 25 */
		break;
	    case TASK_SCHEDULE:
		if (start != -1) {
		    fprintf(stderr,
			    "[%d] Error: Task %d arrives at time %d with arr = %d\n",
			    j, id, ev[j].time, start);
		    exit(-1);
		}
		start = ev[j].time;
		break;
	    case TASK_DESCHEDULE:
		if (start == -1) {
		
		//r_plot(ev[j].time, tid, scale, y0);
		    /*fprintf(stderr,
			    "Error: Task %d ends at time %d with arr = 0\n",
			    id, ev[j].time);
		    exit(-1);*/
		}
		else{
		stop = ev[j].time;

		if ((stop - start) > 0) {	//box size > 0
		    job_plot(start-min, stop-min, colour, tid, scale, y0);
		}

		start = -1;
		stop = 0;
		}
		break;
	    default:
		fprintf(stderr, "Unkonown event type %d\n", ev[j].type);
	    }
	}
    }
}
