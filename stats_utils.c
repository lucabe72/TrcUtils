/*
 * This is free software: see GPL.txt
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stats_events.h"
#include "cdf_record.h"
#include "stats_utils.h"

#define RECORD_UNBLOCK   0
#define RECORD_RESPONSE  1
#define RECORD_EXECUTION 2

static const char *stat_name[] = {
  "exec",
  "resp",
  "intr",
  "util",
};

static void pdf_generic(int pid, unsigned long int time, int type)
{
  struct record **r = record_find(pid);

  r[type] = cdf_record_update(r[type], time);
}

void pdf_response_time(int pid, unsigned long int time)
{
  pdf_generic(pid, time, RECORD_RESPONSE);
}

void pdf_executions(int pid, unsigned long int time)
{
  pdf_generic(pid, time, RECORD_EXECUTION);
}

void pdf_intervalls(int pid, unsigned long int time)
{
  pdf_generic(pid, time, RECORD_UNBLOCK);
}

void pmf_write(FILE *f, int pid, int type)
{
  struct record **r = record_find(pid);

  fprintf(f, "0 0\n");
  cdf_print(f, r[type]);
}

void stats_print_int(void *l, unsigned long int time, int task,
		     int type, unsigned long int val)
{
  if (l == NULL) {
    return;
  }
  //fprintf(l, "%ld %d %s %d %ld %ld %d\n", time, task, getTaskName(task), kind_stat, time_i, pdf, cdf);
  fprintf(l, "%ld %d %s %lu\n", time, task, stat_name[type], val);
}


