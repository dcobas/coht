#================================================================
# Makefile to produce libctr test program
#================================================================

CPU=L865

include /acc/dsc/src/co/Make.auto

CFLAGS = -g -Wall -I. -I.. -I../ctr/driver -rdynamic
LDLIBS = ../libctr.$(CPU).a -ldl

ALL = libctrtest.$(CPU)

SRCS = Cmds.c Cmds.h CtrLibCmds.c DoCmd.c GetAtoms.c libctrtest.c PrintAtoms.c

HDRS = Cmds.h ../libctr.h

all: $(ALL)

clean:
	$(RM) *.$(CPU) *.$(CPU).o ,*

libctrtest.$(CPU).o: $(SRCS)

libctrtest.$(CPU): libctrtest.$(CPU).o

install: libctrtest.$(CPU)
	dsc_install libctrtest.$(CPU) /acc/dsc/lab/$(CPU)/ctr
	dsc_install libctrtest.$(CPU) /acc/dsc/oper/$(CPU)/ctr
	dsc_install libctrtest.$(CPU) /acc/dsc/oplhc/$(CPU)/ctr
