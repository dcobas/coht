#!/bin/sh

RET=`lsmod | grep modulbus_register | wc -l`;
drvr=vmod12a2

if [ $RET -eq 0 ]
then
    echo "${drvr}_inst.sh : ERROR : modulbus_register module is needed. Exiting..."
    exit -1;
fi

perl vmod_install.pl --device=NULLVMOD12A2 ${drvr}

