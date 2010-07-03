#!	/bin/sh
for f in 12e16 mod_pci  modulbus_register ; do
    if grep $f /proc/modules ; then
        rmmod $f
    fi
done

( cd L*/*/
insmod modulbus_register.ko
insmod mod-pci.ko  lun=1,0 bus_number=1,2 slot_number=14,13
#insmod mod-pci.ko  lun=0 bus_number=1 slot_number=14
insmod 12e16.ko lun=0 carrier=mod-pci carrier_number=1 slot=1  )

DEVNO=`grep 12e16 /proc/devices | awk '{print $1}' `
mknod /dev/vmod12e16.0 c $DEVNO 0

