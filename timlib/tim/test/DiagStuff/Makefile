#================================================================
# Makefile to produce TimLib test program
#================================================================

CPU=L866

include /acc/dsc/src/co/Make.auto

ACCS=oper lab

COMPILE_TIME:=$(shell ./gettime)

CFLAGS := -g -Wall -I. -I../.. ../../../../driver/ -I/acc/local/$(CPU)/include -DCOMPILE_TIME=$(COMPILE_TIME)

LDLIBS := ../../libtim.$(CPU).a -ldscrt -ltgm -lerr -lerr_dummy -ltgv -lm -lX11 -lpthread

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
