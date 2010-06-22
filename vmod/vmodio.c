#include <linux/module.h>
#include <linux/interrupt.h>
#include <vmebus.h>
#include "vmodio.h"
#include "modulbus_register.h"

#define	MAX_DEVICES	16
#define	DRIVER_NAME	"vmodio"
#define	PFX		DRIVER_NAME ": "

/*
 * this module is invoked as
 *     $ insmod vmodio lun=0,1,4 
 *			base_address=0x1200,0xA800,0x6000
 *		     	irq=126,130,142
 */
static int lun[MAX_DEVICES];
static int nlun;
module_param_array(lun, int, &nlun, S_IRUGO);

static unsigned long base_address[MAX_DEVICES];
static int nbase_address;
module_param_array(base_address, ulong, &nbase_address, S_IRUGO);

static int irq[MAX_DEVICES];
static int nirq;
module_param_array(irq, int, &nirq, S_IRUGO);

static int irq_to_lun [MAX_DEVICES];
static int slot_irq[MAX_DEVICES][VMODIO_SLOTS];

/* description of a vmodio module */
struct vmodio {
	int		lun;		/* logical unit number */
	unsigned long	vme_addr;	/* vme base address */
	unsigned long	vaddr;		/* virtual address of MODULBUS
							space */
	int 		irq;		/* IRQ */
};

static struct vmodio device_table[MAX_DEVICES];
static int devices;

/* Matrix for register the isr callback functions */
static isrcb_t mezzanines_callback[MAX_DEVICES][VMODIO_SLOTS];

/* obtain the irq of slot from a base_irq in slot 0 */
static inline int vmodio_irq_shift(int base_irq, int slot)
{
	return base_irq - (0xe - (~(1<<slot) & 0xf));
}

static inline int upper_nibble_of(int byte)
{
	return 0xf & (byte >> 4);
}

static int interrupt_to_slot[] = {
	-1, -1, -1, -1,
	-1, -1, -1,  3,
	-1, -1, -1,  2,
	-1,  1,  0, -1,
};

/* get the slot an irq corresponds to */
static inline int irq_to_slot(int tmp)
{
	return interrupt_to_slot[tmp&0xf];
}

/* get the device struct corresponding to a lun */
static struct vmodio *lun_to_dev(int lun)
{
	int i = 0;

	for (i = 0; i < devices; i++) {
		struct vmodio *dev = &device_table[i];
		if (dev->lun == lun)
			return dev;
	}
	return NULL;
}

/* map vmodio VME address space */
static struct pdparam_master param = {
      .iack   = 1,                /* no iack */
      .rdpref = 0,                /* no VME read prefetch option */
      .wrpost = 0,                /* no VME write posting option */
      .swap   = 1,                /* VME auto swap option */
      .dum    = { VME_PG_SHARED,  /* window is sharable */
                  0,              /* XPC ADP-type */
                  0, },		  /* window is sharable */
};

static unsigned long vmodio_map(unsigned long base_address)
{
	return find_controller(base_address, VMODIO_WINDOW_LENGTH,
		VMODIO_ADDRESS_MODIFIER, 0, VMODIO_DATA_SIZE, &param);
}

/* slot memory-mapped I/O offsets */
static int vmodio_offsets[VMODIO_SLOTS] = {
	VMODIO_SLOT0,
	VMODIO_SLOT1,
	VMODIO_SLOT2,
	VMODIO_SLOT3,
};

/**
 * @brief Get virtual address of a mezzanine board AS
 *
 * VMOD/IO assigns a memory-mapped IO area to each slot of slot0..3
 * by the rule slot_address = base_address + slot#*0x200. 
 *
 * @param board_number  - logical module number of VMOD/IO card
 * @param board_positio - slot the requesting mz is plugged in
 * @param address_space_number
 *                      - must be 1 (only one address space available)
 * @return 0 on success
 * @return != 0 on failure
 */
static int get_address_space(
	struct carrier_as *asp,
	int board_number, int board_position, int address_space_number)
{
	struct vmodio *dev;

	/* VMOD/IO has a single space */
	if (address_space_number != 1) {
		printk(KERN_ERR PFX "invalid address space request\n");
		return -1;
	}
	dev = lun_to_dev(board_number);
	if (dev == NULL) {
		printk(KERN_ERR PFX "non-existent lun %d\n", board_number);
		return -1;
	}
	if ((board_position < 0) || (board_position >= VMODIO_SLOTS)) {
		printk(KERN_ERR PFX "invalid VMOD/IO board position %d\n", board_position);
		return -1;
	}

	/* parameters ok, set up mapping information */
	asp->address = dev->vaddr + vmodio_offsets[board_position];
	asp->size = 0x200;
	asp->width = 16;
	asp->is_big_endian = 1;
	return  0;
}

/**
 *  @brief Wrapper around get_address_space
 *  
 *  Same parameters and semantics, only intended for export (via
 *  linux EXPORT_SYMBOL or some LynxOS kludge)
 */
static int  vmodio_get_address_space(
	struct carrier_as *as,
	int board_number, 
	int board_position, 
	int address_space_number)
{
	return	get_address_space(as, board_number, board_position, address_space_number);
}
EXPORT_SYMBOL_GPL(vmodio_get_address_space);


