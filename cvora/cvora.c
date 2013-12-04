
/**
 * =================================================
 * Implement driver to do basic VME IO
 * Julian Lewis BE/CO/HT Tue 19th October 2010
 *
 * These parameters are vectors that must all be the same length,
 * or simply not used (length = 0). For each lun, the driver may
 * have a level, vector, and at least one VME base address to
 * create the address map. Each window size and address modifier
 * is normally the same for each lun and is specified once or not
 * at all.
 *
 * dname, In order to avoid recompiling the driver it is possible
 *        to define its name via this parameter rather than in
 *        the users parameter file. This means the same kernel
 *        object can be used multiple times.
 *
 * luns, Logical unit numbers (not optional)
 * level, Interrupt levels (either not specified or one for each lun)
 * vectors, Interrupt vectors (same as level)
 * base_address1, First VME base address (not optional)
 * base_address2, Second VME base address (either not specified or one for each lun)
 *
 * These parameters would normally be specified once per lun.
 * In this case the driver will use the first value for all luns.
 * There is no restriction however in the driver from specifying
 * them individually for each lun should that be usefull. It may
 * be interesting for example to map one module and use only DMA
 * access for all the others.
 *
 * am1, First address modifier (not optional, at least one specified)
 * am2, Second address modifier (optional)
 * data_width1, First data width (not optional, at least one specified)
 * data_width2, Second data width (optional)
 * size1, First window size (not optional, at least one specified)
 * size2, Second window size (optional)
 * nmap, No map window flag, 1=DMA only (defaults to zero)
 * isrc, Location of interrupt source reg in base_address1 (optional)
 *
 * Installation via insmod parameters example
 *
 * Example: cp vmeio.ko mytest.ko
 *          insmod mytest.ko dname="mytest"
 *                           luns= 136,   99,    46
 *                           base_address1= 0x100, 0x200, 0x300
 *                           am1= 0x29 size1=0x10 data_width1=2
 *
 * Example: insmod ctrv.ko luns= 1,         2,
 *                         base_address1= 0xC100000, 0xC200000
 *                         base_address2= 0x100,     0x200
 *                         level= 2,         2
 *                         vectors= 0xC8,      0xC9
 *                         am1= 0x39
 *                         am2= 0x29
 *                         size1= 0x10000
 *                         size2= 0x100
 *                         data_width1= 4
 *                         data_width2= 2
 *                         isrc= 0
 *
 * ======================================================================
 * Includes
 */

/*
 * These next two includes must be included in this order.
 * Driver configuration definitions.
 * It is supposed that this driver will be compiled under different names
 * and that nodes will be created in /dev for each named kernal module.
 * These definitions must be set up according to a particular useage.
 * DRV_NAME:           is used when printing messages
 * DRV_SYMB:           entry points symbol value
 * DRV_MODULE_VERSION: may change with new features and bug corrections
 * DRV_MAX_DEVICES:    is the number of modules you expect to handle
 */

#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include "vmebus.h"
#include "cvora.h"

#define TSI148_LCSR_DSTA_DON (1<<25)	/* DMA done */

/*
 * ======================================================================
 * Static memory
 */

static int vmeio_major = 0;
static char *vmeio_major_name = "cvora";

MODULE_AUTHOR("Julian Lewis BE/CO/HT CERN");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Raw IO to VME");
MODULE_SUPPORTED_DEVICE("Any VME device");
MODULE_VERSION(GIT_VERSION);

/*
 * ==============================
 * Module parameter storage area
 * Indexed by minor device number
 */

static long lun[DRV_MAX_DEVICES];	/* Logical unit numbers */
static long level[DRV_MAX_DEVICES];	/* Interrupt levels */
static long vector[DRV_MAX_DEVICES];	/* Interrupt vectors */
static long base_address1[DRV_MAX_DEVICES];	/* First VME base address */
static long base_address2[DRV_MAX_DEVICES];	/* Second VME base address */

/* Single value parameter handling */
/* Usually the same for each lun.  */

static long am1[DRV_MAX_DEVICES];	/* First address modifier */
static long am2[DRV_MAX_DEVICES];	/* Second address modifier */
static long data_width1[DRV_MAX_DEVICES];	/* First data width */
static long data_width2[DRV_MAX_DEVICES];	/* Second data width */
static long size1[DRV_MAX_DEVICES];	/* First window size */
static long size2[DRV_MAX_DEVICES];	/* Second window size */
static long nmap[DRV_MAX_DEVICES];	/* No map window flag, 1=DMA only */
static long isrc[DRV_MAX_DEVICES];	/* Location of interrupt source reg in base_address1 */

/* These parameter counts must equal the number of luns */
/* or be equal to zero if not used. */

static unsigned int luns_num;
static unsigned int lvls_num;
static unsigned int vecs_num;
static unsigned int vme1_num;
static unsigned int vme2_num;

/* These parameter counts are normally zero if not used or */
/* one, they can however take any value between 0..luns */

