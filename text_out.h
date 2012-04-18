#ifndef __TEXT_OUT_H__
#define __TEXT_OUT_H__

void trace_dump_event(struct event *e);
void trace_info(struct event *ev, unsigned int last_event, unsigned int last_server);
void trace_write_event(struct event *e);

#endif	/* __TEXT_OUT_H__ */
