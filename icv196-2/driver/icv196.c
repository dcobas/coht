/**
 * =================================================
 * Implement driver for digital IO (ICV196) module
 * Julian Lewis BE/CO/HT Tue 2nd July 2013
 *
 * Here are the insmod parameters...
 *
 * luns Logical unit numbers
 * vecs Interrupt vectors
 * vmeb ICV196 VME base address
 *
 * Installation via insmod parameters example
 *
 * Example: insmod icv196.ko luns=0 vmeb=0xC000000 vecs=0xB8
 *
 * ======================================================================
 * Includes
 */

#include <linux/kernel.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/types.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/delay.h>

#include <asm/page.h>
#include <linux/mm.h>
#include <linux/page-flags.h>
#include <linux/pagemap.h>
#include <linux/time.h>
#include <linux/delay.h>

#include <vmebus.h>
#include <icv196.h>
#include <icv196P.h>

#define ICV196_WINDOW_SIZE 0x100
#define VME_IRQ_LEVEL 2

#ifndef COMPILE_TIME
#define COMPILE_TIME 0
#endif

/*
 * ======================================================================
 * Static memory
 */

static int icv196_major = 0;
static char *icv196_major_name = "icv196";

MODULE_AUTHOR("Julian Lewis BE/CO/HT CERN");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("ICV196 Digital IO Driver");
MODULE_SUPPORTED_DEVICE("ICV196 VME Module");
MODULE_VERSION(GIT_VERSION);

/*
 * ==============================
 * Module parameter storage area
 * Indexed by minor device number
 */

static unsigned long luns[MAX_DEVICES];      /* Logical unit numbers */
static unsigned long vecs[MAX_DEVICES];      /* Interrupt vectors */
static unsigned long vmeb[MAX_DEVICES];      /* First VME base address */

static unsigned int luns_num; /* Lun count */
static unsigned int vecs_num; /* ISR vector count */
static unsigned int vmeb_num; /* Base address count */

module_param_array(luns, ulong, &luns_num, 0444);
module_param_array(vecs, ulong, &vecs_num, 0444);
module_param_array(vmeb, ulong, &vmeb_num, 0444);

MODULE_PARM_DESC(luns, "Logical unit numbers");
MODULE_PARM_DESC(vecs, "Interrupt vectors");
MODULE_PARM_DESC(vmeb, "ICV196 VME base addresses");

/*========================================================================*/

/* Drivers working area global pointer */

static struct working_area_s wa;
struct file_operations icv196_fops;

struct ioctl_name_binding_s {
	icv196_ioctl_function_t  ionr;
	char                    *name;
};

static struct ioctl_name_binding_s ioctl_names[icv196LAST] = {

	{ icv196SET_SW_DEBUG,       "SET_SW_DEBUG" },       /** Set driver debug mode */
	{ icv196GET_SW_DEBUG,       "GET_SW_DEBUG" },       /** Get driver debug mode */
	{ icv196GET_VERSION,        "GET_VERSION" },        /** Get version date */
	{ icv196SET_TIMEOUT,        "SET_TIMEOUT" },        /** Set the read timeout value */
	{ icv196GET_TIMEOUT,        "GET_TIMEOUT" },        /** Get the read timeout value */
	{ icv196RESET,              "RESET" },              /** Reset the module re-establish connections */
	{ icv196CONNECT,            "CONNECT" },            /** Connect to 16-bit (0xFFFF) interrupt mask */
	{ icv196GET_CONNECT,        "GET_CONNECT" },        /** Get 16-bit (0xFFFF) interrupt mask */

	{ icv196GET_DEVICE_COUNT,   "GET_DEVICE_COUNT" },   /** Get the number of installed modules */
	{ icv196GET_DEVICE_ADDRESS, "GET_DEVICE_ADDRESS" }, /** Get the VME module base address */

	{ icv196RAW_READ,           "RAW_READ" },           /** Raw direct read from module address space */
	{ icv196RAW_WRITE,          "RAW_WRITE" },          /** Ditto for direct write */

	{ icv196BYTE_GET_OUTPUT,    "BYTE_GET_OUTPUT" },    /** Get 12-bit (0xFFF) byte output mask 8X12=96 */
	{ icv196BYTE_READ,          "BYTE_READ" },          /** Read bytes from module */
	{ icv196BYTE_SET_OUTPUT,    "BYTE_SET_OUTPUT" },    /** Set 12-bit (0xFFF) byte output mask 8X12=96 */
	{ icv196BYTE_WRITE,         "BYTE_WRITE" },         /** Ditto for write bytes */
};

