#================================================================
# Makefile to produce acdxing library
#================================================================

CPU=L865

include /ps/dsc/src/co/Make.auto

CFLAGS= -g -Wall -I. -I/ps/local/$(CPU)/include -I/dsrc/drivers/coht/acdx/driver

SRCS=libacdx.c libacdx.h

INSTFILES=libacdx.$(CPU).a libacdx.h ../driver/acdxdrvr.h ../driver/acdxdrvrP.h libamp.$(CPU).a libamp.h

ACCS=

all:$(INSTFILES)

libacdx.$(CPU).o: libacdx.c

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
	cp libacdx.$(CPU).a /ps/local/$(CPU)/acdx/libacdx.a
	cp libamp.$(CPU).a /ps/local/$(CPU)/acdx/libamp.a
	rm -f /ps/local/$(CPU)/acdx/acdxdrvr.h
	rm -f /ps/local/$(CPU)/acdx/acdxdrvrP.h
	rm -f /ps/local/$(CPU)/acdx/libacdx.h
	rm -f /ps/local/$(CPU)/acdx/libamp.h
	rm -f /ps/local/$(CPU)/acdx/libampP.h
	cp ../driver/acdxdrvr.h /ps/local/$(CPU)/acdx
	cp ../driver/acdxdrvrP.h /ps/local/$(CPU)/acdx
	cp libamp.h /ps/local/$(CPU)/acdx
	cp libampP.h /ps/local/$(CPU)/acdx
	cp libacdx.h /ps/local/$(CPU)/acdx
