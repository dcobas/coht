#include <linux/module.h>
#include <linux/pci.h>
#include "mod-pci.h"

#define	MAX_DEVICES	64
#define	DRIVER_NAME	"mod-pci"

/*
 * this module is invoked as
 *     $ insmod mod-pci lun=0,1,2,4 bus_number=1,1,1,2 slot_number=2,3,5,1
 * to describe the bus number and slot number associated to devices with
 * prescribed luns.
 */

static int lun[MAX_DEVICES];
static int nlun;
module_param_array(lun, int, &nlun, S_IRUGO);

static int bus_number[MAX_DEVICES];
static int nbus_number;
module_param_array(bus_number, int, &nbus_number, S_IRUGO);

static int slot_number[MAX_DEVICES];
static int nslot_number;
module_param_array(slot_number, int, &nslot_number, S_IRUGO);

/* description of a mod-pci module */
struct mod_pci {
	int		lun;		/* logical unit number */
	int		bus_number;	/* pci bus number */
	int		slot_number;	/* pci slot number */
	unsigned long	vaddr;		/* virtual address of MODULBUS
							space */
};

static struct mod_pci device_table[MAX_DEVICES];
static int devices;


static struct pci_device_id mod_pci_ids[] = {
	{ PCI_VENDOR_ID_JANZ, PCI_DEVICE_ID_JANZ_MOD_PCI_1,
	  PCI_SUBSYSTEM_VENDOR_ID_JANZ, PCI_SUBSYSTEM_ID_MOD_PCI_1_0, },
	{ PCI_VENDOR_ID_JANZ, PCI_DEVICE_ID_JANZ_MOD_PCI_1,
	  PCI_SUBSYSTEM_VENDOR_ID_JANZ, PCI_SUBSYSTEM_ID_MOD_PCI_2_0, },
	{ PCI_VENDOR_ID_JANZ, PCI_DEVICE_ID_JANZ_MOD_PCI_1,
	  PCI_SUBSYSTEM_VENDOR_ID_JANZ, PCI_SUBSYSTEM_ID_MOD_PCI_2_1, },
	{ PCI_VENDOR_ID_JANZ, PCI_DEVICE_ID_JANZ_MOD_PCI_1,
	  PCI_SUBSYSTEM_VENDOR_ID_JANZ, PCI_SUBSYSTEM_ID_MOD_PCI_3_0, },
};

static int probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	int i;
	int bus  = dev->bus->number;
	int slot = PCI_SLOT(dev->devfn);
	unsigned long vaddr;

	/* find the device in config table */
	for (i = 0; i < devices; i++) {
		if (device_table[i].bus_number  == bus &&
		    device_table[i].slot_number == slot)
		    break;
	}
	if (i >= devices) 
		return -1;

	/* found match, configure */
	printk(KERN_INFO "%s: "
		"configuring device at bus = %d, slot %d\n",
		DRIVER_NAME, bus, slot);
	if (pci_read_config_dword(dev, MOD_PCI_MODULBUS_MEMSPACE_BIG_BAR_OFFSET, (u32*)&vaddr) != 0) {
		printk(KERN_ERR "%s: could not read BAR#%d\n",
			DRIVER_NAME, MOD_PCI_MODULBUS_MEMSPACE_BIG_BAR);
		return -1;
	}
	printk(KERN_INFO "%s: "
		"configured device number %d\n"
		"lun = %d, bus = %d, slot = %d, vaddr = %p\n",
		DRIVER_NAME, i, device_table[i].lun, bus, slot, 
		(void*)vaddr);
	return 0;
}

static void remove(struct pci_dev *dev)
{
}

static struct pci_driver pci_driver = {
	.name     = "mod-pci",
	.id_table = mod_pci_ids,
	.probe    = probe,
	.remove   = remove,
};

static int __init init(void)
{
	int device = 0;

	printk(KERN_INFO "%s: initializing driver\n", DRIVER_NAME);
	if (nlun < 0 || nlun >= MAX_DEVICES) {
		printk(KERN_ERR "%s: "
		"invalid number of configured devices (%d)\n",
			DRIVER_NAME, nlun);
		goto failed_init;
	}
	if (nbus_number != nlun || nslot_number != nlun) {
		printk(KERN_ERR "%s: parameter mismatch.\n"
			"Given %d luns, %d bus numbers, %d slot numbers\n",
			DRIVER_NAME, nlun, nbus_number, nslot_number);
		goto failed_init;
	}

	for (device = 0; device < nlun; device++) {
		struct mod_pci *dev = &device_table[device];
		
		dev->lun	= lun[device];
		dev->bus_number	= bus_number[device];
		dev->slot_number= slot_number[device];
		printk(KERN_INFO "should have (%d, %d, %d)\n",
			dev->lun, dev->bus_number, dev->slot_number);
	}
	devices = device;

	printk(KERN_INFO "%s: registering driver\n", DRIVER_NAME);
	return pci_register_driver(&pci_driver);

failed_init:
	printk(KERN_ERR "%s: module exiting\n", DRIVER_NAME);
	return -1;
}

static void __exit exit(void)
{
	pci_unregister_driver(&pci_driver);
}

module_init(init);
module_exit(exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Juan David Gonzalez Cobas <dcobas@cern.ch>");
