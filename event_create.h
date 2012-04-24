struct event *evt_get(void);
void evt_dispatch(int lpid, int rpid, int cpu,
		  unsigned long long int time);
void evt_force_desch(int pid, int cpu, unsigned long long int time);
void evt_initialize(int pid, int cpu, unsigned long long int time);
void evt_activation(int pid, int cpu, unsigned long long int time);
void evt_deactivation(int pid, int cpu, unsigned long long int time);
void evt_force_deactivation(int pid, int cpu, unsigned long long int time);
void evt_creation(int pid, const char *name, int cpu,
		  unsigned long long int time);
void evt_start(unsigned long long int time);
