CPU=L866

ifeq ($(CPU),L865)
KVER=3.2.43-rt63
endif

ifeq ($(CPU),L866)
KVER=3.6.11-rt30.25.el6rt.x86_64
endif

TARGET = vd80-64

KERNELSRC=/acc/sys/$(CPU)/usr/src/kernels/$(KVER)
GIT_VERSION = $(shell cd $(src); git describe --dirty --long --tags --always)

ccflags-y += -g -Wall
ccflags-y += -I /acc/local/$(CPU)/include
ccflags-y += -DGIT_VERSION=\"$(GIT_VERSION)\"

obj-m = $(TARGET).o
$(TARGET)-objs += vd80.o


all: modules

modules: 
	cp Module.symvers.vmebus Module.symvers
	make -C $(KERNELSRC) M=`pwd` CPU=$(CPU) modules
	rm -rf ./$(CPU)
	mkdir ./$(CPU)
	mkdir ./$(CPU)/$(KVER)
	mv $(TARGET).ko ./$(CPU)/$(KVER)/$(TARGET).ko
	chmod 664 ./$(CPU)/$(KVER)/$(TARGET).ko
	rm -f *.o

clean:
	rm -rf *.ko L86[456] *.o *.mod.c modules.order

install: ./$(CPU)/$(KVER)/$(TARGET).ko
	@echo "please specify one of {install-lab|install-oper|install-lhc|install-all}"

install-all: install-lab install-oper install-oplhc

install-lab: ./$(CPU)/$(KVER)/$(TARGET).ko
	dsc_install ./$(CPU)/$(KVER)/$(TARGET).ko /acc/dsc/lab/$(CPU)/$(KVER)/vd80;

install-oper: ./$(CPU)/$(KVER)/vd80.ko
	dsc_install ./$(CPU)/$(KVER)/$(TARGET).ko /acc/dsc/oper/$(CPU)/$(KVER)/vd80;

install-oplhc: ./$(CPU)/$(KVER)/vd80.ko
	dsc_install ./$(CPU)/$(KVER)/$(TARGET).ko /acc/dsc/oplhc/$(CPU)/$(KVER)/vd80;
