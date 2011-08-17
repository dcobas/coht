#================================================================
# Makefile to produce timing library
#================================================================

include /acc/dsc/src/co/Make.auto

CFLAGS= -g -Wall -I. -I/acc/local/$(CPU)/include -I/dsrc/drivers/ctr/src/driver
ifeq ($(CPU),Lces)
CFLAGS += -I/dsrc/drivers/Lces/tg8V5/firmware
endif

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
	cp libtim.$(CPU).a /acc/local/$(CPU)/tim/libtim.a
	rm -f /acc/local/$(CPU)/tim/TimLib.h
	cp TimLib.h /acc/local/$(CPU)/tim
