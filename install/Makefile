###############################################################################
# @file installso.mk
#
# @brief Build shared object user install library
#
# @author Copyright (C) 2009 CERN. Yury GEORGIEVSKIY <ygeorgie@cern.ch>
#
# @date Created on 07/12/2009
###############################################################################

include ../Makefile.base

# redefine implicit rule
(%.$(CPU).o): %.c
	$(Q)$(COMPILE.c) $(CFLAGS) $(CPPFLAGS) -fPIC -c $< -o $*.$(CPU).o

VPATH = user

INSTDIR     = $(UTILINSTDIR)/lib
ADDCFLAGS   = $(STDFLAGS)
ADDINCLUDES = $(KERN_INCLUDES)

INCDIRS = \
	../../general \
	../include \
	../include/user


############################################################################
#      Here comes all that needs to build-up a .so library.	           #
############################################################################
SO_LIB_OBJS = \
	$(addsuffix $(EXTOBJ), $(notdir $(basename $(patsubst %.c, %.o, $(wildcard *.c user/*.c)))))

lib$(DRVR)inst.$(CPU).so: lib$(DRVR)inst.$(CPU).so($(SO_LIB_OBJS))
	$(Q)$(CC) -L../object_$(DRVR) -shared -Wl,-export-dynamic -Wl,-soname,$@ -o $@ $(SO_LIB_OBJS) -ldal.$(CPU)
	$(Q)$(RM) $(IOCTL_LIB_OBJS)

LIBRARIES += lib$(DRVR)inst.$(CPU).so
############################################################################

_build: $(LIBRARIES)
	$(Q)mv $(LIBRARIES) $(FINAL_DEST)

linux:
	$(Q)$(MAKE) _build CPU=L865

lynx:
	$(Q)$(MAKE) _build CPU=ppc4

all:
	$(Q)$(MAKE) _build CPU=L865
	$(Q)$(MAKE) _build CPU=ppc4

# CERN delivery
include ../deliver.mk
deliver:
	$(Q)$(MAKE) _deliver CPU=L865
	$(Q)$(MAKE) _deliver CPU=ppc4