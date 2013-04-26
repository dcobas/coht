include /acc/src/dsc/co/Make.auto

libvmebus.$(CPU).a: libvmebus.$(CPU).o
	ar rv $@ $^
libvmebus.$(CPU).o: libvmebus.c libvmebus.h

clean:
	rm -f *.[ao]
