/*
  curses.h - op-curses
  Copyright 1996,2001 W. Sheldon Simms III

  This file is part of op-curses, a portable curses implementation.

  op-curses is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  op-curses is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 */

#ifndef INCLUDED_CURSES_H
#define INCLUDED_CURSES_H

#include <stdio.h>
#include <stdarg.h>

#ifdef FALSE
#undef FALSE
#endif
#define FALSE 0

#ifdef TRUE
#undef TRUE
#endif
#define TRUE 1

#define ERR -1
#define OK 0

#define A_NORMAL      0x0000
#define A_UNDERLINE   0x0100
#define A_REVERSE     0x0200
#define A_BLINK       0x0400
#define A_DIM         0x0800
#define A_BOLD        0x1000
#define A_PROTECT     0x2000
#define A_INVIS       0x4000
#define A_ALTCHARSET  0x8000
#define A_CHARTEXT    0x00FF
#define A_HORIZONTAL  0x0000
#define A_LEFT        0x0000
#define A_LOW         0x0000
#define A_RIGHT       0x0000
#define A_TOP         0x0000
#define A_VERTICAL    0x0000
#define A_STANDOUT    A_BOLD

#define WA_NORMAL     0x0000
#define WA_UNDERLINE  0x0100
#define WA_REVERSE    0x0200
#define WA_BLINK      0x0400
#define WA_DIM        0x0800
#define WA_BOLD       0x1000
#define WA_ALTCHARSET 0x8000
#define WA_HORIZONTAL 0x0000
#define WA_LEFT       0x0000
#define WA_LOW        0x0000
#define WA_RIGHT      0x0000
#define WA_TOP        0x0000
#define WA_VERTICAL   0x0000
#define WA_STANDOUT   WA_BOLD

#define COLOR_PAIR(n) ((n) << 16)

#define COLOR_BLACK   0
#define COLOR_RED     1
#define COLOR_GREEN   2
#define COLOR_YELLOW  3
#define COLOR_BLUE    4
#define COLOR_MAGENTA 5
#define COLOR_CYAN    6
#define COLOR_WHITE   7

#define ACS_ULCORNER  0x80
#define ACS_LLCORNER  0x81
#define ACS_URCORNER  0x82
#define ACS_LRCORNER  0x83
#define ACS_RTEE      0x84
#define ACS_LTEE      0x85
#define ACS_BTEE      0x86
#define ACS_TTEE      0x87
#define ACS_HLINE     0x88
#define ACS_VLINE     0x89
#define ACS_PLUS      0x8a
#define ACS_S1        0x8b
#define ACS_S2        0x8c
#define ACS_S9        0x8c
#define ACS_DIAMOND   0x8d
#define ACS_CKBOARD   0x8e
#define ACS_DEGREE    0x8f
#define ACS_PLMINUS   0x90
#define ACS_BULLET    0x91
#define ACS_LARROW    0x92
#define ACS_RARROW    0x93
#define ACS_DARROW    0x94
#define ACS_UARROW    0x95
#define ACS_BOARD     0x96
#define ACS_LANTERN   0x97
#define ACS_BLOCK     0x98

/* this stuff is system-dependent, needs to be changed */
#define KEY_BREAK     0x03
#define KEY_DOWN      0x1f
#define KEY_UP        0x1e
#define KEY_LEFT      0x1c
#define KEY_RIGHT     0x1d
#define KEY_ESCAPE    0x1b
#define KEY_DELETE    0x7f

typedef char bool;
typedef unsigned long chtype;
typedef chtype attr_t;

typedef struct WINDOW
{
  short type;
  short curY, curX;
  short cols, lines;
  short scr_top, scr_bottom;
  short top, left, right, bottom;

  chtype bkgd;
  attr_t attrword;
  
  int f_idc:1;
  int f_idl:1;
  int f_clear:1;
  int f_leave:1;
  int f_scroll:1;
  int f_nodelay:1;
  int f_graphics:1;
  chtype ** winBuf;
  short * begLine;
  short * endLine;
  struct WINDOW * orig;
  struct WINDOW * next;
}
WINDOW;

extern int COLS;
extern int LINES;
extern int COLORS;
extern int COLOR_PAIRS;

extern bool f_nl;
extern bool f_echo;

extern WINDOW * stdscr;
extern WINDOW * curscr;

void set_tiles (char *);
int initscr (void);
int endwin (void);

WINDOW * newwin (int nlines, int ncols, int uly, int ulx);
WINDOW * derwin (WINDOW * orig, int lines, int cols, int uly, int ulx);
WINDOW * subwin (WINDOW * orig, int lines, int cols, int uly, int ulx);

int delwin (WINDOW * win);
int mvwin (WINDOW *win, int y, int x);
int mvderwin (WINDOW * win, int par_y, int par_x);

int wtouchln (WINDOW * win, int start, int count, int changed);
bool is_linetouched (WINDOW * win, int line);
bool is_wintouched (WINDOW * win);

int wclrtoeol (WINDOW * win);
int wclrtobot (WINDOW * win);

int wscrl (WINDOW * win, int nlines);
int winsdelln (WINDOW * win, int nlines);
int winsch (WINDOW * win, chtype ch);
int wdelch (WINDOW * win);

int printw (char * fstr, ...);
int wprintw (WINDOW * win, char * fstr, ...);
int mvprintw (int y, int x, char * fstr, ...);
int mvwprintw (WINDOW * win, int y, int x, char * fstr, ...);

int vwprintw (WINDOW * win, char * fstr, va_list arg);
int vw_printw (WINDOW * win, char * fstr, va_list arg);

int copywin( WINDOW *src, WINDOW *dst, int sminrow, int smincol, int dminrow,
				 int dmincol, int dmaxrow, int dmaxcol, int overlay );

int wborder (WINDOW * win,
             chtype ls, chtype rs, chtype ts, chtype bs,
             chtype tl, chtype tr, chtype bl, chtype br);

int idlok (WINDOW *win, bool flag);

int wgetch (WINDOW * win);
int waddch (WINDOW * win, chtype ch);
int wechochar (WINDOW * win, chtype ch);

int waddstr (WINDOW * win, char * s);
int wgetstr (WINDOW * win, char * s);
int wmove (WINDOW * win, int y, int x);

int cbreak (void);
int nocbreak (void);
int raw (void);
int noraw (void);

int beep (void);
int flash (void);

int baudrate (void);
char killchar (void);
char erasechar (void);
char * longname (void);
char * termname (void);
int keypad (WINDOW * win, bool flag);

int wbkgd (WINDOW * win, const chtype ch);
void wbkgdset (WINDOW * win, const chtype ch);
chtype getbkgd (WINDOW * win);

int wattr_set (WINDOW * win, attr_t attrs, void * opts);
int wattrset (WINDOW * win, int attrs);

bool has_colors (void);
bool can_change_color (void);
int init_pair (short pair, short fore, short back);
int init_color (short color, short sr, short sg, short sb);
int pair_content (short pair, short * fore, short * back);
int color_content (short color, short * sr, short * sg, short * sb);
int start_color (void);

int wnoutrefresh (WINDOW * win);
int doupdate (void);
int wrefresh (WINDOW * win);

#include "cmacros.h"

#endif /* INCLUDED_CURSES_H */