/* ==================== */
/* Set control register */

void setcon(struct icv196_device_s *dev, uint8_t valu)
{
	struct icv196_map_s *map = &dev->map;
	iowrite8(valu,&(map->vad[CoReg_Z8536]));
}

/* ==================== */
/* Get status register  */

uint8_t getsts(struct icv196_device_s *dev)
{
	struct icv196_map_s *map = &dev->map;
	uint8_t val = ioread8(&(map->vad[CoReg_Z8536]));
	return val;
}

/* ==================== */
/* Set Z8536 register   */

void setreg(struct icv196_device_s *dev, uint8_t byte_offset, uint8_t valu)
{
	struct icv196_map_s *map = &dev->map;
	iowrite8(byte_offset, &(map->vad[CoReg_Z8536]));          /* Write byte address */
	iowrite8(valu,        &(map->vad[CoReg_Z8536]));          /* then write the byte to that address */
}

/* ==================== */
/* Get Z8536 register   */

uint8_t getreg(struct icv196_device_s *dev, uint8_t byte_offset)
{
	uint8_t valu;
	struct icv196_map_s *map = &dev->map;
	iowrite8(byte_offset, &(map->vad[CoReg_Z8536]));          /* Write byte address */
	valu = ioread8(&(map->vad[CoReg_Z8536]));                 /* then read the byte from that address */
	return valu;
}

/* ==================== */

void *map_window(uint32_t vme, uint32_t am, uint32_t sze)
{
	void *vadr;
	struct pdparam_master param;

	param.iack = 1;                 /* no iack */
	param.rdpref = 0;               /* no VME read prefetch option */
	param.wrpost = 0;               /* no VME write posting option */
	param.swap = 1;                 /* VME auto swap option */
	param.dum[0] = VME_PG_SHARED;   /* window is sharable */
	param.dum[1] = 0;               /* XPC ADP-type */
	param.dum[2] = 0;               /* window is sharable */

	vadr = (void *) find_controller(vme, sze, am, 0, 4, &param);
	if ((long) vadr == -1)
		return NULL;

	return vadr;
}

/* ==================== */

int init_device(struct icv196_device_s *dev)
{
	struct icv196_map_s *map;
	struct icv196_mod_addr_s *mad;

	map = &dev->map;
	mad = &map->mad;

	map->vad = map_window(mad->vmeb,VME_A24_USER_DATA_SCT,ICV196_WINDOW_SIZE); /* Map A24D32 */
	if (map->vad == NULL) {
		printk("ICV196:Unable to map VME:A24D32 address:0x%X\n",mad->vmeb);
		return -1;
	}

	printk("ICV196:mapped VME:A24D32 address:0x%X OK\n",mad->vmeb);
	return 0;
}

/* ==================== */

void unregister_module(struct icv196_device_s *dev) {

	struct icv196_map_s      *map = &dev->map;
	struct icv196_mod_addr_s *mad = &map->mad;

	vme_intclr(mad->vec, NULL);
	return_controller(mad->vmeb, ICV196_WINDOW_SIZE);
}

/* ==================== */
/* Raw 8-bit block read */

static int raw_read(struct icv196_device_s *dev, uint8_t icnt, uint8_t ioff, uint8_t *ibuf)
{
	uint8_t *vad = dev->map.vad;
	int i, j;

	for (i=0,j=ioff; i<icnt; i++,j++)
		ibuf[i] = ioread8(&vad[j]);
	return i;
}

/* ==================== */
/* 8-bit block write    */

static int raw_write(struct icv196_device_s *dev, uint8_t icnt, uint8_t ioff, uint8_t *ibuf)
{
	uint8_t *vad = dev->map.vad;
	int i, j;

	for (i=0,j=ioff; i<icnt; i++,j++)
		iowrite8(ibuf[i], &vad[j]);
	return i;
}

/* ====================================== */
/* Reset device to base state:            */
/* No interrupts, all channels are inputs */

#define ICV196_INTERRUPT_LEVEL_2 0xFD

