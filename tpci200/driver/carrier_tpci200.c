/**
 * carrier_tpci200.c
 *
 * driver for the carrier TEWS TPCI-200 
 * Copyright (c) 2009 Nicolas Serafini, EIC2 SA
 * Copyright (c) 2010,2011 Samuel Iglesias Gonsalvez <siglesia@cern.ch>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#include <asm/io.h>
#include <linux/pci.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/fs.h>

#include "carrier.h"
#include "carrier_common.h"
#include "tpci200.h"

#define CARRIER_DRIVER_DESCRIPTION "Carrier interface driver (PCI-VME)"
#define CARRIER_DRIVER_VERSION "1.0" 
#define CARRIER_DRIVER_AUTHOR "Nicolas Serafini, EIC2 SA"
#define CARRIER_DRIVER_LICENSE "GPL"
#define MAX_CARRIER 16
#define PFX "tpci200: "

static int lun[MAX_CARRIER];
static int num_lun;
module_param_array(lun, int, &num_lun, S_IRUGO);
MODULE_PARM_DESC(lun, "Logical Unit Number");
static int bus[MAX_CARRIER];
static int num_bus;
module_param_array(bus, int, &num_bus, S_IRUGO);
MODULE_PARM_DESC(lun, "PCI Bus Number");
static int slot[MAX_CARRIER];
static int num_slot;
module_param_array(slot, int, &num_slot, S_IRUGO);
MODULE_PARM_DESC(lun, "Slot Number on PCI Bus");
static int vendor_id[MAX_CARRIER];
static int num_vendor_id;
module_param_array(vendor_id, int, &num_vendor_id, S_IRUGO);
MODULE_PARM_DESC(lun, "PCI Vendor ID");
static int device_id[MAX_CARRIER];
static int num_device_id;
module_param_array(device_id, int, &num_device_id, S_IRUGO);
MODULE_PARM_DESC(lun, "PCI Device ID");
static int subdevice_id[MAX_CARRIER];
static int num_subdevice_id;
module_param_array(subdevice_id, int, &num_subdevice_id, S_IRUGO);
MODULE_PARM_DESC(lun, "PCI SubDevice ID");
static int subvendor_id[MAX_CARRIER];
static int num_subvendor_id;
module_param_array(subvendor_id, int, &num_subvendor_id, S_IRUGO);
MODULE_PARM_DESC(lun, "PCI SubVendor ID");

/**
 * struct carrier_infos - informations specific of the TPCI200 carrier.
 * @pci_dev		PCI device
 * @interface_regs	Pointer to IP interface space (Bar 2)
 * @ioidint_space	Pointer to IP ID, IO and INT space (Bar 3)
 * @mem8_space		Pointer to MEM space (Bar 4)
 * @access_lock		Mutex lock for simultaneous access
 *  
 */
struct carrier_infos {
	struct pci_dev *pci_dev;
	void           *interface_regs;
	void           *ioidint_space;
	void           *mem8_space;
	spinlock_t     access_lock;
};

static int carrier_register(struct carrier_board *carrier, void *dev_specific);
static void carrier_unregister(struct carrier_board *carrier);
static int slot_request_irq(struct carrier_board *carrier, struct slot_id *slot_id);
static void slot_free_irq(struct carrier_board *carrier, struct slot_id *slot_id);
static irqreturn_t carrier_interrupt(int irq, void *dev_id);
static unsigned char slot_read_uchar(void *address, unsigned long offset);
static unsigned short slot_read_ushort(void *address, unsigned long offset);
static unsigned int slot_read_uint(void *address, unsigned long offset);
static void slot_write_uchar(unsigned char value, void *address, unsigned long offset);
static void slot_write_ushort(unsigned short value, void *address, unsigned long offset);
static void slot_write_uint(unsigned int value, void *address, unsigned long offset);
static int slot_write_vector(unsigned char vector, struct slot_id *slot_id, slot_space space, unsigned long offset);

static int carrier_open(struct inode *inode, struct file *filp);
static int carrier_release(struct inode *inode, struct file *filp);
static int carrier_install_all(void);

struct file_operations carrierFops =
{
	.owner   = THIS_MODULE,
	.open    = carrier_open,
	.release = carrier_release
};


MODULE_DESCRIPTION(CARRIER_DRIVER_DESCRIPTION);
MODULE_AUTHOR(CARRIER_DRIVER_AUTHOR);
MODULE_LICENSE(CARRIER_DRIVER_LICENSE);
MODULE_VERSION(CARRIER_DRIVER_VERSION);
MODULE_VERSION(GIT_VERSION);

/** TPCI200 controls registers */
static int control_reg[] = {
	TPCI200_CONTROL_A_REG, TPCI200_CONTROL_B_REG, TPCI200_CONTROL_C_REG, TPCI200_CONTROL_D_REG
};

static unsigned int nb_installed_boards = 0;
static struct carrier_board* carrier_boards = NULL;


/**
 * carrier_register - Initialisation of the TPCI200 board.
 *
 * @carrier		Base structure of the board.
 * @dev_specific	Specific device informations.
 *
 */
