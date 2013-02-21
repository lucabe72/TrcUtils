/*
 * This is free software: see GPL.txt
 */
#ifndef TRACE_EVT_HANDLE_H 
#define TRACE_EVT_HANDLE_H

int servers(int cpu);
int trace_read_event(void *h, int start, int end);
const char *srv_name(int i, int cpu);
int last_time(void);
int srv_id(int i, int cpu);

#endif	/* TRACE_EVT_HANDLE_H */
