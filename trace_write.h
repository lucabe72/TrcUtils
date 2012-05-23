/*
 * This is free software: see GPL.txt
 */
#ifndef TRACE_WRITE_H
#define TRACE_WRITE_H

void trc_initialize(int pid, int cpu, unsigned long long int time);
void trc_dispatch(int lpid, int rpid, int cpu,
		  unsigned long long int time);
void trc_activation(int pid, int cpu, unsigned long long int time);
void trc_deactivation(int pid, int cpu, unsigned long long int time);
void trc_creation(int pid, const char *name, int cpu,
		  unsigned long long int time);
void trc_start(unsigned long long int time);
void trc_force_desch(int pid, int cpu, unsigned long long int time);
void trc_force_deactivation(int pid, int cpu, unsigned long long int time);

void trc_write(struct event *e);

#endif	/* TRACE_WRITE_H */

