CPU=L866

include /acc/dsc/src/co/Make.auto

CFLAGS= -g -Wall -I. -I../driver -I../skel -fPIC -DCOMPILE_TIME=$(shell date +%s)

SRCS = vd80lib.c Vd80Lib.h

all: vd80lib.$(CPU).a vd80lib.$(CPU).so

vd80lib.$(CPU).o: $(SRCS)

vd80lib.$(CPU).a: vd80lib.$(CPU).o
	-$(RM) $@
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

vd80lib.$(CPU).so: vd80lib.$(CPU).o
	-$(RM) $@
	$(CC) $(CFLAGS) -o $@ -shared $^

clean:
	-$(RM) vd80lib.$(CPU).so vd80lib.$(CPU).a vd80lib.$(CPU).o
	$(MAKE) -C test CPU=$(CPU) $@

install:
	dsc_install vd80lib.$(CPU).a /acc/local/$(CPU)/vd80
	dsc_install vd80lib.h /acc/local/$(CPU)/vd80
