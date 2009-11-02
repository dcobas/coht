# Standart Lynx settings
CPU  := ppc4
M    := $(PWD)

include ../Makefile.specific

# Standart CERN Makefile rules
include /acc/src/dsc/co/Make.auto

# Standart driver rules
include ./makefiles/drvr-paramount.mk

# That's our default target when none is given on the command line
all: $(OBJDIR) $(FINAL_DEST) $(DRIVER) $(SIMULATOR)

# Extremely simplified Quiet/Verbose
ifdef V
  ifeq ("$(origin V)", "command line")
    VERBOSE = $(V)
  endif
endif
ifndef VERBOSE
  VERBOSE = 0
endif

ifeq ($(VERBOSE),1)
$(DRIVER) $(SIMULATOR): CC = $(KERN_CC)
  QLD =
  QCC =
  QNM =
  Q   =
else
$(DRIVER) $(SIMULATOR): CC = $(QCC); $(KERN_CC)
  QLD = @echo 'LD [M]  $@'
  QCC = @echo 'CC [M]  $@'
  QNM = @echo 'KSYM    $@'
  Q   = @
endif

# Redefine
%.$(CPU).o: %.c
	$(QCC)
	$(Q)$(COMPILE.c) $< $(OUTPUT_OPTION)

# extra functions
DRVROBJS += $(ROOTDIR)/cdcm/cdcmBoth.o
SIMOBJS  += $(ROOTDIR)/cdcm/cdcmBoth.o

# Make.auto expects *.$(CPU).o extention
DRVROBJS := $(DRVROBJS:.o=.$(CPU).o)
SIMOBJS  := $(SIMOBJS:.o=.$(CPU).o)

ADDCFLAGS   = $(STDFLAGS) $(KFLAGS) -DNO_CSP_MODEL -DCPU=\"$(CPU)\"
ADDINCLUDES = $(KERN_INCLUDES)
CFLAGS     += $(ccflags-y) $(ADDINCLUDES) $(ADDCFLAGS)

$(DRIVER) $(SIMULATOR): OUTPUT_OPTION +=$(KFLAGS); mv $*.$(CPU).o $(OBJDIR)

# 0x0.
# Next two rules produce temporary driver/simulator archive files that contain
# unresolved symbols. They in turn will be used as an input files for 'nm'
# command call. And after 'nm' command call, an import files (that holds all
# unresolved symbols) will be build.
# Driver
$(DRIVER_NAME).drvr.$(CPU).tmp: $(DRVROBJS)
	$(QLD)
	$(Q)$(KERN_LD) -o $(OBJDIR)/$@ -berok -r $(addprefix $(OBJDIR)/, $(notdir $^))
# Simulator
$(DRIVER_NAME).sim.$(CPU).tmp: $(SIMOBJS)
	$(QLD)
	$(Q)$(KERN_LD) -o $(OBJDIR)/$@ -berok -r $(addprefix $(OBJDIR)/, $(notdir $^))

# 0x1.
# This rule will put only undefined symbols (those external to an object
# file) to the '*.import' file.
%.import: %.tmp
	$(QNM)
	$(Q)$(NM) -u $(OBJDIR)/$< > $(OBJDIR)/$@

# 0x2.
# Next two rules will create a driver shared object that imports an undefined
# symbols.
# Driver
$(DRIVER): $(DRIVER_NAME).drvr.$(CPU).import $(DRVROBJS)
	$(QLD)
	$(Q)$(KERN_LD) -o $@ -bM:SRE -bimport:$(addprefix $(OBJDIR)/, $(notdir $^))
	$(Q)ln -sf ../driver/$(OBJDIR)/$(DRIVER_NAME)D.ko ../$(FINAL_DEST)/$(DRIVER_NAME)D.$(KVER).ko
# Simulator
$(SIMULATOR): $(DRIVER_NAME).sim.$(CPU).import $(SIMOBJS)
	$(QLD)
	$(Q)$(KERN_LD) -o $@ -bM:SRE -bimport:$(addprefix $(OBJDIR)/, $(notdir $^))
	$(Q)ln -sf ../driver/$(OBJDIR)/$(DRIVER_NAME)S.ko ../$(FINAL_DEST)/$(DRIVER_NAME)S.$(KVER).ko

# Guess what
clean clear:
	$(RM) $(DRIVER) $(SIMULATOR)
	$(RM) -rf $(OBJDIR)