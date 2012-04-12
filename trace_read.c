#include <netinet/in.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

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

int trace_common(FILE *f, int *type, int *time, int *task, int *cpu)
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

char *task_name(FILE *f)
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

int task_dline(FILE *f)
{
  return trace_read_int(f);
}
