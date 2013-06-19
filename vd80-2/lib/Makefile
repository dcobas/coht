CPU=L866

include /acc/dsc/src/co/Make.auto

CFLAGS= -g -Wall -I. -I../../driver -fPIC -DCOMPILE_TIME=$(shell date +%s)

SRCS = libvd80.c libvd80.h

all: libvd80.$(CPU).so libvd80.$(CPU).a

libvd80.$(CPU).o: $(SRCS)

libvd80.$(CPU).so: libvd80.$(CPU).o
	-$(RM) $@
	$(CC) $(CFLAGS) -o $@ -shared $^

libvd80.$(CPU).a: libvd80.$(CPU).o
	-$(RM) $@
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

clean:
	-$(RM) libvd80.$(CPU).so libvd80.$(CPU).o
