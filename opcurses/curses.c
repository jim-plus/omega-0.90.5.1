/*
  curses.c : op-curses
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

#include <assert.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "xcurses.h"

/* Global variables start here */

WINDOW * stdscr;
WINDOW * curscr;

int pCOLS = 80;
int pLINES = 24;

int COLS;
int LINES;
int COLORS = 0;
int COLOR_PAIRS = 0;

bool f_nl = TRUE;
bool f_echo = TRUE;
bool f_has_color = FALSE;
bool f_change_color = FALSE;

static bool * color_pair_init;

static int inputMode = M_CBREAK;

static WINDOW * windowHead;

static char strBuf[4096];
/* A buffer by printw. The length is somewhat arbitrary (this is dangerous) */

static char * terminalName = "OPCurses. Copyright 1996,2001 W. Sheldon Simms III";
/* returned by longname() call */

/* */

int waddch (WINDOW * win, chtype ch)
{
  chtype text;

  /* don't print control characters */
  /* if (isprint(blah)) would be wrong -
     we expect to use a custom character set */

  text = ch & A_CHARTEXT;

  if (text > 0x1f)
    {
      if (' ' == text)
        ch = win->bkgd | win->attrword;
      else
        ch = ch | (win->bkgd & ~A_CHARTEXT) | win->attrword;

      win->winBuf[win->curY][win->curX] = ch;

      /* adjust update markers */
      if (win->curX < win->begLine[win->curY]) win->begLine[win->curY] = win->curX;
      if (win->curX >= win->endLine[win->curY]) win->endLine[win->curY] = win->curX + 1;

      win->curX = win->curX + 1;

      if (win->curX == win->cols)
        {
          win->curX = 0;
          win->curY = win->curY + 1;

          if (win->curY == win->lines)
            {
              win->curY = win->curY - 1;
              wscrl(win, 1);
            }
        }
    }
  else if (LF == text)
    {
      win->curX = 0;
      win->curY = win->curY + 1;

      if (win->curY == win->lines)
        {
          win->curY = win->curY - 1;
          wscrl(win, 1);
        }
    }
  else if (CR == text)
    {
      win->curX = 0;
    }

  return OK;
}

/* */

int wechochar (WINDOW * win, chtype ch)
{
  waddch(win, ch);
  wrefresh(win);
  return OK;
}

/* */

int wgetch (WINDOW * win)
{
  int toggle;
  int toggle_changed;
  /* used for cursor blinking */

  int wcx;
  int wcy;
  int gl_wcx;
  int gl_wcy;

  int result;

  chtype screen_char;

  unsigned long oldtime;
  unsigned long newtime;

  gl_wcy = win->top + win->curY;
  gl_wcx = win->left + win->curX;

  /* save character from screen so that cursor can be blinked */
  screen_char = curscr->winBuf[gl_wcy][gl_wcx];

  toggle = FALSE;
  toggle_changed = TRUE;

  oldtime = local_timer();

  result = ERR;
  while (ERR == result)
    {
      /* if nodelay mode, there don't make a cursor */
      if (!win->f_leave && !win->f_nodelay)
        {
          if (toggle_changed)
            {
              toggle_changed = FALSE;

              /* blink the cursor */
              if (!toggle)
                local_putchar(CURSOR_CHAR, gl_wcx, gl_wcy);
              else
                local_putchar(screen_char, gl_wcx, gl_wcy);

              local_update(gl_wcy, gl_wcx, gl_wcx);
            }
        }

      result = local_getchar();

      /* if nodelay always return */
      if (win->f_nodelay) break;

      /* see if it's time to blink yet */
      newtime = local_timer();
      if ((local_ticks_per_second / 2) <= (newtime - oldtime))
        {
          toggle = toggle ? FALSE : TRUE;
          toggle_changed = TRUE;
          oldtime = newtime;
        }
    }

  /* can only happen in nodelay mode */
  if (ERR == result) return result;

  if (INT == result) raise(SIGINT);
        
  /* map CR->LF if in nl mode */
  if (CR == result && f_nl) result = LF;

  if (!f_echo || result < 0x20)
    {
      /* if not echoing characters, or the user typed a non-printing
         character, then have to put back the old screen character */

      local_putchar(screen_char, gl_wcx, gl_wcy);
      local_update(gl_wcy, gl_wcx, gl_wcx);
    }
  else if (result > 0x1f)
    {
      /* echo typed character */
      wcx = win->curX;
      wcy = win->curY;
        
      local_putchar(win->attrword | result, gl_wcx, gl_wcy);
      local_update(gl_wcy, gl_wcx, gl_wcx);

      win->winBuf[wcy][wcx] = curscr->winBuf[wcy][wcx] = win->attrword | (chtype)result;

      if (win->curX < (win->cols - 1)) win->curX++;
    }
  else if (CR == result)
    {
      win->curX = 0;
    }
  else if (LF == result)
    {
      if (win->curY < (win->lines - 1)) win->curY++;
    }

  return result;
}

