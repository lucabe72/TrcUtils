#include <curses.h>
#include <string.h>
#include <malloc.h>
#include <sys/time.h>
#include <signal.h>

#include "task_names.h"
#include "visual_gui.h"

struct row {
    int pid;
    int p_e, p_u, p_r;
    int y;
    long long int i;
    long long int pdf_i;
    long long int rt;
    long long int pdf_rt;
    long long int et;
    long long int pdf_et;
    long long int ei;
    long long int pdf_ei;
    float cpu_util;
    float cpu_wutil;
    float cpu_autil;
    char name[16];
};

static const char *title =
    "FTrace Stats - Response Time, Execution Time, Intervals and more";
static const char *time_title = "Misured time:";
static const char *tollerance_title = "Tollerance  :";
static const char
*l0 = "R Time - Response Time             ",
    *l1 = "E Time - Execution Time            ",
    *l2 = "I Time - Intervall Time            ",
    *l3 =
    "PDFm - the mode of the probability density function            ",
    *l4 = "A - average case ", *l5 = "W - worst case ", *l6 =
    "Up - u, Down - d ", *l7 = "Quit - q         ", *l8 =
    "Periodicity", *hN = "  Task's name  ", *hNN = "", *h0 =
    " PID ", *h00 = "  #  ", *h1 = "  R Time  ", *h11 = "    us    ", *h2 =
    "PDFm R Time", *h22 = "    us    ", *h3 = "  E Time  ", *h33 =
    "    us    ", *h4 = "PDFm E Time", *h44 = "    us    ", *h7 =
    "  I Time  ", *h77 = "    us    ", *h8 = "PDFm I Time", *h88 =
    "    us    ", *h9 = "CPUs Utiliz", *h99 = "  \% at sec ", *hw =
    "W CPUs Uti.", *hw0 = "     \%     ", *ha = "A CPUs Uti.", *ha0 =
    "     \%     ";
static int xa, xw, xn, x0, x1, x2, x3, x4, x7, x8, x9, t0, COLUMN_Y2_max,
    COLUMN_Y2_min, ipid = 0;
static struct row *rows = NULL;

static int max(int c, int x)
{
    if (c > x)
	return c;
    else
	return x;
}

