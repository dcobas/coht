CPU ?= L865

include /acc/src/dsc/co/Make.auto

INCDIR := ../include

CFLAGS=	-Wall -g -I$(INCDIR)

LIBAD9516 = libad9516.$(CPU).a

all: $(LIBAD9516)

ad9516.$(CPU).o: 
	$(CC) $(CFLAGS) -c -I. ad9516.c -o $@

libad9516.$(CPU).a: ad9516.$(CPU).o
	$(AR) rv $(LIBAD9516) $^

clean:
	$(RM) *.a *.o $(BAKS)