/* */

static void init_window (WINDOW * win, int y, int x, int lines, int cols)
{
  win->curX = 0;
  win->curY = 0;
  win->lines = lines;
  win->cols = cols;
  win->left = x;
  win->top = y;
  win->right = x + cols;
  win->bottom = y + lines;

  win->bkgd = ' ';
  win->attrword = 0;

  win->f_idc = FALSE;
  win->f_idl = FALSE;
  win->f_clear = FALSE;
  win->f_leave = FALSE;
  win->f_scroll = FALSE;
  win->f_nodelay = FALSE;
  win->f_graphics = FALSE;

  win->next = windowHead;
  windowHead = win;
}

/* */

static WINDOW * alloc_window (int y, int x, int lines, int cols)
{
  int idx;
  size_t total;
  WINDOW * newWin;

  /* how much is needed for the WINDOW struct */
  total = sizeof(WINDOW);

  /* how much is needed for the begLine and endLine arrays */
  total += (2 * lines * sizeof(short));

  /* how much is needed for the pointers to each line */
  total += (lines * sizeof(chtype *));

  /* how much is needed for the character buffer */
  total += (lines * cols * sizeof(chtype));

  /* grab it all at once */
  newWin = malloc(total);
  if (!newWin) return 0;

  /* skip past the WINDOW struct and make that address begLine */
  newWin->begLine = (short *)(newWin + 1);

  /* skip the begLine array and make that address endLine */
  newWin->endLine = newWin->begLine + lines;

  /* skip the endLine array and make that address winBuf (the line pointer array) */
  newWin->winBuf = (chtype **)(newWin->endLine + lines);

  /* skip the winBuf array and make that address the character buffer */
  newWin->winBuf[0] = (chtype *)(newWin->winBuf + lines);

  /* set up the line pointers and initialize the dirty line arrays */
  for (idx = 0; idx < lines; ++idx)
    {
      newWin->winBuf[idx] = newWin->winBuf[0] + (cols * idx);
      newWin->begLine[idx] = cols;
      newWin->endLine[idx] = 0;
    }

  /* initialize */
  init_window(newWin, y, x, lines, cols);
  newWin->type = W_WIN;
  newWin->orig = 0;

  return newWin;
}

/* */

WINDOW * newwin (int nlines, int ncols, int uly, int ulx)
{
  WINDOW * nwin;

  if (ulx < 0 || ulx >= COLS) return 0;
  if (uly < 0 || uly >= LINES) return 0;

  if ((ulx + ncols) > COLS) return 0;
  if ((uly + nlines) > LINES) return 0;

  if (0 == ncols) ncols = COLS - ulx;
  if (0 == nlines) nlines = LINES - uly;

  nwin = alloc_window(uly, ulx, nlines, ncols);

  if (nwin) werase(nwin);

  return nwin;
}

/* */

WINDOW * derwin (WINDOW * orig, int lines, int cols, int uly, int ulx)
{
  int idx;
  size_t total;
  WINDOW * win;

  /* bounds checking */
  if (ulx < 0 || ulx >= orig->cols) return 0;
  if (uly < 0 || uly >= orig->lines) return 0;

  if ((ulx + cols) > orig->cols) return 0;
  if ((uly + lines) > orig->lines) return 0;

  /* mirrors newwin() functionality - but ncurses man page doesn't specify this */
  if (0 == cols) cols = orig->cols - ulx;
  if (0 == lines) lines = orig->lines - uly;

  /* how much is needed for the WINDOW struct */
  total = sizeof(WINDOW);

  /* how much is needed for the begLine and endLine arrays */
  total += (2 * lines * sizeof(short));

  /* how much is needed for the pointers to each line */
  total += (lines * sizeof(chtype *));

  /* grab it all at once */
  win = malloc(total);
  if (!win) return 0;

  /* skip past the WINDOW struct and make that address begLine */
  win->begLine = (short *)(win + 1);

  /* skip the begLine array and make that address endLine */
  win->endLine = win->begLine + lines;

  /* skip the endLine array and make that address winBuf (the line pointer array) */
  win->winBuf = (chtype **)(win->endLine + lines);

  /* use the parent window's character buffer as our character buffer */
  win->winBuf[0] = orig->winBuf[uly] + ulx;

  /* set up the line pointers and initialize the dirty line arrays */
  for (idx = 0; idx < lines; ++idx)
    {
      win->winBuf[idx] = win->winBuf[0] + (idx * orig->cols);
      win->begLine[idx] = cols;
      win->endLine[idx] = 0;
    }

  /* initialize */
  init_window(win, uly + orig->top, ulx + orig->left, lines, cols);
  win->type = W_SUBWIN;
  win->orig = orig;

  return win;
}

