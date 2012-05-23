/*
 * This is free software: see GPL.txt
 */
#ifndef STATS_UTILS_H
#define STATS_UTILS_H

#define EXECUTION_TIME 0
#define RESPONSE_TIME 1
#define INTERVALL_TIME 2
#define CPU_UTILIZ 3

struct record {			//FIXME!!!
  int size;
  struct record_entry *entries;
};

unsigned long int pdf_response_time(int pid, unsigned long int time,
				    unsigned long int tollerance);
unsigned long int pdf_intervalls(int pid, unsigned long int time,
				 unsigned long int tollerance);
unsigned long int pdf_executions(int pid, unsigned long int time,
				 unsigned long int tollerance);
float cdf_response_time(int pid, unsigned long int time);

void pmf_write(FILE *f, int pid, int type);

void stats_print_int(void *l, unsigned long int time, int task,
		     int type, unsigned long int val,
		     unsigned long int pdf, float cdf);

float cdf_executions(int pid, unsigned long int time);

float cdf_intervalls(int pid, unsigned long int time);

#endif	/* STATS_UTILS_H */
