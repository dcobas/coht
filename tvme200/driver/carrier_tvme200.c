/**
 * carrier_common.h
 *
 * driver for the carrier TEWS TPCI-200 
 * Copyright (c) 2009 Nicolas Serafini, EIC2 SA
 * Copyright (c) 2010,2011 Samuel Iglesias Gonsalvez <siglesia@cern.ch>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.* Released under the GPL v2. (and only v2, not any later version)
 */

#include <asm/io.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/byteorder/swabb.h>

#include <vmebus.h>
#include <carrier.h>
#include "carrier_common.h"
#include "tvme200.h"

#define CARRIER_DRIVER_DESCRIPTION "Carrier interface driver (PCI-VME)"
#define CARRIER_DRIVER_VERSION "1.0" 
#define CARRIER_DRIVER_AUTHOR "Nicolas Serafini, EIC2 SA"
#define CARRIER_DRIVER_LICENSE "GPL"

#define MAX_CARRIER 16
#define PFX "tvme200: "
#define ENDLIST -1

static int lun[MAX_CARRIER];
static int num_lun;
module_param_array(lun, int, &num_lun, S_IRUGO);

static int mod_address_ioid[MAX_CARRIER];
static int num_address_ioid;
module_param_array(mod_address_ioid, int, &num_address_ioid, S_IRUGO);
static int base_address_ioid[MAX_CARRIER];
static int num_base_address_ioid;
module_param_array(base_address_ioid, int, &num_base_address_ioid, S_IRUGO);
static int data_width_ioid[MAX_CARRIER];
static int num_data_width_ioid;
module_param_array(data_width_ioid, int, &num_data_width_ioid, S_IRUGO);
static int wind_size_ioid[MAX_CARRIER];
static int num_wind_size_ioid;
module_param_array(wind_size_ioid, int, &num_wind_size_ioid, S_IRUGO);

static int mod_address_mem[MAX_CARRIER];
static int num_address_mem;
module_param_array(mod_address_mem, int, &num_address_mem, S_IRUGO);
static int base_address_mem[MAX_CARRIER];
static int num_base_address_mem;
module_param_array(base_address_mem, int, &num_base_address_mem, S_IRUGO);
static int data_width_mem[MAX_CARRIER];
static int num_data_width_mem;
module_param_array(data_width_mem, int, &num_data_width_mem, S_IRUGO);
static int wind_size_mem[MAX_CARRIER];
static int num_wind_size_mem;
module_param_array(wind_size_mem, int, &num_wind_size_mem, S_IRUGO);

/**
 * struct carrier_infos - TVME200 and VIPC626 carrier specific info.
 * @ioid_mapping	inteface with the vmebridge driver for the IoIdInt space
 * @mem_mapping		inteface with the vmebridge driver for the Mem space
 *
 */
struct carrier_infos {
	struct vme_mapping ioid_mapping;
	struct vme_mapping mem_mapping;
};

static int carrier_register(struct carrier_board *carrier, void *dev_specific);
static void carrier_unregister(struct carrier_board *carrier);
static int slot_request_irq(struct carrier_board *carrier, struct slot_id *slot_id);
static void slot_free_irq(struct carrier_board *carrier, struct slot_id *slot_id);
static int set_ioid_params(struct carrier_board *carrier, struct vme_addr_space *ioid_space, struct vme_mapping *ioid_mapping);
static int set_mem_params(struct carrier_board *carrier, struct vme_addr_space *mem_space, struct vme_mapping *mem_mapping);
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

static int data_width[] = { VME_D8, VME_D16, ENDLIST };
static int a16_modifiers[] = { 0x29, 0x2D, ENDLIST };
static int a24_modifiers[] = { 0x39, 0x3A, 0x3D, 0x3E, ENDLIST };
static int a32_modifiers[] = { 0x09, 0x0A, 0x0D, 0x0E, ENDLIST };

