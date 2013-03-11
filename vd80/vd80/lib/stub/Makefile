CPU=L866

include /acc/dsc/src/co/Make.auto

CFLAGS= -g -Wall -I. -I../driver -I../../skel -fPIC -DCOMPILE_TIME=$(shell date +%s)

SRCS = stublib.c StubLib.h

all: stublib.$(CPU).a

stublib.$(CPU).o: $(SRCS)

stublib.$(CPU).a: stublib.$(CPU).o
	-$(RM) $@
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

clean:
	-$(RM) stublib.$(CPU).so stublib.$(CPU).a stublib.$(CPU).o