/**
 * @brief Save a isr_callback of each mezzanine connected.
 *
 * VMOD/IO saves the isr callback function of each mezzanine's driver
 * to call it when an interrupt occurs.
 *
 * @param isr_callback  - IRQ callback function to be called.
 * @param board_number - lun of the carrier where the requesting mezzanine is plugged in
 * @param board_position - slot the requesting mezzanine is plugged in
 * @return 0 on success
 * @retrun != 0 on failure
 */
static int register_isr(int (*isr_callback)(
					    int board_number,
					    int board_position,
					    void* extra),
			    int board_number, int board_position)
{

	/* Adds the isr_callback if the carrier number and slot are correct. */
	if(board_number >= 0 && board_number < MAX_DEVICES &&
			board_position >= 0 && board_position < VMODIO_SLOTS){
		mezzanines_callback[board_number][board_position] = (isrcb_t) isr_callback;
	}
	else{
		printk(KERN_ERR PFX "Invalid VMOD/IO board number %d or board position %d\n",
				board_number, board_position);
		return -1;
	}

        return 0;
}

static int vmodio_interrupt(void *irq_id)
{
	int carrier_number = -1;
	short board_position = -1;

	int irq = *(int *)irq_id;

	/* Get the interrupt vector to know the lun of the matched carrier */
	carrier_number = irq_to_lun[upper_nibble_of(irq)];

	/* Get the interrupt vector to know the slot */
	board_position = irq_to_slot(irq);
	printk(KERN_ERR PFX "Interrupt carrier %d slot %d\n", carrier_number, board_position);

	if (board_position < 0 || board_position >= VMODIO_SLOTS || carrier_number < 0
	    || carrier_number >= MAX_DEVICES){
		printk(KERN_ERR PFX "invalid board_number interrupt: carrier %d board_position %d\n",
			    carrier_number, board_position);
		return IRQ_NONE;
	}

	if ((mezzanines_callback[carrier_number][board_position] == NULL) ||
	    (mezzanines_callback[carrier_number][board_position] (carrier_number, board_position, NULL) == -1))
		return IRQ_NONE;

	return IRQ_HANDLED;
}

static int device_init(struct vmodio *dev, int lun, unsigned long base_address, int irq)
{
	int ret;
	int i;

	dev->lun	= lun;
	dev->vme_addr	= base_address;
	dev->vaddr	= vmodio_map(base_address);
	dev->irq	= irq;

	if (dev->vaddr == -1)
		return -1;

	/* Upper nibble of irq corresponds to a single VMODIO module */
	irq_to_lun[upper_nibble_of(dev->irq)] = lun;

	/* 
	 * The irq corresponding to the first slot is passed as argument to the driver 
	 * To the rest of slots, the irq is calculated by substracting constants 0, 1, 3, 7.
	 */
	for (i = 0; i < VMODIO_SLOTS; i++) {
		int irq = vmodio_irq_shift(dev->irq, i);
		slot_irq[lun][i] = irq;
		ret = vme_request_irq(irq, vmodio_interrupt, &irq, "vmodio");
		if(ret < 0){ 
			printk(KERN_ERR PFX "Cannot register an irq to the device %d, error %d\n", 
				  dev->lun, ret);
			return -1;
		}
	}

	return 0;
}

static int __init init(void)
{
	int device = 0;
	int i;

	printk(KERN_INFO PFX "initializing driver\n");

	/* Initialize the needed matrix for IRQ */
	for (i = 0; i < MAX_DEVICES; i++){
		irq_to_lun[i] = -1;
		slot_irq[i][0] = -1;
		slot_irq[i][1] = -1;
		slot_irq[i][2] = -1;
		slot_irq[i][3] = -1;
	}

	if (nlun >= MAX_DEVICES) {
		printk(KERN_ERR PFX "too many devices (%d)\n", nlun);
		goto failed_init;
	}
	if (nlun != nbase_address || nlun != nirq) {
		printk(KERN_ERR PFX
			"Given %d luns but %d addresses and %d irqs\n",
				nlun, nbase_address, nirq);
		goto failed_init;
	}

	if (modulbus_carrier_register(DRIVER_NAME, get_address_space, register_isr) != 0) {
		printk(KERN_ERR PFX "could not register %s module\n",
			DRIVER_NAME);
		goto failed_init;
	}
	printk(KERN_INFO PFX "registered as %s carrier\n", DRIVER_NAME);

	devices = 0;
	for (device = 0; device < nlun; device++){
		struct vmodio *dev = &device_table[device];
		
		if (device_init(dev, lun[device], base_address[device], irq[device])) {
			printk(KERN_ERR PFX "map failed! not configuring lun %d\n", 
				dev->lun);
			goto failed_register;
		}
		printk(KERN_INFO PFX "mapped at virtual address 0x%08lx\n",
			dev->vaddr);
		devices++;
	}
	printk(KERN_INFO PFX "%d devices configured\n", devices);

	return 0;

failed_register:
	modulbus_carrier_unregister(DRIVER_NAME);
failed_init:
	printk(KERN_ERR PFX "failed to init, exit\n");
	return -1;
}

static void __exit exit(void)
{
	int i, j;

	printk(KERN_INFO PFX "uninstalling driver\n");
	for(i = 0; i < MAX_DEVICES; i++)
	for(j = 0; j < MAX_DEVICES; j++)
		if(slot_irq[i][j] >= 0)
			vme_free_irq(slot_irq[i][j]);
	modulbus_carrier_unregister(DRIVER_NAME);
}

module_init(init);
module_exit(exit);

MODULE_AUTHOR("Juan David Gonzalez Cobas");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("VMODIO driver");

