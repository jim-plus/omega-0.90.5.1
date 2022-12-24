#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include "curses.h"

/*
  tclock - analog/digital clock for curses.
  If it gives you joy, then
  (a) I'm glad
  (b) you need to get out more :-)

  This program is copyright Howard Jones, September 1994
  (ha.jones@ic.ac.uk). It may be freely distributed as
  long as this copyright message remains intact, and any
  modifications are clearly marked as such. [In fact, if
  you modify it, I wouldn't mind the modifications back,
  especially if they add any nice features. A good one
  would be a precalc table for the 60 hand positions, so
  that the floating point stuff can be ditched. As I said,
  it was a 20 hackup minute job.]
  
  COMING SOON: tfishtank. Be the envy of your mac-owning
  colleagues.
*/

/* To compile: cc -o tclock tclock.c -lcurses -lm */

#ifndef PI
#define PI 3.141592654
#endif

#define sign(_x) (_x<0?-1:1)

void sleep( long secs );

/* Plot a point */
void
plot(int x,int y,char col)
{
  mvaddch(y,x,col);
}
 

/* Draw a diagonal(arbitrary) line using Bresenham's alogrithm. */
void
dline(int x1,int y1,int x2,int y2,char ch)
{
	int dx,dy;
	int ax,ay;
	int sx,sy;
	int x,y;
	int d;
	
	dx=x2-x1;
	dy=y2-y1;
	
	ax=abs(dx*2);
	ay=abs(dy*2);

	sx=sign(dx);
	sy=sign(dy);

	x=x1;
	y=y1;
		
	if(ax>ay)
	{
		d=ay-(ax/2);
		
		while(1)
		{
			plot(x,y,ch);
			if(x==x2) return;
			
			if(d>=0)
			{
				y+=sy;
				d-=ax;
			}
			x+=sx;
			d+=ay;			
		}
	}
	else
	{
		d=ax-(ay/2);
		
		while(1)
		{
			plot(x,y,ch);
			if(y==y2) return;
			
			if(d>=0)
			{
				x+=sx;
				d-=ay;
			}
			y+=sy;
			d+=ax;			
		}	
	}
}

int main( void )
{
	int i,cx,cy;
	double mdx,mdy,hdx,hdy,mradius,hradius,mangle,hangle;
	double sdx,sdy,sangle,sradius,hours;
	time_t tim;
	struct tm *t;
	char szChar[10];
	chtype c;
	
	initscr();
   nodelay( stdscr, TRUE );

	cx=COLS/2;
	cy=LINES/2;
	mradius=(double)(cy-3);
	hradius=(double)(cy-6);
	sradius=(double)(cy-4);

	for(i=0;i<12;i++)
	  {
	    sangle=(i+1)*(2.0*PI)/12.0;
	    sradius=(double)(cy-2);
	    sdx=2.0*sradius*sin(sangle);
	    sdy=sradius*cos(sangle);
	    sprintf(szChar,"%d",i+1);

	    mvaddstr((int)(cy-sdy),(int)(cx+sdx),szChar);
	  }

	mvaddstr(0,0,"ASCII Clock by Howard Jones (ha.jones@ic.ac.uk),1994");

	sradius=(double)(cy-4);
	while(1)
	  {
		c = getch();
       if ( c == 3 )
       		break;
       		
	    sleep(1);

	    tim=time(0);
	    t=localtime(&tim);

	    hours=(t->tm_hour + (t->tm_min/60.0));
	    if(hours>12.0) hours-=12.0;

	    mangle=(t->tm_min)*(2*PI)/60.0;
	    mdx=2.0*mradius*sin(mangle);
	    mdy=mradius*cos(mangle);
	    
	    hangle=(hours)*(2.0*PI)/12.0;
	    hdx=2.0*hradius*sin(hangle);
	    hdy=hradius*cos(hangle);
       
	    sangle=(t->tm_sec%60)*(2.0*PI)/60.0;
	    sdx=2.0*sradius*sin(sangle);
	    sdy=sradius*cos(sangle);

	    plot(cx+sdx,cy-sdy,'O');
	    dline(cx,cy,cx+hdx,cy-hdy,'.');
	    dline(cx,cy,cx+mdx,cy-mdy,'#');

	    mvaddstr(LINES-1,0,ctime(&tim));
	    
	    refresh();
	    plot(cx+sdx,cy-sdy,' ');
	    dline(cx,cy,cx+hdx,cy-hdy,' ');
	    dline(cx,cy,cx+mdx,cy-mdy,' ');
	    
	  }
	endwin();
	
	return 0;
}