/* */

WINDOW * subwin (WINDOW * orig, int lines, int cols, int uly, int ulx)
{
  /* Change the given coordinates from global to parent-relative */
  return derwin(orig, lines, cols, uly - orig->top, ulx - orig->left);
}

/* */

int delwin (WINDOW * win)
{
  /* all memory associated with a window is allocated in one chunk */
  if (win) free(win);
  return OK;
}

/* */

int mvwin (WINDOW * win, int y, int x)
{
  /* bounds checking */
  if (x < 0 || x >= COLS) return ERR;
  if (y < 0 || y >= LINES) return ERR;

  if ((x + win->cols) > COLS) return ERR;
  if ((y + win->lines) > LINES) return ERR;

  /* move it */
  win->top = y;
  win->left = x;
  win->right = x + win->cols;
  win->bottom = y + win->lines;

  return OK;
}

/* */

int mvderwin (WINDOW * win, int par_y, int par_x)
{
  int idx;

  /* bounds checking */
  if (par_x < 0 || par_x >= win->orig->cols) return ERR;
  if (par_y < 0 || par_y >= win->orig->lines) return ERR;

  if ((par_x + win->cols) > win->orig->cols) return ERR;
  if ((par_y + win->lines) > win->orig->lines) return ERR;

  /* use the parent window's character buffer as our character buffer */
  win->winBuf[0] = win->orig->winBuf[par_y] + par_x;

  /* set up the line pointers and initialize the dirty line arrays */
  for (idx = 0; idx < win->lines; ++idx)
    {
      win->winBuf[idx] = win->winBuf[0] + (idx * win->orig->cols);
      win->begLine[idx] = win->cols;
      win->endLine[idx] = 0;
    }

  return OK;
}

/* */

void set_tiles (char * path)
{
  static char * local_path = 0;

  if (local_path) free(local_path);

  local_path = malloc(strlen(path) + 1);
  strcpy(local_path, path);

  local_set_tiles(local_path);
}

/* */

int initscr (void)
{
    int result;
    
    result = local_init_curses();
    if (result < 0) return ERR;
        
    LINES = pLINES;
    COLS = pCOLS;
        
    curscr = newwin(0, 0, 0, 0);
    stdscr = newwin(0, 0, 0, 0);

    if (!curscr || !stdscr)
    {
      if (curscr) delwin(curscr);
      if (stdscr) delwin(stdscr);
      return ERR;
    }

    stdscr->f_clear = TRUE;
    return OK;
}

/* */

int endwin (void)
{
  local_end_curses();
  return OK;
}

/* */

int wtouchln (WINDOW * win, int start, int count, int changed)
{
  int end_line;
  short beg;
  short end;

  if (changed)
    {
      beg = 0;
      end = win->cols;
    }
  else
    {
      beg = win->cols;
      end = 0;
    }

  end_line = start + count;

  while (start < end_line)
    {
      win->begLine[start] = beg;
      win->endLine[start] = end;

      ++start;
    }

  return OK;
}

/* */

bool is_linetouched (WINDOW * win, int line)
{
  if (0 != win->endLine[line]) return TRUE;
  if (win->cols != win->begLine[line]) return TRUE;

  return FALSE;
}

/* */

bool is_wintouched (WINDOW * win)
{
  int idx;

  for (idx = 0; idx < win->lines; ++idx)
    if (is_linetouched(win, idx)) return TRUE;

  return FALSE;
}

/* */