static int carrier_mem_size[] = { TVME200_MEM_128k, TVME200_MEM_256k,
	TVME200_MEM_512k, TVME200_MEM_1M,
	TVME200_MEM_2M, TVME200_MEM_4M,
	TVME200_MEM_8M, TVME200_MEM_32M, ENDLIST };

static int carrier_slot_gap[] = { TVME200_MEM_128k_SLOT_GAP, TVME200_MEM_256k_SLOT_GAP,
	TVME200_MEM_512k_SLOT_GAP, TVME200_MEM_1M_SLOT_GAP,
	TVME200_MEM_2M_SLOT_GAP, TVME200_MEM_4M_SLOT_GAP,
	TVME200_MEM_8M_SLOT_GAP, TVME200_MEM_32M_SLOT_GAP, ENDLIST };

static int carrier_slot_size[] = { TVME200_MEM_128k_SLOT_SIZE, TVME200_MEM_256k_SLOT_SIZE,
	TVME200_MEM_512k_SLOT_SIZE, TVME200_MEM_1M_SLOT_SIZE,
	TVME200_MEM_2M_SLOT_SIZE, TVME200_MEM_4M_SLOT_SIZE,
	TVME200_MEM_8M_SLOT_SIZE, TVME200_MEM_32M_SLOT_SIZE, ENDLIST };

static unsigned int nb_installed_boards = 0;
static struct carrier_board* carrier_boards = NULL;

/**
 * carrier_register - Initialisation of the TVME200 board.
 *
 * @carrier		Base structure of the carrier.
 * @dev_specific	Specific device informations.
 *
 */
static int carrier_register(struct carrier_board *carrier, void *dev_specific)
{
	int  i;
	int res = 0;
	struct params_vme *vme_params = (struct params_vme *) dev_specific;
	struct carrier_infos *vme_carrier = NULL;
	int carrier_mem_index;

	if (carrier->carrier_specific != NULL) {
		printk(KERN_ERR PFX "Carrier number %d already registered !\n", carrier->carrier_number);
		res = -EINVAL;
		goto out_err;
	}

	vme_carrier = (struct carrier_infos*) kzalloc(sizeof(struct carrier_infos), GFP_KERNEL);
	if (vme_carrier == NULL) {
		printk(KERN_ERR PFX "Carrier [%s %d] unable to allocate memory for new carrier !\n",
				TVME200_SHORTNAME,
				carrier->carrier_number);
		res = -ENOMEM;
		goto out_err;
	}

	res = set_ioid_params(carrier, &vme_params->ioid_space, &vme_carrier->ioid_mapping);
	if (res){
		goto out_free;
	}

	carrier_mem_index = set_mem_params(carrier, &vme_params->mem_space, &vme_carrier->mem_mapping);
	if (carrier_mem_index < 0){
		res = -EINVAL;
		goto out_free;
	}

	res = vme_find_mapping(&vme_carrier->ioid_mapping, 1);
	if (res){
		printk(KERN_ERR PFX "Carrier [%s %d] unable to find window mapping for IO space !\n",
				TVME200_SHORTNAME,
				carrier->carrier_number);
		goto out_free;
	}

	res = vme_find_mapping(&vme_carrier->mem_mapping, 1);
	if (res){
		printk(KERN_ERR PFX "Carrier [%s %d] unable to find window mapping for MEM space !\n",
				TVME200_SHORTNAME,
				carrier->carrier_number);
		goto out_release_ioid;
	}

	for (i=0; i<TVME200_NB_SLOT; i++) {
		carrier->slots[i].io_phys.address = (void*)(vme_carrier->ioid_mapping.pci_addrl + TVME200_IO_SPACE_OFF + TVME200_IO_SPACE_GAP*i);
		carrier->slots[i].io_phys.size = TVME200_IO_SPACE_SIZE;
		carrier->slots[i].id_phys.address = (void*)(vme_carrier->ioid_mapping.pci_addrl + TVME200_ID_SPACE_OFF + TVME200_ID_SPACE_GAP*i);
		carrier->slots[i].id_phys.size = TVME200_ID_SPACE_SIZE;
		carrier->slots[i].mem_phys.address = (void*)(vme_carrier->mem_mapping.pci_addrl + carrier_slot_gap[carrier_mem_index]*i);
		carrier->slots[i].mem_phys.size = carrier_slot_size[carrier_mem_index];
	}

	carrier->carrier_specific = (void *) vme_carrier;

	return 0;

out_release_ioid :
	vme_release_mapping(&vme_carrier->ioid_mapping, 1);
out_free :
	kfree(vme_carrier);
out_err :
	return res;
}