static int reset(struct icv196_device_s *dev)
{
	struct icv196_map_s *map;
	struct icv196_mod_addr_s *mad;

	uint8_t tmp;

	map = &dev->map;
	mad = &map->mad;

	tmp = getsts(dev);
	setcon(dev,MIC_reg);
	tmp = getsts(dev);

	setcon(dev,MIC_reg);
	setcon(dev,b_RESET);
	setcon(dev,0);

	iowrite8(ICV196_INTERRUPT_LEVEL_2,&map->vad[CSNIT_ICV]);
	iowrite16be(0,&map->vad[CSDIR_ICV]);

	setreg(dev,MCC_reg, PortA_Enable | PortB_Enable);
	setreg(dev,MSpec_Areg, PtrM_Or | Latch_On_Pat_Match);
	setreg(dev,MSpec_Breg, PtrM_Or | Latch_On_Pat_Match);
	setreg(dev,CSt_Areg, CoSt_SeIe);
	setreg(dev,CSt_Breg, CoSt_SeIe);
	setreg(dev,DPPol_Areg, Non_Invert);
	setreg(dev,DPPol_Breg, Non_Invert);
	setreg(dev,DDir_Areg, All_Input);
	setreg(dev,DDir_Breg, All_Input);
	setreg(dev,SIO_Areg, Norm_Inp);
	setreg(dev,SIO_Breg, Norm_Inp);
	setreg(dev,PtrMsk_Areg, All_Masked);
	setreg(dev,PtrTr_Areg, 0);
	setreg(dev,PtrPo_Areg, 0);
	setreg(dev,PtrMsk_Breg, All_Masked);
	setreg(dev,PtrTr_Breg, 0);
	setreg(dev,PtrPo_Breg, 0);

	setreg(dev,MIC_reg, 0);
	setreg(dev,ItVct_Areg,mad->vec);
	setreg(dev,ItVct_Breg,mad->vec);
	setreg(dev,MIC_reg, b_MIE);

	dev->out_msk = 0; /** All inputs */

	return 1;
}

/* ==================== */

#define DEBUG_ARG_PRINT 8

void pbuf(char *cp, int iosz, uint32_t *uip)
{
	int i, isz;

	isz = iosz/sizeof(uint32_t);
	if (isz > DEBUG_ARG_PRINT)
		isz = DEBUG_ARG_PRINT;

	if (isz) {
		printk("%s ArgBuf:[%d]\n",cp,iosz);
		for (i=0; i<isz; i++) {
			if ((i+1 % 4) == 0)
				printk("\n%02d:",i);
			printk("0x%08X-%d\t",uip[i],uip[i]);
		}
		printk("\n");
	}
}

/* ==================== */

#define DEBUG_FROM_USER 1
#define DEBUG_FROM_DRIVER 2

void debug_ioctl(int debl, int iodr, int iosz, unsigned int ionr, void *arb, int flg)
{
	struct ioctl_name_binding_s *inb;
	int i;

	if (!debl)
		return;

	printk("ICV196:Debug:Level:%d:", debl);

	for (i=0; i<icv196LAST; i++) {
		inb = &(ioctl_names[i]);

		if (inb->ionr == ionr) {
			printk("ioctl:%s:%d\n",inb->name,ionr);
			break;
		}
	}
	if (i >= icv196LAST)
		printk("ioctl:illegal:%d\n",ionr);


	if (debl >= ICV196_DEBUG_BUF) {
		if (flg == DEBUG_FROM_USER) {
			if (iodr & _IOC_WRITE)
				pbuf("ICV196:from_user:",iosz,arb);

		} else if (flg == DEBUG_FROM_DRIVER) {
			if (iodr & _IOC_READ)
				pbuf("ICV196:from_drvr",iosz,arb);

		}
	}
}

/* ==================== */