int copywin (WINDOW * src, WINDOW * dst, int sminrow, int smincol,
             int dminrow, int dmincol, int dmaxrow, int dmaxcol, int overlay)
{
  chtype ch;

  while (sminrow < src->lines && dminrow < dst->lines && dminrow < dmaxrow)
    {
      while (smincol < src->cols && dmincol < dst->cols && dmincol < dmaxcol)
        {
          ch = src->winBuf[sminrow][smincol];

          if (!overlay || (' ' != (ch & A_CHARTEXT)))
            dst->winBuf[dminrow][dmincol] = ch;

          ++smincol;
          ++dmincol;
        }

      ++sminrow;
      ++dminrow;
    }

  return OK;
}

/* */

int wborder (WINDOW * win,
             chtype ls, chtype rs, chtype ts, chtype bs,
             chtype tl, chtype tr, chtype bl, chtype br)
{
  int idx;

  if (0 == ls) ls = ACS_VLINE;
  if (0 == rs) rs = ACS_VLINE;
  if (0 == ts) ts = ACS_HLINE;
  if (0 == bs) bs = ACS_HLINE;

  if (0 == tl) tl = ACS_ULCORNER;
  if (0 == tr) tl = ACS_URCORNER;
  if (0 == bl) tl = ACS_LLCORNER;
  if (0 == br) tl = ACS_URCORNER;

  win->winBuf[0][0] = tl;
  win->winBuf[win->lines - 1][0] = bl;

  win->winBuf[0][win->cols - 1] = tr;
  win->winBuf[win->lines - 1][win->cols - 1] = br;

  for (idx = 1; idx < (win->cols - 1); ++idx)
    {
      win->winBuf[0][idx] = ts;
      win->winBuf[win->lines - 1][idx] = bs;
    }

  win->begLine[0] = 0;
  win->endLine[0] = win->cols;

  win->begLine[win->lines - 1] = 0;
  win->endLine[win->lines - 1] = win->cols;

  for (idx = 1; idx < (win->lines - 1); ++idx)
    {
      win->begLine[idx] = 0;
      win->endLine[idx] = win->cols;

      win->winBuf[idx][0] = ls;
      win->winBuf[idx][win->cols - 1] = rs;
    }

  return OK;
}

/* */

int wsetscrreg (WINDOW * win, int top, int bottom)
{
  win->scr_top = top;
  win->scr_bottom = bottom;
  return OK;
}

/* */

int idlok (WINDOW * win, bool flag)
{
  win->f_idl = flag;
  return OK;
}

/* */

int cbreak (void)
{
  inputMode |= M_CBREAK;
  return OK;
}

/* */

int nocbreak (void)
{
  inputMode &= (~M_CBREAK);
  return OK;
}

/* */

int raw (void)
{
  inputMode |= M_RAW;
  return OK;
}

/* */

int noraw (void)
{
  inputMode &= (~M_RAW);
  return OK;
}

/* */

int baudrate (void)
{
  return 0;
}

/* */

char erasechar (void)
{
  return 0;
}

/* */

char killchar (void)
{
  return 0;
}

/* */

char * longname (void)
{
  return terminalName;
}

/* */

char * termname (void)
{
  return terminalName;
}

/* */

int beep (void)
{
  local_beep();
  return OK;
}

/* */

int flash (void)
{
  local_beep();
  return OK;
}

/* */

int keypad (WINDOW * win, bool flag)
{
  return OK;
}

/* */

void wbkgdset (WINDOW * win, const chtype ch)
{
  win->bkgd = ch;
}

/* */

int wbkgd (WINDOW * win, const chtype ch)
{
  int y_idx, x_idx;
  chtype old_bkgd;

  old_bkgd = win->bkgd;
  win->bkgd = ch;

  for (y_idx = 0; y_idx < win->lines; ++y_idx)
    {
      for (x_idx = 0; x_idx < win->cols; ++x_idx)
        {
          int wch;
          wch = win->winBuf[y_idx][x_idx];

          if (wch == old_bkgd)
            wch = ch;
          else
            wch = (wch & A_CHARTEXT) | (ch & ~A_CHARTEXT);

          win->winBuf[y_idx][x_idx] = wch;
        }
    }

  return OK;
}

/* */

chtype getbkgd (WINDOW * win)
{
  return win->bkgd;
}

/* */

int wattr_set (WINDOW * win, attr_t attrs, void * opts)
{
  win->attrword = attrs;
  return OK;
}

/* */

int wattrset (WINDOW * win, int attrs)
{
  win->attrword = (attr_t)attrs;
  return OK;
}

/* */

bool has_colors (void)
{
  return f_has_color;
}

/* */

bool can_change_color (void)
{
  return f_change_color;
}

/* */