static int carrier_register(struct carrier_board *carrier, void *dev_specific)
{
	int  i;
	int res = 0;
	struct params_pci *pci_params = (struct params_pci *) dev_specific;
	struct carrier_infos *pci_carrier = NULL;
	unsigned long ioidint_base = 0;
	unsigned long mem_base = 0;
	unsigned short slot_ctrl;

	/* Check if carrier is already registered */
	if (carrier->carrier_specific != NULL) {
		printk(KERN_ERR PFX "Carrier number %d already registered !",carrier->carrier_number);
		res = -EINVAL;
		goto out_err;
	}

	/* Test the Vendor ID, device ID, .... */
	if ((pci_params->vendor_id != TPCI200_VENDOR_ID) ||
			(pci_params->device_id != TPCI200_DEVICE_ID) ||
			(pci_params->subvendor_id != TPCI200_SUBVENDOR_ID) ||
			(pci_params->subdevice_id != TPCI200_SUBDEVICE_ID)) {
		printk(KERN_ERR PFX "Carrier %s : VendorId 0x%X, DeviceId 0x%X, SubVendorId 0x%X, SubDeviceID 0x%X not supported !",
				TPCI200_SHORTNAME,
				pci_params->vendor_id, pci_params->device_id, pci_params->subvendor_id,
				pci_params->subdevice_id);
		printk(KERN_ERR PFX "VendorId 0x%X, DeviceId 0x%X, SubVendorId 0x%X, SubDeviceID 0x%X only supported !",
				TPCI200_VENDOR_ID, TPCI200_DEVICE_ID, TPCI200_SUBVENDOR_ID,
				TPCI200_SUBDEVICE_ID);
		res = -EINVAL;
		goto out_err;
	}

	pci_carrier = (struct carrier_infos*) kzalloc(sizeof(struct carrier_infos), GFP_KERNEL);
	if (pci_carrier == NULL) {
		printk(KERN_ERR PFX"(Carrier %d) Unable to allocate memory for new carrier !", carrier->carrier_number);
		res = -ENOMEM;
		goto out_err;
	}

	/* Try to find the device */
	pci_carrier->pci_dev = NULL;
	do {
		pci_carrier->pci_dev = pci_get_subsys(TPCI200_VENDOR_ID, TPCI200_DEVICE_ID,
				TPCI200_SUBVENDOR_ID, TPCI200_SUBDEVICE_ID,
				pci_carrier->pci_dev);
		if (pci_carrier->pci_dev){
			if ((pci_carrier->pci_dev->bus->number == pci_params->bus_number) &&
					(PCI_SLOT(pci_carrier->pci_dev->devfn) == pci_params->slot_number)){
				break;
			}
		}
	} while (pci_carrier->pci_dev);

	if (!(pci_carrier->pci_dev)) {
		printk(KERN_ERR PFX "The Carrier %s (bn 0x%X, sn 0x%X, vid 0x%X, did 0x%X, svid 0x%X, sdid 0x%X) not found !",
				TPCI200_SHORTNAME,
				pci_params->bus_number, pci_params->slot_number,
				TPCI200_VENDOR_ID, TPCI200_DEVICE_ID,
				TPCI200_SUBVENDOR_ID, TPCI200_SUBDEVICE_ID);
		res = -ENXIO;
		goto out_free;
	}

	res = pci_enable_device(pci_carrier->pci_dev);
	if (res) {
		printk(KERN_ERR PFX "Could not enable Carrier %s (bn 0x%X, sn 0x%X, vid 0x%X, did 0x%X, svid 0x%X, sdid 0x%X) failed to enable PCI device !",
				TPCI200_SHORTNAME,
				pci_params->bus_number, pci_params->slot_number,
				TPCI200_VENDOR_ID, TPCI200_DEVICE_ID,
				TPCI200_SUBVENDOR_ID, TPCI200_SUBDEVICE_ID);
		goto out_free_pci;
	}

	/* Request IP interface register (Bar 2) */
	res = pci_request_region(pci_carrier->pci_dev, TPCI200_IP_INTERFACE_BAR, "Carrier IP interface registers");
	if (res) {
		printk(KERN_ERR PFX "The Carrier %s (bn 0x%X, sn 0x%X, vid 0x%X, did 0x%X, svid 0x%X, sdid 0x%X) failed to allocate PCI resource for BAR 2 !",
				TPCI200_SHORTNAME,
				pci_params->bus_number, pci_params->slot_number,
				TPCI200_VENDOR_ID, TPCI200_DEVICE_ID,
				TPCI200_SUBVENDOR_ID, TPCI200_SUBDEVICE_ID);
		goto out_disable_pci;
	}

	/* Request IO ID INT space (Bar 3) */
	res = pci_request_region(pci_carrier->pci_dev, TPCI200_IO_ID_INT_SPACES_BAR, "Carrier IO ID INT space");
	if (res) {
		printk(KERN_ERR PFX "The Carrier %s (bn 0x%X, sn 0x%X, vid 0x%X, did 0x%X, svid 0x%X, sdid 0x%X) failed to allocate PCI resource for BAR 3 !",
				TPCI200_SHORTNAME,
				pci_params->bus_number, pci_params->slot_number,
				TPCI200_VENDOR_ID, TPCI200_DEVICE_ID,
				TPCI200_SUBVENDOR_ID, TPCI200_SUBDEVICE_ID);
		goto out_release_ip_space;
	}

	/* Request MEM space (Bar 4) */
	res = pci_request_region(pci_carrier->pci_dev, TPCI200_MEM8_SPACE_BAR, "Carrier MEM space");
	if (res) {
		printk(KERN_ERR PFX "The Carrier %s (bn 0x%X, sn 0x%X, vid 0x%X, did 0x%X, svid 0x%X, sdid 0x%X) failed to allocate PCI resource for BAR %d !",
				TPCI200_SHORTNAME,
				pci_params->bus_number, pci_params->slot_number,
				TPCI200_VENDOR_ID, TPCI200_DEVICE_ID,
				TPCI200_SUBVENDOR_ID, TPCI200_SUBDEVICE_ID, TPCI200_MEM8_SPACE_BAR);
		goto out_release_ioid_int_space;
	}

	/* Map internal carrier driver user space */
	pci_carrier->interface_regs = ioremap(pci_resource_start(pci_carrier->pci_dev, TPCI200_IP_INTERFACE_BAR), TPCI200_IFACE_SIZE);
	pci_carrier->ioidint_space = ioremap(pci_resource_start(pci_carrier->pci_dev, TPCI200_IO_ID_INT_SPACES_BAR), TPCI200_IOIDINT_SIZE);
	pci_carrier->mem8_space = ioremap(pci_resource_start(pci_carrier->pci_dev, TPCI200_MEM8_SPACE_BAR), TPCI200_MEM8_SIZE);

	spin_lock_init(&pci_carrier->access_lock);
	carrier->carrier_specific = (void *) pci_carrier;
	ioidint_base = pci_resource_start(pci_carrier->pci_dev, TPCI200_IO_ID_INT_SPACES_BAR);
	mem_base = pci_resource_start(pci_carrier->pci_dev, TPCI200_MEM8_SPACE_BAR);

	/* Set the default parameters of the slot
	 * INT0 disabled, level sensitive
	 * INT1 disabled, level sensitive
	 * error interrupt disabled
	 * timeout interrupt disabled
	 * recover time disabled
	 * clock rate 8 MHz
	 */
	slot_ctrl = 0;

	/* Set all slot physical address space */
	for (i=0; i<TPCI200_NB_SLOT; i++) {
		carrier->slots[i].io_phys.address = (void*)(ioidint_base + TPCI200_IO_SPACE_OFF + TPCI200_IO_SPACE_GAP*i);
		carrier->slots[i].io_phys.size = TPCI200_IO_SPACE_SIZE;
		carrier->slots[i].id_phys.address = (void*)(ioidint_base + TPCI200_ID_SPACE_OFF + TPCI200_ID_SPACE_GAP*i);
		carrier->slots[i].id_phys.size = TPCI200_ID_SPACE_SIZE;
		carrier->slots[i].mem_phys.address = (void*)(mem_base + TPCI200_MEM8_GAP*i);
		carrier->slots[i].mem_phys.size = TPCI200_MEM8_SIZE;

		writew(slot_ctrl, (unsigned short*)(pci_carrier->interface_regs + control_reg[i]));
	}

	res = request_irq(pci_carrier->pci_dev->irq, carrier_interrupt, IRQF_SHARED, TPCI200_SHORTNAME, (void *) carrier);
	if (res) {
		printk(KERN_ERR PFX "The Carrier %s (bn 0x%X, sn 0x%X, vid 0x%X, did 0x%X, svid 0x%X, sdid 0x%X) unable to register IRQ !",
				TPCI200_SHORTNAME,
				pci_params->bus_number, pci_params->slot_number,
				TPCI200_VENDOR_ID, TPCI200_DEVICE_ID,
				TPCI200_SUBVENDOR_ID, TPCI200_SUBDEVICE_ID);
		carrier_unregister(carrier);
		goto out_err;
	}

	return 0;

out_release_ioid_int_space :
	pci_release_region(pci_carrier->pci_dev, TPCI200_IO_ID_INT_SPACES_BAR);
out_release_ip_space :
	pci_release_region(pci_carrier->pci_dev, TPCI200_IP_INTERFACE_BAR);
out_disable_pci :
	pci_disable_device(pci_carrier->pci_dev);
out_free_pci :
	pci_dev_put(pci_carrier->pci_dev);
out_free :
	kfree(pci_carrier);
out_err :
	return res;
}

