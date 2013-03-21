#================================================================
# Makefile to produce TimLib test program
#================================================================

include /ps/dsc/src/co/Make.auto

CFLAGS = -g -Wall -I. -I.. -I/dsrc/co/lib/tim -I/ps/local/$(CPU)/include -I/dsrc/drivers/ctr/src/driver

LDLIBS = ../../libtim.$(CPU).a -ltgm -lerr -lerr_dummy -ltgv -lm -lX11 -ldtm -lerr -lnc

ALL  = SendSocket.$(CPU) RecvSocket.$(CPU) TimServer.$(CPU) TimClient.$(CPU)

SRCS = SendSocket.c RecvSocket.c

HDRS = SendSocket.h

all: $(ALL)

clean:
	$(RM) *.$(CPU) *.$(CPU).o ,*

# Run on Workstation only

SendSocket.$(CPU).o: SendSocket.c

RecvSocket.$(CPU).o: RecvSocket.c

TimServer.$(CPU).o: TimServer.c

TimClient.$(CPU).o: TimClient.c

install: SendSocket.$(CPU) RecvSocket.$(CPU)
	@for a in $(ACCS); do \
		d=/ps/dsc/$$a/$(CPU)/tim;\
		echo Installing SendSocket in $$d;\
		dsc_install SendSocket.$(CPU) $$d;\
		echo Installing RecvSocket in $$d;\
		dsc_install RecvSocket.$(CPU) $$d;\
		echo Installing TimServer in  $$d;\
		dsc_install TimServer.$(CPU)  $$d;\
		echo Installing TimClient in  $$d;\
		dsc_install TimClient.$(CPU)  $$d;\
	done;
