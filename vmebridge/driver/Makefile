CPU ?= L865
# KVER=2.6.24.7-rt27

# KVER=2.6.24.7-rt21
# KVER=2.6.24.7-rt27
# KVER=2.6.29.4-rt15
# KVER=2.6.31.6-rt19
KVER32 ?= 3.2.33-rt50
KVER64 ?= 3.2.33-rt50.66.el6rt.x86_64

GIT_VERSION = $(shell git describe --dirty --long --tags --always)

ifeq ($(CPU), L866)
	KVER=$(KVER64)
else
	KVER=$(KVER32)
endif

KERNELSRC=/acc/sys/$(CPU)/usr/src/kernels/$(KVER)
OUTPUTDIR=$(PWD)/$(CPU)/$(KVER)

CFLAGS=-g -Wall

all: driver
driver: modules

modules:
	make -C $(KERNELSRC) M=`pwd` KVER=$(KVER) CPU=$(CPU) GIT_VERSION=$(GIT_VERSION) modules
	mkdir -p $(OUTPUTDIR)
	mv *.o *.ko $(OUTPUTDIR)
clean:
	rm -f *.o *.a *.so $(TESTS)
	rm -f *.mod.c
	rm -rf L866 L865 L864 ppc
	make -C $(KERNELSRC) M=`pwd` clean
