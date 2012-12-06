CPU ?= L865

CFLAGS := -Wall -g -I. -I../include -I../../include
LIBCVORB := libcvorb.$(CPU).a 
HLIBCVORB := ../include/libcvorb.h
HCVORB := ../include/cvorb.h
EXAMPLES_DIR := examples
DOXY_DIR := doxygen_output
DOXY_FILES := cvorb.c error.c $(HLIBCVORB) $(HCVORB)
DOXY_INSTDIR := /acc/doc/html/private/coht/doxy/cvorb

all: $(LIBCVORB)

.PHONY: all clean examples

cvorb.$(CPU).o: cvorb.c $(HCVORB) $(HLIBCVORB) libinternal.h cvorbdev.h 
	$(CC) $(CFLAGS) -c -I. cvorb.c -o $@

libcvorb.$(CPU).a: cvorb.$(CPU).o cvorbdev.$(CPU).o error.$(CPU).o $(OBJAD9516)
	$(AR) rv $(LIBCVORB) $^

cvorbdev.$(CPU).o: cvorbdev.c cvorbdev.h
	$(CC) $(CFLAGS) -c -I. cvorbdev.c -o $@

error.$(CPU).o: error.c libinternal.h
	$(CC) $(CFLAGS) -c -I. error.c -o $@

libcvorb.$(CPU).so: cvorb.$(CPU).o cvorbdev.$(CPU).o error.$(CPU).o $(OBJAD9516)
	$(CC) -shared -o $@ $^

examples:
	$(MAKE) -C $(EXAMPLES_DIR) CPU=$(CPU)

clean:
	$(MAKE) clean -C $(EXAMPLES_DIR) CPU=$(CPU)
	$(RM) *.a *.o $(BAKS)
	$(RM) -r doc

doxy: doxy_clean $(DOXY_FILES)
	../../doxy/doxy.sh -n"libcvorb" -o"$(DOXY_DIR)" $(DOXY_FILES)

doxy_clean:
	rm -rf $(DOXY_DIR)

doxy_install: doxy
	mkdir -p $(DOXY_INSTDIR)
	cp -r $(DOXY_DIR)/html/* $(DOXY_INSTDIR)/
