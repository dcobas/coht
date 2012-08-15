#================================================================
# Makefile to produce standard CTR-P test program
#================================================================

CPU=Linux

include /acc/dsc/src/co/Make.auto

DDIR = amp

ACCS=oplhc

CFLAGS= -g -Wall -I.

LDLIBS =

ALL  = amptest.$(CPU) amptest.$(CPU).o

SRCS = amptest.c AmpCmds.c AmpOpen.c DoCmd.c GetAtoms.c Cmds.c

HDRS = Cmds.h

TEST = amptest.c AmpCmds.c AmpOpen.c DoCmd.c GetAtoms.c Cmds.c

all: $(ALL)

clean:
	$(RM) $(ALL) $(BAKS)

# Run on Workstation only

amptest.$(CPU).o: $(TEST) $(HDRS)

install: amptest.$(CPU)
	@for f in $(ACCS); do \
	    dsc_install amptest.$(CPU) /acc/dsc/$$f/$(CPU)/$(BSP)/$(DDIR); \
	    rm -f /acc/dsc/$$f/$(CPU)/$(BSP)/$(DDIR)/Amp.segs; \
	    rm -f /acc/dsc/$$f/data/$(DDIR)/Amp.segs; \
	    cp Amp.segs /acc/dsc/$$f/$(CPU)/$(BSP)/$(DDIR); \
	    chmod 444 /acc/dsc/$$f/$(CPU)/$(BSP)/$(DDIR)/Amp.segs; \
	    cp Amp.segs /acc/dsc/$$f/data/$(DDIR); \
	    chmod 664 /acc/dsc/$$f/data/$(DDIR)/Amp.segs; \
	done;