int init_pair (short pair, short fore, short back)
{
  if (pair < 1 || pair >= COLOR_PAIRS) return ERR;

  local_init_pair(pair, fore, back);

  if (FALSE == color_pair_init[pair])
    {
      touchwin(curscr);
      wrefresh(curscr);
    }

  color_pair_init[pair] = TRUE;

  return OK;
}

/* */

int init_color (short color, short sr, short sg, short sb)
{
  if (!f_change_color) return ERR;

  if (color < 0 || color >= COLORS) return ERR;

  if (sr < 0 || sr > 1000) return ERR;
  if (sg < 0 || sg > 1000) return ERR;
  if (sb < 0 || sb > 1000) return ERR;

  local_init_color(color, sr, sg, sb);

  touchwin(curscr);
  wrefresh(curscr);

  return OK;
}

/* */

int pair_content (short pair, short * fore, short * back)
{
  if (pair < 1 || pair >= COLOR_PAIRS) return ERR;

  local_pair_content(pair, fore, back);
  return OK;
}

/* */

int color_content (short color, short * sr, short * sg, short * sb)
{
  if (color < 0 || color >= COLORS) return ERR;

  local_color_content(color, sr, sg, sb);
  return OK;
}

/* */

int start_color (void)
{
  int idx;

  local_start_color(&f_has_color, &f_change_color, &COLORS, &COLOR_PAIRS);

  if (!f_has_color) goto return_no_color;

  if (COLOR_PAIRS > BEST_CASE_COLOR_PAIRS)
    COLOR_PAIRS = BEST_CASE_COLOR_PAIRS;

  color_pair_init = malloc(COLOR_PAIRS * sizeof(bool));
  if (0 == color_pair_init) goto return_no_color;

  for (idx = 0; idx < COLOR_PAIRS; ++idx)
    color_pair_init[idx] = FALSE;

  return OK;

 return_no_color:

  f_has_color = 0;
  f_change_color = 0;
  COLORS = 2;
  COLOR_PAIRS = 1;
  return OK;
}

/* */

int wclrtoeol (WINDOW * win)
{
  int x_idx;

  /* touch the appropriate part of the line */
  if (win->curX < win->begLine[win->curY]) win->begLine[win->curY] = win->curX;
  win->endLine[win->curY] = win->cols;

  /* fill line with blanks from cursor to eol */
  for (x_idx = win->curX; x_idx < win->cols; ++x_idx)
    win->winBuf[win->curY][x_idx] = win->bkgd;

  return OK;
}

/* */

int wclrtobot (WINDOW * win)
{
  int y_idx;
  int x_idx;

  /* clear to the end of the line the cursor is on */
  wclrtoeol(win);

  /* clear the lines below the cursor */
  for (y_idx = win->curY + 1; y_idx < win->lines; ++y_idx)
    {
      /* touch the line */
      win->begLine[y_idx] = 0;
      win->endLine[y_idx] = win->cols;

      for (x_idx = 0; x_idx < win->cols; ++x_idx)
        win->winBuf[y_idx][x_idx] = win->bkgd;
    }

  return OK;
}

/* */

int wscrl (WINDOW * win, int nlines)
{
  int y_idx;
  int x_idx;
  int dirty;

  if (FALSE == win->f_scroll) return ERR;

  if (0 == nlines) return OK;
  if (abs(nlines) >= win->lines) return wclear(win);

  dirty = TRUE;

  if (has_local_scroll)
    {
      /* make sure everything's out there before scrolling */
      wrefresh(win);

      if (OK == local_scroll(win->top, win->left, win->lines, win->cols, nlines))
        dirty = FALSE;
    }

  /* now do the actual scrolling */
  if (nlines > 0)
    {
      for (y_idx = 0; y_idx < (win->lines - nlines); ++y_idx)
        {
          if (dirty)
            {
              win->begLine[y_idx] = 0;
              win->endLine[y_idx] = win->cols;
            }
          else
            {
              win->begLine[y_idx] = win->begLine[y_idx + nlines];
              win->endLine[y_idx] = win->endLine[y_idx + nlines];
            }

          /* scroll contents */
          for (x_idx = 0; x_idx < win->cols; ++x_idx)
            win->winBuf[y_idx][x_idx] = win->winBuf[y_idx + nlines][x_idx];
        }

      /* leave blank lines at bottom */
      while (y_idx < win->lines)
        {
          if (dirty)
            {
              win->begLine[y_idx] = 0;
              win->endLine[y_idx] = win->cols;
            }
          else
            {
              win->begLine[y_idx] = win->cols;
              win->endLine[y_idx] = 0;
            }

          for (x_idx = 0; x_idx < win->cols; ++x_idx)
            win->winBuf[y_idx][x_idx] = win->bkgd;

          ++y_idx;
        }
    }
  else /* nlines < 0 */
    {
      for (y_idx = win->lines - 1; y_idx >= -nlines; --y_idx)
        {
          if (dirty)
            {
              win->begLine[y_idx] = 0;
              win->endLine[y_idx] = win->cols;
            }
          else
            {
              win->begLine[y_idx] = win->begLine[y_idx + nlines];
              win->endLine[y_idx] = win->endLine[y_idx + nlines];
            }

          /* scroll contents */
          for (x_idx = 0; x_idx < win->cols; ++x_idx)
            win->winBuf[y_idx][x_idx] = win->winBuf[y_idx + nlines][x_idx];
        }

      /* leave blank lines at top */
      while (y_idx >= 0)
        {
          if (dirty)
            {
              win->begLine[y_idx] = 0;
              win->endLine[y_idx] = win->cols;
            }
          else
            {
              win->begLine[y_idx] = win->cols;
              win->endLine[y_idx] = 0;
            }

          for (x_idx = 0; x_idx < win->cols; ++x_idx)
            win->winBuf[y_idx][x_idx] = win->bkgd;

          --y_idx;
        }
    }

  return OK;
}

