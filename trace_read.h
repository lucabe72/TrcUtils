/*
 * This is free software: see GPL.txt
 */
#ifndef TRACE_READ_H
#define TRACE_READ_H

int trace_common(FILE * f, int *type, int *time, int *task, int *cpu);
char *task_name(FILE * f);
int task_dline(FILE * f);

#endif	/* TRACE_READ_H */
