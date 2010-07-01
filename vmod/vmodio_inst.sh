#!/bin/sh

RET=`lsmod | grep modulbus_register | wc -l`;

if [ $RET -eq 0 ]
then
    echo "Loading modulbus_register module..."
    insmod modulbus_register.ko
fi

perl vmodio_install.pl $*
