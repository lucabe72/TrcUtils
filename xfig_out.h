/*
 * This is free software: see GPL.txt
 */
#ifndef XFIG_OUT_H
#define XFIG_OUT_H

void step_compute(int max, int *step, int *scale);
void ax_draw(unsigned long long int min, unsigned long long int max,
	     int step, int scale, int n, int label, int ntot, int ncpu);
void yax_draw(int npids, int npidstot, int ncpu);
void cpu_name(int cpu, int npidstot);
void header_out(void);
void task_plot(struct event ev[], int i, int scale, int id, int tid,
	       int ntot, int ncpu, int cpu, int min);

#endif	/* XFIG_OUT_H */
