CPU=L865

# KVER=2.6.24.7-rt21
# KVER=2.6.24.7-rt27
# KVER=2.6.29.4-rt15
# KVER=2.6.31.6-rt19
# KVER=2.6.24.7-rt21

KVER=3.2.43-rt63

KERNELSRC=/acc/sys/$(CPU)/usr/src/kernels/$(KVER)
OUTPUTDIR=/acc/src/dsc/drivers/coht/acdx/driver/$(CPU)/$(KVER)

CFLAGS=-g -Wall

all: driver 
driver: modules

modules: 
	make -C $(KERNELSRC) M=`pwd` KVER=$(KVER) CPU=$(CPU) modules
	mkdir -p $(OUTPUTDIR)
	mv *.o *.ko $(OUTPUTDIR)
clean:
	rm -f *.o *.a *.pyc *.so $(TESTS)
	rm -f *.mod.c
	rm -rf L865 L864 ppc 
	make -C $(KERNELSRC) M=`pwd` clean


