#
# transfer2insmod.awk - extract insmod parameters from transfer.ref
#
# usage: transfer2insmod.awk DEVICE_NAME [transfer_file]
#
# e.g.:
#  $ awk -f transfer2insmod.awk VD80 /acc/dsc/tst/cfv-864-cdv28/etc/transfer.ref
#
#  produces
#     insmod mil1553.ko bcs=1,2
#                       pci_buses=1,1
#                       pci_slots=13,14
#
# The bus number is not available from transfer.ref but by chance it is always 1
#

BEGIN {
	device_name = ARGV[1]
	delete ARGV[1]
	crateconfig = ARGV[2]
	delete ARGV[2]
	while (getline <crateconfig > 0) {
		slot_to_pci_bus[$1] = $2
		slot_to_pci_slot[$1] = $3
	}
	bcs = ""
	slots = ""
}

/^#\+#/ && $6 == device_name  && $4 == "PCI" {
	# decode transfer.ref line
	lun = $7
	slot = $20
	subslots = $21
	pci_bus  = slot_to_pci_bus[$20]
	pci_slot = slot_to_pci_slot[$20]

        pci_device = sprintf("/sys/bus/pci/devices/0000:%02x:%02s.0/ipack-dev.*.%d", pci_bus, pci_slot, subslots)
	( "echo " pci_device "| sed 's!/sys/bus/pci/devices/0000:..:..\\.0/ipack-dev.\\([0-9]*\\).\\([0-9]*\\)!/dev/ipoctal.\\1.\\2!' " ) | getline x  
	for (i = 0; i < 8; i++) {
		print "ln -s " x "." i " /dev/ipoctal." lun "." i
	}
}
