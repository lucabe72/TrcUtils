/*
 * This is free software: see GPL.txt
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stats_events.h"
#include "stats_utils.h"

struct record_entry {
  int times;
  unsigned long int time;
};

#define RECORD_UNBLOCK   0
#define RECORD_RESPONSE  1
#define RECORD_EXECUTION 2

static const char *stat_name[] = {
  "exec",
  "resp",
  "intr",
  "util",
};

static struct record *updateRecord(unsigned long int time,
				   unsigned long int tollerance,
				   struct record *r)
{
  int i;

  if (time == 0) {
    return r;
  }

  for (i = 0; i < r->size; i++) {
    if ((time <= r->entries[i].time + tollerance)
	&& (time + tollerance >= r->entries[i].time)) {
      r->entries[i].times++;

      return r;
    }
  }

  if (tollerance) {
    time = ((time / tollerance) + 1) * tollerance;
    //time += tollerance + (time % tollerance)
  }
  r->entries = realloc(r->entries, (r->size + 1) * sizeof(struct record));
  r->entries[r->size].time = time;
  r->entries[r->size].times = 1;
  r->size = r->size + 1;

  return r;
}

static void pdf_generic(int pid, unsigned long int time,
				     unsigned long int tollerance,
				     int type)
{
  struct record *r = record_find(pid);

  updateRecord(time, tollerance, &r[type]);

  if (r[type].size == 1) {
    task_set_p(pid, type);
  } else {
    task_unset_p(pid, type);
  }
}

void pdf_response_time(int pid, unsigned long int time,
				    unsigned long int tollerance)
{
  pdf_generic(pid, time, tollerance, RECORD_RESPONSE);
}

void pdf_executions(int pid, unsigned long int time,
				 unsigned long int tollerance)
{
  pdf_generic(pid, time, tollerance, RECORD_EXECUTION);
}

void pdf_intervalls(int pid, unsigned long int time,
				 unsigned long int tollerance)
{
  pdf_generic(pid, time, tollerance, RECORD_UNBLOCK);
}

static int entries_cmp(const void *a, const void *b)
{
  const struct record_entry *pa = a;
  const struct record_entry *pb = b;

  return pa->time > pb->time;
}

void pmf_write(FILE *f, int pid, int type)
{
  int i, sum;
  struct record *r1 = record_find(pid);
  struct record *r = &r1[type];

  qsort(r->entries, r->size, sizeof(struct record_entry), entries_cmp); 
  sum = 0;
  for (i = 0; i < r->size; i++) {
    sum += r->entries[i].times;
  }

  fprintf(f, "0 0\n");
  for (i = 0; i < r->size; i++) {
    fprintf(f, "%lu %lf\n", r->entries[i].time, (double)r->entries[i].times / sum);
  }
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