/**
 * carrier_unregister - Unregister a TPCI200 board.
 *
 * @carrier	Base structure of the board.
 *
 */
static void carrier_unregister(struct carrier_board *carrier)
{
	int i;
	struct carrier_infos *pci_carrier = (struct carrier_infos *) carrier->carrier_specific;

	free_irq(pci_carrier->pci_dev->irq, (void *) carrier);
	carrier->carrier_specific = NULL;

	pci_iounmap(pci_carrier->pci_dev, pci_carrier->interface_regs);
	pci_iounmap(pci_carrier->pci_dev, pci_carrier->ioidint_space);
	pci_iounmap(pci_carrier->pci_dev, pci_carrier->mem8_space);

	pci_release_region(pci_carrier->pci_dev, TPCI200_IP_INTERFACE_BAR);
	pci_release_region(pci_carrier->pci_dev, TPCI200_IO_ID_INT_SPACES_BAR);
	pci_release_region(pci_carrier->pci_dev, TPCI200_MEM8_SPACE_BAR);

	pci_disable_device(pci_carrier->pci_dev);
	pci_dev_put(pci_carrier->pci_dev);

	kfree(pci_carrier);

	for (i=0; i<TPCI200_NB_SLOT; i++) {
		carrier->slots[i].io_phys.address = NULL;
		carrier->slots[i].io_phys.size = 0;
		carrier->slots[i].id_phys.address = NULL;
		carrier->slots[i].id_phys.size = 0;
		carrier->slots[i].mem_phys.address = NULL;
		carrier->slots[i].mem_phys.size = 0;
	}
}

static int slot_request_irq(struct carrier_board *carrier, struct slot_id *slot_id)
{
	unsigned short slot_ctrl;
	struct carrier_infos *pci_carrier = (struct carrier_infos *) carrier->carrier_specific;

	/* Set the default parameters of the slot
	 * INT0 enabled, level sensitive
	 * INT1 enabled, level sensitive
	 * error interrupt disabled
	 * timeout interrupt disabled
	 * recover time disabled
	 * clock rate 8 MHz
	 */
	slot_ctrl = TPCI200_INT0_EN | TPCI200_INT1_EN;
	writew(slot_ctrl, (unsigned short*)(pci_carrier->interface_regs + control_reg[slot_id->slot_position]));

	return 0;
}

static void slot_free_irq(struct carrier_board *carrier, struct slot_id *slot_id)
{
	unsigned short slot_ctrl;
	struct carrier_infos *pci_carrier = (struct carrier_infos *) carrier->carrier_specific;

	/* Set the default parameters of the slot
	 * INT0 disabled, level sensitive
	 * INT1 disabled, level sensitive
	 * error interrupt disabled
	 * timeout interrupt disabled
	 * recover time disabled
	 * clock rate 8 MHz
	 */
	slot_ctrl = 0;
	writew(slot_ctrl, (unsigned short*)(pci_carrier->interface_regs + control_reg[slot_id->slot_position]));
}

static unsigned char slot_read_uchar(void *address, unsigned long offset)
{
	unsigned char value;

	value = ioread8(address + (offset^1));
	return value;
}

static unsigned short slot_read_ushort(void *address, unsigned long offset)
{
	unsigned short value;

	value = ioread16(address + offset);
	return value;
}

static unsigned int slot_read_uint(void *address, unsigned long offset)
{
	unsigned int value;

	value = swahw32(ioread32(address + offset));
	return value;
}

static void slot_write_uchar(unsigned char value, void *address, unsigned long offset)
{
	iowrite8(value, address+(offset^1));
}

static void slot_write_ushort(unsigned short value, void *address, unsigned long offset)
{
	iowrite16(value, address+offset);
}

static void slot_write_uint(unsigned int value, void *address, unsigned long offset)
{
	iowrite32(swahw32(value), address+offset);
}

static int slot_write_vector(unsigned char vector, struct slot_id *slot_id, slot_space space, unsigned long offset)
{
	if(space == SLOT_MEM_SPACE)
		return ip_slot_write_uchar(vector, slot_id, space, offset);

	return 0;
}

