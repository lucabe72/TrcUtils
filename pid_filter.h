/*
 * This is free software: see GPL.txt
 */
#ifndef PID_FILTER_H
#define PID_FILTER_H

int createPidsFilter(const char *pids);
int filterPid(int pid);
void destroyPidsFilter(void);

#endif	/* PID_FILTER_H */