/**
 * carrier_unregister Unregister a TVME200 board.
 *
 * @carrier	Base structure of the board.
 *
 */
static void carrier_unregister(struct carrier_board *carrier)
{
	int i;
	struct carrier_infos *vme_carrier = (struct carrier_infos *) carrier->carrier_specific;

	carrier->carrier_specific = NULL;

	vme_release_mapping(&vme_carrier->ioid_mapping, 1);
	vme_release_mapping(&vme_carrier->mem_mapping, 1);

	kfree(vme_carrier);

	for (i=0; i<TVME200_NB_SLOT; i++) {
		carrier->slots[i].io_phys.address = NULL;
		carrier->slots[i].io_phys.size = 0;
		carrier->slots[i].id_phys.address = NULL;
		carrier->slots[i].id_phys.size = 0;
		carrier->slots[i].mem_phys.address = NULL;
		carrier->slots[i].mem_phys.size = 0;
	}
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

	if (slot_id->slot_position >= TVME200_NB_SLOT) {
		printk(KERN_INFO PFX "Slot [%s %d:%d] doesn't exist! Last carrier slot is %d.\n",
				TVME200_SHORTNAME,
				slot_id->carrier_number, slot_id->slot_position,
				TVME200_NB_SLOT-1);
		return NULL;
	}

	if (carrier->slots[slot_id->slot_position].slot_id == NULL) {
		printk(KERN_INFO PFX "Slot [%s %d:%d] is not registered !\n",
				TVME200_SHORTNAME,
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

	if (slot_position >= TVME200_NB_SLOT){
		printk(KERN_ERR PFX "Slot [%s %d:%d] doesn't exist! Last carrier slot is %d.\n",
				TVME200_SHORTNAME,
				carrier_number, slot_position,
				TVME200_NB_SLOT-1);
		goto out;
	}

	if (mutex_lock_interruptible(&carrier->carrier_mutex)){
		goto out;
	}

	if (carrier->slots[slot_position].slot_id != NULL) {
		printk(KERN_ERR PFX "Slot [%s %d:%d] already installed !\n",
				TVME200_SHORTNAME,
				carrier_number, slot_position);
		goto out_unlock;
	}

	slot_id = (struct slot_id*) kzalloc(sizeof(struct slot_id), GFP_KERNEL);
	if (slot_id == NULL) {
		printk(KERN_ERR PFX "Slot [%s %d:%d] Unable to allocate memory for new slot !\n",
				TVME200_SHORTNAME,
				carrier_number, slot_position);
		goto out_unlock;
	}

	if (strlen(board_name) > SLOT_BOARD_NAME_SIZE) {
		printk(KERN_WARNING PFX "Slot [%s %d:%d] name (%s) too long (%d char > %d char MAX). Will be truncated!\n",
				TVME200_SHORTNAME,
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

	printk(KERN_INFO PFX "Slot registered.\n");

out_unlock:
	mutex_unlock(&carrier->carrier_mutex);
out:
	return slot_id;
}
EXPORT_SYMBOL(ip_slot_register);

/**
 * ip_slot_unregister - Unregister a IP Slot.
 *
 * @slot_id - Slot to unregister
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
 * ip_slot_map_space - Map an IP Slot specific space.
 *
 * @slot_id		Slot to map space
 * @memory_size		Memory Size needed by the IP module. Used only for Memory space.
 * @space		Specific space to map
 *
 */
int ip_slot_map_space(struct slot_id *slot_id, unsigned int memory_size, slot_space space)
{
	int res = 0;
	unsigned int size_to_map = 0;
	void *phy_address = NULL;
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
						TVME200_SHORTNAME,
						carrier->carrier_number, slot_id->slot_position);
				res = -EINVAL;
				goto out_unlock;
			}
			virt_addr_space = &slot_id->io_space;

			phy_address = carrier->slots[slot_id->slot_position].io_phys.address;
			size_to_map = carrier->slots[slot_id->slot_position].io_phys.size;
			break;
		case SLOT_ID_SPACE :
			printk(KERN_INFO PFX "ID space mapping...\n");
			if (slot_id->id_space.address != NULL) {
				printk(KERN_ERR PFX "Slot [%s %d:%d] ID space already mapped !\n",
						TVME200_SHORTNAME,
						carrier->carrier_number, slot_id->slot_position);
				res = -EINVAL;
				goto out_unlock;
			}
			virt_addr_space = &slot_id->id_space;

			phy_address = carrier->slots[slot_id->slot_position].id_phys.address;
			size_to_map = carrier->slots[slot_id->slot_position].id_phys.size;
			break;
		case SLOT_MEM_SPACE :
			printk(KERN_INFO PFX "MEM space mapping...\n");
			if (slot_id->mem_space.address != NULL) {
				printk(KERN_ERR PFX "Slot [%s %d:%d] MEM space already mapped !\n",
						TVME200_SHORTNAME,
						carrier->carrier_number, slot_id->slot_position);
				res = -EINVAL;
				goto out_unlock;
			}
			virt_addr_space = &slot_id->mem_space;

			if (memory_size > carrier->slots[slot_id->slot_position].mem_phys.size){
				printk(KERN_ERR PFX "Slot [%s %d:%d] request is 0x%X memory, only 0x%X available !\n",
						TVME200_SHORTNAME,
						slot_id->carrier_number,
						slot_id->slot_position,
						memory_size,
						carrier->slots[slot_id->slot_position].mem_phys.size);
				res = -EINVAL;
				goto out_unlock;
			}

			phy_address = carrier->slots[slot_id->slot_position].mem_phys.address;
			size_to_map = memory_size;
			break;
		default :
			printk(KERN_ERR PFX "Slot [%s %d:%d] space %d doesn't exist !\n",
					TVME200_SHORTNAME,
					carrier->carrier_number, slot_id->slot_position, space);
			res = -EINVAL;
			goto out_unlock;
			break;
	}

	virt_addr_space->size = size_to_map;
	virt_addr_space->address = ioremap((unsigned long)phy_address, size_to_map);

	printk(KERN_INFO PFX "Size=0x%x mapped to 0x%p.\n", size_to_map, virt_addr_space->address);

out_unlock:
	mutex_unlock(&carrier->carrier_mutex);
out:
	return res;
}
EXPORT_SYMBOL(ip_slot_map_space);

/**
 * @ip_slot_unmap_space - Unmap an IP Slot specific space.
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
						TVME200_SHORTNAME,
						slot_id->carrier_number, slot_id->slot_position);
				goto out_unlock;
			}
			virt_addr_space = &slot_id->io_space;
			break;
		case SLOT_ID_SPACE :
			printk(KERN_INFO PFX "ID space unmapping...\n");
			if (slot_id->id_space.address == NULL){
				printk(KERN_INFO PFX "Slot [%s %d:%d] ID space not mapped !\n",
						TVME200_SHORTNAME,
						slot_id->carrier_number, slot_id->slot_position);
				goto out_unlock;
			}
			virt_addr_space = &slot_id->id_space;
			break;
		case SLOT_MEM_SPACE :
			printk(KERN_INFO PFX "MEM space unmapping...\n");
			if (slot_id->mem_space.address == NULL){
				printk(KERN_INFO PFX "Slot [%s %d:%d] MEM space not mapped !\n",
						TVME200_SHORTNAME,
						slot_id->carrier_number, slot_id->slot_position);
				goto out_unlock;
			}
			virt_addr_space = &slot_id->mem_space;
			break;
		default :
			printk(KERN_ERR PFX "Slot [%s %d:%d] space number %d doesn't exist !\n",
					TVME200_SHORTNAME,
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
				TVME200_SHORTNAME,
				slot_id->carrier_number, slot_id->slot_position);
		res = -EINVAL;
		goto out_unlock;
	}

	slot_irq = (struct slot_irq*) kzalloc(sizeof(struct slot_irq), GFP_KERNEL);
	if (slot_irq == NULL) {
		printk(KERN_ERR PFX "Slot [%s %d:%d] unable to allocate memory for IRQ !\n",
				TVME200_SHORTNAME,
				slot_id->carrier_number, slot_id->slot_position);
		res = -ENOMEM;
		goto out_unlock;
	}

	slot_irq->vector = vector;
	slot_irq->handler = handler;
	slot_irq->arg = arg;
	if (name){
		if (strlen(name) > SLOT_IRQ_NAME_SIZE) {
			printk(KERN_WARNING PFX "Slot [%s %d:%d] IRQ name (%s) too long (%d char > %d char MAX). Will be truncated!\n",
					TVME200_SHORTNAME,
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
	printk(KERN_INFO PFX "IRQ registered.\n");

out_unlock:
	mutex_unlock(&carrier->carrier_mutex);
out:
	return res;
}
EXPORT_SYMBOL(ip_slot_request_irq);

/**
 * ip_slot_free_irq - Free the IRQ of the slot
 *
 * @slot_id	Slot to uninstall IRQ
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
				TVME200_SHORTNAME,
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

	struct slot_addr_space *slot_addr_space;
	switch (space){
		case SLOT_IO_SPACE :
			slot_addr_space = &slot_id->io_space;
			break;
		case SLOT_ID_SPACE :
			slot_addr_space = &slot_id->id_space;
			break;
		case SLOT_MEM_SPACE :
			slot_addr_space = &slot_id->mem_space;
			break;
		default :
			printk(KERN_ERR PFX "Slot [%s %d:%d] space number %d doesn't exist !\n",
					TVME200_SHORTNAME,
					slot_id->carrier_number, slot_id->slot_position, space);
			return NULL;
	}

	if ((slot_addr_space->size == 0) || (slot_addr_space->address == NULL)) {
		printk(KERN_ERR PFX "Error, slot space not mapped !\n");
		return NULL;
	}

	return slot_addr_space;
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
 * ip_slot_write_uchar - Write an unsigned char value on space with offset.
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
 * ip_slot_write_uint - Write an unsigned int value on space with offset.
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
 * ip_slot_write_vector Write the vector into the specified offset of memory space.
 *
 * @vector	Vector to write
 * @slot_id	Slot to write
 * @space	Space of the slot to write
 * @offset	Offset
 *
 * This function is like ip_slot_write_uchar but work only for VME carrier.
 * PCI carrier doesn't need vector value but to have a standard interface use this API
 * to write vector.
 *
 */
int ip_slot_write_vector(unsigned char vector, struct slot_id *slot_id, slot_space space, unsigned long offset)
{
	struct carrier_board *carrier = NULL;

	carrier = check_slot(slot_id);
	if (carrier == NULL)
		return -EINVAL;

	return slot_write_vector(vector, slot_id, space, offset);
}
EXPORT_SYMBOL(ip_slot_write_vector);

static int slot_request_irq(struct carrier_board *carrier, struct slot_id *slot_id)
{
	struct slot_irq *slot_irq = carrier->slots[slot_id->slot_position].irq;

	return vme_request_irq(slot_irq->vector, slot_irq->handler, slot_irq->arg, slot_irq->name);
}

static void slot_free_irq(struct carrier_board *carrier, struct slot_id *slot_id)
{
	struct slot_irq *slot_irq = carrier->slots[slot_id->slot_position].irq;

	vme_free_irq(slot_irq->vector);
}

static unsigned char slot_read_uchar(void *address, unsigned long offset)
{
	unsigned char value;

	value = ioread8(address + offset);
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
	iowrite8(value, address + offset);
}

static void slot_write_ushort(unsigned short value, void *address, unsigned long offset)
{
	iowrite16(value, address + offset);
}

static void slot_write_uint(unsigned int value, void *address, unsigned long offset)
{
	iowrite32(swahw32(value), address + offset);
}

static int slot_write_vector(unsigned char vector, struct slot_id *slot_id, slot_space space, unsigned long offset)
{
	return ip_slot_write_uchar(vector, slot_id, space, offset);
}

static int set_ioid_params(struct carrier_board *carrier, struct vme_addr_space *ioid_space, struct vme_mapping *ioid_mapping)
{
	int i = 0;

	printk(KERN_INFO PFX "Carrier [%s %d] IO space size is ignored and fixed to 0x%X.\n",
			TVME200_SHORTNAME,
			carrier->carrier_number, TVME200_IOIDINT_SIZE);

	i = 0;
	while (a16_modifiers[i] != ioid_space->address_modifier){
		if (a16_modifiers[i] == ENDLIST){
			printk(KERN_ERR PFX "Carrier [%s %d] address modifier (0x%X) of ID and IO space is not supported !\n",
					TVME200_SHORTNAME,
					carrier->carrier_number, ioid_space->address_modifier);
			return -EINVAL;
		}
		i++;
	};

	i = 0;
	while (data_width[i] != ioid_space->data_width){
		if (data_width[i] == ENDLIST){
			printk(KERN_ERR PFX "Carrier [%s %d] data width (%d) of ID and IO space is not supported !\n",
					TVME200_SHORTNAME,
					carrier->carrier_number, ioid_space->data_width);
			return -EINVAL;
		}
		i++;
	};

	if (ioid_space->base_address % TVME200_IOIDINT_SIZE){
		printk(KERN_ERR PFX "Carrier [%s %d] base address of ID and IO space must be a boundary equal of the space size (0x%X) !\n",
				TVME200_SHORTNAME,
				carrier->carrier_number, TVME200_IOIDINT_SIZE);
		return -EINVAL;
	}

	ioid_mapping->window_num = 0;
	ioid_mapping->am = ioid_space->address_modifier;
	ioid_mapping->data_width = ioid_space->data_width;
	ioid_mapping->vme_addru = 0;
	ioid_mapping->vme_addrl = ioid_space->base_address;
	ioid_mapping->sizeu = 0;
	ioid_mapping->sizel = TVME200_IOIDINT_SIZE;

	return 0;
}

static int set_mem_params(struct carrier_board *carrier, struct vme_addr_space *mem_space, struct vme_mapping *mem_mapping)
{
	int index = 0;
	int i = 0;
	int *addr_modifier;

	index = 0;
	while (carrier_mem_size[index] != mem_space->window_size){
		if (carrier_mem_size[index] == ENDLIST){
			printk(KERN_ERR PFX "Carrier [%s %d] memory size (0x%X) is not supported !\n",
					TVME200_SHORTNAME,
					carrier->carrier_number, mem_space->window_size);
			return -EINVAL;
		}
		index++;
	};

	if(carrier_mem_size[index] == TVME200_MEM_32M){
		addr_modifier = &a32_modifiers[0];
	}
	else {
		addr_modifier = &a24_modifiers[0];
	}

	i = 0;
	while (addr_modifier[i] != mem_space->address_modifier){
		if (addr_modifier[i] == ENDLIST){
			printk(KERN_ERR PFX "Carrier [%s %d] address modifier (0x%X) of MEM space is not supported !\n",
					TVME200_SHORTNAME,
					carrier->carrier_number, mem_space->address_modifier);
			return -EINVAL;
		}
		i++;
	};

	i = 0;
	while (data_width[i] != mem_space->data_width){
		if (data_width[i] == ENDLIST){
			printk(KERN_ERR PFX "Carrier [%s %d] The data width (%d) of MEM space is not supported !\n",
					TVME200_SHORTNAME,
					carrier->carrier_number, mem_space->data_width);
			return -EINVAL;
		}
		i++;
	};

	if (mem_space->base_address % carrier_mem_size[index]){
		printk(KERN_ERR PFX "Carrier [%s %d] base address of MEM space must be a boundary equal of the space size (0x%X) !\n",
				TVME200_SHORTNAME,
				carrier->carrier_number, carrier_mem_size[index]);
		return -EINVAL;
	}

	mem_mapping->window_num = 0;
	mem_mapping->am = mem_space->address_modifier;
	mem_mapping->data_width = mem_space->data_width;
	mem_mapping->vme_addru = 0;
	mem_mapping->vme_addrl = mem_space->base_address;
	mem_mapping->sizeu = 0;
	mem_mapping->sizel = mem_space->window_size;

	return index;
}

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

	nb_installed_boards = num_lun;
	printk(KERN_INFO PFX "Carrier driver loaded.\n");
	carrier_install_all();

	return 0;
}

