
CC=gcc
CFLAGS=-Wall -I../inter
LIBS = ../inter/libinter.a -lcurses

all:  lib life

lib:
	(cd ../inter; make)

life:	life.o life_i.o ../inter/libinter.a
	$(CC) $(CFLAGS) -o life life.o life_i.o $(LIBS)

life0:  life0.o life_i0.o ../inter/libinter.a
	$(CC) $(CFLAGS) -o life0 life0.o life_i0.o $(LIBS)

life.o:		life.c life.h
life_i.o:	life_i.c life.h

clean:
	rm -f *.o *.obj life.exe life
