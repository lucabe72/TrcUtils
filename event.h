#define TASK_ARRIVAL 0
#define TASK_SCHEDULE 1
#define TASK_DESCHEDULE 2
#define TASK_END 3
#define TASK_DLINEPOST 4
#define TASK_DLINESET 5
#define TASK_WAIT 6
#define TASK_SIGNAL 7
#define TASK_IDLE 8
#define TASK_NAME 9
#define TASK_FORCE_END -3
#define TASK_FORCE_DESCHEDULE -2


struct event {
    /* Common part... */
    int type;
    int task;
    int time;
    int cpu;

    /* Extra part */
    int new_dl;
    int old_dl;
};


