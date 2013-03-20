.suffixes:	.obj

CFLAGS=-ml -v -DMSDOS -I../inter
CC=bcc

.c.obj:
	$(CC) -c $(CFLAGS) $<

all:	libs life.exe

life.exe:  life.obj life_i.obj ../inter/inter.lib
	$(CC) $(CFLAGS) life.obj life_i.obj ../inter/inter.lib

libs:
	cd ..\inter
	tmake -finter.mak
	cd ..\life

life.obj:	life.c life.h
life_i.obj:	life_i.c life.h
