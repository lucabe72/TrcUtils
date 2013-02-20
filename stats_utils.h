#define EXECUTION_TIME 0
#define RESPONSE_TIME 1
#define INTERVALL_TIME 2
#define CPU_UTILIZ 3

struct record {			//FIXME!!!
  int size;
  struct record_entry *entries;
};

void pdf_response_time(int pid, unsigned long int time,
				    unsigned long int tollerance);
void pdf_intervalls(int pid, unsigned long int time,
				 unsigned long int tollerance);
void pdf_executions(int pid, unsigned long int time,
				 unsigned long int tollerance);

void pmf_write(FILE *f, int pid, int type);

void stats_print_int(void *l, unsigned long int time, int task,
		     int type, unsigned long int val);