/* */

int winsdelln (WINDOW * win, int nlines)
{
  int y_idx;
  int x_idx;
  int dirty;

  if (0 == nlines) return OK;
  if (abs(nlines) >= (win->lines - win->curY))
    {
      int curX;

      curX = win->curX;
      win->curX = 0;

      wclrtobot(win);

      win->curX = curX;
      return OK;
    }

  dirty = TRUE;

  if (win->f_idl && has_local_idl)
    {
      /* make sure everything's out there before inserting/deleting */
      wrefresh(win);

      if (OK == local_idl(win->top, win->left, win->lines, win->cols, win->curY, nlines))
        dirty = FALSE;
    }

  if (nlines > 0)
    {
      /* scroll down the current line and all lines beneath it */

      for (y_idx = win->lines - nlines - 1; y_idx >= win->curY; --y_idx)
        {
          if (dirty)
            {
              win->begLine[y_idx + nlines] = 0;
              win->endLine[y_idx + nlines] = win->cols;
            }
          else
            {
              win->begLine[y_idx + nlines] = win->begLine[y_idx];
              win->endLine[y_idx + nlines] = win->endLine[y_idx];
            }

          for (x_idx = 0; x_idx < win->cols; ++x_idx)
            win->winBuf[y_idx + nlines][x_idx] = win->winBuf[y_idx][x_idx];
        }

      /* blank the 'inserted' lines */

      for (y_idx = win->curY + nlines - 1; y_idx >= win->curY; --y_idx)
        {
          if (dirty)
            {
              win->begLine[y_idx] = 0;
              win->endLine[y_idx] = win->cols;
            }
          else
            {
              win->begLine[y_idx] = win->cols;
              win->endLine[y_idx] = 0;
            }

          for (x_idx = 0; x_idx < win->cols; ++x_idx)
            win->winBuf[y_idx][x_idx] = win->bkgd;
        }
    }
  else
    {
      nlines = -nlines;

      /* scroll up the lines underneath the current line */

      for (y_idx = win->curY + nlines; y_idx < win->lines; ++y_idx)
        {
          if (dirty)
            {
              win->begLine[y_idx - nlines] = 0;
              win->endLine[y_idx - nlines] = win->cols;
            }
          else
            {
              win->begLine[y_idx - nlines] = win->begLine[y_idx];
              win->endLine[y_idx - nlines] = win->endLine[y_idx];
            }

          for (x_idx = 0; x_idx < win->cols; ++x_idx)
            win->winBuf[y_idx - nlines][x_idx] = win->winBuf[y_idx][x_idx];
        }

      /* blank lines at the bottom of the screen */

      for (y_idx = win->lines - nlines ; y_idx < win->lines; ++y_idx)
        {
          if (dirty)
            {
              win->begLine[y_idx] = 0;
              win->endLine[y_idx] = win->cols;
            }
          else
            {
              win->begLine[y_idx] = win->cols;
              win->endLine[y_idx] = 0;
            }

          for (x_idx = 0; x_idx < win->cols; ++x_idx)
            win->winBuf[y_idx][x_idx] = win->bkgd;
        }
    }

  return OK;
}

