#================================================================
# Makefile to produce TimLib test program
#================================================================

CPU=L866

include /acc/dsc/src/co/Make.auto

CFLAGS = -g -Wall -I. -I../../.. -I../../../../../driver/ -I/acc/local/$(CPU)/include

LDLIBS = ../../../libtim.$(CPU).a -ltgm -lerr -lerr_dummy -ltgv -lm -lX11 -ldtm -lerr -lnc

ALL  = SendCNGS.$(CPU) RecvCNGS.$(CPU)

SRCS = SendCNGS.c RecvCNGS.c

HDRS = CNGS.h

all: $(ALL)

clean:
	$(RM) *.$(CPU) *.$(CPU).o ,*

# Run on Workstation only

SendCNGS.$(CPU).o: SendCNGS.c

RecvCNGS.$(CPU).o: RecvCNGS.c

install: SendCNGS.$(CPU) RecvCNGS.$(CPU)
	@for a in $(ACCS); do \
		d=/ps/dsc/$$a/$(CPU)/tim;\
		echo Installing SendCNGS in $$d;\
		dsc_install SendCNGS.$(CPU) $$d;\
		echo Installing RecvCNGS in $$d;\
		dsc_install RecvCNGS.$(CPU) $$d;\
		echo Installing SendCngs.bat $$d; \
		dsc_install SendCngs.bat $$d; \
	done;
