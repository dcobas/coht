#
# vd80-2.awk - extract insmod parameters from transfer.ref
#
# usage: vd80.awk DEVICE_NAME [transfer_file]
#
# e.g.:
#  $ awk -f vd80.awk VD80 /acc/dsc/tst/cfv-864-cdv28/etc/transfer.ref
#
#  produces
#     insmod vd80.ko dname=vd80 luns=1,2,3
#                               vme1=0x980000,0xa00000,0xa80000
#                               vme2=0x980000,0xa00000,0xa80000
#                               vecs=0xB0,0xB1,0xB2
#

BEGIN	{
	device_name = ARGV[1]
	delete ARGV[1]
	luns = ""
	base_address1 = ""
	base_address2 = ""
	vectors = ""
	slots= ""
}

/^#\+#/ && $6 == device_name  && $4 == "VME" {
	# decode transfer.ref line
	luns =  luns "," $7
	base_address2 =  base_address2 "," "0x" $16
	vectors =  vectors "," $23
	slots = slots "," $20
	base_address1 = sprintf("%s,0x%08x", base_address1, $20 * 0x80000)
}

END {

	insmod_params = " "

	if (luns) insmod_params = insmod_params "luns=" substr(luns, 2)

	if (vectors) insmod_params = insmod_params " vecs=" substr(vectors, 2)

	if (slots) insmod_params = insmod_params " slot=" substr(slots, 2)

	if (base_address2) insmod_params = insmod_params " vmeb=" substr(base_address2, 2)

	print insmod_params

}
