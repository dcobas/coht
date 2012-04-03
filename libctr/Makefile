#================================================================
# Makefile to produce timing library
#================================================================

CPU=L865

include /acc/dsc/src/co/Make.auto

CFLAGS= -g -Wall -I. -I./ctr/driver

SRCS=libctr.c libctr.h

INSTFILES=libctr.$(CPU).a libctr.h

ACCS=tst

all:libctr.$(CPU).a

libctr.$(CPU).o: libctr.c

libctr.$(CPU).a: libctr.$(CPU).o
	-$(RM) $@
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

clean:
	rm -f libctr.$(CPU).o
	rm -f libctr.$(CPU).a

install: $(INSTFILES)
	dsc_install libctr.$(CPU).a /acc/local/$(CPU)/tim
	rm -f /acc/local/$(CPU)/ctr/libctr.h
	dsc_install libctr.h /acc/local/$(CPU)/ctr