static unsigned int amd1_num;	/* Normally this value is = "1" */
static unsigned int amd2_num;	/* Can be = "0" if not used */
static unsigned int dwd1_num;	/* Normally this value is = "1" */
static unsigned int dwd2_num;	/* Can be = "0" if not used */
static unsigned int win1_num;	/* Normally this value is = "1" */
static unsigned int win2_num;	/* Can be = "0" if not used */
static unsigned int nmap_num;	/* Its quite possible this is > "1" */
static unsigned int isrc_num;	/* Normally this value is = "1" */

module_param_array(lun, long, &luns_num, 0444);	/* Vector */
module_param_array(level, long, &lvls_num, 0444);	/* Vector */
module_param_array(vector, long, &vecs_num, 0444);	/* Vector */
module_param_array(base_address1, long, &vme1_num, 0444);	/* Vector */
module_param_array(base_address2, long, &vme2_num, 0444);	/* Vector */

module_param_array(am1, long, &amd1_num, 0444);
module_param_array(am2, long, &amd2_num, 0444);
module_param_array(data_width1, long, &dwd1_num, 0444);
module_param_array(data_width2, long, &dwd2_num, 0444);
module_param_array(size1, long, &win1_num, 0444);
module_param_array(size2, long, &win2_num, 0444);
module_param_array(nmap, long, &nmap_num, 0444);
module_param_array(isrc, long, &isrc_num, 0444);

MODULE_PARM_DESC(lun, "Logical unit numbers");
MODULE_PARM_DESC(level, "Interrupt levels");
MODULE_PARM_DESC(vector, "Interrupt vectors");
MODULE_PARM_DESC(base_address1, "First map base addresses");
MODULE_PARM_DESC(base_address2, "Second map base addresses");

MODULE_PARM_DESC(am1, "First VME address modifier");
MODULE_PARM_DESC(am2, "Second VME address modifier");
MODULE_PARM_DESC(data_width1, "First data width 1,2,4,8 bytes");
MODULE_PARM_DESC(data_width2, "Second data width 1,2,4,8 bytes");
MODULE_PARM_DESC(size1, "First window size in bytes");
MODULE_PARM_DESC(size2, "Second window size in bytes");
MODULE_PARM_DESC(nmap, "No VME map flags, 1=DMA only");
MODULE_PARM_DESC(isrc, "Location of interrupt source reg in base_address1");

static char dname[64] = { 0 };

module_param_string(dname, dname, sizeof(dname), 0);
MODULE_PARM_DESC(dname, "Driver name");

/*
 * This structure describes all the relevant information about a mapped
 * window
 */
struct vmeio_map {
	unsigned long	base_address;
	unsigned long	address_modifier;
	unsigned long	data_width;
	unsigned long	window_size;
	void		*vaddr;		/* NULL if not mapped */
	struct vme_berr_handler
			*bus_error_handler;	/* NULL if inexistent */
};

/*
 * vmeio device descriptor:
 *	maps[max_maps]		array of mapped VME windows
 *
 *	isrfl			1 if interrupt handler installed
 *	isrc			offset of int source reg in map0
 *	isr_source_address	interrupt source reg address
 *	isr_source_mask		result of the read
 *
 *	queue			interrupt waits
 *	timeout			timeout value for wait queue
 *	icnt			interrupt counter
 *
 *	debug			debug level
 */

#define MAX_MAPS	2

struct vmeio_device {
	int			lun;
	struct vmeio_map	maps[MAX_MAPS];
	int			nmap;

	int			vec;
	int			lvl;
	unsigned		isrc;
	int			isrfl;
	void			*isr_source_address;
	int			isr_source_mask;

	wait_queue_head_t	queue;
	int			timeout;
	int			icnt;

	int			debug;
};

static struct vmeio_device devices[DRV_MAX_DEVICES];

struct file_operations vmeio_fops;

/* ================= */

int check_minor(long num)
{
	if (num < 0 || num >= DRV_MAX_DEVICES) {
		printk("%s:minor:%d ", vmeio_major_name, (int) num);
		printk("BAD not in [0..%d]\n", DRV_MAX_DEVICES - 1);
		return 0;
	}
	return 1;
}

/*
 * =========================================================
 * VMEIO with bus error handling
 */

#define CLEAR_BUS_ERROR ((int) 1)
#define BUS_ERR_PRINT_THRESHOLD 10

static int bus_error_count = 0;	/* For all modules */
static int isr_bus_error = 0;	/* Bus error in an ISR */
static int last_bus_error = 0;	/* Last printed bus error */

/* ==================== */

static void BusErrorHandler(struct vme_bus_error *error)
{
	bus_error_count++;
}

/* ==================== */

static int GetClrBusErrCnt(void)
{
	int res;

	res = bus_error_count;

	bus_error_count = 0;
	last_bus_error = 0;
	isr_bus_error = 0;

	return res;
}

/* ==================== */