static void showPidRow(int key, int y)
{
    char cpu_wutil[max(CO, strlen(hw))];
    char cpu_autil[max(CO, strlen(ha))];
    char cpu_util[max(CO, strlen(h9))];
    char pdf_i[max(CO, strlen(h8))];
    char _i[max(CO, strlen(h7))];
    char pdf_et[max(CO, strlen(h4))];
    char et[max(CO, strlen(h3))];
    char pdf_rt[max(CO, strlen(h2))];
    char rt[max(CO, strlen(h1))];
    char pid_s[max(C0, strlen(h0))];
    char name_t[max(CN, strlen(hN))];
    int i;
    for (i = 0; i < max(CO, strlen(hw)); i++) {
	cpu_wutil[i] = ' ';
    }
    cpu_wutil[i] = '\0';
    for (i = 0; i < max(CO, strlen(ha)); i++) {
	cpu_autil[i] = ' ';
    }
    cpu_autil[i] = '\0';
    for (i = 0; i < max(CO, strlen(h9)); i++) {
	cpu_util[i] = ' ';
    }
    cpu_util[i] = '\0';
    for (i = 0; i < max(CO, strlen(h8)); i++) {
	pdf_i[i] = ' ';
    }
    pdf_i[i] = '\0';
    for (i = 0; i < max(CO, strlen(h7)); i++) {
	_i[i] = ' ';
    }
    _i[i] = '\0';
    for (i = 0; i < max(CO, strlen(h4)); i++) {
	pdf_et[i] = ' ';
    }
    pdf_et[i] = '\0';
    for (i = 0; i < max(CO, strlen(h3)); i++) {
	et[i] = ' ';
    }
    et[i] = '\0';
    for (i = 0; i < max(CO, strlen(h2)); i++) {
	pdf_rt[i] = ' ';
    }
    pdf_rt[i] = '\0';
    for (i = 0; i < max(CO, strlen(h1)); i++) {
	rt[i] = ' ';
    }
    rt[i] = '\0';
    for (i = 0; i < max(C0, strlen(h0)); i++) {
	pid_s[i] = ' ';
    }
    pid_s[i] = '\0';
    for (i = 0; i < max(CN, strlen(hN)); i++) {
	name_t[i] = ' ';
    }
    name_t[i] = '\0';

    mvaddstr(y, xa, cpu_autil);
    mvaddstr(y, xw, cpu_wutil);
    mvaddstr(y, x9, cpu_util);
    mvaddstr(y, x8, pdf_i);
    mvaddstr(y, x7, _i);
    mvaddstr(y, x4, pdf_et);
    mvaddstr(y, x3, et);
    mvaddstr(y, x2, pdf_rt);
    mvaddstr(y, x1, rt);
    mvaddstr(y, x0, pid_s);
    mvaddstr(y, xn, name_t);

    if (rows[key].cpu_util == 0) {	//No data
	sprintf(cpu_autil, "%*c", max(CO, strlen(ha)), '0');
    } else {
	sprintf(cpu_autil, "%*f", max(CO, strlen(ha)),
		rows[key].cpu_autil);
    }
    if (rows[key].cpu_wutil == 0) {
	sprintf(cpu_wutil, "%*c", max(CO, strlen(hw)), '0');
    } else {
	sprintf(cpu_wutil, "%*f", max(CO, strlen(hw)),
		rows[key].cpu_wutil);
    }
    if (rows[key].cpu_util == 0) {
	sprintf(cpu_util, "%*c", max(CO, strlen(h9)), '0');
    } else {
	sprintf(cpu_util, "%*f", max(CO, strlen(h9)), rows[key].cpu_util);
    }
    if (rows[key].pdf_i == -1) {
	sprintf(pdf_i, "%*c", max(CO, strlen(h8)), '-');
    } else {
	sprintf(pdf_i, "%*lld", max(CO, strlen(h8)), rows[key].pdf_i);
    }
    if (rows[key].i == -1) {
	sprintf(_i, "%*c", max(CO, strlen(h7)), '-');
    } else {
	sprintf(_i, "%*lld", max(CO, strlen(h7)), rows[key].i);
    }
    if (rows[key].pdf_et == -1) {
	sprintf(pdf_et, "%*c", max(CO, strlen(h4)), '-');
    } else {
	sprintf(pdf_et, "%*lld", max(CO, strlen(h4)), rows[key].pdf_et);
    }
    if (rows[key].et == -1) {
	sprintf(et, "%*c", max(CO, strlen(h3)), '-');
    } else {
	sprintf(et, "%*lld", max(CO, strlen(h3)), rows[key].et);
    }
    if (rows[key].pdf_rt == -1) {
	sprintf(pdf_rt, "%*c", max(CO, strlen(h2)), '-');
    } else {
	sprintf(pdf_rt, "%*lld", max(CO, strlen(h2)), rows[key].pdf_rt);
    }
    if (rows[key].rt == -1) {
	sprintf(rt, "%*c", max(CO, strlen(h1)), '-');
    } else {
	sprintf(rt, "%*lld", max(CO, strlen(h1)), rows[key].rt);
    }
    sprintf(pid_s, "%*d", max(C0, strlen(h0)), rows[key].pid);
    sprintf(name_t, "%*s", max(CN, strlen(hN)), rows[key].name);

    mvaddstr(y, xa, cpu_autil);
    mvaddstr(y, xw, cpu_wutil);
    mvaddstr(y, x9, cpu_util);
    if (rows[key].p_u == 1) {
	attron(A_BOLD | COLOR_PAIR(5));
    }
    mvaddstr(y, x8, pdf_i);
    attroff(A_BOLD | COLOR_PAIR(5));
    mvaddstr(y, x7, _i);
    if (rows[key].p_e == 1) {
	attron(A_BOLD | COLOR_PAIR(5));
    }
    mvaddstr(y, x4, pdf_et);
    attroff(A_BOLD | COLOR_PAIR(5));
    mvaddstr(y, x3, et);
    if (rows[key].p_r == 1) {
	attron(A_BOLD | COLOR_PAIR(5));
    }
    mvaddstr(y, x2, pdf_rt);
    attroff(A_BOLD | COLOR_PAIR(5));
    mvaddstr(y, x1, rt);

    if (rows[key].p_u == 1 || rows[key].p_r == 1 || rows[key].p_e == 1) {
	attron(A_BOLD | COLOR_PAIR(5));
    }
    mvaddstr(y, x0, pid_s);
    mvaddstr(y, xn, name_t);
    attroff(A_BOLD | COLOR_PAIR(5));
}

static void printRows(void)
{
    int i, j;

    for (j = 1, i = COLUMN_Y2_min;
	 i < COLUMN_Y2_min + COLUMN_Y2_max - COLUMN_Y2 && i < ipid;
	 i++, j++) {
	showPidRow(i, j + COLUMN_Y2);
    }
}

static void update(int t)
{
    printRows();
    refresh();
}

static void setAlarm(void)
{
    signal(SIGALRM, update);
    struct itimerval t;

    t.it_value.tv_sec = REFRESH_TIME_VALUE / 1000000;
    t.it_value.tv_usec = REFRESH_TIME_VALUE % 1000000;
    t.it_interval.tv_sec = REFRESH_TIME_INTERVAL / 1000000;
    t.it_interval.tv_usec = REFRESH_TIME_INTERVAL % 1000000;

    setitimer(ITIMER_REAL, &t, NULL);
}

