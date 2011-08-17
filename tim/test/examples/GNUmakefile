#================================================================
# Makefile to produce TimLib test program
#================================================================

include /ps/dsc/src/co/Make.auto

CFLAGS := -g -Wall -I. -I.. -I../.. -I/ps/local/$(CPU)/include -I/ps/src/dsc/drivers/ctr/src/driver
LDLIBS := ../libtim.$(CPU).a -ltgm -lerr -lerr_dummy -ltgv -lm -lX11 $(XTRALIBS) -lnetinet -lnc

ifneq ($(CPU),Lces)
CFLAGS += -fwritable-strings
else
LDLIBS += -lXau -lXdmcp -ldl
endif


ALL  = RocsTimLib.$(CPU).o

SRCS = RocsTimLib.c

HDRS = RocsTimLib.h

all: $(ALL)

clean:
	$(RM) *.$(CPU) *.$(CPU).o ,*

# Run on Workstation only

RocsTimLib.$(CPU).o: $(SRCS) $(HDRS)

# RocsTimLib.$(CPU): RocsTimLib.$(CPU).o
