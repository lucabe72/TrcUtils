/*
 * This is free software: see GPL.txt
 */
#include <netinet/in.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

#include "event.h"
#include "event_list.h"
#include "task_names.h"
#include "trace_read.h"

static uint32_t swap(uint32_t val)
{
  return ntohl(val);
}

static uint32_t trace_read_int(FILE * f)
{
  uint32_t sw;

  fread(&sw, 4, 1, f);
  return swap(sw);
}

static int trace_common_nocpu(FILE * f, int *type, int *time, int *task)
{
  *type = trace_read_int(f);
  *time = trace_read_int(f);
  *task = trace_read_int(f);
  if (feof(f)) {
    return -1;
  }

  return 0;
}

int trace_common(FILE * f, int *type, int *time, int *task, int *cpu)
{
  *type = trace_read_int(f);
  *time = trace_read_int(f);
  *task = trace_read_int(f);
  *cpu = trace_read_int(f);
  if (feof(f)) {
    return -1;
  }

  return 0;
}

char *task_name(FILE * f)
{
  int size;
  char *name;

  size = trace_read_int(f);
  name = malloc(size + 1);
  if (name == NULL) {
    return name;
  }
  fread(name, size, 1, f);
  name[size] = 0;

  return name;
}

int task_dline(FILE * f)
{
  return trace_read_int(f);
}

int trace_read(FILE *f, int nocpu)
{
  int type, time, task, cpu, res, old_dl = 0, new_dl = 0;
  const char *name;

  if (nocpu) {
    res = trace_common_nocpu(f, &type, &time, &task);
    cpu = 0;
  } else {
    res = trace_common(f, &type, &time, &task, &cpu);
  }
  if (res < 0) {
    return res;
  }

  switch (type) {
    case TASK_ARRIVAL:
    case TASK_SCHEDULE:
    case TASK_END:
    case TASK_DESCHEDULE:
      evt_store(type, time, task, cpu);
      break;
    case TASK_NAME:
      name = task_name(f);
      name_register(task, cpu, name);
      evt_store(type, time, task, cpu);
      break;
    case TASK_DLINEPOST:
      old_dl = task_dline(f);
    case TASK_DLINESET:
      new_dl = task_dline(f);
      if (name_get(task, cpu) == NULL) {
        return -5;
      }
      evt_store_dl(type, time, task, cpu, old_dl, new_dl);
      break;
    default:
      fprintf(stderr, "Strange event type %d\n", type);
      return -4;
  }

  return 0;
}
