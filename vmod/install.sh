#!	/bin/sh
insmod modulbus_register.ko
insmod vmodio.ko lun=0 base_address=0xa800 irq=126
insmod vmodttl.ko lun=0 carrier=vmodio carrier_number=0 slot=3
