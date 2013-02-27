/*
 * This is free software: see GPL.txt
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <curses.h>

#include "task_names.h"
#include "event_list.h"
#include "trace_evt_handle.h"
#include "period_detect.h"

static int analysis_period = 700000;

static void help(const char *name)
{
  fprintf(stdout, "Usage:\n");
  fprintf(stdout, "%s [options] <input file>\n\n", name);

  fprintf(stdout, "-h t\tHelp\n");
  exit(-1);
}

static unsigned int opts_parse(int argc, char *argv[])
{
  int c;

  while ((c = getopt(argc, argv, "P:h")) != -1)
    switch (c) {
      case 'P':
	analysis_period = atoi(optarg);
	break;
      case 'h':
	help(argv[0]);
	break;
      default:
        fprintf(stderr, "Unknown option %c\n", c);
        exit(-1);
    }

  return optind;
}

static void visual_cleanup(int status, void *win)
{
  delwin(win);
  endwin();
  refresh();
}

static void visual_init(void)
{
  WINDOW *win;

  win = initscr();
  cbreak();
  noecho();
//  start_color();
//  assume_default_colors(COLOR_WHITE, COLOR_BLACK);
  clear();
  refresh();
  on_exit(visual_cleanup, win);
}

static const char *task_name(unsigned int p)
{
  unsigned int cpu;
  
  for (cpu = 0; cpu < 16; cpu++) {
    const char *name;

    name = name_get(p, cpu);

    if (name) return name;
  }

  return "";
}

static void visual_update(void)
{
  int p, todo;
  unsigned int j = 0, line = 0;

  todo = 1;
  while(todo >= 0) {
    todo = pid_get(j++);
    if (todo > 0) {
      p = pdetect_period(todo);
      if (p) {
        char string[128];

        sprintf(string, "[%d] %s\t%d", todo, task_name(todo), p);
        mvaddstr(line++ + 4, 4, string);
      }
    }
  }
  refresh();
}

int main(int argc, char *argv[])
{
  FILE *f;
  int res, done, told;
  char *fname;
  int first_parameter;

  first_parameter = opts_parse(argc, argv);
  if (first_parameter < argc) {
    fname = argv[first_parameter];
    f = fopen(fname, "r");
    if (f == NULL) {
      perror(fname);

      return -1;
    }
  } else {
    help(argv[0]);
  }

  visual_init();

  done = 0; told = 0;
  while (!done) {
    struct event *e;

    res = trace_read_event(f, 0, 0);
    while ((e = evt_get())) {
      int t;

      t = pdetect_event_handle(e);
      if (t - told > analysis_period) {
        told = t;
        visual_update();
        pdetect_reset();
      }
      free(e);
    }
    done = feof(f) || (res < 0);
  }
  mvaddstr(20, 10, "Press 'q' to exit");
  while (getch() != 'q');

  return 0;
}
