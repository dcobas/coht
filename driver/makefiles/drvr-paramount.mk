# That's our default target when none is given on the command line
PHONY := _all
_all:

ROOTDIR := /acc/src/dsc/drivers/coht
abs2rel = $(shell $(ROOTDIR)/makefiles/abs2rel.sh $(1) $(2))

# Executable && drivers goes here
FINAL_DEST := object_$(DRIVER_NAME)
$(FINAL_DEST):
	@mkdir -p ../$@

# Compiled files goes here
OBJDIR := $(CPU)/$(KVER)
$(OBJDIR):
	@mkdir -p $@

comma = ,
EXTOBJ = .o
SRCSFXS = .c
SRCFILES := $(wildcard $(M)/*.c) $(wildcard $(M)/user/*.c)
FLTSRCFILES := $(filter-out $(M)/$(comma)% $(M)/user/$(comma)% $(M)/%.inc.c, $(SRCFILES))
OBJFILES = $(patsubst $(M)/%,%,$(FLTSRCFILES:.c=.o))

# filter out objects
DRVROBJS := $(filter-out %Sim.o, $(OBJFILES))
SIMOBJS  := $(filter-out %Drvr.o, $(OBJFILES))

# What will be finally created
DRIVER    := $(OBJDIR)/$(DRIVER_NAME)D.ko
SIMULATOR := $(OBJDIR)/$(DRIVER_NAME)S.ko

ccflags-y := \
	-I$(M)/../include \
	-I$(M)/../include/user \
	-I$(ROOTDIR) \
	-I$(ROOTDIR)/include \
	-I$(M)/../../general

PHONY += all
_all: all

PHONY += FORCE
FORCE:

# Declare the contents of the .PHONY variable as phony.  We keep that
# information in a variable so we can use it in if_changed and friends.
.PHONY: $(PHONY)