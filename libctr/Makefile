#================================================================
# Makefile to produce timing library
#================================================================

CPU=L865
KVER=2.6.24.7-rt27
SOVER=1.0

include /acc/dsc/src/co/Make.auto

CFLAGS= -g -Wall -I. -I./ctr/driver -fPIC

SRCS=libctr.c libctr.h libctrP.h libctrp.c libctrv.c

INSTFILES=libctr.$(CPU).a libctr.h libctrp.$(CPU).so libctrv.$(CPU).so

ACCS=tst

all:libctr.$(CPU).a libctrp.$(CPU).so libctrv.$(CPU).so

libctr.$(CPU).o: libctr.c

libctrp.$(CPU).o: libctrp.c libctr_common.c

libctrv.$(CPU).o: libctrv.c libctr_common.c

libctr.$(CPU).a: libctr.$(CPU).o
	-$(RM) $@
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

libctrp.$(CPU).so: libctrp.$(CPU).o
	-$(RM) $@
	$(CC) $(CFLAGS) -o $@ -shared $^

libctrv.$(CPU).so: libctrv.$(CPU).o
	-$(RM) $@
	$(CC) $(CFLAGS) -o $@ -shared $^

clean:
	rm -f libctr.$(CPU).o
	rm -f libctr.$(CPU).a
	rm -f libctrp.o
	rm -f libctrp.so
	rm -f libctrv.o
	rm -f libctrv.so

install: $(INSTFILES)
	dsc_install libctr.$(CPU).a /acc/local/$(CPU)/tim
	rm -f /acc/local/$(CPU)/ctr/libctr.h
	dsc_install libctr.h /acc/local/$(CPU)/ctr
	rm -f /acc/dsc/lab/$(CPU)/$(KVER)/ctrp/libctrp.so.$(SOVER)
	rm -f /acc/dsc/lab/$(CPU)/$(KVER)/ctrv/libctrv.so.$(SOVER)
	cp libctrp.$(CPU).so /acc/dsc/lab/$(CPU)/$(KVER)/ctrp/libctrp.so.$(SOVER)
	cp libctrv.$(CPU).so /acc/dsc/lab/$(CPU)/$(KVER)/ctrv/libctrv.so.$(SOVER)
