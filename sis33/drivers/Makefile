CPU	:= L865
KVER	:= 2.6.24.7-rt27
KERN_DIR:= "/acc/sys/$(CPU)/usr/src/kernels/$(KVER)"
VME_DIR := "../../vmebridge/driver"
VME_OBJDIR := "../../vmebridge/driver/$(CPU)/$(KVER)"

GIT_VERSION = $(shell git describe --dirty --tags --always --long)

.PHONY: all clean help

all: modules

modules:
	cp Module.symvers.vme Module.symvers
	$(MAKE) -C $(KERN_DIR) M=`pwd` GIT_VERSION=$(GIT_VERSION) modules

clean:
	$(MAKE) -C $(KERN_DIR) M=`pwd` clean

help:
	$(MAKE) -C $(KERN_DIR) M=`pwd` help
