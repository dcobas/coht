#  
# Native compile
#

KVER=$(shell uname -r)

modules:
	make -C /lib/modules/$(KVER)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(KVER)/build M=$(PWD) clean
	rm -f *.o *.ko
