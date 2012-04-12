#define EXECUTION_TIME 0
#define RESPONSE_TIME 1
#define INTERVALL_TIME 2
#define CPU_UTILIZ 3

unsigned long int response_time(int pid, unsigned long int time);
unsigned long int pdf_response_time(int pid, unsigned long int time,
				    unsigned long int tollerance);
unsigned long int pdf_intervalls(int pid, unsigned long int time,
				 unsigned long int tollerance);
unsigned long int pdf_executions(int pid, unsigned long int time,
				 unsigned long int tollerance);
float cdf_response_time(int pid, unsigned long int time);


void encod_stats_time(void *l, unsigned long int time, int task,
		      int kind_stat, unsigned long int time_i,
		      unsigned long int pdf, float cdf);

void blocks_task(int pid);

float cdf_executions(int pid, unsigned long int time);
void start_execution(int pid, unsigned long int time);
unsigned long int end_execution(int pid, unsigned long int time);

unsigned long int intervalls(int pid, unsigned long int time);
float cdf_intervalls(int pid, unsigned long int time);

void create_task(int pid, unsigned long int time);

void calculateCPUsUtil(void *f, unsigned long int time);

