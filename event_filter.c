#include "event.h"
#include "pid_filter.h"
#include "event_filter.h"

void filterEvent(struct event *e)
{
  e->task = filterPid(e->task);
}
