
/*
 *
 * From yale-com!nglasser Sun Jan 16 18:02:05 1983
 * In-Real-Life: Nathan Glasser
 * Subject: Suicide.c (yale-com.659)
 * Newsgroups: net.suicide
 * 
 * The following is a program for those of you who actually like to watch people
 * committing suicide. This program uses the Berkely Curses library, and should
 * be compiled with the following flags:
 *	cc suicide.c -lcurses -ltermcap -o suicide
 * 
 * 				 -- Nathan Glasser
 * 				 ..decvax!yale-comix!nglasser
 *
 * Modified for Apple IIgs by Sheldon Simms, 1996
 * function usleep() added and used to slow down the animation.
 */

#include <time.h>
#include <unistd.h>
#include "curses.h"

#define DELAY 100000

void printpic(char **pic,int len,int y,int x)
{
    int i;
    for (i = 0; i < len; i++)
	mvprintw(i + y,x,pic[i]);
    refresh();
}

int main( void )
{
    int i, j;
    static char *tree[] = {
	"  ((( (   ( )) ) ))",
	"    | ( (((()( ( )|",
	"\\   |    )  ))) ) |",
	" \\  |             |",
	"\\ \\ |             |",
	" \\ \\   _____      ---------------.",
	"  \\   /     \\                ___/",
	"   \\  |      \\    ----------/",
	"    | |       |   |",
	"    |  \\_____/    |",
	"    |             |",
	"    |             |",
	"    |             |",
	"    |             |",
	"    |             |",
	"    |             |",
	"    |             |",
	"    |             |",
	"    |             |",
	"   /               \\",
	"  |                 |",
	" /                   \\",
    "                      ------------------------------------------------"};

    static char *man1[] = {
	" __o__",
	"   I",
	"  / \\"};

    static char *man2[] = {
	"   o",
	"  /\\",
	" |_"};

    static char *man3[] = {
	"  o",
	" /\\",
	"|",
	" \\"};

    static char *man4[] = {
	"    ",
	"\\o/",
	" I ",
	"/ \\"};

    static char *man5[] = {
	"     ",
	" \\o/ ",
	"\\_I_/"};

    static char *man6[] = {
	"    o    /",
	"  /    ",
	"\\      I",
	"----\\"};

    static struct {
	int y;
	int x;
    } divecoord[] = {
	{2,34},
	{1,34},
	{1,35},
	{1,36},
	{1,37},
	{1,38},
	{1,39},
	{0,40},
	{0,41},
	{0,42}};


    initscr();
    clear();
    printpic(tree,23,0,0);
    for (i = 20; i < 31; i++)
    {
    	printpic(man1,3,2,i);
        usleep( DELAY );
    }
    for (i = 31; i < 33; i++)
    {
		printpic(man2,3,2,i);
        usleep( DELAY );
	 }
    move(2,33); clrtoeol();
    move(3,33); clrtoeol();
    move(4,33); clrtoeol();
    mvprintw(0,19,"Aaaaa");
    for (i = 0; i < 10; i++)
    {
	if (i == 5)
	    mvprintw(0,24,"uuuuuuuu");
	printpic(man3,4,divecoord[i].y,divecoord[i].x);
    usleep( DELAY );
	for (j = 0; j < 4; j++)
	{
	    move(divecoord[i].y + j,divecoord[i].x);
	    clrtoeol();
	}
    }
    mvprintw(0,32,"gggggggggggggggg");
    printpic(man4 + 1,3,1,43);
    for (i = 1; i < 11; i++)
    {
	printpic(man4,4,i,43);
    usleep( DELAY );
   }
    move(0,48); printw("hhhhhhhhhhhhhhhhhhhhh");
    for (i = 11; i < 21; i++)
    {
	printpic(man5,3,i,42);
    usleep( DELAY );
   }
    mvprintw(0,69,"!!!!!!!!!!");
    printpic(man6,4,19,41);
    move(23,0);
    refresh();

    {
      int ch;
      do ch = getch(); while (ERR == ch);
    }

    endwin();
    return 0;
}