static void CheckBusError(char *dw, char *dir, void *x)
{
	if (bus_error_count > last_bus_error &&
	    bus_error_count <= BUS_ERR_PRINT_THRESHOLD) {
		printk("%s:BUS_ERROR:%s:%s-Address:0x%p\n",
			       vmeio_major_name, dw, dir, x);
		if (isr_bus_error)
			printk("%s:BUS_ERROR:In ISR occured\n",
				       vmeio_major_name);
		if (bus_error_count == BUS_ERR_PRINT_THRESHOLD)
			printk("%s:BUS_ERROR:PrintSuppressed\n",
				       vmeio_major_name);
		isr_bus_error = 0;
		last_bus_error = bus_error_count;
	}
}

/* ==================== */
/* Used in ISR only     */

static char IHRd8(void *x)
{
	char res;

	isr_bus_error = 0;
	res = ioread8(x);
	if (bus_error_count > last_bus_error)
		isr_bus_error = 1;
	return res;
}

/* ==================== */
/* Used in ISR only     */

static short IHRd16(void *x)
{
	short res;

	isr_bus_error = 0;
	res = ioread16be(x);
	if (bus_error_count > last_bus_error)
		isr_bus_error = 1;
	return res;
}

/* ==================== */
/* Used in ISR only     */

static int IHRd32(void *x)
{
	int res;

	isr_bus_error = 0;
	res = ioread32be(x);
	if (bus_error_count > last_bus_error)
		isr_bus_error = 1;
	return res;
}

/* ==================== */

static int HRd32(void *x)
{
	int res;

	res = ioread32be(x);
	CheckBusError("D32", "READ", x);
	return res;
}

/* ==================== */

static void HWr32(int v, void *x)
{
	iowrite32be(v, x);
	CheckBusError("D32", "WRITE", x);
	return;
}

/* ==================== */

static short HRd16(void *x)
{
	short res;

	res = ioread16be(x);
	CheckBusError("D16", "READ", x);
	return res;
}

/* ==================== */

static void HWr16(short v, void *x)
{
	iowrite16be(v, x);
	CheckBusError("D16", "WRITE", x);
	return;
}

/* ==================== */

static char HRd8(void *x)
{
	char res;

	res = ioread8(x);
	CheckBusError("D8", "READ", x);
	return res;
}

/* ==================== */

static void HWr8(char v, void *x)
{
	iowrite8(v, x);
	CheckBusError("D8", "WRITE", x);
	return;
}

/*
 * =========================================================
 * Interrupt service routine
 * =========================================================
 */

static irqreturn_t vmeio_irq(void *arg)
{
	struct vmeio_device *dev = arg;
	long data;

	if (dev->isr_source_address) {
		unsigned long data_width = dev->maps[0].data_width;
		if (data_width == 4)
			data = IHRd32(dev->isr_source_address);
		else if (data_width == 2)
			data = IHRd16(dev->isr_source_address);
		else
			data = IHRd8(dev->isr_source_address);
		dev->isr_source_mask = data;
	}
	dev->icnt++;
	wake_up(&dev->queue);
	return IRQ_HANDLED;
}

/* ==================== */

void set_remaining_null(long argarray[], unsigned int argnum)
{
	int i;

	if (!argnum)
		return;
	for (i = argnum; i < luns_num; i++)
		argarray[i] = argarray[0];
}

/* ==================== */

void *map_window(int vme, int amd, int dwd, int win) {

	unsigned long vmeaddr;
	struct pdparam_master param;
	char *msg;

	if (!(vme && amd && win && dwd)) return NULL;

	param.iack = 1;                 /* no iack */
	param.rdpref = 0;               /* no VME read prefetch option */
	param.wrpost = 0;               /* no VME write posting option */
	param.swap = 1;                 /* VME auto swap option */
	param.dum[0] = VME_PG_SHARED;   /* window is sharable */
	param.dum[1] = 0;               /* XPC ADP-type */
	param.dum[2] = 0;               /* window is sharable */

	vmeaddr = find_controller(vme, win, amd, 0, dwd, &param);

	if (vmeaddr == -1UL) {
		msg = "ERROR:NotMapped";
		vmeaddr = 0;
	} else {
		msg = "OK:Mapped";
	}
	printk("%s:%s:Address:0x%X Window:0x%X"
			":AddrMod:0x%X DWidth:0x%X:VirtAddr:0x%lX\n",
			vmeio_major_name, msg,
			vme, win, amd, dwd, vmeaddr);
	return (void *)vmeaddr;
}

struct vme_berr_handler *set_berr_handler(int vme, int win, int amd) {

	struct vme_bus_error berr;
	struct vme_berr_handler *handler;

	if (!(vme && amd && win)) return NULL;

	berr.address = (long) vme;
	berr.am      = amd;
	handler      = vme_register_berr_handler(&berr, win, BusErrorHandler);

	printk("%s:BusErrorHandler:", vmeio_major_name);