void updateTime(unsigned long long int t)
{
    float t1;
    char tmp[10];
    if (t >= 60000000) {	//m
	t1 = t / 60000000.0;
	sprintf(tmp, "%-.3f m ", t1);
    } else if (t > 1000000) {	//s
	t1 = t / 1000000.0;
	sprintf(tmp, "%-.3f s ", t1);
    } else if (t > 1000) {	//ms
	t1 = t / 1000.0;
	sprintf(tmp, "%-.3f ms", t1);
    } else if (t == 0) {
	sprintf(tmp, "...");
    } else {			//us
	sprintf(tmp, "%-lld us", t);
    }
    attron(A_BOLD);
    mvaddstr(TIME_Y, t0, "          ");
    mvaddstr(TIME_Y, t0, tmp);
    attroff(A_BOLD);

    refresh();
}

static void footerT(void)
{
    attron(A_BOLD | COLOR_PAIR(3));
    mvaddstr(COLUMN_Y2_max + 1, COLUMN_X, l0);
    mvaddstr(COLUMN_Y2_max + 2, COLUMN_X, l1);
    mvaddstr(COLUMN_Y2_max + 3, COLUMN_X, l2);
    mvaddstr(COLUMN_Y2_max + 1, COLUMN_X + strlen(l0), l3);
    mvaddstr(COLUMN_Y2_max + 2, COLUMN_X + strlen(l1), l4);
    mvaddstr(COLUMN_Y2_max + 3, COLUMN_X + strlen(l2), l5);
    mvaddstr(COLUMN_Y2_max + 1, COLUMN_X + strlen(l0) + strlen(l3), l6);
    mvaddstr(COLUMN_Y2_max + 2, COLUMN_X + strlen(l0) + strlen(l3), l7);
    mvaddstr(COLUMN_Y2_max + 3, 2 + COLUMN_X + strlen(l0) + strlen(l3),
	     l8);
    attroff(COLOR_PAIR(3));
    attron(COLOR_PAIR(4));
    mvaddch(COLUMN_Y2_max + 3, COLUMN_X + strlen(l0) + strlen(l3),
	    ACS_CKBOARD);
    attroff(A_BOLD | COLOR_PAIR(4));
    refresh();
}

static void headerT(unsigned long int tollerance)
{
    //title
    attron(A_BOLD | COLOR_PAIR(1));
    mvaddstr(TITLE_Y, 0, title);
    refresh();
    attroff(A_BOLD | COLOR_PAIR(1));

    t0 = strlen(time_title) + 1;
    mvaddstr(TIME_Y, x0, time_title);
    char toller[15];
    sprintf(toller, "%ld us", tollerance);
    mvaddstr(TOLL_Y, x0, tollerance_title);
    attron(A_BOLD);
    mvaddstr(TOLL_Y, strlen(tollerance_title) + 1, toller);
    attroff(A_BOLD);
    updateTime(0);

    //tab header
    xn = COLUMN_X;
    x0 = xn + strlen(hN) + 1;
    x1 = x0 + strlen(h0) + 1;
    x2 = x1 + strlen(h1) + 1;
    x3 = x2 + strlen(h2) + 1;
    x4 = x3 + strlen(h3) + 1;
    x7 = x4 + strlen(h4) + 1;
    x8 = x7 + strlen(h7) + 1;
    x9 = x8 + strlen(h8) + 1;
    xw = x9 + strlen(h9) + 1;
    xa = xw + strlen(hw) + 1;

    attron(A_BOLD | COLOR_PAIR(2));
    mvaddstr(COLUMN_Y1, xa, ha);
    mvaddstr(COLUMN_Y2, xa, ha0);
    mvaddstr(COLUMN_Y1, xw, hw);
    mvaddstr(COLUMN_Y2, xw, hw0);
    mvaddstr(COLUMN_Y1, xn, hN);
    mvaddstr(COLUMN_Y2, xn, hNN);
    mvaddstr(COLUMN_Y1, x0, h0);
    mvaddstr(COLUMN_Y2, x0, h00);
    mvaddstr(COLUMN_Y1, x1, h1);
    mvaddstr(COLUMN_Y2, x1, h11);
    mvaddstr(COLUMN_Y1, x2, h2);
    mvaddstr(COLUMN_Y2, x2, h22);
    mvaddstr(COLUMN_Y1, x3, h3);
    mvaddstr(COLUMN_Y2, x3, h33);
    mvaddstr(COLUMN_Y1, x4, h4);
    mvaddstr(COLUMN_Y2, x4, h44);
    mvaddstr(COLUMN_Y1, x7, h7);
    mvaddstr(COLUMN_Y2, x7, h77);
    mvaddstr(COLUMN_Y1, x8, h8);
    mvaddstr(COLUMN_Y2, x8, h88);
    mvaddstr(COLUMN_Y1, x9, h9);
    mvaddstr(COLUMN_Y2, x9, h99);
    attroff(A_BOLD | COLOR_PAIR(2));

    refresh();
}

