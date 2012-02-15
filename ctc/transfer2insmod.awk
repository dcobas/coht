#
# transfer2insmod.awk - extract insmod parameters from transfer.ref
#
# usage: transfer2insmod.awk DEVICE_NAME [transfer_file]
#
# e.g.:
#  $ awk -f transfer2insmod.awk VD80 /acc/dsc/tst/cfv-864-cdv28/etc/transfer.ref
#
#  produces
#      luns=0,2 vectors=0xb0,0xb2 level=2 am1=0x2F data_width1=32 size1=0x80000 \
#      base_address1=0x100000,0x200000 am2=0x29 data_width2=32 size2=0x80000 \
#      base_address2=0x100000,0x200000
#

BEGIN	{
	device_name = ARGV[1]
	delete ARGV[1]
	luns = ""
	base_address1 = ""
	base_address1 = ""
	vectors = ""

	am["EX"] = "0x9"
	am["SH"] = "0x29"
	am["ST"] = "0x39"
	am["CR"] = "0x2F"
}

/^#\+#/ && $6 == device_name {
	# decode transfer.ref line
	luns =  luns "," $7
	base_address1 =  base_address1 "," "0x" $11
	base_address2 =  base_address2 "," "0x" $16
	vectors =  vectors "," $23
	data_width1 = substr($10, 3)
	data_width2 = substr($15, 3)
	size1 = "0x" $12
	size2 = "0x" $17
	am1 = am[$9]
	am2 = am[$14]
	level = $22
}

END	{
	if (luns)
	    insmod_params = "lun=" substr(luns, 2)
	if (level) {
	    # insmod_params = insmod_params " vector=" substr(vectors, 2)
	    # insmod_params = insmod_params " level=" level
	}
	if (am1) {
	    insmod_params = insmod_params " am1=" am1
	    insmod_params = insmod_params " data_width1=" data_width1
	    insmod_params = insmod_params " size1=" size1
	    insmod_params = insmod_params " base_address1=" substr(base_address1, 2)
	}
	if (am2) {
	    insmod_params = insmod_params " am2=" am2
	    insmod_params = insmod_params " data_width2=" data_width2
	    insmod_params = insmod_params " size2=" size2
	    insmod_params = insmod_params " base_address2=" substr(base_address2, 2)
	}
	print insmod_params
}