	if (IS_ERR(handler)) {
	   printk("ERROR:NotRegistered");
	   handler = NULL;
	} else {
	   printk("OK:Registered");
	}
	printk(":Address:0x%X Window:0x%X AddrMod:0x%X\n",vme,win,amd);
	return handler;
}


static void vmeio_map_init(struct vmeio_map *map,
		int vme, int win, int amd, int dwd)
{
	map->base_address	= vme;
	map->window_size	= win;
	map->address_modifier	= amd;
	map->data_width		= dwd;
	map->vaddr		= NULL;
	map->bus_error_handler	= NULL;
}

static void vmeio_map_register(struct vmeio_map *map)
{
	map->vaddr = map_window(map->base_address, map->address_modifier,
				map->data_width, map->window_size);
	map->bus_error_handler = set_berr_handler(map->base_address,
			map->window_size, map->address_modifier);
}

static void vmeio_map_unregister(struct vmeio_map *map)
{
	if (map->base_address)
		return_controller((unsigned long)map->vaddr, map->window_size);
	if (map->bus_error_handler)
		vme_unregister_berr_handler(map->bus_error_handler);
}


void register_isr(struct vmeio_device *dev, unsigned vector, unsigned level)
{
	int err;

	err = vme_intset(vector, (int (*)(void *)) vmeio_irq, dev, 0);
	dev->isrfl = !(err < 0);
	printk("%s:ISR:Level:0x%X Vector:0x%X:%s\n",
		vmeio_major_name, level, vector,
		(err < 0) ? "ERROR:NotRegistered" : "OK:Registered");
}

void register_int_source(struct vmeio_device *dev, void *map, unsigned offset)
{
	dev->isr_source_address = dev->maps[0].vaddr + dev->isrc;
	printk("SourceRegister:0x%p", dev->isr_source_address);
}

void set_interrrupt_vector(unsigned int vec)
{
	
}

/*
 * =====================================================
 * Install
 * =====================================================
 */

int vmeio_install(void)
{
	int i, cc;

	if (luns_num <= 0 || luns_num > DRV_MAX_DEVICES) {
		printk("%s:Fatal:No logical units defined.\n",
		       vmeio_major_name);
		return -EACCES;
	}

	/* Vector parameters must all be the same size or zero */

	if (lvls_num != luns_num && lvls_num != 0) {
		printk("%s:Fatal:Missing interrupt level.\n",
		       vmeio_major_name);
		return -EACCES;
	}

	if (vecs_num != luns_num && vecs_num != 0) {
		printk("%s:Fatal:Missing interrupt vector.\n",
		       vmeio_major_name);
		return -EACCES;
	}

	if (vme1_num != luns_num && vme1_num != 0) {
		printk("%s:Fatal:Missing first base address.\n",
		       vmeio_major_name);
		return -EACCES;
	}
	if (vme2_num != luns_num && vme2_num != 0) {
		printk("%s:Fatal:Missing second base address.\n",
		       vmeio_major_name);
		return -EACCES;
	}

	/* Default single parameters to the first specified */

	set_remaining_null(am1, amd1_num);
	set_remaining_null(am2, amd2_num);
	set_remaining_null(data_width1, dwd1_num);
	set_remaining_null(data_width2, dwd2_num);
	set_remaining_null(size1, win1_num);
	set_remaining_null(size2, win2_num);
	set_remaining_null(nmap, nmap_num);
	set_remaining_null(isrc, isrc_num);
	set_remaining_null(am2, amd2_num);

	/* Build module contexts */

	for (i = 0; i < luns_num; i++) {
		struct vmeio_device *dev = &devices[i];
		struct vmeio_map *map0 = &dev->maps[0];
		struct vmeio_map *map1 = &dev->maps[1];

		memset(dev, 0, sizeof(*dev));

		dev->lun = lun[i];

		map0->base_address     = base_address1[i];
		map0->address_modifier = am1[i];
		map0->data_width       = data_width1[i]/8;
		map0->window_size      = size1[i];
		map0->vaddr            = NULL;
		map0->bus_error_handler = NULL;

		map1->base_address     = base_address2[i];
		map1->address_modifier = am2[i];
		map1->data_width       = data_width2[i]/8;
		map1->window_size      = size2[i];
		map1->vaddr            = NULL;
		map1->bus_error_handler = NULL;

		dev->isrc = isrc[i];
		dev->lvl  = level[i];
		dev->vec  = vector[i];
		dev->nmap = nmap[i];

		init_waitqueue_head(&dev->queue);
	}

	/* Register driver */
	cc = register_chrdev(vmeio_major, vmeio_major_name, &vmeio_fops);
	if (cc < 0) {
		printk("%s:Fatal:Error from register_chrdev [%d]\n",
		       vmeio_major_name, cc);
		return cc;
	}
	if (vmeio_major == 0)
		vmeio_major = cc;	/* dynamic */

	/* Create VME mappings and register ISRs */

	for (i = 0; i < luns_num; i++) {
		struct vmeio_device *dev = &devices[i];
		struct vmeio_map *map0 = &dev->maps[0];
		struct vmeio_map *map1 = &dev->maps[1];

		dev->debug = DEBUG;
		dev->timeout = msecs_to_jiffies(TIMEOUT);
		dev->icnt = 0;

		if (strlen(dname))
			vmeio_major_name = dname;

		if (dev->nmap != 0) {
			printk("%s:Logical unit:%d is not mapped: DMA only\n",
			     vmeio_major_name, dev->lun);
			continue;
		}

		printk("%s:Mapping:Logical unit:%d\n", vmeio_major_name, dev->lun);

		map0->vaddr = map_window(map0->base_address, map0->address_modifier,
						map0->data_width, map0->window_size);
		map0->bus_error_handler = set_berr_handler(map0->base_address,
					map0->window_size, map0->address_modifier);
		map1->vaddr = map_window(map1->base_address, map1->address_modifier,
						map1->data_width, map1->window_size);
		map1->bus_error_handler = set_berr_handler(map1->base_address,
					map1->window_size, map1->address_modifier);

		if (dev->lvl && dev->vec) {
			unsigned int cr;

			register_isr(dev, dev->vec, dev->lvl);
			if (isrc_num)
				register_int_source(dev, map0->vaddr, dev->isrc);
			/* set cvora interrupt vector */
			cr = ioread32be(map0->vaddr);
			cr &= 0xffff00ff;
			cr |= ((dev->vec & 0xff) << 8);
			iowrite32be(cr, map0->vaddr);
		}
	}
	return 0;
}

