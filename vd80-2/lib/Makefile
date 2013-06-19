CPU ?= L866

include /acc/dsc/src/co/Make.auto

CFLAGS= -g -Wall -I ../driver -fPIC -DCOMPILE_TIME=$(shell date +%s)

GIT_VERSION = $(shell git describe --dirty --long --tags --always)
CFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"

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
	-$(RM) *.so *.a *.o
