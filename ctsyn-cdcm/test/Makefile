#================================================================
# Makefile to produce standard CTX1 test applications
#================================================================

CPU=L865

include /acc/dsc/src/co/Make.auto

DRV = ../driver

ACCS=tst mcr

CFLAGS = -g -Wall -I. -I$(DRV) -I$(PS) \
	 -I/ps/local/$(CPU)/include -I./jtag -I../lib -lX11 -DPROTOTYPE
ALL  = syntest.$(CPU)
SRCS = syntest.c SynCmds.inc SynOpen.inc DoCmd.inc GetAtoms.inc Cmds.inc
HDRS = Cmds.h syndrvr.h syndrvrP.h

all: $(ALL)

clean:
	$(RM) $(ALL) $(BAKS)

# Run on Workstation only

syntest.$(CPU).o: $(SRCS) $(HDRS)

syntest.$(CPU): syntest.$(CPU).o

install: syntest.$(CPU)
	@for a in $(ACCS); do \
		d=/acc/src/dsc/$$a/$(CPU)/$(BSP)/ctg;\
		echo Installing syntest.$(CPU) in $$d;\
		dsc_install syntest.$(CPU) $$d;\
		echo Installing Syn.xsvf in $$d;\
		dsc_install Syn.xsvf $$d;\
	done;