/**
 * carrier_interrupt - Interrupt handler of the TPCI200 carrier
 *
 * @irq		PCI device interrupt line
 * @dev_id	Parameters passed to the handler.
 *
 */
static irqreturn_t carrier_interrupt(int irq, void *dev_id)
{
	struct carrier_board *carrier = (struct carrier_board *) dev_id;
	struct carrier_infos *pci_carrier = (struct carrier_infos *) carrier->carrier_specific;
	int i;
	unsigned short status_reg, reg_value;
	unsigned short unhandled_ints = 0;
	irqreturn_t ret = IRQ_NONE;

	unsigned short dummy;

	spin_lock(&pci_carrier->access_lock);

	status_reg = readw((unsigned short *)(pci_carrier->interface_regs + TPCI200_STATUS_REG));
	if (status_reg & TPCI200_SLOT_INT_MASK) {
		unhandled_ints = status_reg & TPCI200_SLOT_INT_MASK;

		for (i=0 ; i<TPCI200_NB_SLOT ; i++){
			if ((carrier->slots[i].irq != NULL) &&
					(status_reg & ((TPCI200_A_INT0 | TPCI200_A_INT1) << (2*i)))){
				(carrier->slots[i].irq->handler)(carrier->slots[i].irq->arg);

				dummy = readw((unsigned short *)(carrier->slots[i].slot_id->io_space.address + 0xC0));
				dummy = readw((unsigned short *)(carrier->slots[i].slot_id->io_space.address + 0xC2));
	
				unhandled_ints &= ~(((TPCI200_A_INT0 | TPCI200_A_INT1) << (2*i)));
				ret = IRQ_HANDLED;
			}
		}
	}

	if (unhandled_ints){
		for (i=0 ; i<TPCI200_NB_SLOT ; i++){
			if (unhandled_ints & ((TPCI200_INT0_EN | TPCI200_INT1_EN) << (2*i))){
				printk(KERN_ERR PFX "No registered ISR for interrupt on slot [%s %d:%d]!. Interrupt will be disabled.\n",
						TPCI200_SHORTNAME,
						carrier->carrier_number, i);
				reg_value = readw((unsigned short*)(pci_carrier->interface_regs + control_reg[i]));
				reg_value &= ~(TPCI200_INT0_EN | TPCI200_INT1_EN);
				writew(reg_value, (unsigned short*)(pci_carrier->interface_regs + control_reg[i]));
			}
		}
	}

	spin_unlock(&pci_carrier->access_lock);
	return ret;
}

/**
 * check_slot - Check if a slot exist and is registered.
 *
 * @carrier_number	Carrier on which slot to test
 * @slot_position	Position on the carrier
 *
 */
static struct carrier_board *check_slot(struct slot_id *slot_id)
{
	struct carrier_board *carrier = NULL;

	if (slot_id == NULL){
		printk(KERN_INFO PFX "Slot doesn't exist.\n");
		return NULL;
	}

	if (slot_id->carrier_number >= nb_installed_boards) {
		printk(KERN_INFO PFX "The Carrier(nb. %d) of this slot doesn't exist ! Last Carrier is nb.%d\n", slot_id->carrier_number, nb_installed_boards-1);
		return NULL;
	}

	carrier = &carrier_boards[slot_id->carrier_number];

	if (carrier->carrier_specific == NULL) {
		printk(KERN_INFO PFX "Carrier [%d] not registered!\n", slot_id->carrier_number);
	}

	if (slot_id->slot_position >= TPCI200_NB_SLOT) {
		printk(KERN_INFO PFX "Slot [%s %d:%d] doesn't exist! Last carrier slot is %d.\n",
				TPCI200_SHORTNAME,
				slot_id->carrier_number, slot_id->slot_position,
				TPCI200_NB_SLOT-1);
		return NULL;
	}

	if (carrier->slots[slot_id->slot_position].slot_id == NULL) {
		printk(KERN_INFO PFX "Slot [%s %d:%d] is not registered !\n",
				TPCI200_SHORTNAME,
				slot_id->carrier_number, slot_id->slot_position);
		return NULL;
	}

	return carrier;
}

/**
 * ip_slot_register - Register a new IP Slot,\n
 *
 * @board_name		Name of the slot board
 * @carrier_number	Carrier board number (0..N)
 * @slot_position	Position on the carrier(0..N)
 *
 */
struct slot_id* ip_slot_register(char* board_name, unsigned int carrier_number,
		unsigned int slot_position)
{
	struct slot_id  *slot_id = NULL;
	struct carrier_board *carrier = NULL;

	printk(KERN_INFO PFX "Register slot [%d:%d] request...\n", carrier_number, slot_position);

	if (carrier_number >= nb_installed_boards) {
		printk(KERN_ERR PFX "Carrier nb.%d doesn't exist! Last carrier is %d.\n", carrier_number, nb_installed_boards-1);
		return NULL;
	}

	carrier = &carrier_boards[carrier_number];

	if (slot_position >= TPCI200_NB_SLOT){
		printk(KERN_ERR PFX "Slot [%s %d:%d] doesn't exist! Last carrier slot is %d.\n",
				TPCI200_SHORTNAME,
				carrier_number, slot_position,
				TPCI200_NB_SLOT-1);
		goto out;
	}

	if (mutex_lock_interruptible(&carrier->carrier_mutex)){
		goto out;
	}

	if (carrier->slots[slot_position].slot_id != NULL) {
		printk(KERN_ERR PFX "Slot [%s %d:%d] already installed !\n",
				TPCI200_SHORTNAME,
				carrier_number, slot_position);
		goto out_unlock;
	}

	slot_id = (struct slot_id*) kzalloc(sizeof(struct slot_id), GFP_KERNEL);
	if (slot_id == NULL) {
		printk(KERN_ERR PFX "Slot [%s %d:%d] Unable to allocate memory for new slot !\n",
				TPCI200_SHORTNAME,
				carrier_number, slot_position);
		goto out_unlock;
	}

	if (strlen(board_name) > SLOT_BOARD_NAME_SIZE) {
		printk(KERN_WARNING PFX "Slot [%s %d:%d] name (%s) too long (%zd char > %d char MAX). Will be truncated!\n",
				TPCI200_SHORTNAME,
				carrier_number, slot_position,
				board_name, strlen(board_name), SLOT_BOARD_NAME_SIZE);
	}

	strncpy(slot_id->board_name, board_name, SLOT_BOARD_NAME_SIZE-1);
	slot_id->carrier_number = carrier_number;
	slot_id->slot_position = slot_position;
	slot_id->id_space.address = NULL;
	slot_id->id_space.size = 0;
	slot_id->io_space.address = NULL;
	slot_id->io_space.size = 0;
	slot_id->mem_space.address = NULL;
	slot_id->mem_space.size = 0;

	carrier->slots[slot_position].slot_id = slot_id;

	printk(KERN_INFO PFX "Slot registered.");

out_unlock:
	mutex_unlock(&carrier->carrier_mutex);
out:
	return slot_id;
}
EXPORT_SYMBOL(ip_slot_register);

