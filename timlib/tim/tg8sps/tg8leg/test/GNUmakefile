include /ps/dsc/src/co/Make.auto

TG8 = ..
DRV = $(TG8)/driver
F = $(TG8)/firmware

CFLAGS = -g -O -O -I. -I/usr/local/include -I$(F) -I$(DRV) \
	 -I/ps/local/$(CPU)/include

all: tg8test.$(CPU)

tg8test.$(CPU).o: tg8test.c tg8test.h $(DRV)/tg8drvr.h $(F)/tg8Pub.h

tg8test.$(CPU): tg8test.$(CPU).o
	$(CC) $(CFLAGS) -o tg8test.$(CPU) tg8test.$(CPU).o
