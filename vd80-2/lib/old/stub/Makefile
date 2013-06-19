CPU=L866

include /acc/dsc/src/co/Make.auto

CFLAGS= -g -Wall -I. -fPIC -DCOMPILE_TIME=$(shell date +%s) -DLDVER=2.0 -DSOVER=2.0 -DSUBVER=1

SRCS = libvd80.c libvd80.h libvd80P.h

all: libvd80.$(CPU).a

libvd80.$(CPU).o: $(SRCS)

libvd80.$(CPU).a: libvd80.$(CPU).o
	-$(RM) $@
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

clean:
	-$(RM) libvd80.$(CPU).so libvd80.$(CPU).a libvd80.$(CPU).o
