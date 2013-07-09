CPU=L866

ifeq ($(CPU),L865)
KVER=3.2.43-rt63
endif

ifeq ($(CPU),L866)
KVER=3.6.11-rt30.25.el6rt.x86_64
endif

GIT_VERSION = $(shell git describe --dirty --tags --always --long)
KERNELSRC=/acc/sys/$(CPU)/usr/src/kernels/$(KVER)

CFLAGS=-g -Wall

all: modules
modules: 
	cp Module.symvers.vmebus Module.symvers
	make -C $(KERNELSRC) M=`pwd` GIT_VERSION=$(GIT_VERSION) modules
	rm -rf ./$(CPU)
	mkdir ./$(CPU)
	mkdir ./$(CPU)/$(KVER)
	mv icv196.ko ./$(CPU)/$(KVER)/icv196.ko
	chmod 664 ./$(CPU)/$(KVER)/icv196.ko
	rm -f *.o

clean:
	rm -f icv196.ko
	rm -f  Module.symvers modules.order *.mod.c
	rm -f ./$(CPU)/$(KVER)/icv196.ko
	rm -f *.o
	rm -rf L86[56]

install: ./$(CPU)/$(KVER)/icv196.ko
	@echo "please specify one of {install-lab|install-oper|install-lhc|install-all}"

install-all: install-lab install-oper install-oplhc

install-lab: ./$(CPU)/$(KVER)/icv196.ko
	dsc_install ./$(CPU)/$(KVER)/icv196.ko /acc/dsc/lab/$(CPU)/$(KVER)/icv196;

install-oper: ./$(CPU)/$(KVER)/icv196.ko
	dsc_install ./$(CPU)/$(KVER)/icv196.ko /acc/dsc/oper/$(CPU)/$(KVER)/icv196;

install-oplhc: ./$(CPU)/$(KVER)/icv196.ko
	dsc_install ./$(CPU)/$(KVER)/icv196.ko /acc/dsc/oplhc/$(CPU)/$(KVER)/icv196;
