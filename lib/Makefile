###############################################################################
# @file libModule.mk
#
# @brief Buildup driver library suite
#
# @author Copyright (C) 2009 CERN. Yury GEORGIEVSKIY <ygeorgie@cern.ch>
#
# @date Created on 29/06/2009
###############################################################################

include ../Makefile.base

# redefine .a rule
(%.$(CPU).o): %.c
	$(Q)$(COMPILE.c) $(CFLAGS) $(CPPFLAGS) -c $< -o $*.$(CPU).o

VPATH = user

INSTDIR     = $(UTILINSTDIR)/lib
ADDCFLAGS   = $(STDFLAGS)
ADDINCLUDES = $(KERN_INCLUDES)

INCDIRS = \
	../../general \
	../include \
	../include/user


############################################################################
#      Here comes all that needs to build-up 'ioctl' library.	           #
############################################################################
IOCTL_LIB_OBJS = \
	$(addsuffix $(EXTOBJ), $(notdir $(basename $(patsubst %.c, %.o, $(wildcard *.c)))))

lib$(DRVR)ioctla.$(CPU).a: lib$(DRVR)ioctla.$(CPU).a($(IOCTL_LIB_OBJS))
	$(Q)$(AR) -rv $@ $^
	$(Q)$(RANLIB) $@
	$(Q)$(RM) $(IOCTL_LIB_OBJS)

LIBRARIES += lib$(DRVR)ioctla.$(CPU).a
############################################################################


############################################################################
#    Here comes all that needs to build-up 'User-Defined Access' library.  #
############################################################################
USR_DEF_LIB_OBJS = \
	$(addsuffix $(EXTOBJ),$(notdir $(basename $(patsubst %.c, %.o, $(wildcard user/*.c)))))

lib$(DRVR)uda.$(CPU).a: lib$(DRVR)uda.$(CPU).a($(USR_DEF_LIB_OBJS))
	$(Q)$(AR) -rv $@ $^
	$(Q)$(RANLIB) $@
	$(Q)$(RM) $(USR_DEF_LIB_OBJS)

LIBRARIES += lib$(DRVR)uda.$(CPU).a
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