long __icv196_ioctl(struct file *filp, uint32_t cmd, uintptr_t arg)
{
	void *arb;
	long cc = 0;

	uint8_t bl, br, bt;
	uint32_t i, msk;

	uint32_t iodr, iosz, ionr;
	uint32_t *ival;

	uint8_t  icnt;
	uint8_t  ioff;
	uint8_t *ibuf;

	struct icv196_riob_s *riob;
	struct icv196_digiob_s *digiob;
	struct icv196_version_s *ver;

	struct icv196_device_s   *dev = filp->private_data;
	struct icv196_map_s      *map = &dev->map;
	struct icv196_mod_addr_s *mad = &map->mad;

	iodr = _IOC_DIR(cmd);
	iosz = _IOC_SIZE(cmd);
	ionr = _IOC_NR(cmd);

	if ((arb = kmalloc(iosz, GFP_KERNEL)) == NULL)
		return -ENOMEM;

	ival = arb; /** N.B. This is OK because arg points to at least one uint32_t !! */

	if ((iodr & _IOC_WRITE) && copy_from_user(arb, (void *) arg, iosz)) {
		cc = -EACCES;
		goto out;
	}

	if (dev->deb_lvl)
		debug_ioctl(dev->deb_lvl, iodr, iosz, ionr, arb, DEBUG_FROM_USER);

	switch (cmd) {

	/* ============================ */
	/* General support calls        */

	case ICV196_SET_SW_DEBUG:            /** Set driver debug level 0..15 */
		dev->deb_lvl = *ival;
		break;

	case ICV196_GET_SW_DEBUG:            /** Get driver debug level */
		*ival = dev->deb_lvl;
		break;

	case ICV196_GET_VERSION:             /** Get version date (struct icv196_version_s) */
		ver = (struct icv196_version_s *) arb;
		ver->drvrver = (COMPILE_TIME);
		ver->vhdlver = dev->revid;
		break;

	case ICV196_SET_TIMEOUT:             /** Set the read timeout value */
		dev->timeout = msecs_to_jiffies(*ival);
		break;

	case ICV196_GET_TIMEOUT:             /** Get the read timeout value */
		*ival = jiffies_to_msecs(dev->timeout);
		break;

	case ICV196_RESET:                   /** Reset the module to base state */
		reset(dev);                  /** No interrupts enabled, all channels inputs */
		break;

	case ICV196_CONNECT:                 /** Connect/Disconnect interrupts */

		if (dev->out_msk & 3) { /* Bytes 0/1 must be set as an input */
			cc = -EBUSY;
			goto out;
		}

		dev->int_msk = *ival & 0xFFFF;

		br = *ival & 0x00FF;            /* 16 lines of J1 input right byte */
		bl = *ival & 0xFF00 >> 8;       /* 16 lines of J1 input left byte */

		bt = getreg(dev,MIC_reg) | b_MIE;
		setreg(dev,MIC_reg, bt);

		/** Porta */

		if (br) {

			setreg(dev,CSt_Areg, CoSt_ClIpIus); /* Enable porta interrupts */

			bt = getreg(dev,PtrPo_Areg) | br;
			setreg(dev,PtrPo_Areg, bt);
			bt = getreg(dev,PtrTr_Areg) | br;
			setreg(dev,PtrTr_Areg, bt);
			bt = getreg(dev,PtrMsk_Areg) | br;
			setreg(dev,PtrMsk_Areg, bt);

			bt = getreg(dev,PtrPo_Areg);
			bt = getreg(dev,PtrTr_Areg);
			bt = getreg(dev,PtrMsk_Areg);

			setreg(dev,CSt_Areg, CoSt_ClIpIus); /* Enable porta interrupts */
		}

		/** Portb */

		if (bl) {

			setreg(dev,CSt_Breg, CoSt_ClIpIus); /* Enable portb interrupts */

			bt = getreg(dev,PtrPo_Breg) | bl;
			setreg(dev,PtrPo_Breg, bt);
			bt = getreg(dev,PtrTr_Breg) | bl;
			setreg(dev,PtrTr_Breg, bt);
			bt = getreg(dev,PtrMsk_Breg) | bl;
			setreg(dev,PtrMsk_Breg, bt);

			bt = getreg(dev,PtrPo_Breg);
			bt = getreg(dev,PtrTr_Breg);
			bt = getreg(dev,PtrMsk_Breg);

			setreg(dev,CSt_Breg, CoSt_ClIpIus); /* Enable portb interrupts */
		}

		break;

	case ICV196_GET_CONNECT:                 /** Connect/Disconnect interrupts */
		*ival = dev->int_msk;
		break;

	case ICV196_GET_DEVICE_COUNT:
		*ival = luns_num;
		break;

	case ICV196_GET_DEVICE_ADDRESS:      /** Get the VME module base address (struct icv196_mod_addr_s) */
		mad = (struct icv196_mod_addr_s *) arb;
		mad->lun  = dev->map.mad.lun;
		mad->vec  = dev->map.mad.vec;
		mad->vmeb = dev->map.mad.vmeb;
		break;

	case ICV196_RAW_READ:                /** Raw direct read from module address space (struct icv196_riob_s) */
		riob = (struct icv196_riob_s *) arb;
		icnt = riob->bsize;
		ioff = riob->boffs;
		ibuf = kmalloc(icnt, GFP_KERNEL);
		if (ibuf  == NULL) {
			cc = -ENOMEM;
			goto out;
		}
		if (raw_read(dev,icnt,ioff,ibuf) != icnt) {
			cc = -EIO;
			kfree(ival);
			goto out;
		}
		if (copy_to_user(riob->buffer, ibuf, icnt)) {
			kfree(ibuf);
			cc = -EACCES;
			goto out;
		}
		kfree(ibuf);
		riob->bsize = icnt;
		riob->boffs = ioff;
		break;

	case ICV196_RAW_WRITE:               /** Raw direct write to module address space (struct icv196_riob_s) */
		riob = (struct icv196_riob_s *) arb;
		icnt = riob->bsize;
		ioff = riob->boffs;
		ibuf = kmalloc(icnt, GFP_KERNEL);
		if (ibuf  == NULL) {
			cc = -ENOMEM;
			goto out;
		}
		if (copy_from_user(ibuf, riob->buffer, icnt)) {
			kfree(ival);
			cc = -EACCES;
			goto out;
		}
		if (raw_write(dev,icnt,ioff,ibuf) != icnt) {
			cc = -EIO;
			kfree(ibuf);
			goto out;
		}
		kfree(ibuf);
		riob->bsize = icnt;
		riob->boffs = ioff;
		break;

	case ICV196_BYTE_SET_OUTPUT:
		if ( ((dev->int_msk & 1) && (*ival & 1))
		||   ((dev->int_msk & 2) && (*ival & 2)) ) {
			cc = -EISCONN;                        /* That byte is connected to interrupts */
			goto out;                             /* it cant be an output */
		}
		dev->out_msk = *ival & 0xFFF;
		iowrite16be(dev->out_msk,&map->vad[CSDIR_ICV]);
		break;


	case ICV196_BYTE_WRITE:
		digiob = (struct icv196_digiob_s *) arb;
		for (i=0; i<MAX_BYTES; i++) {

			msk = 1 << i;
			if (!(digiob->msk & msk))
				continue;

			if (!(dev->out_msk & msk)) {
				cc = -EPERM;
				goto out;
			}

			iowrite8(digiob->val[i],&map->vad[CSGbase_ICV+i]);
		}
		break;

	case ICV196_BYTE_GET_OUTPUT:
		*ival = dev->out_msk;
		break;

	case ICV196_BYTE_READ:
		digiob = (struct icv196_digiob_s *) arb;
		for (i=0; i<MAX_BYTES; i++) {

			msk = 1 << i;
			if (!(digiob->msk & msk))
				continue;

			digiob->val[i] = ioread8(&map->vad[CSGbase_ICV+i]);
		}
		break;

	default:
		cc = -ENOENT;
		goto out;
		break;
	}

	if (dev->deb_lvl)
		debug_ioctl(dev->deb_lvl, iodr, iosz, ionr, arb, DEBUG_FROM_DRIVER);

	if ((iodr & _IOC_READ) && copy_to_user((void *) arg, arb, iosz)) {
			cc = -EACCES;
			goto out;
	}
out:	kfree(arb);
	return cc;
}

