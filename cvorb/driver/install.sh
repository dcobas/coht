#!/bin/sh

rmmod cvorb
rm /dev/cvorb.*

insmod L865/2.6.24.7-rt27/cvorb.ko lun=0,1 base_address=0x800,0x1000
