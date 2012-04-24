CFLAGS = -Wall -Wextra

WARN = -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual \
	-Wcast-align -Wwrite-strings
CFLAGS += $(WARN) -g

APPS = export stats import 

all: $(APPS)

export: export.o trace_read.o trace_evt_handle.o xfig_out.o text_out.o event_list.o task_names.o trace_write.o

stats: stats.o trace_read.o trace_evt_handle.o stats_utils.o event_list.o task_names.o stats_events.o

import: import.o event_create.o event_list.o trace_write.o pid_filter.o task_names.o jtrace_read.o ftrace_read.o

clean:
	rm -f $(APPS) *.o