/* ==================== */

void unregister_module(struct vmeio_device *dev) {
	struct vmeio_map *map0 = &dev->maps[0];
	struct vmeio_map *map1 = &dev->maps[1];

	if (dev->vec)
		vme_intclr(dev->vec, NULL);
	if (map0->base_address)
		return_controller((unsigned long)map0->vaddr, map0->window_size);
	if (map1->base_address)
		return_controller((unsigned long)map1->vaddr, map1->window_size);
	if (map0->bus_error_handler)
		vme_unregister_berr_handler(map0->bus_error_handler);
	if (map1->bus_error_handler)
		vme_unregister_berr_handler(map1->bus_error_handler);
}

/*
 * =====================================================
 * Uninstall the driver
 * =====================================================
 */

void vmeio_uninstall(void)
{
	int i;

	for (i = 0; i < luns_num; i++) {
		unregister_module(&devices[i]);
	}
	unregister_chrdev(vmeio_major, vmeio_major_name);
}

/*
 * =====================================================
 * Open
 * =====================================================
 */

int vmeio_open(struct inode *inode, struct file *filp)
{
	long num;

	num = MINOR(inode->i_rdev);
	if (!check_minor(num))
		return -EACCES;

	return 0;
}

/*
 * =====================================================
 * Close
 * =====================================================
 */

int vmeio_close(struct inode *inode, struct file *filp)
{
	long num;

	num = MINOR(inode->i_rdev);
	if (!check_minor(num))
		return -EACCES;

	return 0;
}

/*
 * =====================================================
 * Read
 * =====================================================
 */

ssize_t vmeio_read(struct file * filp, char *buf, size_t count,
		   loff_t * f_pos)
{
	int cc;
	long minor;
	struct inode *inode;

	struct vmeio_read_buf_s rbuf;
	struct vmeio_device *dev;
	int icnt;

	inode = filp->f_dentry->d_inode;
	minor = MINOR(inode->i_rdev);
	if (!check_minor(minor))
		return -EACCES;
	dev = &devices[minor];

	if (dev->debug) {
		printk("%s:read:count:%zd minor:%d\n", vmeio_major_name,
		       count, (int) minor);
		if (dev->debug > 1) {
			printk("%s:read:timout:%d\n", vmeio_major_name,
			       dev->timeout);
		}
	}

	if (count < sizeof(rbuf)) {
		if (dev->debug) {
			printk("%s:read:Access error buffer too small\n",
			       vmeio_major_name);
		}
		return -EACCES;
	}

	icnt = dev->icnt;
	if (dev->timeout) {
		cc = wait_event_interruptible_timeout(dev->queue,
						      icnt != dev->icnt,
						      dev->timeout);
	} else {
		cc = wait_event_interruptible(dev->queue,
					      icnt != dev->icnt);
	}

	if (dev->debug > 2) {
		printk("%s:wait_event:returned:%d\n", vmeio_major_name,
		       cc);
	}

	if (cc == -ERESTARTSYS)
		return cc;
	if (cc == 0 && dev->timeout)
		return -ETIME;	/* Timer expired */
	if (cc < 0)
		return cc;	/* Error */

	rbuf.logical_unit = dev->lun;
	rbuf.interrupt_mask = dev->isr_source_mask;
	rbuf.interrupt_count = dev->icnt;

	cc = copy_to_user(buf, &rbuf, sizeof(rbuf));
	if (cc != 0) {
		printk("%s:Can't copy to user space:cc=%d\n", vmeio_major_name, cc);
		return -EACCES;
	}
	return sizeof(struct vmeio_read_buf_s);
}

