CFLAGS = -Wall -Wextra

WARN = -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual \
	-Wcast-align -Wwrite-strings
CFLAGS += $(WARN) -g

APPS = trc2fig stats ftrace2trc

all: $(APPS)

trc2fig: trc2fig.o trace_read.o trace_evt_handle.o xfig_out.o text_out.o event_list.o task_names.o trace_write.o

stats: stats.o trace_read.o trace_evt_handle.o stats_utils.o event_list.o task_names.o stats_events.o

ftrace2trc: ftrace2trc.o event_create.o event_list.o trace_write.o pid_filter.o task_names.o jtrace_read.o

clean:
	rm -f $(APPS) *.o

