CPU ?= L866
#KVER=2.6.24.7-rt27
#KVER=3.2.33-rt50

ifeq($(CPU),L866)
KVER ?= 3.6.11-rt30.25.el6rt.x86_64
endif

ifeq($(CPU),L865)
KVER ?= 3.2.43-rt63
endif

KERNELSRC=/acc/sys/$(CPU)/usr/src/kernels/$(KVER)
OUTPUTDIR=$(CURDIR)/$(CPU)/$(KVER)
GIT_VERSION=$(shell git describe --always --dirty --tags --all)

CFLAGS=-g -Wall -Wstrict-prototypessss
all: driver 
driver: modules

# compile with V=1 to see compilation details
modules: 
	cp Module.symvers.vme Module.symvers
	make -C $(KERNELSRC) M=`pwd` GIT_VERSION=$(GIT_VERSION) modules
	mkdir -p $(OUTPUTDIR)
	mv *.o *.ko $(OUTPUTDIR)
clean:
	rm -f *.o *.a *.pyc *.so $(TESTS)
	rm -rf L865 L864 ppc 
	make -C $(KERNELSRC) M=`pwd` clean