static void carrier_uninstall(struct carrier_board *carrier)
{
	int i;

	if (carrier->carrier_specific != NULL) {
		for (i=0;i<TVME200_NB_SLOT;i++)
			ip_slot_unregister(carrier->slots[i].slot_id);

		carrier_unregister(carrier);
	}
	if (carrier->slots != NULL)
		kfree(carrier->slots);
}

static void __exit carrier_drvr_exit_module(void)
{
	int i;

	printk(KERN_INFO PFX "Carrier driver unloading...\n");

	for (i=0;i<nb_installed_boards;i++)
		carrier_uninstall(&carrier_boards[i]);

	nb_installed_boards = 0;
	kfree(carrier_boards);

	printk(KERN_INFO PFX "Carrier driver unloaded.\n");
}

static int carrier_install(struct carrier_board *carrier, void *params, unsigned int carrier_number)
{
	int res = 0;

	printk(KERN_INFO PFX "Carrier TVME200 number %d installing...\n", carrier_number);

	carrier->carrier_number = carrier_number;
	carrier->carrier_specific = NULL;
	carrier->slots = NULL;
	carrier->slots = (struct carrier_slot*) kzalloc(TVME200_NB_SLOT * sizeof(struct carrier_slot), GFP_KERNEL);
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
	printk(KERN_INFO PFX"Carrier %s number %d installed.\n", TVME200_SHORTNAME, carrier_number);
	return 0;

out_free :
	kfree(carrier->slots);
	carrier->slots = NULL;
out_err :
	return res;
}


