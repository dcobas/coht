#************************************************************
# makefile (for the Tg8 Dialog program)
#
# Original version for Tg8test by Vova 
# Mod. by Bruno ( 05/08/97) 
# Mod.#2  of 24/09/99 add "-l lynx" for lynx_sem_signal
# 
#************************************************************

CC = gcc

TG8 = ..
DRV = $(TG8)/driver
FIR = $(TG8)/firmware
TST = $(TG8)/test

CFLAGS = -g -O -I. -I/usr/local/include -I$(FIR) -I$(DRV) -llynx
CESLIB = /lib/ces/libvme.a
VMELIB = /usr/src/ces/examples/vlib/vlib.o

all: tg8dial tg8dpr_dump

tg8dial_s1 : tg8dial_s1.o
	$(CC) tg8dial_s1.c

tg8dial.o: tg8dial.c tg8dial_s1.o \
	$(TST)/tg8test.h $(DRV)/tg8drvr.h $(FIR)/tg8Pub.h

tg8dial: tg8dial.o
	$(CC) tg8dial.c $(CFLAGS) -o tg8dial tg8dial_s1.o \
	-lnetinet -lm

#***********************************************************

tg8dpr_dump: tg8dpr_dump.c 
	$(CC) tg8dpr_dump.c $(VMELIB) $(CESLIB) -o tg8dpr_dump
	
#***********************************************************