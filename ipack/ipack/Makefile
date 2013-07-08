# Assign sensible defaults, but allow overriding with environment variables.
#

CPU	?= L865
KVER	?= 3.2.43-rt63
LINUX	?= /acc/sys/$(CPU)/usr/src/kernels/$(KVER)

GIT_VERSION = $(shell cd $(src); git describe --dirty --long --tags --always)

ccflags-y += -DDEBUG
ccflags-y += -DGIT_VERSION=\"$(GIT_VERSION)\"

obj-m := ipack.o

all: modules

.PHONY: all modules clean help install modules_install

modules help install modules_install:
	$(MAKE) -C $(LINUX) M=$(shell pwd) CPU=$(CPU) \
		KVER=$(KVER) GIT_VERSION=$(GIT_VERSION) \
		$@

# be able to run the "clean" rule even if $(LINUX) is not valid
clean:
	rm -rf *.o *~  .*.cmd *.ko *.mod.c .tmp_versions obj-$(CONFIG_BOARD_TPCI200) += tpci200.o \
                         Module.markers modules.order
all:
	make -C $(LINUX) M=$(PWD) modules
