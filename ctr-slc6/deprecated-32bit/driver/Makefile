CPU=L865

# KVER=2.6.24.7-rt21
# KVER=2.6.24.7-rt27
# KVER=2.6.29.4-rt15
# KVER=2.6.31.6-rt19
# KVER=2.6.24.7-rt27
# KVER=2.6.33.9-rt31.66.el6rt.x86_64

ifeq ($(CPU),L865)
KVER=2.6.24.7-rt27
endif

ifeq ($(CPU),L866)
KVER=2.6.33.9-rt31.66.el6rt.x86_64
endif

KERNELSRC=/acc/sys/$(CPU)/usr/src/kernels/$(KVER)
OUTPUTDIR=$(PWD)/$(CPU)/$(KVER)

CFLAGS=-g -Wall

all: driver
driver: modules

modules:
	cp Module.symvers.vme Module.symvers
	make -C $(KERNELSRC) M=`pwd` KVER=$(KVER) CPU=$(CPU) modules
	mkdir -p $(OUTPUTDIR)
	mv *.o *.ko $(OUTPUTDIR)
clean:
	rm -f *.o *.a *.pyc *.so $(TESTS)
	rm -f *.mod.c
	rm -rf $(CPU) L864 ppc
	make -C $(KERNELSRC) M=`pwd` clean

install:
	@echo "please specify one of {install-lab|install-oper|install-lhc|install-all}"

install-all: install-lab install-oper install-oplhc

install-lab: ./$(CPU)/$(KVER)/ctrp.ko
	dsc_install ./$(CPU)/$(KVER)/ctrp.ko /acc/dsc/lab/$(CPU)/$(KVER)/ctrp;

install-oper: ./$(CPU)/$(KVER)/ctrp.ko
	dsc_install ./$(CPU)/$(KVER)/ctrp.ko /acc/dsc/oper/$(CPU)/$(KVER)/ctrp;

install-oplhc: ./$(CPU)/$(KVER)/ctrp.ko
	dsc_install ./$(CPU)/$(KVER)/ctrp.ko /acc/dsc/oplhc/$(CPU)/$(KVER)/ctrp;
