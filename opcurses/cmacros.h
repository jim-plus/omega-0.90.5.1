/*
 * cmacros.h - ocurses
 * Copyright 1996,2001 W. Sheldon Simms III
 */

#ifndef INCLUDED_CMACROS_H
#define INCLUDED_CMACROS_H

/* generic macros */

#define getyx(w,y,x)         do { y = w->curY; x = w->curX; } while (0);
#define getbegyx(w,y,x)      do { y = w->top; x = w->left; } while (0);
#define getmaxyx(w,y,x)      do { y = w->lines; x = w->cols; } while (0);
#define winch(w)             (w->winBuf[w->curY][w->curX] & 0xff)
#define gettmode()
#define crmode()             cbreak()
#define nocrmode()           nocbreak()
#define scroll(w)            wscrl(w, 1)
#define overlay(s,d)         copywin(s,d,0,0,0,0,d->lines,d->cols,1)
#define overwrite(s,d)       copywin(s,d,0,0,0,0,d->lines,d->cols,0)
#define werase(w)            (wmove(w,0,0), wclrtobot(w))
#define wclear(w)            (werase(w), clearok(w,1))
#define idcok(w,f)           (w->f_idc = f, OK)
#define clearok(w,f)         (w->f_clear = f, OK)
#define leaveok(w,f)         (w->f_leave = f, OK)
#define scrollok(w,f)        (w->f_scroll = f, OK)
#define nodelay(w,f)         (w->f_nodelay = f, OK)
#define nl()                 (f_nl = TRUE, OK)
#define nonl()               (f_nl = FALSE, OK)
#define echo()               (f_echo = TRUE, OK)
#define noecho()             (f_echo = FALSE, OK)
#define touchline(w,s,c)     wtouchln(w,s,c,1);
#define touchwin(w)          wtouchln(w,0,w->lines,1)
#define untouchwin(w)        wtouchln(w,0,w->lines,0)
#define wdeleteln(w)         winsdelln(win, -1)
#define winsertln(w)         winsdelln(win, 1)
#define box(w,v,h)           wborder(w,v,v,h,h,0,0,0,0)
#define wattr_on(w,a,o)      wattr_set(w,w->attrword | (a),o)
#define wattr_off(w,a,o)     wattr_set(w,w->attrword & ~(a),o)
#define wattron(w,a)         wattrset(w,w->attrword | (attr_t)(a))
#define wattroff(w,a)        wattrset(w,w->attrword & ~(attr_t)(a))
#define wstandout(w)         wattron(w,A_STANDOUT)
#define wstandend(w)         wattrset(w,A_NORMAL)

/* stdscr macros */

#define bkgdset(c)              wbkgdset(stdscr,c)
#define bkgd(c)                 wbkgd(stdscr,c)
#define border(a,b,c,d,e,f,g,h) wborder(stdscr,a,b,c,d,e,f,g,h)
#define setscrreg(t,b)          wsetscrreg(stdscr,t,b)
#define deleteln()              wdeleteln(stdscr)
#define echochar(c)             wechochar(stdscr)
#define insertln()              winsertln(stdscr)
#define insdelln(n)             winsdelln(stdscr,n)
#define scrl(n)                 wscrl(stdscr,n)
#define attr_set(a,o)           wattr_set(stdscr,a,o)
#define attr_on(a,o)            wattr_on(stdscr,a,o)
#define attr_off(a,o)           wattr_off(stdscr,a,o)
#define attrset(a)              wattrset(stdscr,a)
#define attron(a)               wattron(stdscr,a)
#define attroff(a)              wattroff(stdscr,a)
#define standout()              wstandout(stdscr)
#define standend()              wstandend(stdscr)
#define addch(c)                waddch(stdscr,c)
#define addstr(s)               waddstr(stdscr,s)
#define clear()                 wclear(stdscr)
#define clrtobot()              wclrtobot(stdscr)
#define clrtoeol()              wclrtoeol(stdscr)
#define delch()                 wdelch(stdscr)
#define deleteln()              wdeleteln(stdscr)
#define erase()                 werase(stdscr)
#define getch()                 wgetch(stdscr)
#define getstr(s)               wgetstr(stdscr,s)
#define getnstr(s)              wgetstr(stdscr,s,n)
#define inch()                  winch(stdscr)
#define insch(c)                winsch(stdscr,c)
#define insertln()              winsertln(stdscr)
#define move(y,x)               wmove(stdscr,y,x)
#define refresh()               wrefresh(stdscr)
#define standout()              wstandout(stdscr)
#define standend()              wstandend(stdscr)

/* move macros */

#define mvwaddch(w,y,x,c)     ((ERR == wmove(w,y,x)) ? ERR : waddch(w,c))
#define mvwaddstr(w,y,x,s)    ((ERR == wmove(w,y,x)) ? ERR : waddstr(w,s))
#define mvwdelch(w,y,x)       ((ERR == wmove(w,y,x)) ? ERR : wdelch(w))
#define mvwgetch(w,y,x)       ((ERR == wmove(w,y,x)) ? ERR : wgetch(w))
#define mvwgetstr(w,y,x,s)    ((ERR == wmove(w,y,x)) ? ERR : wgetstr(w,s))
#define mvwgetnstr(w,y,x,s,n) ((ERR == wmove(w,y,x)) ? ERR : wgetnstr(w,s,n))
#define mvwinch(w,y,x)        ((ERR == wmove(w,y,x)) ? ERR : winch(w))
#define mvwinsch(w,y,x,c)     ((ERR == wmove(w,y,x)) ? ERR : winsch(w,c))

/* move stdscr macros */

#define mvaddch(y,x,c)       mvwaddch(stdscr,y,x,c)
#define mvaddstr(y,x,s)      mvwaddstr(stdscr,y,x,s)
#define mvdelch(y,x)         mvwdelch(stdscr,y,x)
#define mvgetch(y,x)         mvwgetch(stdscr,y,x)
#define mvgetstr(y,x,s)      mvwgetstr(stdscr,y,x,s)
#define mvgetnstr(y,x,s,n)   mvwgetstr(stdscr,y,x,s,n)
#define mvinch(y,x)          mvwinch(stdscr,y,x)
#define mvinsch(y,x,c)       mvwinsch(stdscr,y,x,c)

#endif /* INCLUDED_CMACROS_H */
