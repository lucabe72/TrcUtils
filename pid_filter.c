#include <stdlib.h>
#include <string.h>

#include "pid_filter.h"

static int *filter;
static unsigned int size;

static int createFilterSupport(char *temp, int j)
{
  int num = -1;
  temp[j] = '\0';
  size++;

  num = atoi(temp);

  if (num == 0 && (strlen(temp) > 1 || temp[0] != '0')) {
    return 1;
  } else {
    filter = (int *) realloc(filter, size * sizeof(int));
    filter[size - 1] = num;
    return 0;
  }
}

void destroyPidsFilter(void)
{
  free(filter);
}

int createPidsFilter(const char *pids)
{
  unsigned int i, j;
  char *temp;

  if (!pids)
    return 0;
  temp = malloc(strlen(pids) + 1);
  for (i = 0, j = 0; i < strlen(pids); i++) {
    if (pids[i] == ':') {
      if (createFilterSupport(temp, j) == 1) {
	free(temp);
	return 1;
      }
      j = 0;
    } else {
      temp[j++] = pids[i];
    }
  }

  if (createFilterSupport(temp, j) == 1) {
    free(temp);
    return 1;
  }

  free(temp);
  return 0;
}

int filterPid(int pid)
{
  unsigned int i;

  if (filter == NULL) {
    return pid;
  }
  for (i = 0; i < size; i++) {
    if (pid == filter[i]) {
      return pid;
    }
  }

  return 0;
}
