#================================================================
# Makefile to produce acdxing library
#================================================================

CPU=L865

include /acc/dsc/src/co/Make.auto

CFLAGS= -g -Wall -I. -I/acc/local/$(CPU)/include -I/acc/src/dsc/drivers/coht/acdx/driver

SRCS=libacdx.c libacdx.h

INSTFILES=libacdx.$(CPU).a libacdx.h ../driver/acdxdrvr.h ../driver/acdxdrvrP.h libamp.$(CPU).a libamp.h

ACCS=

all:$(INSTFILES)

libacdx.$(CPU).o: libacdx.c
libamp.$(CPU).o: libamp.c

libacdx.$(CPU).a: libacdx.$(CPU).o
	-$(RM) $@
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

libamp.$(CPU).a: libamp.$(CPU).o
	-$(RM) $@
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

clean:
	rm -f libacdx.$(CPU).o
	rm -f libacdx.$(CPU).a

install: $(INSTFILES)
	cp libacdx.$(CPU).a /acc/local/$(CPU)/acdx/libacdx.a
	cp libamp.$(CPU).a /acc/local/$(CPU)/acdx/libamp.a
	rm -f /acc/local/$(CPU)/acdx/acdxdrvr.h
	rm -f /acc/local/$(CPU)/acdx/acdxdrvrP.h
	rm -f /acc/local/$(CPU)/acdx/libacdx.h
	rm -f /acc/local/$(CPU)/acdx/libamp.h
	rm -f /acc/local/$(CPU)/acdx/libampP.h
	cp ../driver/acdxdrvr.h /acc/local/$(CPU)/acdx
	cp ../driver/acdxdrvrP.h /acc/local/$(CPU)/acdx
	cp libamp.h /acc/local/$(CPU)/acdx
	cp libampP.h /acc/local/$(CPU)/acdx
	cp libacdx.h /acc/local/$(CPU)/acdx
