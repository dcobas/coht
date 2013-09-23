#================================================================
# Makefile to produce libctr test program
#================================================================

CPU=L866

include /acc/dsc/src/co/Make.auto

DRVR = ../../../driver

CFLAGS = -g -Wall -I. -I.. -I$(DRVR) -rdynamic
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
	rm -f /acc/dsc/lab/$(CPU)/bin/libctrtest
	rm -f /acc/dsc/oper/$(CPU)/bin/libctrtest
	rm -f /acc/dsc/oplhc/$(CPU)/bin/libctrtest
	cd /acc/dsc/lab/$(CPU)/bin; ln -s ../ctr/libctrtest libctrtest
	cd /acc/dsc/oper/$(CPU)/bin; ln -s ../ctr/libctrtest libctrtest
	cd /acc/dsc/oplhc/$(CPU)/bin; ln -s ../ctr/libctrtest libctrtest
