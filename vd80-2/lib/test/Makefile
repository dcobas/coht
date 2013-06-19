#================================================================
# Makefile to produce standard CTX1 test applications
#================================================================

CPU=L866

include /acc/dsc/src/co/Make.auto

COMPILE_TIME:=$(shell date +%s)

VD80LB = ..
DRIVER = $(VD80LB)/../driver


CFLAGS = -g -Wall -I $(VD80LB) -I $(DRIVER) -DCOMPILE_TIME=$(COMPILE_TIME)

LDLIBS = $(VD80LB)/libvd80.$(CPU).a

ALL  = vd80test.$(CPU)
SRCS = vd80test.c vd80Cmds.c DoCmd.c GetAtoms.c Cmds.c
HDRS = Cmds.h $(VD80LB)/libvd80.h $(DRIVER)/vd80.h

all: $(ALL)

clean:
	$(RM) $(ALL) $(BAKS)

# Run on Workstation only

vd80test.$(CPU).o: $(SRCS) $(HDRS)

vd80test.$(CPU): vd80test.$(CPU).o
