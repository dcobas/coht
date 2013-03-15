#
# Makefile for the IPACK bridge device drivers.
#
obj-$(CONFIG_IPACK_BUS)		+= ipack.o
obj-y				+= devices/
obj-y				+= carriers/

all:
	make -C $(LINUX) M=$(PWD) modules
