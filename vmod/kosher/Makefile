CPU=L865

# KVER=2.6.24.7-rt21
# KVER=2.6.24.7-rt27
# KVER=2.6.29.4-rt15
# KVER=2.6.31.6-rt19

KVER=2.6.24.7-rt27

KERNELSRC=/acc/sys/$(CPU)/usr/src/kernels/$(KVER)
OUTPUTDIR=$(PWD)/$(CPU)/$(KVER)

CFLAGS=-g -Wall
OBJS=libvmod12a2.o libvmod16a2.o libvmod12e16.o
LIBS=libvmod.a libvmod.so
TESTS=tstlib12a2  tstlib16a2 test12e16

all: driver lib tests
driver: modules

modules: 
	cp Module.symvers.vme Module.symvers
	make -C $(KERNELSRC) M=`pwd` KVER=$(KVER) CPU=$(CPU) modules
	mkdir -p $(OUTPUTDIR)
	mv *.o *.ko $(OUTPUTDIR)
clean:
	rm -f *.o *.a *.pyc *.so $(TESTS)
	rm -f vmodio-skel.c mod-pci-skel.c
	rm -rf L865 L864 ppc 
	make -C $(KERNELSRC) M=`pwd` clean

lib: $(LIBS)
libvmod.so: $(OBJS)
	gcc -shared -o $@ $^
libvmod.a: $(OBJS)
	ar r $@ $^

# test programs
tests:  $(TESTS)
tstlib12a2: libvmod12a2.a libvmod12a2.h
tstlib16a2: libvmod16a2.a libvmod16a2.h
test12e16: libvmod12e16.a libvmod12e16.h

libvmod12a2.o:  libvmod12a2.h  vmod12a2.h
libvmod16a2.o:  libvmod16a2.h  vmod16a2.h
libvmod12e16.o: libvmod12e16.h vmod12e16.h vmod12e16drvr.h

libvmod12a2.a: libvmod12a2.o
	ar r $@ $^
libvmod16a2.a: libvmod16a2.o
	ar r $@ $^
libvmod12e16.a: libvmod12e16.o
	ar r $@ $^
