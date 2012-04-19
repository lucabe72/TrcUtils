#define EXECUTION_TIME 0
#define RESPONSE_TIME 1
#define INTERVALL_TIME 2
#define CPU_UTILIZ 3

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

float cdf_executions(int pid, unsigned long int time);

float cdf_intervalls(int pid, unsigned long int time);

