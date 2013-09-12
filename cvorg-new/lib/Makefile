CPU ?= L865

include /acc/src/dsc/co/Make.auto

CFLAGS := -Wall -g -D_GNU_SOURCE -I. -I../include -I../../include -I../ad9516
LIBCVORG := libcvorg.$(CPU).a 
HLIBCVORG := ../include/libcvorg.h
HCVORG := ../include/cvorg.h
OBJAD9516 := ../ad9516/ad9516.$(CPU).o
EXAMPLES_DIR := examples
DOXY_DIR := doxygen_output
DOXY_FILES := cvorg.c error.c $(HLIBCVORG) $(HCVORG)
DOXY_INSTDIR := /acc/doc/html/private/coht/doxy/cvorg

all: $(LIBCVORG) examples

.PHONY: all clean examples

cvorg.$(CPU).o: cvorg.c $(HCVORG) $(HLIBCVORG) libinternal.h cvorgdev.h 
	$(CC) $(CFLAGS) -c -I. -I../ad9516 cvorg.c -o $@

libcvorg.$(CPU).a: cvorg.$(CPU).o cvorgdev.$(CPU).o error.$(CPU).o $(OBJAD9516)
	$(AR) rv $(LIBCVORG) $^

cvorgdev.$(CPU).o: cvorgdev.c cvorgdev.h
	$(CC) $(CFLAGS) -c -I. cvorgdev.c -o $@

error.$(CPU).o: error.c libinternal.h
	$(CC) $(CFLAGS) -c -I. error.c -o $@

libcvorg.$(CPU).so: cvorg.$(CPU).o cvorgdev.$(CPU).o error.$(CPU).o $(OBJAD9516)
	$(CC) -shared -o $@ $^

examples:
	$(MAKE) -C $(EXAMPLES_DIR) CPU=$(CPU)

clean:
	$(MAKE) clean -C $(EXAMPLES_DIR) CPU=$(CPU)
	$(RM) *.a *.o $(BAKS)
	$(RM) -r doc

doxy: doxy_clean $(DOXY_FILES)
	../../doxy/doxy.sh -n"libcvorg" -o"$(DOXY_DIR)" $(DOXY_FILES)

doxy_clean:
	rm -rf $(DOXY_DIR)

doxy_install: doxy
	mkdir -p $(DOXY_INSTDIR)
	cp -r $(DOXY_DIR)/html/* $(DOXY_INSTDIR)/
