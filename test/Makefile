include ../Makefile.base

VPATH = user

DRVRDIR    = object_$(DRVR)
INSTDIR    = $(addsuffix /$(DLVRYPLS)/$(CPU), $(EXECINSTDIR))
ADDCFLAGS  = $(STDFLAGS)

# We will link with local DAL copy (if any)
# Otherwise will link with general DAL copy
define create-lib-symlink
	if [ ! -d ../../general/dal ]; then                 \
			echo '  SYMLINK $@ -> /acc/local/$(CPU)/lib'; \
			ln -fs /acc/local/$(CPU)/lib/libdal.a $@;     \
	else \
			echo '  SYMLINK $@ -> ../../general/dal'; \
			ln -fs ../../general/dal/libdal.$(CPU).a $@; \
	fi
endef

../object_$(DRVR)/libdal.$(CPU).a: FORCE
	$(Q)$(create-lib-symlink)

# libraries (and their pathes) to link executable file with
XTRALIBDIRS = ../../general/dal ../object_$(DRVR)
LOADLIBES  := $(addprefix -L,$(XTRALIBDIRS)) $(LOADLIBES)
LDLIBS      = \
	-ltermcap \
	-l$(DRVR)ioctla.$(CPU) -l$(DRVR)uda.$(CPU) \
	-ldal.$(CPU) -ldrvrutil $(XTRALIBS)

SRCFILES = \
	$(DRVR)Test.c \
	$(wildcard user/*.c)

INCDIRS = \
	./ \
	./user \
	../include \
	../include/user \
	../../general

EXEC_OBJS = $(DRVR)Test.$(CPU)

$(EXEC_OBJS): $(OBJFILES) ../object_$(DRVR)/libdal.$(CPU).a

_build: $(EXEC_OBJS) $(OBJDIR) move_objs

linux:
	$(Q)$(MAKE) _build CPU=L865

lynx:
	$(Q)$(MAKE) _build CPU=ppc4

all:
	$(Q)$(MAKE) _build CPU=L865
	$(Q)$(MAKE) _build CPU=ppc4


# Move compiled files to proper place
move_objs:
	$(Q)mv $(OBJFILES) $(OBJDIR)
	$(Q)mv $(EXEC_OBJS) $(FINAL_DEST)

# CERN delivery
include ../deliver.mk
deliver:
	$(Q)$(MAKE) _deliver $(filter $(strip $(ACCS)),$(MAKECMDGOALS)) CPU=L865
	$(Q)$(MAKE) _deliver $(filter $(strip $(ACCS)),$(MAKECMDGOALS)) CPU=ppc4

cleanloc clearloc:: abort
	@ if [ -n "$(ISP)" ]; then \
		rm -rf $(ISP)* ; \
	fi
	-rm -f ../$(DRVRDIR)/testprog $(DRVR)Test
	-find ./ -name '*~' | xargs rm -f
