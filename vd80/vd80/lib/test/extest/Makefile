CPU = L866

include /acc/src/dsc/co/Make.auto

INST_HEADERS = extest.h general_usr.h extest_common.h general_both.h list.h data_tables.h config_data.h list_extra.h var_decl.h
LIB_DIR=/acc/local/$(CPU)/drv/extest

CFLAGS := -Wall -g -I. -I/acc/sys/$(CPU)/usr/include

all: libextest.$(CPU).a

extest.$(CPU).o: extest.c extest.h extest_common.h
	$(CC) $(CFLAGS) -c -I. extest.c -o $@
cmd_generic.$(CPU).o: cmd_generic.c extest.h extest_common.h cmd_handlers.h
	$(CC) $(CFLAGS) -c -I. cmd_generic.c -o $@

libextest.$(CPU).a: extest.$(CPU).o cmd_generic.$(CPU).o 
	$(AR) $(ARFLAGS) $@ $^

install:
	dsc_install libextest.$(CPU).a $(INST_HEADERS) $(LIB_DIR)

clean:
	$(RM) *.$(CPU) *.o *.$(CPU).a $(BAKS)
