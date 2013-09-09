CPU ?= L865

ifeq ($(CPU),L866)
	KVER ?= 3.6.11-rt30.25.el6rt.x86_64
endif
ifeq ($(CPU),L865)
	KVER ?= 2.6.24.7-rt27
endif

include /acc/src/dsc/co/Make.auto

GIT_VERSION = $(shell git describe --dirty --tags --always --long)

KERNELSRC=/acc/sys/$(CPU)/usr/src/kernels/$(KVER)
OUTPUTDIR=$(shell pwd)/$(CPU)/$(KVER)

CFLAGS=-g -Wall

all: driver
driver: modules

modules: 
	cp Module.symvers.vme Module.symvers
	make -C $(KERNELSRC) M=`pwd` GIT_VERSION=$(GIT_VERSION) KVER=$(KVER) CPU=$(CPU) modules
	mkdir -p $(OUTPUTDIR)
	mv *.o *.ko $(OUTPUTDIR)
clean:
	rm -f *.o *.a *.pyc *.so $(TESTS)
	rm -f *.mod.c
	rm -rf L86[4-6] ppc
	make -C $(KERNELSRC) M=`pwd` clean


