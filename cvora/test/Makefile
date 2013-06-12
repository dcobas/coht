#================================================================
# Makefile to produce vmeio support library test program
#================================================================

CPU=L865
KVER=2.6.24.7-rt27
include /acc/dsc/src/co/Make.auto

COMPILE_TIME:=$(shell date +%s)

CFLAGS = -g -Wall -I.. -DCOMPILE_TIME=$(COMPILE_TIME) -DDRIVER_NAME=\"cvora\"

LDLIBS= ../vmeio_support.L865.a

ALL  = vmeiotest.$(CPU).o vmeiotest.$(CPU)

SRCS = vmeiotest.c VmeioCmds.c DoCmd.c GetAtoms.c Cmds.c

HDRS = Cmds.h

all: $(ALL)

# Run on Workstation only

vmeiotest.$(CPU).o: $(SRCS) $(HDRS)

vmeiotest.$(CPU): vmeiotest.$(CPU).o

clean:
	rm -f *.o *.$(CPU)
