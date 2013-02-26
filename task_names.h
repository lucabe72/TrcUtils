/*
 * This is free software: see GPL.txt
 */
#ifndef TASK_NAMES_H
#define TASK_NAMES_H

int task_ith(unsigned int i, int cpu);
void name_register(int pid, int cpu, const char *name);
const char *name_get(int pid, int cpu);

#endif	/* TASK_NAMES_H */