/**
 * ip_slot_unregister - Unregister a IP Slot.
 *
 * @slot_id	Slot to unregister
 *
 */
int ip_slot_unregister(struct slot_id *slot_id)
{
	struct carrier_board *carrier = NULL;

	if (slot_id != NULL)
		printk(KERN_INFO PFX "Removing slot [%d:%d] request...\n", slot_id->carrier_number, slot_id->slot_position);

	carrier = check_slot(slot_id);
	if (carrier == NULL){
		return -EINVAL;
	}

	ip_slot_free_irq(slot_id);

	ip_slot_unmap_space(slot_id, SLOT_IO_SPACE);
	ip_slot_unmap_space(slot_id, SLOT_ID_SPACE);
	ip_slot_unmap_space(slot_id, SLOT_MEM_SPACE);

	if (mutex_lock_interruptible(&carrier->carrier_mutex)){
		return -ERESTARTSYS;
	}

	carrier_boards[slot_id->carrier_number].slots[slot_id->slot_position].slot_id = NULL;
	kfree(slot_id);
	mutex_unlock(&carrier->carrier_mutex);

	printk(KERN_INFO PFX "Slot removed.\n");

	return 0;
}
EXPORT_SYMBOL(ip_slot_unregister);

/**
 * @brief Map an IP Slot specific space.
 *
 * @slot_id		Slot to map space
 * @memory_size		Memory _size needed by the IP module.
 * 			This parameter is only used to map memory space
 *			For other space size is hardware fixed
 * @space		Specific space to map
 *
 * If space address is NULL after mapping it's because space size is 0 or an error occured.
 */
int ip_slot_map_space(struct slot_id *slot_id, unsigned int memory_size, slot_space space)
{
	int res = 0;
	unsigned int size_to_map = 0;
	void *phys_address = NULL;
	struct slot_addr_space *virt_addr_space = NULL;
	struct carrier_board *carrier = NULL;

	if (slot_id != NULL)
		printk(KERN_INFO PFX "Mapping space for slot [%d:%d] request...\n", slot_id->carrier_number, slot_id->slot_position);

	carrier = check_slot(slot_id);
	if (carrier == NULL){
		res = -EINVAL;
		goto out;
	}

	if (mutex_lock_interruptible(&carrier->carrier_mutex)){
		res = -ERESTARTSYS;
		goto out;
	}

	switch (space){
		case SLOT_IO_SPACE :
			printk(KERN_INFO PFX "IO space mapping...\n");
			if (slot_id->io_space.address != NULL) {
				printk(KERN_ERR PFX "Slot [%s %d:%d] IO space already mapped !\n",
						TPCI200_SHORTNAME,
						carrier->carrier_number, slot_id->slot_position);
				res = -EINVAL;
				goto out_unlock;
			}
			virt_addr_space = &slot_id->io_space;

			phys_address = carrier->slots[slot_id->slot_position].io_phys.address;
			size_to_map = carrier->slots[slot_id->slot_position].io_phys.size;
			break;
		case SLOT_ID_SPACE :
			printk(KERN_INFO PFX "ID space mapping...\n");
			if (slot_id->id_space.address != NULL) {
				printk(KERN_ERR PFX "Slot [%s %d:%d] ID space already mapped !\n",
						TPCI200_SHORTNAME,
						carrier->carrier_number, slot_id->slot_position);
				res = -EINVAL;
				goto out_unlock;
			}
			virt_addr_space = &slot_id->id_space;

			phys_address = carrier->slots[slot_id->slot_position].id_phys.address;
			size_to_map = carrier->slots[slot_id->slot_position].id_phys.size;
			break;
		case SLOT_MEM_SPACE :
			printk(KERN_INFO PFX "MEM space mapping...");
			if (slot_id->mem_space.address != NULL) {
				printk(KERN_ERR PFX "Slot [%s %d:%d] MEM space already mapped !\n",
						TPCI200_SHORTNAME,
						carrier->carrier_number, slot_id->slot_position);
				res = -EINVAL;
				goto out_unlock;
			}
			virt_addr_space = &slot_id->mem_space;

			if (memory_size > carrier->slots[slot_id->slot_position].mem_phys.size){
				printk(KERN_ERR PFX "Slot [%s %d:%d] request is 0x%X memory, only 0x%X available !\n",
						TPCI200_SHORTNAME,
						slot_id->carrier_number,
						slot_id->slot_position,
						memory_size,
						carrier->slots[slot_id->slot_position].mem_phys.size);
				res = -EINVAL;
				goto out_unlock;
			}

			phys_address = carrier->slots[slot_id->slot_position].mem_phys.address;
			size_to_map = memory_size;
			break;
		default :
			printk(KERN_ERR PFX "Slot [%s %d:%d] space %d doesn't exist !\n",
					TPCI200_SHORTNAME,
					carrier->carrier_number, slot_id->slot_position, space);
			res = -EINVAL;
			goto out_unlock;
			break;
	}

	virt_addr_space->size = size_to_map;
	virt_addr_space->address = ioremap((unsigned long)phys_address, size_to_map);

	printk(KERN_INFO PFX "_size=0x%x mapped to 0x%p.\n", size_to_map, virt_addr_space->address);

out_unlock:
	mutex_unlock(&carrier->carrier_mutex);
out:
	return res;
}
EXPORT_SYMBOL(ip_slot_map_space);