/* */

int winsch (WINDOW * win, chtype ch)
{
  int x_idx;

  /* move current line right */
  for (x_idx = win->cols - 1; x_idx > win->curX; --x_idx)
    win->winBuf[win->curY][x_idx ] = win->winBuf[win->curY][x_idx - 1];

  /* mark line dirty to the right of the cursor */
  if (win->curX < win->begLine[win->curY]) win->begLine[win->curY] = win->curX;
  win->endLine[win->curY] = win->cols;

  /* put in the new character */
  waddch(win, ch);

  return OK;
}

/* */

int wdelch (WINDOW * win)
{
  int x_idx;

  for (x_idx = win->curX + 1; x_idx < win->cols; ++x_idx)
    win->winBuf[win->curY][x_idx - 1] = win->winBuf[win->curY][x_idx];

  if (win->curX < win->begLine[win->curY]) win->begLine[win->curY] = win->curX;
  win->endLine[win->curY] = win->cols;

  win->winBuf[win->curY][win->cols - 1] = win->bkgd;

  return OK;
}

/* */

int wmove (WINDOW * win, int y, int x)
{
  /* check arguments */
  if (x < 0 || x >= win->cols) return ERR;
  if (y < 0 || y >= win->lines) return ERR;

  win->curY = y;
  win->curX = x;

  return OK;
}

/* */

int waddstr (WINDOW * win, char * s)
{
  while (*s)
    waddch(win, (unsigned char)(*s++));

  return OK;
}

/* */

int printw (char * fstr, ...)
{
  int idx;
  int count;
  va_list arglist;

  va_start(arglist, fstr);
  count = vsprintf(strBuf, fstr, arglist);
  va_end(arglist);

  for (idx = 0; idx < count; ++idx)
    waddch(stdscr, (unsigned char)strBuf[idx]);

  return count;
}

/* */

int wprintw (WINDOW * win, char *fstr, ...)
{
  int idx;
  int count;
  va_list arglist;

  va_start(arglist, fstr);
  count = vsprintf(strBuf, fstr, arglist);
  va_end(arglist);

  for (idx = 0; idx < count; ++idx)
    waddch(win, (unsigned char)strBuf[idx]);

  return count;
}

/* */

int vw_printw (WINDOW * win, char * fstr, va_list arg)
{
  int idx;
  int count;

  count = vsprintf(strBuf, fstr, arg);

  for (idx = 0; idx < count; ++idx)
    waddch(win, (unsigned char)strBuf[idx]);

  return count;
}

/* */

int vwprintw (WINDOW * win, char * fstr, va_list arg)
{
  return vw_printw(win, fstr, arg);
}

/* */

int mvprintw (int y, int x, char * fstr, ...)
{
  int idx;
  int count;
  va_list arglist;

  stdscr->curY = y;
  stdscr->curX = x;

  va_start(arglist, fstr);
  count = vsprintf(strBuf, fstr, arglist);
  va_end(arglist);

  for (idx = 0; idx < count; ++idx)
    waddch(stdscr, (unsigned char)strBuf[idx]);

  return count;
}

/* */

int mvwprintw (WINDOW * win, int y, int x, char * fstr, ...)
{
  int idx;
  int count;
  va_list arglist;

  win->curY = y;
  win->curX = x;

  va_start(arglist, fstr);
  count = vsprintf(strBuf, fstr, arglist);
  va_end(arglist);

  for (idx = 0; idx < count; ++idx)
    waddch(win, (unsigned char)strBuf[idx]);

  return count;
}

/* */

int wgetstr (WINDOW * win, char * s)
{
  int ch;
  char * ptr;

  ptr = s;
  ch = wgetch(win);

  while (ch != LF && ch != CR)
    {
      if (ch != ERR)
        *ptr++ = (char)ch;

      ch = wgetch(win);
    }

  *ptr = 0;

  return OK;
}

/* */

int wgetnstr (WINDOW * win, char * s, int n)
{
  int ch;
  int idx = 0;

  ch = wgetch(win);
  while (ch != LF && ch != CR)
    {
      if (ch != ERR)
        {
          if (idx < n)
            {
              ++idx;
              *s++ = (char)ch;
            }
          else
            {
              beep();
            }
        }

      ch = wgetch(win);
    }

  *s = 0;

  return OK;
}

/* */

