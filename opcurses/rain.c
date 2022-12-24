/*
 *	No, this isn't the standard curses "rain" ...
 *
 *	Col. G. L. Sicherman.  1994
 *
 * modified for Apple IIgs by Sheldon Simms, 1996.
 * Source ANSIfied, function usleep() added, usage of signal() removed
 * curses nodelay() call used to allow trapping of Cntl-C with getch()
 * rather than with SIGINT.
 */

#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "curses.h"

int count, rows, cols, dryup = 0;
char **vec;

void die( void )
{
  erase();
  refresh();
  endwin();
  exit(0);
}

void inter( void )
{
  if (dryup++) die();
}

void coalesce( void )
{
  int y, x;

  for (y=rows-1; y>=0; y--) for (x=0; x<cols; x++) {
    count++;
    switch(vec[y][x]) {
    case ' ': count--; break;
    case '|':
    case '/':
    case '\\':
      vec[y][x]=' ';
      mvaddch(y, x, ' ');
      break;
    case 'o':
    case 'O':
      if (y==rows-1) break;
      switch(vec[y+1][x]) {
      case 'o':
        vec[y+1][x] = 'O';
        mvaddch(y+1, x, 'O');
        /* fall through */
      case 'O':
        vec[y][x] = '|';
        mvaddch(y, x, '|');
      }
    }
  }
}

void fall( void )
{
  int y, x, xnew;
  char c, p;

  for (y=rows-1; y>=0; y--) for (x=0; x<cols; x++)
    switch (c=vec[y][x]) {
    case 'o':
      if (rand()%3 == 0) break;
      /* fall through */
    case 'O':
      if (rand()%5) break;
      switch(rand()%9) {
      case 0: xnew = x - 1; p = '/'; break;
      case 1: xnew = x + 1; p = '\\'; break;
      default: xnew = x; p = '|'; break;
      }
      if (y<rows-1 && xnew >= 0 && xnew < cols) {
        vec[y+1][xnew] = c;
        mvaddch(y+1, xnew, c);
      }
      vec[y][x] = p;
      mvaddch(y, x, p);
    }
}

void addone( void )
{
  int y, x;
  y = rand() % rows;
  y = y * y / rows;
  x = rand() % cols;
  switch(vec[y][x]) {
  case 'O':
    break;
  case 'o':
    vec[y][x] = 'O';
    mvaddch(y, x, 'O');
    break;
  default:
    vec[y][x] = 'o';
    mvaddch(y, x, 'o');
    break;
  }
}

void advance( void )
{
  int i;

  count = 0;
  coalesce();
  fall();
  if (!dryup) for (i=rand()%3/2; i>=0; i--) addone();
  refresh();
  /* usleep(250000); */	/* microseconds */
  if (dryup && count==0) die();
}

int main( void )
{
  int x, y;
  chtype c;

  initscr();
  noecho();
  nodelay( stdscr, TRUE );
  cols = COLS;
  rows = LINES-1;
  vec = (char **)malloc(rows * sizeof(char *));
  for (y=0; y<rows; y++) vec[y] = (char *)malloc(cols);
  for (y=0; y<rows; y++) for (x=0; x<cols; x++)
    vec[y][x] = ' ';
  clear();
  refresh();
  srand((int)time(0));
  for (;;)
    {
      c = getch();
      /*
      if ( c == 3 )
        inter();
      */
      if (c == 3 )
        die();
      advance();
    }
}
