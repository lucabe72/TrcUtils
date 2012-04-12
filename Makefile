CFLAGS = -Wall -Wextra

WARN = -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual \
	-Wcast-align -Wwrite-strings
CFLAGS += $(WARN) -g

APPS = trc2fig stats ftrace2trc

all: $(APPS)

trc2fig: trc2fig.o trace_read.o trace_evt_handle.o xfig_out.o text_out.o

stats: stats.o trace_read.o trace_evt_handle.o stats_utils.o

ftrace2trc: ftrace2trc.o trace_write.o pid_filter.o

clean:
	rm -f $(APPS) *.o

