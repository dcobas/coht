#================================================================
# Makefile to produce standard CTR-P test program
#================================================================

CPU=L865
include /acc/dsc/src/co/Make.auto

DDIR = acdx

PWD:=$(shell pwd)

COMPILE_TIME:=$(shell $(PWD)/gettime)

ACCS=oplhc

CFLAGS= -g -Wall -I. -I../driver -I../linux -DCOMPILE_TIME=$(COMPILE_TIME)

LDLIBS = -lm

ALL  = acdxtest.$(CPU) acdxtest.$(CPU).o

SRCS = acdxtest.c AcdxCmds.c AcdxOpen.c DoCmd.c GetAtoms.c Cmds.c

HDRS = Cmds.h

TEST = acdxtest.c AcdxCmds.c AcdxOpen.c DoCmd.c GetAtoms.c Cmds.c

all: $(ALL)

clean:
	$(RM) $(ALL) $(BAKS)

# Run on Workstation only

acdxtest.$(CPU).o: $(TEST) $(HDRS)

install: acdxtest.$(CPU)

