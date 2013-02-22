#================================================================
# Makefile to produce timing library
#================================================================

CPU=L866

include /acc/dsc/src/co/Make.auto

DRVR = ../../driver

CFLAGS= -g -Wall -I. -I$(DRVR) -I/acc/local/$(CPU)/include

SRCS=TimLib.c TimLib.h ctr/CtrLib.c tg8cps/Tg8CpsLib.c tg8sps/Tg8SpsLib.c netwrk/NetWrkLib.c

INSTFILES=libtim.$(CPU).a TimLib.h

ACCS=tst

all:$(INSTFILES)

libtim.c: $(SRCS)
	rm -f libtim.c
	rm -f libtim.$(CPU).o
	rm -f libtim.$(CPU).a
	cp TimLib.c libtim.c

libtim.$(CPU).o: libtim.c

libtim.$(CPU).a: libtim.$(CPU).o
	-$(RM) $@
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

clean:
	rm -f libtim.c
	rm -f libtim.$(CPU).o
	rm -f libtim.$(CPU).a

install: $(INSTFILES)
	dsc_install libtim.$(CPU).a /acc/local/$(CPU)/tim
	rm -f /acc/local/$(CPU)/tim/TimLib.h
	dsc_install TimLib.h /acc/local/$(CPU)/tim