/*
 * =====================================================
 * Write
 * Used to simulate interrupts
 * =====================================================
 */

ssize_t vmeio_write(struct file * filp, const char *buf, size_t count,
		    loff_t * f_pos)
{
	long minor;
	struct vmeio_device *dev;
	struct inode *inode;
	int cc, mask;

	inode = filp->f_dentry->d_inode;
	minor = MINOR(inode->i_rdev);
	if (!check_minor(minor))
		return -EACCES;
	dev = &devices[minor];

	if (count >= sizeof(int)) {
		cc = copy_from_user(&mask, buf, sizeof(int));
		if (cc != 0) {
			printk("%s:write:Error:%d could not copy from user\n",
					     vmeio_major_name, cc);
			return -EACCES;
		}
	}

	if (dev->debug) {
		printk("%s:write:count:%zd minor:%d mask:0x%X\n",
		       vmeio_major_name, count, (int) minor, mask);
	}

	dev->isr_source_mask = mask;
	dev->icnt++;
	wake_up(&dev->queue);
	return sizeof(int);
}

/*
 * =====================================================
 * Ioctl
 * =====================================================
 */

#define VME_NO_ADDR_INCREMENT 1
#define DMA_BLOCK_SIZE        4096
#define SAMPLES_IN_DMA_BLOCK  2048

/*
 * ====================================================================
 * Debug routines
 */

static char *ioctl_names[vmeioLAST - vmeioFIRST] = {
	"Unknown IOCTL number",
	"SET_DEBUG",
	"GET_DEBUG",
	"GET_VERSION",
	"SET_TIMEOUT",
	"GET_TIMEOUT",
	"GET_DEVICE",
	"RAW_READ",
	"RAW_WRITE",
	"RAW_READ_DMA",
	"RAW_WRITE_DMA",
	"SET_DEVICE"
};

static void debug_ioctl(int ionr, int iodr, int iosz, void *arg, long num,
			int dlevel)
{
	int c;
	int *iargp = arg;

	if (dlevel <= 0)
		return;

	printk("%s:debug_ioctl:ionr:%d", vmeio_major_name, ionr);
	if (ionr <= vmeioFIRST || ionr >= vmeioLAST) {
		printk(" BAD:");
	} else {
		c = ionr - vmeioFIRST;
		printk(" %s:", ioctl_names[c]);
	}

	printk(" iodr:%d:", iodr);
	if (iodr & _IOC_WRITE)
		printk("WR:");
	if (iodr & _IOC_READ)
		printk("RD:");

	if (arg)
		c = *iargp;
	else
		c = 0;
	printk(" iosz:%d arg:0x%p[%d] minor:%d\n", iosz, arg, c,
	       (int) num);
}


static void vmeio_set_debug(struct vmeio_device *dev, int *debug)
{
	dev->debug = *debug;
}

static void vmeio_get_debug(struct vmeio_device *dev, int *debug)
{
	*debug = dev->debug;
}

static void vmeio_get_version(int *version)
{
	*version = 0;
}

static void vmeio_set_timeout(struct vmeio_device *dev, int *timeout)
{
	dev->timeout = msecs_to_jiffies(*timeout);
}

static void vmeio_get_timeout(struct vmeio_device *dev, int *timeout)
{
	*timeout = jiffies_to_msecs(dev->timeout);
}

static void vmeio_get_device(struct vmeio_device *dev,
		struct vmeio_get_window_s *win)
{
	struct vmeio_map *map0 = &dev->maps[0];
	struct vmeio_map *map1 = &dev->maps[1];

	win->lun	= dev->lun;
	win->lvl	= dev->lvl;
	win->vec	= dev->vec;
	win->nmap	= dev->nmap;
	win->isrc	= dev->isrc;

	win->amd1	= map0->address_modifier;
	win->dwd1	= map0->data_width;
	win->vme1	= map0->base_address;
	win->win1	= map0->window_size;

	win->amd2	= map1->address_modifier;
	win->dwd2	= map1->data_width;
	win->vme2	= map1->base_address;
	win->win2	= map1->window_size;
}

static void vmeio_set_device(struct vmeio_device *dev,
		struct vmeio_get_window_s *win)
{
	struct vmeio_map *map0 = &dev->maps[0];
	struct vmeio_map *map1 = &dev->maps[1];

	vmeio_map_unregister(map0);
	vmeio_map_init(map0, win->vme1, win->win1, win->amd1, win->dwd1);
	vmeio_map_unregister(map1);
	vmeio_map_init(map1, win->vme2, win->win2, win->amd2, win->dwd2);
	if (dev->nmap)		/* DMA only */
		return;
	vmeio_map_register(map0);
	vmeio_map_register(map1);
}

