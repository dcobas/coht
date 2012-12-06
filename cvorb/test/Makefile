CPU ?= L865

include /acc/src/dsc/co/Make.auto

VPATH = ../../utils/extest ../lib
INCDIR := ../include
CFLAGS := -Wall -g -I$(INCDIR) -I../../include -I../../utils/extest -I../lib \
	-I../../utils/user -I/acc/local/$(CPU)/include -DDRIVER_NAME=\"cvorb\"

LOADLIBES := -L../lib/ -L../../utils/install -L/usr/lib -L/acc/sys/$(CPU)/usr/lib
LDLIBS= -lcvorb.$(CPU) -lutils.$(CPU) -lreadline -ltermcap -lxml2 -lz -lrt -lm 

all: cvorbtest.$(CPU)

.PHONY: all clean

cvorbtest.$(CPU): cvorbtest.$(CPU).o extest.$(CPU).o cmd_generic.$(CPU).o time_stamp_counter.$(CPU).o $(LIBUTILS)
cvorbtest.$(CPU).o: cvorbtest.c time_stamp_counter.c extest.h time_stamp_counter.h
extest.$(CPU).o: extest.c extest.h extest_common.h
cmd_generic.$(CPU).o: cmd_generic.c extest.h extest_common.h cmd_handlers.h

$(LIBUTILS):
	$(MAKE) -C $(LIBUTILSPATH) CPU=$(CPU)

clean:
	$(RM) *.$(CPU) *.o *.$(CPU).a $(BAKS)
