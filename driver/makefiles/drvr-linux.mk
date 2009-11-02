CPU            := L865
IS_CDCM        := y
CONFIG_IS_DG   := y
CONFIG_BUS_VME := y
CONFIG_BUS_PCI := n
M              := $(PWD)

include ../Makefile.specific
include ../Makefile.base
include ./makefiles/drvr-paramount.mk

KSRC := /acc/sys/$(CPU)/usr/src/kernels/$(KVER)

export IS_CDCM CONFIG_BUS_VME CONFIG_BUS_PCI CONFIG_IS_DG DRIVER_NAME

all: $(OBJDIR) $(FINAL_DEST)
# 1. compile the driver
	$(Q)$(MAKE) -C $(KSRC) M=$(PWD) CPU=$(CPU) KVER=$(KVER) BLD=drvr modules
	$(DO_MOVE)
	$(Q)$(MAKE) -C $(KSRC) M=$(PWD) CPU=$(CPU) KVER=$(KVER) BLD=sim modules
	$(DO_MOVE)

# 2. Link kernel object
	$(Q)ln -sf ../driver/$(OBJDIR)/$(DRIVER_NAME)D.ko ../$(FINAL_DEST)/$(DRIVER_NAME)D.$(KVER).ko
	$(Q)ln -sf ../driver/$(OBJDIR)/$(DRIVER_NAME)S.ko ../$(FINAL_DEST)/$(DRIVER_NAME)S.$(KVER).ko

# Guess what
clean clear:
	$(MAKE) -C $(KSRC) M=$(PWD) ROOTDIR=$(ROOTDIR) clean
	$(RM) $(DRIVER) $(SIMULATOR)
	$(RM) -rf $(OBJDIR)

# Move all the compiled stuff in the object directory. Suppress any errors.
define DO_MOVE
	$(Q)@-mv -f .tmp_versions/* *.o .*.o.cmd .*.ko.cmd *.mod.c *.symvers \
	user/*.o user/.*.o.cmd \
	Module.markers modules.order *.ko $(OBJDIR) 2>/dev/null; \
	rm -rf .tmp_versions/; exit 0
endef