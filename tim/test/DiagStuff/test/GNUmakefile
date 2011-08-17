#================================================================
# Makefile to produce TimLib test program
#================================================================

CPU=L864

include /ps/dsc/src/co/Make.auto

ACCS=lab oper

CFLAGS := -g -Wall -I. -I.. -I/ps/local/$(CPU)/include
LDLIBS := /dsrc/co/lib/tim/libtim.$(CPU).a -ldscrt

ALL  = tsvytest.$(CPU)

SRCS = tsvytest.c TsvyCmds.c DoCmd.c GetAtoms.c Cmds.c

HDRS = Cmds.h

all: $(ALL)

clean:
	$(RM) *.$(CPU) *.$(CPU).o ,*

# Run on Workstation only

tsvytest.$(CPU).o: $(SRCS) $(HDRS)

tsvytest.$(CPU): tsvytest.$(CPU).o

install: tsvytest.$(CPU) tsvytest.config
	rm -f /mcr/tim/tsvytest.config
	cp tsvytest.config /mcr/tim
	rm -f /mcr/tim/tsvytest
	cp tsvytest.$(CPU) /mcr/tim/tsvytest
	rm -f /mcr/tim/tsvytest_cmd.sh
	cp tsvytest_cmd.sh /mcr/tim/tsvytest_cmd.sh
