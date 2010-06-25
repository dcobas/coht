#include <linux/module.h>
#include <linux/pci.h>
#include "mod-pci.h"

#define	MAX_DEVICES	16
#define	DRIVER_NAME	"mod-pci"
#define PFX		DRIVER_NAME ": "

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
	void		*vaddr;		/* virtual address of MODULBUS
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

static struct mod_pci *find_device_config(struct pci_dev *dev)
{
	int bus  = dev->bus->number;
	int slot = PCI_SLOT(dev->devfn);
	int i;

	/* find the device in config table */
	for (i = 0; i < devices; i++) {
		struct mod_pci *entry = &device_table[i];
		if (entry->bus_number  == bus &&
		    entry->slot_number == slot)
		    return entry;
	}
	return NULL;
}

static int probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	struct mod_pci *cfg_entry;

	/* check static config is present */
	cfg_entry = find_device_config(dev);
	if (!cfg_entry) {
		printk(KERN_INFO PFX
			"no config data for bus = %d, slot %d\n",
			dev->bus->number, PCI_SLOT(dev->devfn));
		goto failed_enable;
	}

	/* found match, configure */
	printk(KERN_INFO PFX
		"configuring device at bus = %d, slot %d\n",
		cfg_entry->bus_number, cfg_entry->slot_number);
	if (pci_enable_device(dev) < 0) {
		printk(KERN_ERR PFX "could not enable device\n");
		goto failed_enable;
	}
	if (!(pci_resource_flags(dev, MOD_PCI_MODULBUS_MEMSPACE_BIG_BAR) & IORESOURCE_MEM)) {
		printk(KERN_ERR PFX "BAR#2 not a MMIO, device not present?\n");
		goto failed_request;
	}
	if (pci_resource_len(dev,
		MOD_PCI_MODULBUS_MEMSPACE_BIG_BAR_OFFSET) < MOD_PCI_MODULBUS_WINDOW_SIZE) {
		printk(KERN_ERR PFX "wrong BAR# size\n");
		goto failed_request;
	}
	if (pci_request_region(dev,
		MOD_PCI_MODULBUS_MEMSPACE_BIG_BAR, DRIVER_NAME) != 0) {
		printk(KERN_ERR PFX "could not request BAR#2\n");
		goto failed_request;
	}
	cfg_entry->vaddr = ioremap(
		pci_resource_start(dev, MOD_PCI_MODULBUS_MEMSPACE_BIG_BAR),
		MOD_PCI_MODULBUS_WINDOW_SIZE);
	if (cfg_entry->vaddr == NULL) {
		printk(KERN_ERR PFX "could not map BAR#2\n");
		goto failed_map;
	}
	printk(KERN_INFO PFX "configured device "
		"lun = %d, bus = %d, slot = %d, vaddr = %p\n",
		cfg_entry->lun, 
		cfg_entry->bus_number, cfg_entry->slot_number, 
		cfg_entry->vaddr);

	return 0;

failed_map:
	pci_release_regions(dev);
failed_request:
	pci_disable_device(dev);
failed_enable:
	return -ENODEV;
}

static void remove(struct pci_dev *dev)
{
	struct mod_pci *cfg = find_device_config(dev);

	iounmap(cfg->vaddr);
	pci_release_regions(dev);
	pci_disable_device(dev);
}

static struct pci_driver pci_driver = {
	.name     = DRIVER_NAME,
	.id_table = mod_pci_ids,
	.probe    = probe,
	.remove   = remove,
};

static int __init init(void)
{
	int device = 0;

	printk(KERN_INFO PFX "initializing driver\n");
	if (nlun < 0 || nlun >= MAX_DEVICES) {
		printk(KERN_ERR PFX
		"invalid number of configured devices (%d)\n", nlun);
		goto failed_init;
	}
	if (nbus_number != nlun || nslot_number != nlun) {
		printk(KERN_ERR PFX "parameter mismatch.\n"
			"Given %d luns, %d bus numbers, %d slot numbers\n",
			nlun, nbus_number, nslot_number);
		goto failed_init;
	}

	for (device = 0; device < nlun; device++) {
		struct mod_pci *dev = &device_table[device];
		
		dev->lun	= lun[device];
		dev->bus_number	= bus_number[device];
		dev->slot_number= slot_number[device];
		printk(KERN_INFO PFX "shall config lun %d," "bus %d, slot %d)\n",
			dev->lun, dev->bus_number, dev->slot_number);
	}
	devices = device;

	printk(KERN_INFO PFX "registering driver\n");
	return pci_register_driver(&pci_driver);

failed_init:
	printk(KERN_ERR PFX "module exiting\n");
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
