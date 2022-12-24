
CC = gcc
CFLAGS = -Wall -O2 -g
CPPFLAGS = `gtk-config --cflags`
LIBS = `gtk-config --libs`

CURSRC = curses.c curgtk.c
CUROBJ = curses.o curgtk.o
ALLPROGS = rain tclock scroll insdel suicide
ALLSRC = rain.c tclock.c suicide.c scroll.c insdel.c

default: all

curses: $(CUROBJ)

rain: $(CUROBJ) rain.o
	$(CC) $(CFLAGS) $(CUROBJ) rain.o $(LIBS) -o rain

tclock: $(CUROBJ) tclock.o
	$(CC) $(CFLAGS) $(CUROBJ) tclock.o $(LIBS) -o tclock

suicide: $(CUROBJ) suicide.o
	$(CC) $(CFLAGS) $(CUROBJ) suicide.o $(LIBS) -o suicide

scroll: $(CUROBJ) scroll.o
	$(CC) $(CFLAGS) $(CUROBJ) scroll.o $(LIBS) -o scroll

insdel: $(CUROBJ) insdel.o
	$(CC) $(CFLAGS) $(CUROBJ) insdel.o $(LIBS) -o insdel

all: $(ALLPROGS)

clean:
	rm -f *.o $(ALLPROGS)

depend:
	makedepend $(CPPFLAGS) $(CURSRC) $(ALLSRC)
