#ifndef __TEXT_OUT_H__
#define __TEXT_OUT_H__

void trace_dump_event(struct event *e, int last_server, int cpu);
void trace_info(struct trace *trc, int cpu);
void trace_write_event(struct event *e, int last_server, int cpu);

#endif	/* __TEXT_OUT_H__ */
