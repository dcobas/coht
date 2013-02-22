#================================================================
# Makefile to produce standard CTR-P test program
#================================================================

CPU=L865

include /acc/dsc/src/co/Make.auto

CFLAGS= -g -Wall -I../driver -I. -DEMULATE_LYNXOS_ON_LINUX
LDLIBS= -lm

ALL  =	ctrptest.$(CPU) \
	ctrvtest.$(CPU) \
	CtrReadInfo.$(CPU) \
	CtrWriteInfo.$(CPU) \
	CtrLookat.$(CPU) \
	CtrClock.$(CPU) \
	Launch.$(CPU)

NOPSALL = ctrptest.$(CPU) \
	ctrvtest.$(CPU)

HDRS = Cmds.h

TEST = CtrCmds.c CtrOpen.c DoCmd.c GetAtoms.c Cmds.c plx9030.c
READ = CtrReadInfo.c CtrOpen.c
WRIT = CtrWriteInfo.c CtrOpen.c
LOOK = CtrLookat.c DisplayLine.c CtrOpen.c
CLOK = CtrClock.c CtrOpen.c
LNCH = Launch.c

all: $(ALL)
all: CFLAGS += -DPS_VER -I/acc/local/$(CPU)/include/tgm 
all: LDLIBS += -ltgm -ltgv -lerr -lerr_dummy -lX11
nops: $(NOPSALL)

# Run on Workstation only

ctrptest.$(CPU).o: ctrptest.c $(TEST) $(HDRS)
ctrvtest.$(CPU).o: ctrvtest.c $(TEST) $(HDRS)
ctrtest.$(CPU): ctrtest.$(CPU).o
CtrReadInfo.$(CPU).o: $(READ) $(HDRS)
CtrWriteInfo.$(CPU).o: $(WRIT) $(HDRS)
Launch.$(CPU).o: $(LNCH)
CtrClock.$(CPU).o: $(CLOK) $(HDRS)
CtrLookat.$(CPU).o: $(LOOK) $(HDRS)
CtrClock.$(CPU): CtrClock.$(CPU).o
CtrLookat.$(CPU): CtrLookat.$(CPU).o

clean:
	$(RM) $(ALL) $(BAKS) *.o

install: ctrptest.$(CPU) ctrvtest.$(CPU)
	dsc_install $(ALL) /acc/dsc/lab/$(CPU)/ctr
	dsc_install $(ALL) /acc/dsc/oper/$(CPU)/ctr
	dsc_install $(ALL) /acc/dsc/oplhc/$(CPU)/ctr
