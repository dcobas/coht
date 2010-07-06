#!	/bin/sh

sh remove.sh

lun=0

insmod modulbus_register.ko
insmod mod-pci.ko lun=0 bus_number=1 slot_number=14
insmod vmod12e16.ko lun=$lun carrier=mod-pci carrier_number=0 slot=0

devno=`grep vmod12e16 /proc/devices | awk '{print $1}'`
mknod /dev/vmod12e16.$lun c $devno $lun
