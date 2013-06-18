CPU ?= L865

include /acc/src/dsc/co/Make.auto

CFLAGS=-g -Wall
CC=gcc
HEADERS := ../include/ -I/acc/local/$(CPU)/include -I/usr/include/libxml2 -I/acc/src/dsc/drivers/coht/include \
	-I../extest/ -I/usr/include/ -I../lib -I/acc/sys/$(CPU)/usr/include -I../ad9516

LOADLIBES := -L../lib/ -L../ad9516
LDLIBS=-lcvorg.$(CPU) -lreadline -lxml2 -lad9516.$(CPU) -lrt -lm -ltermcap
all: extest.o cvorgtest.$(CPU) 

cvorgtest.$(CPU): 
	mkdir -p obj
	$(CC) $(CFLAGS) -c -D$(CPU) -I$(HEADERS) cvorgtest.c -o ./obj/cvorgtest.o
	$(CC) $(CFLAGS) -o ./obj/$@ ./obj/cvorgtest.o ./obj/extest.o ./obj/cmd_noskel.o $(LOADLIBES) $(LDLIBS)

extest.o:
	mkdir -p obj
	$(CC) $(CFLAGS) -c -D$(CPU) -I$(HEADERS) ../extest/cmd_noskel.c -o ./obj/cmd_noskel.o
	$(CC) $(CFLAGS) -c -D$(CPU) -I$(HEADERS) ../extest/extest.c -o ./obj/extest.o

clean:
	rm -rf ./obj/
