#!/bin/sh

RET=`lsmod | grep modulbus_register | wc -l`;

if [ $RET -eq 0 ]
then
    echo "vmodttl_inst.sh : ERROR : modulbus_register module is needed. Exiting..."
    exit -1;
fi

perl vmod_install.pl --device=NULLVMODTTL vmodttl

