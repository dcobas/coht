CPU=L865
KVER=2.6.24.7-rt27
KERNELPATH=/acc/sys/$(CPU)/usr/src/kernels/$(KVER)/

modules:
	make -C $(KERNELPATH) M=$(PWD) modules
clean:
	make -C $(KERNELPATH) M=$(PWD) clean
