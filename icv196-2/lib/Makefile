CPU=L866

include /acc/dsc/src/co/Make.auto

CFLAGS= -g -Wall -I. -I../driver -fPIC -DCOMPILE_TIME=$(shell date +%s)

SRCS = libicv196.c libicv196.h

all: libicv196.$(CPU).so libicv196.$(CPU).a

libicv196.$(CPU).o: $(SRCS)

libicv196.$(CPU).so: libicv196.$(CPU).o
	-$(RM) $@
	$(CC) $(CFLAGS) -o $@ -shared $^

libicv196.$(CPU).a: libicv196.$(CPU).o
	-$(RM) $@
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

clean:
	-$(RM) libicv196.$(CPU).so libicv196.$(CPU).[oa]

docs:
	doxygen -s -g t
	sed -f Doxyfile.sed t > Doxyfile
	rm t
	doxygen

docs-update: docs
	rsync -av html/ /acc/doc/html/private/coht/doxy/icv196


install:
	dsc_install libicv196.$(CPU).a /acc/local/$(CPU)/drv/icv196vme/
