#  
# Compilation en natif -> Linux i686 
# (make install en natif)
#

DRV = /dsrc/drivers/acdipole/rs485/driver/

CFLAGS += -I. -I$(DRV) -DHARD_FLOAT -mhard-float -w

OSREL=$(shell uname -r)

BAKS = ,* *~ *.bak *.BAK .es1* .B* %*% .ec1 .ek1 .*~ core a.out *JNL *.lst \\\#*\\\# .nfs* *%

SRCS = ftdi_sio.c
PWD:=$(shell pwd)
obj-m   := $(SRCS:.c=.o)

default:
	$(MAKE) -C /lib/modules/$(OSREL)/build M=$(PWD) clean
	$(MAKE) -C /lib/modules/$(OSREL)/build M=$(PWD) modules

install:
	sudo dsc_install ftdi_sio.ko /lib/modules/$(OSREL)/kernel/drivers/misc
	sudo /sbin/depmod -a
