#================================================================
# Makefile to produce TimLib test program
#================================================================

include /ps/dsc/src/co/Make.auto

ACCS=oper lab

TIMLIB = /dsrc/co/lib/tim

COMPILE_TIME:=$(shell ./gettime)

CFLAGS := -g -Wall -I. -I$(TIMLIB) -I/ps/local/$(CPU)/include -I/ps/src/dsc/drivers/ctr/src/driver \
	  -DCOMPILE_TIME=$(COMPILE_TIME)
LDLIBS := $(TIMLIB)/libtim.$(CPU).a -ldscrt -ltgm -lerr -lerr_dummy -ltgv -lm -lX11 $(XTRALIBS) -lnetinet -lnc

ifneq ($(CPU),L864)
LDLIBS += -mthreads
else
LDLIBS += -lpthread
endif

ALL  = TimSurvey.$(CPU)

SRCS = TimSurvey.c

HDRS = $(TIMLIB)/TimLib.h

all: $(ALL)

clean:
	$(RM) *.$(CPU) *.$(CPU).o ,*

# Run on Workstation only

TimSurvey.$(CPU).o: TimSurvey.c

TimSurvey.$(CPU): TimSurvey.$(CPU).o

install: TimSurvey.$(CPU)
	@for a in $(ACCS); do \
		d=/acc/dsc/$$a/$(CPU)/tim;\
		echo Installing TimSurvey.$(CPU) in $$d; \
		sudo dsc_install TimSurvey.$(CPU) $$d; \
		sudo chmod +s $$d/TimSurvey; \
		sudo chown root $$d/TimSurvey; \
	done;
