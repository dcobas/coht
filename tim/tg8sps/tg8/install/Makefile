TG8 = ..
DRV = $(TG8)/driver
INC = -I$(TG8)/firmware -I$(DRV)
CC = gcc

CFLAGS = -g -I/usr/include -I/usr/local/include -I. ${INC} -DCONVERTOR=0

all: tg8install tg8drvr.info

tg8install.o: tg8install.c ${DRV}/tg8drvr.h ${DRV}/tg8drvrP.h

tg8install: tg8install.o
	$(CC) $(CFLAGS) -o tg8install tg8install.o

tg8drvr.info: tg8install ins.bat
	      ins.bat

#eof