/*
 * =========================================================
 * Interrupt service routine
 * =========================================================
 */

static irqreturn_t icv196_irq(void *arg)
{
	struct icv196_device_s *dev = arg;
	struct icv196_map_s    *map = &dev->map;

	uint8_t porta = getreg(dev,CSt_Areg) & CoSt_Ius;
	uint8_t portb = getreg(dev,CSt_Breg) & CoSt_Ius;
	uint32_t msk;

	if (!(porta || portb))
		return IRQ_NONE;

	setreg(dev,CSt_Areg, CoSt_ClIpIus);
	setreg(dev,CSt_Breg, CoSt_ClIpIus);

	porta = ioread8(&map->vad[CSGbase_ICV+0]);
	portb = ioread8(&map->vad[CSGbase_ICV+1]);

	msk = ( ((portb << 8) | porta) & dev->int_msk );
	if (msk) {
		wa.ibuf.src = msk;
		wa.ibuf.cnt++;
		wa.ibuf.dev = luns[dev->dev_idx];
		wake_up(&wa.queue);
	}
	return IRQ_HANDLED;
}

/*
 * =====================================================
 * Install
 * =====================================================
 */

int icv196_install(void)
{
	int i, cc;

	printk("ICV196:Installing driver for %d luns\n",luns_num);

	if (luns_num <= 0 || luns_num > MAX_DEVICES) {
		printk("ICV196:Fatal:No logical units defined.\n");
		return -EACCES;
	}
	wa.devcnt = luns_num;

	if (vecs_num != luns_num) {
		printk("ICV196:Fatal:Missing interrupt vector.\n");
		return -EACCES;
	}

	if (vmeb_num != luns_num) {
		printk("ICV196:Fatal:Missing VME base address.\n");
		return -EACCES;
	}

	init_waitqueue_head(&wa.queue);

	for (i=0; i<luns_num; i++) {

		struct icv196_device_s   *dev = &wa.devs[i];
		struct icv196_map_s      *map = &dev->map;
		struct icv196_mod_addr_s *mad = &map->mad;

		mad->lun  = luns[i];
		mad->vec  = vecs[i];
		mad->vmeb = vmeb[i];

		mutex_init(&dev->mutex);

		if (init_device(dev) < 0) {
			printk("ICV196:lun:%d Not installed\n",mad->lun);
			continue;
		}
		vme_intset(mad->vec, (int (*)(void *)) icv196_irq, dev, 0);

		printk("ICV196:Installed:lun:%d vec:0x%X vme:0x%X -> map:0x%p\n",
		       mad->lun,mad->vec,mad->vmeb,map->vad);

		dev->dev_idx = i;

		reset(dev);
	}

	/* Register driver */

	cc = register_chrdev(icv196_major, icv196_major_name, &icv196_fops);
	if (cc < 0) {
		printk("ICV196:Fatal:Error from register_chrdev [%d]\n",cc);
		return cc;
	}
	if (icv196_major == 0)
		icv196_major = cc;       /* dynamic */

	return 0;
}

