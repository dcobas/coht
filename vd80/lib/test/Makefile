#================================================================
# Makefile to produce libvd80 test program
#================================================================

CPU ?= L866

include /acc/dsc/src/co/Make.auto

COMPILE_TIME:=$(shell date +%s)

CFLAGS = -g -Wall -I. -I./extest -I../stub -DCOMPILE_TIME=$(COMPILE_TIME)

LDLIBS += extest/libextest.$(CPU).a ../stub/stublib.$(CPU).a
LDFLAGS += -ldl -lreadline -lcurses

ALL  = vd80Test-stub.$(CPU).o vd80Test-stub.$(CPU)

SRCS = vd80Test-stub.c vd80libCmds.c

HDRS = vd80libTst.h

all: $(ALL)

vd80Test-stub.$(CPU).o: $(SRCS) $(HDRS)

vd80Test-stub.$(CPU): vd80Test-stub.$(CPU).o $(LDLIBS)

extest/libextest.$(CPU).a:
	$(MAKE) -C extest CPU=$(CPU)

clean:
	rm -f *.o *.$(CPU)
	$(MAKE) -C extest $@ CPU=$(CPU)
