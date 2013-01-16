#!/bin/sh

rmmod cvorb
rm /dev/cvorb.*

insmod L865/3.2.33-rt50/cvorb.ko lun=0,1 base_address=0x400,0x800
