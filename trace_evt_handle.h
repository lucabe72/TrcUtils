struct cpu {
    unsigned int cpus;
    int max;
};

int servers(int cpu);
int trace_read_event(void *h, struct cpu *upc, int start, int end);
const char *srv_name(int i, int cpu);
int last_time(struct cpu *upc);
int srv_id(int i, int cpu);
struct cpu *cpus_alloc(void);