/**
 * ip_slot_unmap_space - Unmap an IP Slot specific space.
 *
 * @slot_id	Slot to map space
 * @space	Specific space to unmap
 *
 */
int ip_slot_unmap_space(struct slot_id *slot_id, slot_space space)
{
	int res = 0;
	struct slot_addr_space *virt_addr_space = NULL;
	struct carrier_board *carrier = NULL;

	if (slot_id != NULL)
		printk(KERN_INFO PFX "Unmapping space for slot [%d:%d] request...\n", slot_id->carrier_number, slot_id->slot_position);

	carrier = check_slot(slot_id);
	if (carrier == NULL){
		res = -EINVAL;
		goto out;
	}

	if (mutex_lock_interruptible(&carrier->carrier_mutex)){
		res = -ERESTARTSYS;
		goto out;
	}

	switch (space){
		case SLOT_IO_SPACE :
			printk(KERN_INFO PFX "IO space unmapping...\n");
			if (slot_id->io_space.address == NULL){
				printk(KERN_INFO PFX "Slot [%s %d:%d] IO space not mapped !\n",
						TPCI200_SHORTNAME,
						slot_id->carrier_number, slot_id->slot_position);
				goto out_unlock;
			}
			virt_addr_space = &slot_id->io_space;
			break;
		case SLOT_ID_SPACE :
			printk(KERN_INFO PFX "ID space unmapping...\n");
			if (slot_id->id_space.address == NULL){
				printk(KERN_INFO PFX "Slot [%s %d:%d] ID space not mapped !\n",
						TPCI200_SHORTNAME,
						slot_id->carrier_number, slot_id->slot_position);
				goto out_unlock;
			}
			virt_addr_space = &slot_id->id_space;
			break;
		case SLOT_MEM_SPACE :
			printk(KERN_INFO PFX "MEM space unmapping...\n");
			if (slot_id->mem_space.address == NULL){
				printk(KERN_INFO PFX "Slot [%s %d:%d] MEM space not mapped !\n",
						TPCI200_SHORTNAME,
						slot_id->carrier_number, slot_id->slot_position);
				goto out_unlock;
			}
			virt_addr_space = &slot_id->mem_space;
			break;
		default :
			printk(KERN_ERR PFX "Slot [%s %d:%d] space number %d doesn't exist !\n",
					TPCI200_SHORTNAME,
					slot_id->carrier_number, slot_id->slot_position, space);
			res = -EINVAL;
			goto out_unlock;
			break;
	}

	iounmap(virt_addr_space->address);

	virt_addr_space->address = NULL;
	virt_addr_space->size = 0;

	printk(KERN_INFO PFX "Space unmapped.\n");

out_unlock:
	mutex_unlock(&carrier->carrier_mutex);
out:
	return res;
}
EXPORT_SYMBOL(ip_slot_unmap_space);

/**
 * ip_slot_request_irq - Install the IRQ for the slot
 *
 * @slot_id	Slot to install IRQ
 * @vector	Interrupt vector
 * @handler	Interrupt handler
 * @arg		Interrupt handler argument
 * @name	Interrupt name
 *
 * WARNING: Setup Interrupt Vector in the IndustryPack device before an IRQ request.
 * This is necessary for VMEbus carrier interrupt identification.
 * Read the User Manual of your IndustryPack device to know where writing the vector in memory.
 *
 */
int ip_slot_request_irq(struct slot_id *slot_id, int vector, int (*handler)(void *),
		void *arg, char *name)
{
	int res = 0;
	struct slot_irq *slot_irq = NULL;
	struct carrier_board *carrier = NULL;

	if (slot_id != NULL)
		printk(KERN_INFO PFX "IRQ request for slot [%d:%d]...\n", slot_id->carrier_number, slot_id->slot_position);

	carrier = check_slot(slot_id);
	if (carrier == NULL){
		res = -EINVAL;
		goto out;
	}

	if (mutex_lock_interruptible(&carrier->carrier_mutex)){
		res = -ERESTARTSYS;
		goto out;
	}

	if (carrier->slots[slot_id->slot_position].irq != NULL) {
		printk(KERN_ERR PFX "Slot [%s %d:%d] IRQ already registered !\n",
				TPCI200_SHORTNAME,
				slot_id->carrier_number, slot_id->slot_position);
		res = -EINVAL;
		goto out_unlock;
	}

	slot_irq = (struct slot_irq*) kzalloc(sizeof(struct slot_irq), GFP_KERNEL);
	if (slot_irq == NULL) {
		printk(KERN_ERR PFX "Slot [%s %d:%d] unable to allocate memory for IRQ !\n",
				TPCI200_SHORTNAME,
				slot_id->carrier_number, slot_id->slot_position);
		res = -ENOMEM;
		goto out_unlock;
	}

	slot_irq->vector = vector;
	slot_irq->handler = handler;
	slot_irq->arg = arg;
	if (name){
		if (strlen(name) > SLOT_IRQ_NAME_SIZE) {
			printk(KERN_WARNING PFX "Slot [%s %d:%d] IRQ name (%s) too long (%zd char > %d char MAX). Will be truncated!\n",
					TPCI200_SHORTNAME,
					slot_id->carrier_number, slot_id->slot_position,
					name, strlen(name), SLOT_IRQ_NAME_SIZE);
		}
		strncpy(slot_irq->name, name, SLOT_IRQ_NAME_SIZE-1);
	}
	else {
		strcpy(slot_irq->name, "Unknown");
	}

	carrier->slots[slot_id->slot_position].irq = slot_irq;
	res = slot_request_irq(carrier, slot_id);
	printk(KERN_ERR PFX "IRQ registered.\n");

out_unlock:
	mutex_unlock(&carrier->carrier_mutex);
out:
	return res;
}
EXPORT_SYMBOL(ip_slot_request_irq);

/**
 * ip_slot_free_irq - Free the IRQ of the slot
 *
 * @slot_id - Slot to uninstall IRQ
 *
 */