static int carrier_install_all(void)
{
	struct params_vme params_vme;
	int i,j;
	int res = 0;

	nb_installed_boards = num_lun;

	carrier_boards = (struct carrier_board*) kzalloc(nb_installed_boards * sizeof(struct carrier_board), GFP_KERNEL);
	if (carrier_boards == NULL) {
		printk(KERN_ERR PFX "Unable to allocate carrier boards structure !\n");
		res = -ENOMEM;
		goto out_err;
	}

	memset(&params_vme, 0, sizeof(struct params_vme));
	for(i = 0; i < num_lun; i++) {
		params_vme.ioid_space.address_modifier = mod_address_ioid[i];
		params_vme.ioid_space.base_address = base_address_ioid[i];
		params_vme.ioid_space.data_width = data_width_ioid[i];
		params_vme.ioid_space.window_size = wind_size_ioid[i];
		params_vme.mem_space.address_modifier = mod_address_mem[i];
		params_vme.mem_space.base_address = base_address_mem[i];
		params_vme.mem_space.data_width = data_width_mem[i];
		params_vme.mem_space.window_size = wind_size_mem[i];

		res = carrier_install(&carrier_boards[i], &params_vme, i);
		if (res) {
			printk(KERN_ERR PFX "Error during carrier install !\n");
			goto out_uninst;
		}
	}; 

	return 0;

out_uninst :
	for (j = 0; j<i; j++)
		carrier_uninstall(&carrier_boards[i]);

	nb_installed_boards = 0;
	kfree(carrier_boards);
out_err :
	return res;
}


module_init(carrier_drvr_init_module);
module_exit(carrier_drvr_exit_module);
