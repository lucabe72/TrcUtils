/*
 * This is free software: see GPL.txt
 */
#ifndef TEXT_OUT_H
#define TEXT_OUT_H

void trace_dump_event(struct event *e);
void trace_info(struct event *ev, unsigned int last_event,
		unsigned int last_server);

#endif	/* TEXT_OUT_H */
