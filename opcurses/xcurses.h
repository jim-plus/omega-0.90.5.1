/*
  xcurses.h : op-curses
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

#ifndef INCLUDED_XCURSES_H
#define INCLUDED_XCURSES_H

#include "curses.h"

#define LF 0x0a
#define CR 0x0d
#define INT 0x03

#define M_CBREAK 1  /* In this implementation CBREAK == RAW */
#define M_RAW    2
#define M_DELAY  4

#define W_WIN    1
#define W_SUBWIN 2
#define W_PAD    3

#define CURSOR_CHAR '_'

#define BEST_CASE_COLOR_PAIRS 32767

extern int pCOLS;
extern int pLINES;

extern bool has_local_idc;
extern bool has_local_idl;
extern bool has_local_scroll;
extern bool local_ticks_per_second;

/* system-dependent prototypes */
void local_beep (void);
void local_clear (void);
void local_end_curses (void);
void local_putchar (chtype c, int x, int y);
void local_update (int line, int beg, int end);
void local_start_color (bool *, bool *, int *, int *);
void local_pair_content (short pair, short * fore, short * back);
void local_init_pair (short pair, short fore, short back);
void local_init_color (short color, short sr, short sg, short sb);
void local_color_content (short color, short * sr, short * sg, short * sb);
void local_set_tiles (char *);

int local_init_curses (void);
int local_scroll (int top, int left, int height, int width, int nlines);
int local_idc (int w_left, int w_right, int idcol, int id);
int local_idl (int w_top, int w_left, int w_height, int w_width, int idline, int nlines);
int local_getchar (void);

unsigned long local_timer (void);

#endif /* INCLUDED_XCURSES_H */
