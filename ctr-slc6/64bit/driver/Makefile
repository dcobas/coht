CPU=L866

ifeq ($(CPU),L865)
KVER=2.6.24.7-rt27
endif

ifeq ($(CPU),L866)
KVER=3.2.33-rt50.66.el6rt.x86_64
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
	rm -f ,* *~ *.bak *.BAK .es1* .B* %*% .ec1 .ek1 .*~ core a.out *JNL *.lst \\\#*\\\# .nfs* *%
	rm -f a.out out 32 64

install:
	@echo "please specify one of {install-lab|install-oper|install-lhc|install-all}"

install-all: install-lab install-oper install-oplhc

install-lab: ./$(CPU)/$(KVER)/ctrp.ko ./$(CPU)/$(KVER)/ctrv.ko
	dsc_install ./$(CPU)/$(KVER)/ctrp.ko /acc/dsc/lab/$(CPU)/$(KVER)/ctr;
	dsc_install ./ipp /acc/dsc/lab/$(CPU)/$(KVER)/ctr;
	dsc_install ./$(CPU)/$(KVER)/ctrv.ko /acc/dsc/lab/$(CPU)/$(KVER)/ctr;
	dsc_install ./ivv /acc/dsc/lab/$(CPU)/$(KVER)/ctr;

install-oper: ./$(CPU)/$(KVER)/ctrp.ko ./$(CPU)/$(KVER)/ctrv.ko
	dsc_install ./$(CPU)/$(KVER)/ctrp.ko /acc/dsc/oper/$(CPU)/$(KVER)/ctr;
	dsc_install ./ipp /acc/dsc/oper/$(CPU)/$(KVER)/ctr;
	dsc_install ./$(CPU)/$(KVER)/ctrv.ko /acc/dsc/oper/$(CPU)/$(KVER)/ctr;
	dsc_install ./ivv /acc/dsc/oper/$(CPU)/$(KVER)/ctr;

install-oplhc: ./$(CPU)/$(KVER)/ctrp.ko ./$(CPU)/$(KVER)/ctrv.ko
	dsc_install ./$(CPU)/$(KVER)/ctrp.ko /acc/dsc/oplhc/$(CPU)/$(KVER)/ctr;
	dsc_install ./ipp /acc/dsc/oplhc/$(CPU)/$(KVER)/ctr;
	dsc_install ./$(CPU)/$(KVER)/ctrv.ko /acc/dsc/oplhc/$(CPU)/$(KVER)/ctr;
	dsc_install ./ivv /acc/dsc/oplhc/$(CPU)/$(KVER)/ctrv;
