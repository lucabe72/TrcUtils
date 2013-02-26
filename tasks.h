struct task_set;

struct task_set *taskset_init(void);
int taskset_add_task(struct task_set *p, unsigned int pid, int cpu, void *attr);
void *taskset_find_task(const struct task_set *p, unsigned int pid, int cpu);
void *taskset_nth_task(const struct task_set *ts, unsigned int i, unsigned int *pid, int *cpu);

