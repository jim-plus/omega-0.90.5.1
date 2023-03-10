### These two definitions are used if you 'make install'
### the value of LIBDIR should be the same as OMEGALIB in defs.h
BINDIR = ./omega/
LIBDIR = ./lib/

### choose your optimization level
OFLAGS = -O2

### One of these should be uncommented, as appropriate, unless your compiler
### does it for you.  You can test this by simply trying to 'make' omega -
### it will fail if none of them are defined.  If you do uncomment
### one, make sure you comment out the other definition of CFLAGS lower down

CFLAGS = -ggdb -Wall $(OFLAGS) -DBSD # -ansi
#CFLAGS = -DSYSV $(OFLAGS)
#CFLAGS = -DMSDOS $(OFLAGS)
#CFLAGS = -DAMIGA $(OFLAGS)

### CPP should contain the command to run the C preprocessor.
#CPP = cc -E
#CPP = /lib/cpp
CPP = gcc -E

### define a compiler
CC = gcc

### Select one of the following that is appropriate for your curses...
### Comment ALL of them out if you are using opcurses
#LIBS = -lncurses -ltermcap
#LIBS = -lcurses -ltermlib
#LIBS = -lpdcurses
#Linux links in term?? automatically.
LIBS = -lncurses

### uncomment to use "op-curses" package
#OPDEF = -DUSE_OPCURSES
#CUROBJ = ../opcurses/curses.o ../opcurses/curgtk.o

### uncomment to compile using opcurses GTK+ driver
#CPPFLAGS = `gtk-config --cflags` -DUSE_OPCURSES
#LDFLAGS = `gtk-config --libs`

#################### that's it for changing the Makefile ####################

OLIBSTUFF = tools/libsrc/maps.dat lib/maps.dat

TOOLS = tools/crypt tools/decrypt tools/makedate

CLROBJ = clrgen.o

OBJ = omega.o abyss.o aux1.o aux2.o aux3.o bank.o char.o city.o\
      command1.o command2.o command3.o country.o date.o effect1.o\
      effect2.o effect3.o etc.o env.o file.o gen1.o gen2.o guild1.o guild2.o\
      house.o init.o inv.o item.o itemf1.o itemf2.o itemf3.o lev.o map.o\
      mmelee.o mmove.o mon.o move.o movef.o mspec.o mstrike.o mtalk.o\
      newrand.o priest.o pdump.o save.o scr.o site1.o site2.o spell.o\
      stats.o time.o trap.o util.o village.o

all: maps.dat omega

omega: $(CUROBJ) $(CLROBJ) $(OBJ)
	$(CC) $(LDFLAGS) $(OFLAGS) $(CUROBJ) $(CLROBJ) $(OBJ) $(LIBS) -o omega
	rm date.c date.o

date.c:
	tools/makedate > date.c

maps.dat:
	cd tools; make
	cd tools/libsrc; make maps.dat

install: omega maps.dat $(BINDIR) $(LIBDIR)
	cp omega $(BINDIR)
	chmod 4711 $(BINDIR)/omega
	- cp lib/* $(LIBDIR)
	cp maps.dat $(LIBDIR)
	chmod 0644 $(LIBDIR)/help*.txt $(LIBDIR)/license.txt $(LIBDIR)/motd.txt $(LIBDIR)/thanks.txt $(LIBDIR)/update.txt
	chmod 0600 $(LIBDIR)/abyss.txt $(LIBDIR)/scroll[1234].txt $(LIBDIR)/maps.dat
	chmod 0600 $(LIBDIR)/omega.hi $(LIBDIR)/omega.log $(LIBDIR)/omegahi.bak

install_not_suid: omega maps.dat $(BINDIR) $(LIBDIR)
	cp omega $(BINDIR)
	chmod 0711 $(BINDIR)/omega
	- cp lib/* $(LIBDIR)
	cp maps.dat $(LIBDIR)
	chmod 0644 $(LIBDIR)/help*.txt $(LIBDIR)/license.txt $(LIBDIR)/motd.txt $(LIBDIR)/thanks.txt $(LIBDIR)/update.txt
	chmod 0644 $(LIBDIR)/abyss.txt $(LIBDIR)/scroll[1234].txt $(LIBDIR)/*.dat
	chmod 0666 $(LIBDIR)/omega.hi $(LIBDIR)/omega.log
	chmod 0600 $(LIBDIR)/omegahi.bak

clean:
	rm -f $(OBJ) clrgen.h clrgen.c genclr.o genclr omega

distclean:
	rm -f $(OBJ) $(CUROBJ) $(TOOLS) $(OLIBSTUFF) clrgen.h clrgen.c genclr.o genclr omega
	cp lib/omega.hi.backup lib/omega.hi
	cp lib/omega.log.backup lib/omega.log

$(CUROBJ): ../opcurses/curses.h ../opcurses/cmacros.h ../opcurses/xcurses.h

$(CLROBJ): clrgen.h

$(OBJ): defs.h extern.h glob.h iinit.h minit.h clrgen.h

clrgen.h clrgen.c: genclr.c minit.h defs.h
	$(MAKE) genclr
	$(CPP) $(OPDEF) -DOMEGA_CLRGEN *.[ch] | ./genclr clrgen.c clrgen.h

genclr: genclr.o
	$(CC) $(LDFLAGS) genclr.o -o genclr
