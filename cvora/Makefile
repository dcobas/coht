# CPU=L865
# KVER=2.6.24.7-rt21
# KVER=2.6.24.7-rt27
# KVER=2.6.29.4-rt15
# KVER=2.6.31.6-rt19
# KVER=2.6.24.7-rt27

CPU ?= L865
KVER ?= 3.2.43-rt63
KERNELSRC=/acc/sys/$(CPU)/usr/src/kernels/$(KVER)

GIT_VERSION = $(shell git describe --dirty --long --tags --always)

include /acc/dsc/src/co/Make.auto

all: modules libs test

modules: 
	cp Module.symvers.vmebus Module.symvers
	make -C $(KERNELSRC) M=`pwd` KVER=$(KVER) GIT_VERSION=$(GIT_VERSION) modules
	mkdir -p $(CPU)/$(KVER)
	cp cvora.ko $(CPU)/$(KVER)
clean:
	rm -f *.so
	make -C $(KERNELSRC) M=`pwd` KVER=$(KVER) clean
	make -C test clean
	make -C doc clean
docs:
	make -C doc

CFLAGS= -g -Wall -fPIC
CFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"

libs: libcvora.$(CPU).a libcvora.$(CPU).so

libcvora.$(CPU).o: libcvora.c libcvora.h
libcvora.$(CPU).so: libcvora.$(CPU).o
	$(CC) $(CFLAGS) -shared -o $@ $^
libcvora.$(CPU).a: libcvora.$(CPU).o
	-$(RM) $@
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

DRVRPATH = /acc/dsc/$(ACC)/$(CPU)/$(KVER)/cvora
LIBPATH = /acc/local/$(CPU)/drv/cvora

install-lab:
	$(MAKE) install-driver ACC=lab
install-oper:
	$(MAKE) install-driver ACC=oper
install-all: install-lab install-oper install-oplhc install-opctf

install-driver:
	dsc_install $(CPU)/$(KVER)/cvora.ko $(DRVRPATH)

install-lib:
	dsc_install libcvora.h libcvora.$(CPU).a $(LIBPATH)
install-test:
	dsc_install test/cvoratest.py $(LIBPATH)
	dsc_install libcvora.$(CPU).so $(LIBPATH)/cvora