/*
 * =====================================================
 * Uninstall the driver
 * =====================================================
 */

void icv196_uninstall(void)
{
	int i;

	for (i=0; i<wa.devcnt; i++) {
		reset(&wa.devs[i]);
		unregister_module(&wa.devs[i]);
	}
	unregister_chrdev(icv196_major, icv196_major_name);
	printk("ICV196:driver uninstalled\n");
}

/*
 * =====================================================
 * Open
 * =====================================================
 */

int icv196_open(struct inode *inode, struct file *filp)
{
	int lun;
	struct icv196_device_s *dev;

	lun = MINOR(inode->i_rdev);
	if (lun >= MAX_DEVICES) {
		printk("ICV196:Open:Illegal or uninstalled device:%d\n",lun);
		return -ENODEV;
	}
	dev = &wa.devs[lun];
	filp->private_data = dev;
	dev->timeout = 10000;
	return 0;
}

/*
 * =====================================================
 * Close
 * =====================================================
 */

int icv196_close(struct inode *inode, struct file *filp)
{
	filp->private_data = NULL;
	return 0;
}

/*
 * =====================================================
 * Read
 * This waits for any interrupt on any connected device
 * No locking is needed here or in the ISR
 * There's no queue, always waits for the next interrupt
 * =====================================================
 */

ssize_t icv196_read(struct file * filp, char *buf, size_t count, loff_t * f_pos)
{
	struct icv196_device_s *dev;

	uint32_t icnt;
	ssize_t  wcnt;
	int      cc;

	dev = filp->private_data;

	icnt = wa.ibuf.cnt;
	if (dev->timeout) {
		cc = wait_event_interruptible_timeout(wa.queue,
						      icnt != wa.ibuf.cnt,
						      dev->timeout);
		if (cc == 0)
			return -ETIME;
	} else
		cc = wait_event_interruptible(wa.queue,
					      icnt != wa.ibuf.cnt);
	if (cc < 0)
		return cc;

	wcnt = count;
	if (wcnt > sizeof(struct icv196_int_buf_s))
		wcnt = sizeof(struct icv196_int_buf_s);

	cc = copy_to_user(buf, &wa.ibuf, wcnt);
	if (cc)
		return -EACCES;

	return wcnt;
}

/*
 * =====================================================
 * IOCTL calls
 * =====================================================
 */

long icv196_ioctl(struct file *filp, uint32_t cmd, unsigned long arg)
{
	int res;
	struct icv196_device_s *dev;

	dev = filp->private_data;
	mutex_lock(&dev->mutex);
	res = __icv196_ioctl(filp, cmd, arg);
	mutex_unlock(&dev->mutex);

	return res;
}

/* ==================== */

struct file_operations icv196_fops = {
	.read = icv196_read,
	.unlocked_ioctl = icv196_ioctl,
	.open = icv196_open,
	.release = icv196_close,
};

/* ==================== */

module_init(icv196_install);
module_exit(icv196_uninstall);
