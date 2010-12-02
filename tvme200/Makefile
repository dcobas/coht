CPU=L865

# KVER=2.6.24.7-rt21
# KVER=2.6.24.7-rt27
# KVER=2.6.29.4-rt15
# KVER=2.6.31.6-rt19

KVER=2.6.24.7-rt27
KERNELSRC=/acc/sys/$(CPU)/usr/src/kernels/$(KVER)

OUTPUTDIR=$(PWD)/$(CPU)/$(KVER)

modules:
	cp Module.symvers.vmebridge Module.symvers
	make -C $(KERNELSRC) M=$(PWD) modules
	mkdir -p $(OUTPUTDIR)
	mv *.o *.ko $(OUTPUTDIR)
clean:
	make -C $(KERNELSRC) M=$(PWD) clean

test: carrierTstDrvr.$(CPU) carrierTst.$(CPU)
carrierTstDrvr.$(CPU): carrierTstDrvr.o
carrierTst.$(CPU): carrierTst.o