static int raw_dma(struct vmeio_device *dev,
	struct vmeio_riob_s *riob, enum vme_dma_dir direction)
{
	struct vme_dma dma_desc;
	struct vmeio_map *map = &dev->maps[riob->winum];
	unsigned long buf = (unsigned long)riob->buffer;
	unsigned int bu, bl;
	int cc, winum;
	unsigned int haddr;

#ifdef __64BIT
	bl = buf & 0xFFFFFFFF;
	bu = buf >> 32;
#else
	bl = buf;
	bu = 0;
#endif
	memset(&dma_desc, 0, sizeof(dma_desc));

	dma_desc.dir = direction;
	dma_desc.novmeinc = 0;
	dma_desc.length = riob->bsize;

	dma_desc.ctrl.pci_block_size = VME_DMA_BSIZE_4096;
	dma_desc.ctrl.pci_backoff_time = VME_DMA_BACKOFF_0;
	dma_desc.ctrl.vme_block_size = VME_DMA_BSIZE_4096;
	dma_desc.ctrl.vme_backoff_time = VME_DMA_BACKOFF_0;

	winum = riob->winum -1;
	if (winum < 0) winum = 0;

	map = &dev->maps[winum];

	dma_desc.dst.data_width = map->data_width * 8;
	dma_desc.dst.am = map->address_modifier;
	dma_desc.src.data_width = map->data_width * 8;
	dma_desc.src.am = VME_A24_USER_BLT;

	haddr = (unsigned int) map->base_address + riob->offset;

	if (direction == VME_DMA_TO_DEVICE) {
		dma_desc.src.addrl = bl;
		dma_desc.src.addru = bu;
		dma_desc.dst.addrl = haddr;
	} else {
		dma_desc.src.addrl = haddr;
		dma_desc.dst.addrl = bl;
		dma_desc.dst.addru = bu;
	}

	if (dev->debug > 1) {
		char *msg = (direction == VME_DMA_FROM_DEVICE) ?
			"DMA:READ:win:%d src:0x%p amd:0x%x dwd:%d len:%d dst:0x%08x%08x\n" :
			"DMA:WRIT:win:%d dst:0x%p amd:0x%x dwd:%d len:%d src:0x%08x%08x\n";
		printk(msg, riob->winum, haddr, map->address_modifier,
		     map->data_width, riob->bsize, bu, bl);
	}

	if ((cc = vme_do_dma(&dma_desc)) < 0)
		return cc;

	if (!(dma_desc.status & TSI148_LCSR_DSTA_DON)) {
		printk("%s:DMA:NotDone:Status:0x%X\n",
		       vmeio_major_name, dma_desc.status);
		return -EIO;
	}

	return 0;
}

union vmeio_word {
	int	width4;
	short	width2;
	char	width1;
};


static int raw_read(struct vmeio_device *dev, struct vmeio_riob_s *riob)
{
	struct vmeio_map *mapx = &dev->maps[riob->winum-1];	
	int dwidth = mapx->data_width;
	int i, j, cc;
	char *map, *iob;
	int cnt;

	if (dev->nmap)
		return -ENODEV;
	if (riob->bsize > vmeioMAX_BUF)
		return -E2BIG;
	iob = kmalloc(riob->bsize, GFP_KERNEL);
	if (!iob)
		return -ENOMEM;
	if ((map = mapx->vaddr) == NULL) {
		kfree(iob);
		return -ENODEV;
	}
	if (dev->debug > 1) {
		printk("RAW:READ:win:%d map:0x%p offs:0x%X amd:0x%2lx dwd:%d len:%d\n",
		     riob->winum, map, riob->offset,
		     mapx->address_modifier, dwidth,
		     riob->bsize);
	}

	cnt = GetClrBusErrCnt();

	for (i = 0, j = riob->offset; i < riob->bsize; i += dwidth, j += dwidth) {
		union vmeio_word *dst = (void *)&iob[i];
		if (dwidth == 4)
			dst->width4 = HRd32(&map[j]);
		else if (dwidth == 2)
			dst->width4 = HRd16(&map[j]);
		else
			dst->width1 = HRd8( &map[j]);
		if (GetClrBusErrCnt()) {
			kfree(iob);
			return -EIO;
		}
	}
	cc = copy_to_user(riob->buffer, iob, riob->bsize);
	kfree(iob);
	if (cc)
		return -EACCES;
	return 0;
}

