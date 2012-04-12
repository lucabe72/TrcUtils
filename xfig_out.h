#ifndef __XFIG_OUT_H__
#define __XFIG_OUT_H__

void step_compute(int max, int *step, int *scale);
void ax_draw(unsigned long long int min,unsigned long long int max, int step, int scale, int n,
	     int label, int ntot, int ncpu);
void yAX(int npids, int npidstot, int ncpu);
void cpu_name(int cpu, int npidstot);
void header(void);
void task_plot(struct event ev[], int i, int scale, int id, int tid,
	       int ntot, int ncpu, int cpu, int last_server, int min);

#endif
