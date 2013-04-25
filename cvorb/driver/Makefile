CPU ?=L866
#KVER=2.6.24.7-rt27
#KVER=3.2.33-rt50
KVER ?=3.2.33-rt50.66.el6rt.x86_64

KERNELSRC=/acc/sys/$(CPU)/usr/src/kernels/$(KVER)
OUTPUTDIR=$(CURDIR)/$(CPU)/$(KVER)

CFLAGS=-g -Wall -Wstrict-prototypessss
all: driver 
driver: modules

# compile with V=1 to see compilation details
modules: 
	cp Module.symvers.vme Module.symvers
	make -C $(KERNELSRC) M=`pwd` KVER=$(KVER) CPU=$(CPU) modules 
	mkdir -p $(OUTPUTDIR)
	mv *.o *.ko $(OUTPUTDIR)
clean:
	rm -f *.o *.a *.pyc *.so $(TESTS)
	rm -rf L865 L864 ppc 
	make -C $(KERNELSRC) M=`pwd` clean


