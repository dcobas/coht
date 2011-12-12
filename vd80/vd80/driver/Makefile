CPU=L865

ifeq ($(CPU),L865)
# KVER=2.6.24.7-rt21
# KVER=2.6.24.7-rt27
# KVER=2.6.29.4-rt15
# KVER=2.6.31.6-rt19
KVER=2.6.24.7-rt27
endif

ifeq ($(CPU),L866)
# KVER=2.6.32-131.17.1.el6.x86_64
KVER=2.6.33.9-rt31.75.el6rt.x86_64
endif

KERNELSRC=/acc/sys/$(CPU)/usr/src/kernels/$(KVER)

CFLAGS=-g -Wall

modules: 
	cp Module.symvers.vmebus Module.symvers
	make -C $(KERNELSRC) M=`pwd` KVER=$(KVER) CPU=$(CPU) modules
	rm -rf ./$(CPU)/$(KVER)
	mkdir ./$(CPU)/$(KVER)
	mv skelmodule.ko ./$(CPU)/$(KVER)/skelmodule.ko
	chmod 664 ./$(CPU)/$(KVER)/skelmodule.ko
	rm -f *.o

clean:
	rm -f skelmodule.ko
	rm -f ./$(CPU)/$(KVER)/skelmodule.ko
	rm -f *.o

install: ./$(CPU)/$(KVER)/skelmodule.ko
	@echo "please specify one of {install-lab|install-oper|install-lhc|install-all}"

install-all: install-lab install-oper install-oplhc

install-lab: ./$(CPU)/$(KVER)/skelmodule.ko
	dsc_install ./$(CPU)/$(KVER)/skelmodule.ko /acc/dsc/lab/$(CPU)/$(KVER)/skelmodule;

install-oper: ./$(CPU)/$(KVER)/skelmodule.ko
	dsc_install ./$(CPU)/$(KVER)/skelmodule.ko /acc/dsc/oper/$(CPU)/$(KVER)/skelmodule;

install-oplhc: ./$(CPU)/$(KVER)/skelmodule.ko
	dsc_install ./$(CPU)/$(KVER)/skelmodule.ko /acc/dsc/oplhc/$(CPU)/$(KVER)/skelmodule;
