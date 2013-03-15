CPU?=L865

include /acc/src/dsc/co/Make.auto

VPATH = ../lib
INCDIR := ../include
CFLAGS := -Wall -g -I$(INCDIR) -I../lib -I/acc/local/$(CPU)/drv  \
	-I/acc/local/$(CPU)/include -DDRIVER_NAME=\"cvorb\"

LOADLIBES := -L../lib/ -L/acc/sys/$(CPU)/usr/lib -L/acc/local/$(CPU)/drv/extest -L/acc/local/$(CPU)/lib
LDLIBS= -lcvorb.$(CPU) -lextest -lreadline -ltermcap -lz -lrt -lm

all: cvorbtest.$(CPU)

.PHONY: all clean
cvorbtest.$(CPU): cvorbtest.$(CPU).o time_stamp_counter.$(CPU).o $(LIBUTILS)

cvorbtest.$(CPU).o: cvorbtest.c time_stamp_counter.c time_stamp_counter.h

$(LIBUTILS):
	$(MAKE) -C $(LIBUTILSPATH) CPU=$(CPU)

clean:
	$(RM) *.$(CPU) *.o *.$(CPU).a $(BAKS)