int ip_slot_free_irq(struct slot_id *slot_id)
{
	int res = 0;
	struct slot_irq *slot_irq = NULL;
	struct carrier_board *carrier = NULL;

	if (slot_id != NULL)
		printk(KERN_INFO PFX "Free IRQ request for slot [%d:%d]...\n", slot_id->carrier_number, slot_id->slot_position);

	carrier = check_slot(slot_id);
	if (carrier == NULL){
		res = -EINVAL;
		goto out;
	}

	if (mutex_lock_interruptible(&carrier->carrier_mutex)){
		res = -ERESTARTSYS;
		goto out;
	}

	if (carrier->slots[slot_id->slot_position].irq == NULL) {
		printk(KERN_INFO PFX "Slot [%s %d:%d] unable to free unregistered IRQ :o)!\n",
				TPCI200_SHORTNAME,
				slot_id->carrier_number, slot_id->slot_position);
		res = -EINVAL;
		goto out_unlock;
	}

	slot_free_irq(carrier, slot_id);
	slot_irq = carrier->slots[slot_id->slot_position].irq;
	carrier->slots[slot_id->slot_position].irq = NULL;

	kfree(slot_irq);

	printk(KERN_INFO PFX "IRQ unregistered.\n");

out_unlock:
	mutex_unlock(&carrier->carrier_mutex);
out:
	return res;
}
EXPORT_SYMBOL(ip_slot_free_irq);

struct slot_addr_space *get_slot_address_space(struct slot_id *slot_id, slot_space space)
{
	struct slot_addr_space *addr;

	switch (space){
		case SLOT_IO_SPACE :
			addr = &slot_id->io_space;
			break;
		case SLOT_ID_SPACE :
			addr = &slot_id->id_space;
			break;
		case SLOT_MEM_SPACE :
			addr = &slot_id->mem_space;
			break;
		default :
			printk(KERN_ERR PFX "Slot [%s %d:%d] space number %d doesn't exist !\n",
					TPCI200_SHORTNAME,
					slot_id->carrier_number, slot_id->slot_position, space);
			return NULL;
			break;
	}

	if ((addr->size == 0) || (addr->address == NULL)) {
		printk(KERN_ERR PFX "Error, slot space not mapped !\n");
		return NULL;
	}

	return addr;
}

/**
 * ip_slot_read_uchar - Read an unsigned char value on space with offset.
 *
 * @value	Read value
 * @slot_id	Slot to read
 * @space	Space of the slot to read
 * @offset	Offset
 *
 */
int ip_slot_read_uchar(unsigned char *value, struct slot_id *slot_id, slot_space space, unsigned long offset)
{
	struct slot_addr_space *slot_addr_space = NULL;
	struct carrier_board *carrier = NULL;

	carrier = check_slot(slot_id);
	if (carrier == NULL){
		return -EINVAL;
	}

	slot_addr_space = get_slot_address_space(slot_id, space);
	if (slot_addr_space == NULL)
		return -EINVAL;

	if (offset >= slot_addr_space->size) {
		printk(KERN_ERR PFX "Error, slot space offset error !\n");
		return -EFAULT;
	}

	*value = slot_read_uchar(slot_addr_space->address, offset);

	return 0;
}
EXPORT_SYMBOL(ip_slot_read_uchar);

/**
 * ip_slot_read_ushort - Read an unsigned short value on space with offset.
 *
 * @value	Read value
 * @slot_id	Slot to read
 * @space	Space of the slot to read
 * @offset	Offset
 *
 */
int ip_slot_read_ushort(unsigned short *value, struct slot_id *slot_id, slot_space space, unsigned long offset)
{
	struct slot_addr_space *slot_addr_space = NULL;
	struct carrier_board *carrier = NULL;

	carrier = check_slot(slot_id);
	if (carrier == NULL){
		return -EINVAL;
	}

	slot_addr_space = get_slot_address_space(slot_id, space);
	if (slot_addr_space == NULL)
		return -EINVAL;

	if ((offset+2) >= slot_addr_space->size) {
		printk(KERN_ERR PFX "Error, slot space offset error !\n");
		return -EFAULT;
	}
	*value = slot_read_ushort(slot_addr_space->address, offset);

	return 0;
}
EXPORT_SYMBOL(ip_slot_read_ushort);

/**
 * ip_slot_read_uint - Read an unsigned int value on space with offset.
 *
 * @value	Read value
 * @slot_id	Slot to read
 * @space	Space of the slot to read
 * @offset	Offset
 *
 */
int ip_slot_read_uint(unsigned int *value, struct slot_id *slot_id, slot_space space, unsigned long offset)
{
	struct slot_addr_space *slot_addr_space = NULL;
	struct carrier_board *carrier = NULL;

	carrier = check_slot(slot_id);
	if (carrier == NULL){
		return -EINVAL;
	}

	slot_addr_space = get_slot_address_space(slot_id, space);
	if (slot_addr_space == NULL)
		return -EINVAL;

	if ((offset+4) >= slot_addr_space->size) {
		printk(KERN_ERR PFX "Error, slot space offset error !\n");
		return -EFAULT;
	}

	*value = slot_read_uint(slot_addr_space->address, offset);

	return 0;
}
EXPORT_SYMBOL(ip_slot_read_uint);

/**
 * ip_slot_write_uchar Write an unsigned char value on space with offset.
 *
 * @value	Value to write
 * @slot_id	Slot to write
 * @space	Space of the slot to write
 * @offset	Offset
 *
 */
int ip_slot_write_uchar(unsigned char value, struct slot_id *slot_id, slot_space space, unsigned long offset)
{
	struct slot_addr_space *slot_addr_space = NULL;
	struct carrier_board *carrier = NULL;

	carrier = check_slot(slot_id);
	if (carrier == NULL){
		return -EINVAL;
	}

	slot_addr_space = get_slot_address_space(slot_id, space);
	if (slot_addr_space == NULL)
		return -EINVAL;

	if (offset >= slot_addr_space->size) {
		printk(KERN_ERR PFX "Error, slot space offset error !\n");
		return -EFAULT;
	}

	slot_write_uchar(value, slot_addr_space->address, offset);

	return 0;
}
EXPORT_SYMBOL(ip_slot_write_uchar);

/**
 * ip_slot_write_ushort - Write an unsigned short value on space with offset.
 *
 * @value	Value to write
 * @slot_id	Slot to write
 * @space	Space of the slot to write
 * @offset	Offset
 *
 */
