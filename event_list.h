void evt_store_dl(int type, int time, int pid, int cpu, int old_dl, int new_dl);
void evt_store(int type, int time, int pid, int cpu);
struct event *evt_get(void);

