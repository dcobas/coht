#================================================================
# Makefile to produce standard CTR-P test program
#================================================================

CPU=Linux

include /acc/dsc/src/co/Make.auto

DDIR = acdx

ACCS=oplhc

CFLAGS= -g -Wall -I.

LDLIBS =

ALL  = amptest.$(CPU) amptest.$(CPU).o

SRCS = amptest.c AmpCmds.c AmpOpen.c DoCmd.c GetAtoms.c Cmds.c

HDRS = Cmds.h

TEST = amptest.c AmpCmds.c AmpOpen.c DoCmd.c GetAtoms.c Cmds.c

LHC = /acc/dsc/oplhc/L865/2.6.24.7-rt21/acdx/test/
TST = /acc/dsc/lab/L865/2.6.24.7-rt21/acdx/test/
LOCAL = /acc/dsc/src/drivers/coht/acdx/acdx/test/

all: $(ALL)

clean:
	$(RM) $(ALL) $(BAKS)

# Run on Workstation only

amptest.$(CPU).o: $(TEST) $(HDRS)

install: amptest.$(CPU)
	 dsc_install amptest.$(CPU) $(LHC)
	 dsc_install amptest.config $(LHC)
	 dsc_install amp_news       $(LHC)
	 dsc_install RefAmpDspBin0  $(LHC)
	 dsc_install RefAmpDspBin1  $(LHC)
	 dsc_install RefAmpDspBin2  $(LHC)
	 dsc_install RefAmpStatus   $(LHC)
	 dsc_install amptest.$(CPU) $(TST)
	 dsc_install amptest.config $(TST)
	 dsc_install amp_news       $(TST)
	 dsc_install RefAmpDspBin0  $(TST)
	 dsc_install RefAmpDspBin1  $(TST)
	 dsc_install RefAmpDspBin2  $(TST)
	 dsc_install RefAmpStatus   $(TST)
	 dsc_install amptest.$(CPU) $(LOCAL)
	 dsc_install amptest.config $(LOCAL)
	 dsc_install amp_news       $(LOCAL)
	 dsc_install RefAmpDspBin0  $(LOCAL)
	 dsc_install RefAmpDspBin1  $(LOCAL)
	 dsc_install RefAmpDspBin2  $(LOCAL)
	 dsc_install RefAmpStatus   $(LOCAL)
