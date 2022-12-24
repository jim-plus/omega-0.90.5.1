
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include "curses.h"

static char * message[4] =
{
  "My favorite film is \"From Russia With Love\"\n",
  "I've never seen \"In Her Majesty's Secret Service\"\n",
  "\"You Only Live Twice\" is set in Japan\n",
  "Roger Moore stars in \"The Man With the Golden Gun\"\n"
};

static char * filler = "Tell me about some James Bond films";

void type_line (WINDOW * w, char * message, int maxchars, unsigned long utime)
{
  int cnt;
  char * ptr;

  cnt = 0;
  ptr = message;

  while (*ptr && cnt < maxchars)
    {
      ++cnt;
      waddch(w, *ptr++);
      wrefresh(w);
      if (utime) usleep(utime);
    }
}

void initrand (void)
{
  struct timeval tv;
  struct timezone trash;

  gettimeofday(&tv, &trash);

  srand(tv.tv_usec);
}

int random_range (int max)
{
  /* yes, it's crap, but I don't care for this application */
  return rand() % max;
}

void insert_demo (WINDOW * w)
{
  int wcols, wlines;
  int iline;
  int num;
  int ch;
  int id = 1;

  getmaxyx(w, wlines, wcols);

  for (iline = 0; iline < wlines; ++iline)
    {
      wmove(w, iline, iline);
      type_line(w, filler, wcols - iline, 0);
    }

  wrefresh(w);
  iline = wlines / 2;
  nodelay(w, TRUE);
  idlok(w, TRUE);

  while (1)
    {
      ch = wgetch(w);
      if (ERR != ch)
        {
          if (1 == id) id = -1;
          else break;
        }

      num = random_range(4);

      wmove(w, iline, 0);

      winsdelln(w, id * (num+1));
      wrefresh(w);

      while (num >= 0)
        type_line(w, message[num--], wcols, 50000);
    }

  werase(w);
  wrefresh(w);
}

int main (void)
{
  initscr();

  insert_demo(stdscr);

  endwin();
  return 0;
}
