#include <netinet/in.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define JJOB_ARRIVAL		0
#define JTASK_EXECUTION		1
#define JTASK_PREEMPTION	2
#define JJOB_END		3
#define JTASK_CREATION		9
#define JDEADLINE_POSTPONING	4
#define JDEADLINE_SETTING	5
#define JDEADLINE_MISS		10

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

static char *string_read(FILE *f)
{
  char *res;
  int size = 16, v = 1, i = 0;

  res = malloc(size);

  while (v != 0) {
    v = fgetc(f);
    if (v == EOF) {
      free(res);

      return NULL;
    }
    res[i++] = (char)v;
    if (i == size) {
      size += 16;
      res = realloc(res, size);
    }
  }

  return res;
}

static int header_read(FILE *f)
{
  char *header;
  int res = -1;
  
  header = string_read(f);
  if (header) {
    if (memcmp(header, "version ", 8)) {
      fprintf(stderr, "Wrong header: %s\n", header);

      res = -2;
    } else {
      printf("Header: %s\n", header);
      res = 0;
    }

    free(header);
  }

  return res;
}

static int jtrace_common(FILE *f, int *len, int *type, int *time, int *task)
{
  *len = trace_read_int(f);
  *len = *len - 3 * 4;
  *type = trace_read_int(f);
  *time = trace_read_int(f);
  *task = trace_read_int(f);
  if (feof(f)) {
    return -1;
  }

  return 0;
}

int jtrace_read(FILE *f)
{
  int len, type, time, task, cpu, res;
  char name[32];
  int name_len, name_len1, d;

  res = jtrace_common(f, &len, &type, &time, &task);
  if (res < 0) {
    return res;
  }
  printf("Len: %d Type: %d Time: %d Task: %d\n", len, type, time, task);
  switch (type) {
    case JTASK_CREATION:
      name_len = len - 4;
      name_len1 = trace_read_int(f);
      if (name_len != name_len1) {
        fprintf(stderr, "Error: name len %d != %d\n", name_len, name_len1);
      }
      fread(name, name_len1, 1, f);
      name[name_len1] = 0;
      printf("Create %s\n", name);
      break;
    case JTASK_EXECUTION:
      if (len != 4) {
        fprintf(stderr, "Error: len = %d != 4 (CPU) - %d\n", len, type);
        return -3;
      }
      cpu = trace_read_int(f);
      printf("Task %d dispatched on CPU %d at time %d\n", task, cpu, time);
      break;
    case JTASK_PREEMPTION:
      if (len != 4) {
        fprintf(stderr, "Error: len = %d != 4 (CPU) - %d\n", len, type);
        return -3;
      }
      cpu = trace_read_int(f);
      printf("Task %d preempted on CPU %d at time %d\n", task, cpu, time);
      break;
    case JJOB_ARRIVAL:
      if (len != 0) {
        fprintf(stderr, "Error: len = %d != 0 - %d\n", len, type);
        return -3;
      }
      printf("Task %d wakes up at time %d\n", task, time);
      break;
    case JJOB_END:
      if (len != 4) {
        fprintf(stderr, "Error: len = %d != 4 (CPU) - %d\n", len, type);
        return -3;
      }
      cpu = trace_read_int(f);
      printf("Task %d blocks on CPU %d at time %d\n", task, cpu, time);
      break;
    case JDEADLINE_SETTING:
      if (len != 4) {
        fprintf(stderr, "Error: len = %d != 4 (deadline) - %d\n", len, type);
        return -3;
      }
      d = trace_read_int(f);
      printf("Deadline for task %d se to %d at time %d\n", task, d, time);
      break;
    default:
      printf("Event %d - Len %d\n", type, len);
      return -4;
  }

  return 0;
}

int main(int argc, char *argv[])
{
  FILE *f;

  f = fopen(argv[1], "r");
  header_read(f);
  while(jtrace_read(f) >= 0);

  return 0;  
}
