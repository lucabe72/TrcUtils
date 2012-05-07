#ifndef __STATS_GUI_LIB_H__
#define __STATS_GUI_LIB_H__

#define REFRESH_TIME_VALUE 500000	//us
#define REFRESH_TIME_INTERVAL 500000	//us
#define TIME_Y 2
#define TITLE_Y 0
#define TOLL_Y 3
#define COLUMN_Y1 5
#define COLUMN_Y2 6
#define COLUMN_X 1
#define C0 5
#define CO 6
#define CN 15

void initializeT(unsigned long int tollerance);
void exitT(void);
char suEzo(void);
void updateTime(unsigned long long int time);
void createPidRow(int pid, int cpu);
void updateRT(int pid, long long int t);
void updatePDFRT(int pid, long long int t, int periodic);
void updateET(int pid, long long int t);
void updatePDFET(int pid, long long int t, int periodic);
void updateI(int pid, long long int t);
void updatePDFI(int pid, long long int t, int periodic);
void updateCPUsUtil(int pid, float cpu_util, float cpu_wutil,
		    float cpu_autil);

#endif
