#================================================================
# Makefile to produce standard CTR-P test program
#================================================================

CPU=L865

include /acc/dsc/src/co/Make.auto

CFLAGS= -g -Wall -I/acc/local/$(CPU)/include/tgm -I../driver -I. \
	-DCTR_VME -DPS_VER -DEMULATE_LYNXOS_ON_LINUX

LDLIBS= -ltgm -lvmtg -ltgv -lerr -lerr_dummy -lX11 -lm

ALL  = Launch.$(CPU) \
       CtrReadInfo.$(CPU) \
       CtrWriteInfo.$(CPU) \
       CtrLookat.$(CPU) \
       CtrClock.$(CPU) \
       ctrtest.config

ALL_ppc4 = Launch.ppc4 \
       CtrReadInfo.ppc4 \
       CtrWriteInfo.ppc4 \
       CtrLookat.ppc4 \
       CtrClock.ppc4 \
       ctrptest.ppc4 \
       ctrvtest.ppc4

SRCS = CtrOpen.c DoCmd.c GetAtoms.c Cmds.c \
       plx9030.c CtrReadInfo.c CtrWriteInfo.c CtrClock.c CtrLookat.c
HDRS = Cmds.h

LOOK = CtrLookat.c DisplayLine.c CtrOpen.c
CLOK = CtrClock.c CtrOpen.c
LNCH = Launch.c

all: $(ALL)

clean:
	$(RM) $(ALL) $(BAKS)

# Run on Workstation only

Launch.$(CPU).o: $(LNCH)

CtrClock.$(CPU).o: $(CLOK) $(HDRS)

CtrLookat.$(CPU).o: $(LOOK) $(HDRS)

ctrtest.$(CPU): ctrtest.$(CPU).o

CtrReadInfo.$(CPU): CtrReadInfo.$(CPU).o

CtrWriteInfo.$(CPU): CtrWriteInfo.$(CPU).o

CtrClock.$(CPU): CtrClock.$(CPU).o

CtrLookat.$(CPU): CtrLookat.$(CPU).o

CtrReadInfo.$(CPU):
	make -f GNUmakefile.vme

CtrWriteInfo.$(CPU):
	make -f GNUmakefile.vme

ifeq ($(CPU),ppc4)
ctrtest.config: ctrtest.config.lynxos
	cp ctrtest.config.lynxos ctrtest.config
else
ctrtest.config: ctrtest.config.linux
	cp ctrtest.config.linux ctrtest.config
endif

Ctrv.xsvf:

Vhdl.versions:

install: $(ALL)
	dsc_install $(ALL) /acc/dsc/lab/$(CPU)/ctr
	dsc_install $(ALL) /acc/dsc/oper/$(CPU)/ctr
	dsc_install $(ALL) /acc/dsc/oplhc/$(CPU)/ctr

install_ppc4: $(ALL_ppc4)
	dsc_install $(ALL_ppc4) /acc/dsc/lab/ppc4/ces/ctr
	dsc_install $(ALL_ppc4) /acc/dsc/oper/ppc4/ces/ctr
	dsc_install $(ALL_ppc4) /acc/dsc/oplhc/ppc4/ces/ctr

