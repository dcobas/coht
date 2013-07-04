#
# transfer2insmod.awk - extract insmod parameters from transfer.ref
#
# usage: transfer2insmod.awk DEVICE_NAME [transfer_file]
#
# e.g.:
#  $ awk -f transfer2insmod.awk CVORB /acc/dsc/tst/cfv-864-cdv28/etc/transfer.ref
#
#  produces
#     lun=1,2 base_address=0x400,0x800
#

BEGIN {
	device_name = ARGV[1]
	delete ARGV[1]
	bcs = ""
	slots = ""
}

/^#\+#/ && $6 == device_name  && $4 == "VME" {
	# decode transfer.ref line
	lun = lun "," $7
	base_address = base_address "," "0x" $11
}

END {
	insmod_params = " "

	if (lun)
		insmod_params = insmod_params "lun=" substr(lun, 2)
	if (base_address)
		insmod_params = insmod_params " base_address=" substr(base_address, 2)
	print insmod_params
}

