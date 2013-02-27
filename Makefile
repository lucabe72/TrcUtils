CFLAGS = -Wall -Wextra

WARN = -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual \
	-Wcast-align -Wwrite-strings
CFLAGS += $(WARN) -g

APPS = export stats import visual pest periodic_monitor

all: $(APPS)

export: export.o trace_read.o trace_evt_handle.o xfig_out.o text_out.o event_list.o task_names.o trace_write.o jtrace_write.o tasks.o

stats: stats.o trace_read.o trace_evt_handle.o stats_utils.o event_list.o task_names.o stats_events.o cdf_record.o tasks.o tasks.o

import: import.o event_create.o event_list.o trace_write.o pid_filter.o task_names.o jtrace_read.o ftrace_read.o trace_read.o oftrace_read.o l4trace_read.o event_filter.o tasks.o

visual: LDLIBS+=-lncurses
visual: visual.o trace_read.o trace_evt_handle.o event_list.o task_names.o stats_events.o visual_gui.o tasks.o

pest: LDLIBS+=-lm
pest: pest.o period_detect.o event_list.o trace_evt_handle.o task_names.o trace_read.o tasks.o
periodic_monitor: LDLIBS+=-lncurses -lm
periodic_monitor: periodic_monitor.o period_detect.o event_list.o trace_evt_handle.o task_names.o trace_read.o tasks.o

clean:
	rm -f $(APPS) *.o

