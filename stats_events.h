/*
 * This is free software: see GPL.txt
 */
#ifndef STATS_EVENTS_H
#define STATS_EVENTS_H

void start_execution(int pid, unsigned long int time);
unsigned long int end_execution(int pid, unsigned long int time);
unsigned long int intervalls(int pid, unsigned long int time);
void blocks_task(int pid);
unsigned long int response_time(int pid, unsigned long int time);
void create_task(int pid, unsigned long int time);

void task_set_p(int pid, int type);
void task_unset_p(int pid, int type);
struct record *record_find(int pid);

void calculateCPUsUtil(void *f, unsigned long int time);

#endif	/* STATS_EVENTS_H */