static int raw_write(struct vmeio_device *dev, struct vmeio_riob_s *riob)
{
	struct vmeio_map *mapx = &dev->maps[riob->winum-1];	
	int dwidth = mapx->data_width;
	int i, j, cc;
	char *map, *iob;
	int cnt;

	if (dev->nmap)
		return -ENODEV;
	if (riob->bsize > vmeioMAX_BUF)
		return -E2BIG;
	iob = kmalloc(riob->bsize, GFP_KERNEL);
	if (!iob)
		return -ENOMEM;
	if ((map = mapx->vaddr) == NULL) {
		kfree(iob);
		return -ENODEV;
	}

	cc = copy_from_user(iob, riob->buffer, riob->bsize);
	if (cc < 0) {
		kfree(iob);
		return -EACCES;
	}

	if (dev->debug > 1) {
		printk("RAW:WRITE:win:%d map:0x%p ofs:0x%X amd:0x%2lx dwd:%d len:%d\n",
		     riob->winum, map, riob->offset,
		     mapx->address_modifier, dwidth,
		     riob->bsize);
	}

	cnt = GetClrBusErrCnt();
	for (i = 0, j = riob->offset; i < riob->bsize; i += dwidth, j += dwidth) {
		union vmeio_word *src = (void *)&iob[i];
		if (dwidth == 4)
			HWr32(src->width4, &map[j]);
		else if (dwidth == 2)
			HWr16(src->width2, &map[j]);
		else
			HWr8( src->width1, &map[j]);
		if (GetClrBusErrCnt()) {
			kfree(iob);
			return -EIO;
		}
	}
	kfree(iob);
	return 0;
}

/*
 * =====================================================
 */

int vmeio_ioctl(struct inode *inode, struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	void *arb;		/* Argument buffer area */

	struct vmeio_device *dev;

	int iodr;		/* Io Direction */
	int iosz;		/* Io Size in bytes */
	int cc = 0;		/* erro return code */

	long minor;

	iodr = _IOC_DIR(cmd);
	iosz = _IOC_SIZE(cmd);

	minor = MINOR(inode->i_rdev);
	if (!check_minor(minor))
		return -EACCES;
	dev = &devices[minor];

	if ((arb = kmalloc(iosz, GFP_KERNEL)) == NULL)
		return -ENOMEM;

	if ((iodr & _IOC_WRITE) && copy_from_user(arb, (void *)arg, iosz)) {
		cc = -EACCES;
		goto out;
	}
	debug_ioctl(_IOC_NR(cmd), iodr, iosz, arb, minor, dev->debug);

	if (!dev) {
		cc = -EACCES;
		goto out;
	}

	switch (cmd) {

	case VMEIO_SET_DEBUG:
		vmeio_set_debug(dev, arb);
		break;

	case VMEIO_GET_DEBUG:
		vmeio_get_debug(dev, arb);
		break;

	case VMEIO_GET_VERSION:
		vmeio_get_version(arb);
		break;

	case VMEIO_SET_TIMEOUT:
		vmeio_set_timeout(dev, arb);
		break;

	case VMEIO_GET_TIMEOUT:
		vmeio_get_timeout(dev, arb);
		break;

	case VMEIO_GET_DEVICE:	   /** Get the device described in struct vmeio_get_device_s */
		vmeio_get_device(dev, arb);
		break;

	case VMEIO_SET_DEVICE:     /** Changes the device memory map */
				  /** Super dangerous, experts only */
		vmeio_set_device(dev, arb);
		if (dev->maps[0].vaddr == NULL && dev->maps[1].vaddr == NULL)
			goto out;
		break;

	case VMEIO_RAW_READ_DMA:   /** Raw read VME registers */

		cc = raw_dma(dev, arb, VME_DMA_FROM_DEVICE);
		if (cc < 0)
			goto out;
		break;

	case VMEIO_RAW_WRITE_DMA:  /** Raw write VME registers */

		cc = raw_dma(dev, arb, VME_DMA_TO_DEVICE);
		if (cc < 0)
			goto out;
		break;

	case VMEIO_RAW_READ:	   /** Raw read VME registers */

		cc = raw_read(dev, arb);
		if (cc < 0)
			goto out;
		break;

	case VMEIO_RAW_WRITE:	   /** Raw write VME registers */
		cc = raw_write(dev, arb);
		if (cc < 0) 
			goto out;
		break;

	default:
		cc = -ENOENT;
		goto out;
		break;
	}

	if ((iodr & _IOC_READ) && copy_to_user((void *)arg, arb, iosz)) {
			cc = -EACCES;
			goto out;
	}
out:	kfree(arb);
	return cc;
}

/* ===================================================== */

static DEFINE_MUTEX(driver_mutex);

/* ===================================================== */

long vmeio_ioctl64(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int res;
	mutex_lock(&driver_mutex);
	res = vmeio_ioctl(filp->f_dentry->d_inode, filp, cmd, arg);
	mutex_unlock(&driver_mutex);
	return res;
}

/* ===================================================== */

long vmeio_ioctl32(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int res;
	mutex_lock(&driver_mutex);
	res = vmeio_ioctl(filp->f_dentry->d_inode, filp, cmd, arg);
	mutex_unlock(&driver_mutex);
	return res;
}

struct file_operations vmeio_fops = {
	.read = vmeio_read,
	.write = vmeio_write,
	.unlocked_ioctl = vmeio_ioctl32,
	.compat_ioctl = vmeio_ioctl64,
	.open = vmeio_open,
	.release = vmeio_close,
};


module_init(vmeio_install);
module_exit(vmeio_uninstall);
