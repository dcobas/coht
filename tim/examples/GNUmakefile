#================================================================
# Makefile to produce TimLib test program
#================================================================

include /ps/dsc/src/co/Make.auto

CFLAGS := -g -Wall -I. -I.. -I/ps/local/$(CPU)/include -I/ps/src/dsc/drivers/ctr/src/driver
LDLIBS := ../libtim.$(CPU).a -ltgm -lerr -lerr_dummy -ltgv -lm -lX11 $(XTRALIBS) -lnetinet -lnc

ifneq ($(CPU),Lces)
CFLAGS += -fwritable-strings
else
LDLIBS += -lXau -lXdmcp -ldl
endif


ALL  = mathieu.$(CPU) lhcinject.$(CPU)

SRCS = mathieu.c lhcinject.c

HDRS =

all: $(ALL)

clean:
	$(RM) *.$(CPU) *.$(CPU).o ,*

# Run on Workstation only

mathieu.$(CPU).o: mathieu.c

lhcinject.$(CPU).o: lhcinject.c
