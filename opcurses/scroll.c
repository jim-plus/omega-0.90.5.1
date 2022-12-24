
#include <ctype.h>
#include <unistd.h>
#include "curses.h"

void split_scroll (WINDOW * w1, WINDOW * w2, int nlines, unsigned long utime)
{
  wscrl(w1, nlines);
  wscrl(w2, -nlines);

  wrefresh(w1);
  wrefresh(w2);

  usleep(utime);
}

void split_demo (WINDOW * w)
{
  int ch;
  int wcols;
  int wlines;

  WINDOW * w1;
  WINDOW * w2;

  getmaxyx(w, wlines, wcols);

  w1 = derwin(w, wlines, wcols / 2, 0, 0);
  w2 = derwin(w, wlines, wcols / 2, 0, wcols / 2);

  getmaxyx(w1, wlines, wcols);
  for (--wlines; wlines >= 0; --wlines)
    {
      wmove(w1, wlines, 0);
      for (ch = 0; ch < wcols; ++ch)
        waddch(w1, '1');
    }

  getmaxyx(w2, wlines, wcols);
  for (--wlines; wlines >= 0; --wlines)
    {
      wmove(w2, wlines, 0);
      for (ch = 0; ch < wcols; ++ch)
        waddch(w2, '2');
    }

  nodelay(w, TRUE);
  scrollok(w1, TRUE);
  scrollok(w2, TRUE);

  while (1)
    {
      ch = wgetch(w);
      if (ERR != ch) break;

      split_scroll(w1, w2, 1, 100000);
      split_scroll(w1, w2, 1, 100000);
      split_scroll(w1, w2, 1, 100000);
      split_scroll(w1, w2, 1, 100000);

      split_scroll(w1, w2, -1, 100000);
      split_scroll(w1, w2, -1, 100000);
      split_scroll(w1, w2, -1, 100000);
      split_scroll(w1, w2, -1, 100000);
    }

  delwin(w2);
  delwin(w1);

  werase(w);
  wrefresh(w);
}

void add_demo (WINDOW * w)
{
  int ch;
  int out = 0;

  nodelay(w, TRUE);
  scrollok(w, TRUE);

  while (1)
    {
      ch = wgetch(w);
      if (ERR != ch) break;

      while (!isprint(out++))
        {
          if (out > 127) out = 0;
        }

      waddch(w, out);
      wrefresh(w);
    }

  werase(w);
  wrefresh(w);
}

int main (void)
{
  WINDOW * w;

  initscr();

  bkgdset('+');
  w = newwin(LINES - 8, COLS - 16, 4, 8);

  add_demo(stdscr);
  add_demo(w);

  split_demo(stdscr);
  split_demo(w);

  endwin();
  return 0;
}