void initializeT(unsigned long int tollerance)
{
    int x;
    WINDOW *win;
    win = initscr();
    timeout(0);
    cbreak();
    noecho();
    start_color();
    assume_default_colors(COLOR_WHITE, COLOR_BLACK);
    clear();
    refresh();

    setAlarm();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_WHITE, COLOR_BLUE);

    getmaxyx(win, COLUMN_Y2_max, x);
    COLUMN_Y2_max -= 4;
    COLUMN_Y2_min = 0;

    headerT(tollerance);
    footerT();
}

static int find(int id)
{
    int i, found = -1;
    for (i = 0; i < ipid && found == -1; i++) {
	if (rows[i].pid == id) {
	    found = i;
	}
    }
    return found;
}

void updateCPUsUtil(int pid, float t, float w, float a)
{
    rows[find(pid)].cpu_util = t;
    rows[find(pid)].cpu_wutil = w;
    rows[find(pid)].cpu_autil = a;
}

void updatePDFRT(int pid, long long int t, int periodic)
{
    rows[find(pid)].pdf_rt = t;
    rows[find(pid)].p_r = periodic;
}

void updateRT(int pid, long long int t)
{
    int i = find(pid);

    if (i < 0) {
      createPidRow(pid, 0);		//FIXME: CPU
      i = find(pid);
    }
    rows[i].rt = t;
}

void updatePDFET(int pid, long long int t, int periodic)
{
    rows[find(pid)].pdf_et = t;
    rows[find(pid)].p_e = periodic;
}

void updateET(int pid, long long int t)
{
    int i = find(pid);

    if (i < 0) {
      createPidRow(pid, 0);		//FIXME: CPU
      i = find(pid);
    }
    rows[i].et = t;
}

void updateI(int pid, long long int t)
{
    int i = find(pid);

    if (i < 0) {
      createPidRow(pid, 0);		//FIXME: CPU
      i = find(pid);
    }
    rows[i].i = t;
}

void updatePDFI(int pid, long long int t, int periodic)
{
    rows[find(pid)].pdf_i = t;
    rows[find(pid)].p_u = periodic;
}

void createPidRow(int id, int cpu)
{
    int i;
    const char *name = name_get(id, cpu);

    rows = (struct row *) realloc(rows, ++ipid * sizeof(struct row));
    rows[ipid - 1].pid = id;
    rows[ipid - 1].i = -1;
    rows[ipid - 1].pdf_i = -1;
    rows[ipid - 1].rt = -1;
    rows[ipid - 1].pdf_rt = -1;
    rows[ipid - 1].ei = -1;
    rows[ipid - 1].pdf_ei = -1;
    rows[ipid - 1].et = -1;
    rows[ipid - 1].pdf_et = -1;
    rows[ipid - 1].cpu_util = 0;
    rows[ipid - 1].p_r = 0;
    rows[ipid - 1].p_e = 0;
    rows[ipid - 1].p_u = 0;

    for (i = 0; name[i] != '\0'; i++) {
	(rows[ipid - 1].name)[i] = name[i];
    }
    (rows[ipid - 1].name)[i] = '\0';

    if (COLUMN_Y2_min + ipid + COLUMN_Y2 < COLUMN_Y2_max) {
	showPidRow(ipid - 1, ipid + COLUMN_Y2);
    }
}

static void upRows(void)
{
    int i, j;

    if (COLUMN_Y2_min > 0) {
	COLUMN_Y2_min--;
	for (j = 1, i = COLUMN_Y2_min;
	     i < COLUMN_Y2_min + COLUMN_Y2_max - COLUMN_Y2; i++, j++) {
	    showPidRow(i, j + COLUMN_Y2);
	}
    }
}

static void downRows(void)
{
    int i, j, last;

    last = 1 + COLUMN_Y2_min + COLUMN_Y2_max - COLUMN_Y2;
    if (last < ipid) {
	COLUMN_Y2_min++;
	for (j = 1, i = COLUMN_Y2_min; i < last; i++, j++) {
	    showPidRow(i, j + COLUMN_Y2);
	}
    }
}

char suEzo(void)
{
    char d = getch();
    if (d == 'd') {
	downRows();
    } else if (d == 'u') {
	upRows();
    }
    return d;
}

void exitT(void)
{
    while (suEzo() != 'q') sleep(1);
    free(rows);
    rows = NULL;
    endwin();
}
