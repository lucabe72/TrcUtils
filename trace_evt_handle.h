struct trace {
    struct event *ev;
    unsigned long int last_event;
    unsigned int last_server;
};

struct cpu {
    struct trace *trc;
    unsigned int cpus;
};

int trace_read_event(void *h, struct cpu *upc, int start, int end);
const char *srv_name(int i, int cpu);
int last_time(struct cpu *upc);
int srv_id(int i, int cpu);
struct cpu *cpus_alloc(void);