int wnoutrefresh (WINDOW * win)
{
  int y_idx;
  int x_idx;

  if (win->f_clear)
    {
      werase(curscr);
      local_clear();
      win->f_clear = FALSE;
    }
  else if (curscr->f_clear || win == curscr)
    {
      werase(curscr);
      local_clear();
      curscr->f_clear = FALSE;
      return OK;
    }

  if (win == stdscr)
    return OK;

  for (y_idx = 0; y_idx < win->lines; ++y_idx)
    {
      if (win->endLine[y_idx] > win->begLine[y_idx])
        {
          for (x_idx = win->begLine[y_idx]; x_idx < win->endLine[y_idx]; ++x_idx)
            stdscr->winBuf[win->top + y_idx][win->left + x_idx] = win->winBuf[y_idx][x_idx];

          if (stdscr->begLine[win->top + y_idx] > win->begLine[y_idx])
            stdscr->begLine[win->top + y_idx] = win->begLine[y_idx];

          if (stdscr->endLine[win->top + y_idx] < win->endLine[y_idx])
            stdscr->endLine[win->top + y_idx] = win->endLine[y_idx];

          win->endLine[y_idx] = 0;
          win->begLine[y_idx] = win->cols;
        }
    }

  return OK;
}

/* */

int doupdate (void)
{
  int y_idx;
  int charcount = 0;

  for (y_idx = 0; y_idx < LINES; ++y_idx)
    {
      if (stdscr->endLine[y_idx] > stdscr->begLine[y_idx])
        {
          int x_idx = stdscr->begLine[y_idx];
          while (x_idx < stdscr->endLine[y_idx])
            {
              int x_start;

              /* find the first character that needs to be updated */
              while (x_idx < stdscr->endLine[y_idx])
                {
                  if (stdscr->winBuf[y_idx][x_idx] != curscr->winBuf[y_idx][x_idx])
                    break;
                  ++x_idx;
                }

              /* output only characters that need to be updated */
              x_start = x_idx;
              while (x_idx < stdscr->endLine[y_idx])
                {
                  chtype ch;
                  ch = stdscr->winBuf[y_idx][x_idx];
                  if (ch == curscr->winBuf[y_idx][x_idx])
                    break;
                  curscr->winBuf[y_idx][x_idx] = ch;
                  local_putchar(ch, x_idx, y_idx);
                  ++charcount;
                  ++x_idx;
                }

              local_update(y_idx, x_start, x_idx - 1);
            }

          /* mark the line clean */
          stdscr->begLine[y_idx] = 0;
          stdscr->endLine[y_idx] = COLS;
        }
    }

  return charcount;
}

#if 1
int wrefresh (WINDOW * win)
{
  wnoutrefresh (win);
  return doupdate();
}
#else
int wrefresh (WINDOW * win)
{
  int y_idx;
  int charcount = 0;

  /* if f_clear is set, erase curscr to force a full screen redraw */
  if (win->f_clear)
    {
      werase(curscr);
      local_clear();
      win->f_clear = FALSE;
    }

  for (y_idx = 0; y_idx < win->lines; ++y_idx)
    {
      /* go through the update arrays and draw the dirty parts of the lines */
      if (win->endLine[y_idx] > win->begLine[y_idx])
		{
          int x_idx;

          assert(win->begLine[y_idx] >= 0 && win->begLine[y_idx] < win->cols);
          assert(win->endLine[y_idx] > 0 && win->endLine[y_idx] <= win->cols);

          x_idx = win->begLine[y_idx];
          while (x_idx < win->endLine[y_idx])
            {
              int x_start;

              /* find the first character that needs to be updated */
              while (x_idx < win->endLine[y_idx])
                {
                  if (win->winBuf[y_idx][x_idx] != curscr->winBuf[win->top + y_idx][win->left + x_idx])
                    break;
                  ++x_idx;
                }

              /* output only characters that need to be updated */
              x_start = x_idx;
              while (x_idx < win->endLine[y_idx])
                {
                  chtype ch;
                  ch = win->winBuf[y_idx][x_idx];
                  if (ch == curscr->winBuf[win->top + y_idx][win->left + x_idx])
                    break;
                  curscr->winBuf[win->top + y_idx][win->left + x_idx] = ch;
                  local_putchar(ch, win->left + x_idx, win->top + y_idx);
                  ++charcount;
                  ++x_idx;
                }

              local_update(win->top + y_idx, win->left + x_start, win->left + x_idx - 1);
            }

          /* mark the line clean */
          win->endLine[y_idx] = 0;
          win->begLine[y_idx] = win->cols;
        }
	}

  return charcount;
}
#endif