int ip_slot_write_ushort(unsigned short value, struct slot_id *slot_id, slot_space space, unsigned long offset)
{
	struct slot_addr_space *slot_addr_space = NULL;
	struct carrier_board *carrier = NULL;

	carrier = check_slot(slot_id);
	if (carrier == NULL){
		return -EINVAL;
	}

	slot_addr_space = get_slot_address_space(slot_id, space);
	if (slot_addr_space == NULL)
		return -EINVAL;

	if ((offset+2) >= slot_addr_space->size) {
		printk(KERN_ERR PFX "Error, slot space offset error !\n");
		return -EFAULT;
	}

	slot_write_ushort(value, slot_addr_space->address, offset);

	return 0;
}
EXPORT_SYMBOL(ip_slot_write_ushort);

/**
 * ip_slot_write_uint Write an unsigned int value on space with offset.
 *
 * @value	Value to write
 * @slot_id	Slot to write
 * @space	Space of the slot to write
 * @offset	Offset
 *
 */
int ip_slot_write_uint(unsigned int value, struct slot_id *slot_id, slot_space space, unsigned long offset)
{
	struct slot_addr_space *slot_addr_space = NULL;
	struct carrier_board *carrier = NULL;

	carrier = check_slot(slot_id);
	if (carrier == NULL){
		return -EINVAL;
	}

	slot_addr_space = get_slot_address_space(slot_id, space);
	if (slot_addr_space == NULL)
		return -EINVAL;

	if ((offset+4) >= slot_addr_space->size) {
		printk(KERN_ERR PFX "Error, slot space offset error !\n");
		return -EFAULT;
	}

	slot_write_uint(value, slot_addr_space->address, offset);

	return 0;
}
EXPORT_SYMBOL(ip_slot_write_uint);

/**
 * ip_slot_write_vector - Write the vector into the specified offset of memory space.
 *
 * @vector	Vector to write
 * @slot_id	Slot to write
 * @space	Space of the slot to write
 * @offset	Offset
 *
 * This function is like ip_slot_write_uchar but work only for VME carrier.\n
 * PCI carrier doesn't need vector value but to have a standard interface use this API
 * to write vector.
 *
 */
int ip_slot_write_vector(unsigned char vector, struct slot_id *slot_id, slot_space space, unsigned long offset)
{
	struct carrier_board *carrier = NULL;

	carrier = check_slot(slot_id);
	if (carrier == NULL){
		return -EINVAL;
	}

	return slot_write_vector(vector, slot_id, space, offset);
}
EXPORT_SYMBOL(ip_slot_write_vector);

static int carrier_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int carrier_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static int __init carrier_drvr_init_module(void)
{

	printk(KERN_INFO PFX "Carrier driver loading...\n");
	carrier_install_all();
	nb_installed_boards = num_lun;
	printk(KERN_INFO PFX "Carrier driver loaded.\n");

	return 0;
}

static void carrierUninstall(struct carrier_board *carrier)
{
	int i;

	if (carrier->carrier_specific != NULL) {
		for (i=0;i<TPCI200_NB_SLOT;i++){
			ip_slot_unregister(carrier->slots[i].slot_id);
		}
		carrier_unregister(carrier);
	}
	if (carrier->slots != NULL) {
		kfree(carrier->slots);
	}
}

static void __exit carrier_drvr_exit_module(void)
{
	int i;

	printk(KERN_INFO PFX "Carrier driver unloading...\n");


	for (i=0;i<nb_installed_boards;i++) {
		carrierUninstall(&carrier_boards[i]);
	}

	nb_installed_boards = 0;
	kfree(carrier_boards);
	printk(KERN_INFO PFX "Carrier driver unloaded.\n");
}

static int carrier_install(struct carrier_board *carrier, void *params, unsigned int carrier_number)
{
	int res = 0;

	printk(KERN_INFO PFX "Carrier TPCI200 number %d installing...\n", carrier_number);

	carrier->carrier_number = carrier_number;
	carrier->carrier_specific = NULL;
	carrier->slots = NULL;
	carrier->slots = (struct carrier_slot*) kzalloc(TPCI200_NB_SLOT * sizeof(struct carrier_slot), GFP_KERNEL);
	if (carrier->slots == NULL) {
		printk(KERN_ERR PFX "Carrier %d, unable to allocate slots memory !\n", carrier->carrier_number);
		res = -ENOMEM;
		goto out_err;
	}

	res = carrier_register(carrier, params);
	if (res){
		printk(KERN_ERR PFX "Carrier %d, unable to init board !\n", carrier->carrier_number);
		goto out_free;
	}

	mutex_init(&carrier->carrier_mutex);
	printk(KERN_INFO PFX "Carrier %s number %d installed.\n", TPCI200_SHORTNAME, carrier_number);

	return 0;

out_free :
	kfree(carrier->slots);
	carrier->slots = NULL;
out_err :
	return res;
}


static int carrier_install_all(void)
{
	struct params_pci params_pci;
	int i,j;
	int res = 0;

	nb_installed_boards = num_lun;

	carrier_boards = (struct carrier_board*) kzalloc(nb_installed_boards * sizeof(struct carrier_board), GFP_KERNEL);
	if (carrier_boards == NULL) {
		printk(KERN_ERR PFX "Unable to allocate carrier boards structure !\n");
		res = -ENOMEM;
		goto out_err;
	}

	memset(&params_pci, 0, sizeof(struct params_pci));
	for(i = 0; i < num_lun; i++) {
		params_pci.bus_number = bus[i];
		params_pci.slot_number = slot[i];
		params_pci.vendor_id = vendor_id[i];
		params_pci.device_id = device_id[i];
		params_pci.subvendor_id = subvendor_id[i];
		params_pci.subdevice_id = subdevice_id[i];

		res = carrier_install(&carrier_boards[i], &params_pci, i);
		if (res) {
			printk(KERN_ERR PFX "Error during carrier install !\n");
			goto out_uninst;
		}
	}; 

	return 0;

out_uninst :
	for (j = 0; j<i; j++){
		carrierUninstall(&carrier_boards[i]);
	}

	nb_installed_boards = 0;
	kfree(carrier_boards);
out_err :
	return res;
}

module_init(carrier_drvr_init_module);
module_exit(carrier_drvr_exit_module);
