#!/bin/bash

if [ $# -eq 0 ]; then
	echo "./tvme200_install_test.sh <base_address_ioid> <base_address_mem>"
	echo "Example: ./tvme200_install_test.sh 0x8000 0xD00000 0x9000 0xF00000"
	echo "                                   |----lun 0----| |----lun 1----|"
	exit 1
fi

declare -i NUM
NUM=0
let DEVICES=$#/2;

args=( $* )

until [ $NUM -eq $DEVICES ]; do
	if [ $NUM -eq 0 ]; then
		LUN=0;
		MOD_ADDRESS_IOID="0x29"
		DATA_WIDTH_IOID="16"
		WIND_SIZE_IOID="0x0400"
		BASE_ADDRESS_IOID=$1
		MOD_ADDRESS_MEM="0x39"
		DATA_WIDTH_MEM="16"
		WIND_SIZE_MEM="0x20000"
		BASE_ADDRESS_MEM=$2
	else
		LUN=$LUN","$NUM
		MOD_ADDRESS_IOID=$MOD_ADDRESS_IOID",0x29"
		DATA_WIDTH_IOID=$DATA_WIDTH_IOID",16"
		WIND_SIZE_IOID=$WIND_SIZE_IOID",0x0400"
		BASE_ADDRESS_IOID=$BASE_ADDRESS_IOID","${args[$(($NUM * 2))]}
		MOD_ADDRESS_MEM=$MOD_ADDRESS_MEM",0x39"
		DATA_WIDTH_MEM=$DATA_WIDTH_MEM",16"
		WIND_SIZE_MEM=$WIND_SIZE_MEM",0x20000"
		BASE_ADDRESS_MEM=$BASE_ADDRESS_MEM","${args[$(($NUM * 2 + 1))]}
	fi
	NUM=${NUM}+1;
done

echo "Installing " $NUM " devices..."

insmod ./tvme200.ko lun=$LUN mod_address_ioid=$MOD_ADDRESS_IOID data_width_ioid=$DATA_WIDTH_IOID wind_size_ioid=$WIND_SIZE_IOID base_address_ioid=$1 mod_address_mem=$MOD_ADDRESS_MEM data_width_mem=$DATA_WIDTH_MEM wind_size_mem=$WIND_SIZE_MEM base_address_mem=$2

