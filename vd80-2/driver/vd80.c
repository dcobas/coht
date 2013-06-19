/**
 * =================================================
 * Implement driver for ADC sampler (VD80) module
 * Julian Lewis BE/CO/HT Thu 14th Mar 2013
 *
 * Here are the insmod parameters...
 *
 * luns Logical unit numbers
 * vecs Interrupt vectors
 * vmeb VD80 VME base address
 *
 * Installation via insmod parameters example
 *
 * Example: insmod btg.ko luns=0 vmeb=0xC000000 vecs=0xB8
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
#include "vd80.h"
#include "vd80P.h"

#define TSI148_LCSR_DSTA_DON (1<<25) /* DMA done bit */

#define CSR_WINDOW_SIZE 0x80000
#define VD80_WINDOW_SIZE 0x10000
#define VME_IRQ_LEVEL 2

#ifndef COMPILE_TIME
#define COMPILE_TIME 0
#endif

/*
 * ======================================================================
 * Static memory
 */

static int vd80_major = 0;
static char *vd80_major_name = "vd80";

MODULE_AUTHOR("Julian Lewis BE/CO/HT CERN");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("VD80 ADC Sampler Driver");
MODULE_SUPPORTED_DEVICE("VD80 VME Module");

/*
 * ==============================
 * Module parameter storage area
 * Indexed by minor device number
 */

static unsigned long luns[MAX_DEVICES];      /* Logical unit numbers */
static unsigned long vecs[MAX_DEVICES];      /* Interrupt vectors */
static unsigned long vmeb[MAX_DEVICES];      /* First VME base address */
static unsigned long slot[MAX_DEVICES];      /* Slot number of CSR space */

static unsigned int luns_num; /* Lun count */
static unsigned int vecs_num; /* ISR vector count */
static unsigned int vmeb_num; /* Base address count */
static unsigned int slot_num; /* Crate slot position count */

module_param_array(luns, ulong, &luns_num, 0444);
module_param_array(vecs, ulong, &vecs_num, 0444);
module_param_array(vmeb, ulong, &vmeb_num, 0444);
module_param_array(slot, ulong, &slot_num, 0444);

MODULE_PARM_DESC(luns, "Logical unit numbers");
MODULE_PARM_DESC(vecs, "Interrupt vectors");
MODULE_PARM_DESC(vmeb, "VD80 VME base addresses");
MODULE_PARM_DESC(slot, "Crate slot position 1..24 of CSR space");

/*========================================================================*/

/* Drivers working area global pointer */

static struct working_area_s wa;
struct file_operations vd80_fops;

struct ioctl_name_binding_s {
	vd80_ioctl_function_t  ionr;
	char                  *name;
};

static struct ioctl_name_binding_s ioctl_names[vd80LAST] = {

	{ vd80SET_SW_DEBUG,           "SET_SW_DEBUG" },            /** Set driver debug mode */
	{ vd80GET_SW_DEBUG,           "GET_SW_DEBUG" },            /** Get driver debug mode */
	{ vd80GET_VERSION,            "GET_VERSION" },             /** Get version date */
	{ vd80SET_TIMEOUT,            "SET_TIMEOUT" },             /** Set the read timeout value */
	{ vd80GET_TIMEOUT,            "GET_TIMEOUT" },             /** Get the read timeout value */
	{ vd80RESET,                  "RESET" },                   /** Reset the module re-establish connections */
	{ vd80CONNECT,                "CONNECT" },                 /** Connect to and object interrupt */
	{ vd80GET_CONNECT,            "GET_CONNECT" },             /** Get current connections */

	{ vd80GET_MODULE_COUNT,       "GET_MODULE_COUNT" },        /** Get the number of installed modules */
	{ vd80GET_MODULE_ADDRESS,     "GET_MODULE_ADDRESS" },      /** Get the VME module base address */

	{ vd80SET_CLOCK,              "SET_CLOCK" },               /** Set to a Vd80 clock (Clock) */
	{ vd80SET_CLOCK_DIVIDE_MODE,  "SET_CLOCK_DIVIDE_MODE" },   /** Sub sample or clock (DivideMode) */
	{ vd80SET_CLOCK_DIVISOR,      "SET_CLOCK_DIVISOR" },       /** A 16 bit integer so the lowest frequency is */
	{ vd80SET_CLOCK_EDGE,         "SET_CLOCK_EDGE" },          /** Set to a Vd80 edge (Edge) */
	{ vd80SET_CLOCK_TERMINATION,  "SET_CLOCK_TERMINATION" },   /** Set to a Vd80 termination (Termination) */
	{ vd80GET_CLOCK,              "GET_CLOCK" },               /** (Clock) */
	{ vd80GET_CLOCK_DIVIDE_MODE,  "GET_CLOCK_DIVIDE_MODE" },   /** (DivideMode) */
	{ vd80GET_CLOCK_DIVISOR,      "GET_CLOCK_DIVISOR" },       /** Diviser -1 -> 0 = 20KHz */
	{ vd80GET_CLOCK_EDGE,         "GET_CLOCK_EDGE" },          /** (Edge) */
	{ vd80GET_CLOCK_TERMINATION,  "GET_CLOCK_TERMINATION" },   /** (Termination) */

	{ vd80SET_TRIGGER,            "SET_TRIGGER" },             /** (Trigger) */
	{ vd80SET_TRIGGER_EDGE,       "SET_TRIGGER_EDGE" },        /** (Edge) */
	{ vd80SET_TRIGGER_TERMINATION,"SET_TRIGGER_TERMINATION" }, /** (Termination) */
	{ vd80GET_TRIGGER,            "GET_TRIGGER" },             /** (Trigger) */
	{ vd80GET_TRIGGER_EDGE,       "GET_TRIGGER_EDGE" },        /** (Edge) */
	{ vd80GET_TRIGGER_TERMINATION,"GET_TRIGGER_TERMINATION" }, /** (Termination) */
	{ vd80SET_ANALOGUE_TRIGGER,   "SET_ANALOGUE_TRIGGER" },    /** (AnalogTrig) */
	{ vd80GET_ANALOGUE_TRIGGER,   "GET_ANALOGUE_TRIGGER" },    /** (AnalogTrig) */

	{ vd80GET_STATE,              "GET_STATE" },               /** Returns a Vd80 state (State) */
	{ vd80SET_COMMAND,            "SET_COMMAND" },             /** Set to a Vd80 command (Command) */
	{ vd80READ_ADC,               "READ_ADC" },                /** 16Bit ADC vaule is signed and extended to int32_t */
	{ vd80READ_SAMPLE,            "READ_SAMPLE" },             /** Get acquired sample buffer */
	{ vd80GET_POSTSAMPLES,        "GET_POSTSAMPLES" },         /** Get the number of post samples you set */
	{ vd80SET_POSTSAMPLES,        "SET_POSTSAMPLES" },         /** Set the number of post samples you want */
	{ vd80GET_TRIGGER_CONFIG,     "GET_TRIGGER_CONFIG" },      /** Get Trig delay and min pre trig samples */
	{ vd80SET_TRIGGER_CONFIG,     "SET_TRIGGER_CONFIG" },      /** Set Trig delay and min pre trig samples */
	{ vd80RAW_READ,               "RAW_READ" },                /** Raw direct read from module address space */
	{ vd80RAW_WRITE,              "RAW_WRITE" },               /** Ditto for direct write */
};

/* ==================== */

int is_big_endian(void)
{
	int big = 1;
	char *cp = (char *) &big;
	return *cp ? 0: 1;
}

/*
 * =========================================================
 * VMEIO with bus error handling
 */

#define BUS_ERR_PRINT_THRESHOLD 2

static int bus_error_count = 0;	/* For all modules */
static int isr_bus_error = 0;	/* Bus error in an ISR */
static int last_bus_error = 0;	/* Last printed bus error */

/* ==================== */

static void bus_error_handler(struct vme_bus_error *error)
{
	bus_error_count++;
}

/* ==================== */

static int get_clr_bus_err_cnt(void)
{
	int res;

	res = bus_error_count;

	bus_error_count = 0;
	last_bus_error = 0;
	isr_bus_error = 0;

	return res;
}

/* ==================== */

static void check_bus_error(char *dw, char *dir, void *x)
{
	if (bus_error_count > last_bus_error &&
	    bus_error_count <= BUS_ERR_PRINT_THRESHOLD) {
		printk("VD80:BUS_ERROR:%s:%s-Address:0x%p\n",
			       dw, dir, x);

		if (isr_bus_error)
			printk("VD80:BUS_ERROR:In ISR occured\n");

		if (bus_error_count == BUS_ERR_PRINT_THRESHOLD)
			printk("VD80:BUS_ERROR:PrintSuppressed\n");

		isr_bus_error = 0;
		last_bus_error = bus_error_count;
	}
}

/* ==================== */
/* Used in ISR only     */

static uint32_t IHRd32(uint32_t *x)
{
	uint32_t res;

	isr_bus_error = 0;
	res = ioread32be(x);
	if (bus_error_count > last_bus_error)
		isr_bus_error = 1;
	return res;
}

/* ==================== */

static uint32_t HRd32(uint32_t *x)
{
	uint32_t res;

	res = ioread32be(x);
	check_bus_error("D32", "READ", x);
	return res;
}

/* ==================== */

static void IHWr32(uint32_t v, uint32_t *x)
{
	isr_bus_error = 0;
	iowrite32be(v, x);
	if (bus_error_count > last_bus_error)
		isr_bus_error = 1;
	return;
}

/* ==================== */

static void HWr32(uint32_t v, uint32_t *x)
{
	iowrite32be(v, x);
	check_bus_error("D32", "WRITE", x);
	return;
}

/* ==================== */

void setreg(struct vd80_device_s *dev, uint32_t byte_offset, uint32_t valu)
{
   uint32_t ioff = byte_offset/sizeof(uint32_t);
   struct vd80_map_s *map = &dev->map;
   HWr32(valu, &(map->vad[ioff]));
}

/* ==================== */

uint32_t getreg(struct vd80_device_s *dev, uint32_t byte_offset)
{
   uint32_t ioff = byte_offset/sizeof(uint32_t);
   struct vd80_map_s *map = &dev->map;
   return HRd32(&(map->vad[ioff]));
}

/* ==================== */

void setcsr(uint32_t *csr, uint32_t byte_offset, uint32_t int_offset, uint32_t valu)
{
   uint32_t ioff = byte_offset/sizeof(uint32_t) + int_offset;
   HWr32((uint32_t) valu, &(csr[ioff]));
}

/* ==================== */

uint32_t getcsr(uint32_t *csr, uint32_t byte_offset, uint32_t int_offset)
{
   uint32_t ioff = byte_offset/sizeof(uint32_t) + int_offset;
   return HRd32(&(csr[ioff]));
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

void unmap_window(uint32_t vme, uint32_t len)
{
	return_controller(vme,len);
}

/* ==================== */

int init_device(struct vd80_device_s *dev)
{
	uint32_t vme;
	struct vd80_map_s *map;
	uint32_t *csr;
	uint32_t tval;
	int i;

	struct vd80_mod_addr_s *mad;

	char *board_id = "VD80"; /* The board ID */

	map = &dev->map;
	mad = &map->mad;
	vme = mad->slot * CSR_WINDOW_SIZE;

	printk("VD80:Map CSR:Slot:%d VME:0x%X AM:0x%02X SZ:0x%X\n",mad->slot,vme,VME_CR_CSR,CSR_WINDOW_SIZE);
	csr = map_window(vme,VME_CR_CSR,CSR_WINDOW_SIZE);
	if (csr == NULL) {
		printk("VD80:Unable to map VME:CSR at slot:%d\n",mad->slot);
		return -1;
	}
	printk("VD80:CSR:Mapped:0x%p OK\n",csr);

	/* Check we have a CSR ROM at the given slot */

	if (((getcsr(csr,VD80_CR_SIG1,0) & 0xff) != 'C')
	||  ((getcsr(csr,VD80_CR_SIG2,0) & 0xff) != 'R')) {
		printk("VD80:Configuration Rom at slot:%d not found\n",mad->slot);
		return -1;
	}
	printk("VD80:Configuration Rom at slot:%d found OK\n",mad->slot);

	/* Check that the CSR space has the string "VD80" in the BOARD_ID location */

	for (i=0; i<strlen(board_id); i++) {
		tval = getcsr(csr,VD80_CR_BOARD_ID,i);
		if (board_id[i] != (char) (tval & 0xff)) {
			printk("VD80:Configuration Rom at slot:%d is NOT a VD80 board\n",mad->slot);
			return -1;
		}
	}
	printk("VD80:Board ID at slot:%d OK\n",mad->slot);

	/* Now get the VD80 firmware revision number from the CSR ROM */

	dev->revid = 0;
	printk("VD80:Firmware reverse revision ID:");
	for (i=sizeof(uint32_t)-1; i>=0; i--) {
		tval = getcsr(csr,VD80_CR_REV_ID,i);
		dev->revid |= ((tval & 0xFF) << (i*8));
		printk("%c",(char) tval);
	}
	printk("\n");

	/* Set up interrupts registers Vector and Level */

	setcsr(csr,VD80_CSR_IRQ_VECTOR,0,mad->vec);
	setcsr(csr,VD80_CSR_IRQ_LEVEL,0,VME_IRQ_LEVEL);

	/* Set little endian for MBLT DMA transfers if needed */

	if (is_big_endian()) {
		setcsr(csr,VD80_CSR_MBLT_ENDIAN,0,VD80_CSR_MBLT_BIG_ENDIAN);
		printk("VD80:Big-endian DMA: Hardware MBLT swap ON\n");
	} else {
		setcsr(csr,VD80_CSR_MBLT_ENDIAN,0,VD80_CSR_MBLT_LITTLE_ENDIAN);
		printk("VD80:Little-endian DMA: Hardware MBLT swap OFF\n");
	}

	/* Set up address spaces and modifiers */

	setcsr(csr,VD80_CSR_ADER0,0,(mad->vmeb >> 24) & 0xff);
	setcsr(csr,VD80_CSR_ADER0,1,(mad->vmeb >> 16) & 0xff);
	setcsr(csr,VD80_CSR_ADER0,2,(mad->vmeb >>  8) & 0xff);
	setcsr(csr,VD80_CSR_ADER0,3,VME_A24_USER_DATA_SCT*4);

	setcsr(csr,VD80_CR_FUNC0_AM_MASK,3,VME_A24_USER_MBLT*4);
	setcsr(csr,VD80_CR_FUNC1_AM_MASK,3,VME_A24_USER_MBLT*4);

	setcsr(csr,VD80_CSR_ADER1,0,(mad->vmeb >> 24) & 0xff);
	setcsr(csr,VD80_CSR_ADER1,1,(mad->vmeb >> 16) & 0xff);
	setcsr(csr,VD80_CSR_ADER1,2,(mad->vmeb >>  8) & 0xff);
	setcsr(csr,VD80_CSR_ADER1,3,VME_A24_USER_MBLT*4);

	setcsr(csr,VD80_CSR_FUNC_ACEN,0,0);
	setcsr(csr,VD80_CSR_BITSET,0,VD80_CSR_ENABLE_MODULE);

	/* Release the CSR space */

	printk("VD80:Unmap CSR window\n");
	unmap_window(vme,CSR_WINDOW_SIZE);

	map->vad = map_window(mad->vmeb,VME_A24_USER_DATA_SCT,VD80_WINDOW_SIZE); /* Map A24D32 */
	if (map->vad == NULL) {
		printk("VD80:Unable to map VME:A24D32 at slot:%d address:0x%X\n",mad->slot,mad->vmeb);
		return -1;
	}

	printk("VD80:mapped VME:A24D32 at slot:%d address:0x%X OK\n",mad->slot,mad->vmeb);
	return 0;
}

/* ==================== */

struct vme_berr_handler *set_berr_handler(uint32_t vme) {

	struct vme_bus_error berr;
	struct vme_berr_handler *handler;

	berr.address = vme;
	berr.am      = VME_A24_USER_DATA_SCT;
	handler      = vme_register_berr_handler(&berr, VD80_WINDOW_SIZE, bus_error_handler);

	if (IS_ERR(handler))
		handler = NULL;

	return handler;
}

/* ==================== */

void unregister_module(struct vd80_device_s *dev) {

	struct vd80_map_s      *map = &dev->map;
	struct vd80_mod_addr_s *mad = &map->mad;

	vme_intclr(mad->vec, NULL);
	return_controller(mad->vmeb, VD80_WINDOW_SIZE);
	vme_unregister_berr_handler(map->bus_error_handler);
}

/* ==================== */

static int raw_read(struct vd80_device_s *dev, int icnt, int ioff, uint32_t *ibuf)
{
	uint32_t *vad = dev->map.vad;
	int i, j;

	for (i=0,j=ioff; i<icnt; i++,j++) {
		ibuf[i] = HRd32(&vad[j]);
		if (get_clr_bus_err_cnt())
			break;
	}
	return i;
}

/* ==================== */

static int raw_write(struct vd80_device_s *dev, int icnt, int ioff, uint32_t *ibuf)
{
	uint32_t *vad = dev->map.vad;
	int i, j;

	for (i=0,j=ioff; i<icnt; i++,j++) {
		HWr32(ibuf[i], &vad[j]);
		if (get_clr_bus_err_cnt())
			break;
	}

	return i;
}

/* ==================== */

static int reset(struct vd80_device_s *dev)
{
	uint32_t tval;

	/*
	 * Beware the documentation is wrong !!
	 * This is what we need to do, trial and error
	 */

	tval = getreg(dev,VD80_GCR2);
	if (is_big_endian()) {
		tval &= ~VD80_BIGEND;
		tval &= ~VD80_BYTESWAP;
	} else {
		tval |= VD80_BIGEND;
		tval |= VD80_BYTESWAP;
	}
	setreg(dev,VD80_GCR2,tval);

	return 1;
}

/* ==================== */

#define DEBUG_ARG_PRINT 16

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

	printk("VD80:Debug:Level:%d:", debl);

	for (i=0; i<vd80LAST; i++) {
		inb = &(ioctl_names[i]);

		if (inb->ionr == ionr) {
			printk("ioctl:%s:%d\n",inb->name,ionr);
			break;
		}
	}
	if (i >= vd80LAST)
		printk("ioctl:illegal:%d\n",ionr);


	if (debl >= VD80_DEBUG_BUF) {
		if (flg == DEBUG_FROM_USER) {
			if (iodr & _IOC_WRITE)
				pbuf("VD80:from_user:",iosz,arb);

		} else if (flg == DEBUG_FROM_DRIVER) {
			if (iodr & _IOC_READ)
				pbuf("VD80:from_drvr",iosz,arb);

		}
	}
}

/* ==================== */

#define KLUDGE_DELAY_US 100000

void wait_for_idle(struct vd80_device_s *dev)
{
	uint32_t state;
	uint32_t delay;

	for (delay = 0; delay < KLUDGE_DELAY_US; delay += 10) {
		state = getreg(dev,VD80_GSR) & VD80_STATE_MASK;
		if (state == VD80_STATE_IDLE)
			return;
		udelay(10);
	}
	printk("vd80: wait_for_idle timed out after %d us\n", delay-10);
}

/**
 * ===========================================================
 * @brief Given a pointer split it into upr and lwr parts
 * @param upr address where upper part of arddress will be stored
 * @param lwr address where lower part of arddress will be stored
 * @param ptr is the given pointer value to be split
 */

void split_address(uint32_t *upr, uint32_t *lwr, uintptr_t ptr)
{
	*upr = (uint32_t) ((uint64_t) ptr >> 32);
	*lwr = (uint32_t) (ptr  & 0xFFFFFFFF);
}

/* =========================================================== */
/* Build a read DMA descriptor                                 */
/* Transfer one VME_DMA_BSIZE_4096 block or less               */

#define VME_NO_ADDR_INCREMENT 1
#define DMA_BLOCK_SIZE        4096
#define SAMPLES_IN_DMA_BLOCK  2048

void build_dma_desc(struct vd80_device_s *dev,
		    uintptr_t             dest,
		    uint32_t              len,
		    struct vme_dma       *dma_desc) {

	memset(dma_desc, 0, sizeof(struct vme_dma));

	dma_desc->dir            = VME_DMA_FROM_DEVICE;
	dma_desc->src.data_width = VME_D32;
	dma_desc->src.am         = VME_A24_USER_MBLT;
	dma_desc->novmeinc       = VME_NO_ADDR_INCREMENT;

	dma_desc->ctrl.pci_block_size   = VME_DMA_BSIZE_4096;
	dma_desc->ctrl.pci_backoff_time = VME_DMA_BACKOFF_0;
	dma_desc->ctrl.vme_block_size   = VME_DMA_BSIZE_4096;
	dma_desc->ctrl.vme_backoff_time = VME_DMA_BACKOFF_0;

	split_address(&dma_desc->src.addru, &dma_desc->src.addrl, (uintptr_t) dev->map.mad.vmeb);
	split_address(&dma_desc->dst.addru, &dma_desc->dst.addrl, dest);
	dma_desc->length    = len;
}

/* ==================== */

long __vd80_ioctl(struct file *filp, uint32_t cmd, uintptr_t arg)
{
	void *arb;
	long cc = 0;

	uint32_t iodr, iosz, ionr;
	uint32_t *ival;
	uint32_t tval, ireg;

	struct vme_dma dma_desc;    /* Vme driver DMA structure */

	uint32_t actpostrig = 0; /* The actual number of post trigger samples */
	uint32_t actpretrig = 0; /* The actual number of pre  trigger samples */
	uint32_t shotlength = 0; /* The total number od sample both pre and post trigger */
	uint32_t samplestrt = 0; /* Start position for reading */

	uint32_t  icnt;
	uint32_t  ioff;
	uint32_t *ibuf;

	struct vd80_sample_buf_s    *sbuf = NULL; /* Users sample buffer */
	struct vd80_analogue_trig_s *atrg = NULL; /* Analog trigger */
	struct vd80_trig_config_s   *tcon = NULL; /* Trigger configuration */

	struct vd80_riob_s *riob;
	struct vd80_mod_addr_s *mad;
	struct vd80_version_s *ver;
	struct vd80_device_s *dev = filp->private_data;

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

	case VD80_SET_SW_DEBUG:            /** Set driver debug level 0..15 */
		dev->deb_lvl = *ival;
		break;

	case VD80_GET_SW_DEBUG:            /** Get driver debug level */
		*ival = dev->deb_lvl;
		break;

	case VD80_GET_VERSION:             /** Get version date (struct vd80_version_s) */
		ver = (struct vd80_version_s *) arb;
		ver->drvrver = (COMPILE_TIME);
		ver->vhdlver = dev->revid;
		break;

	case VD80_SET_TIMEOUT:             /** Set the read timeout value */
		dev->timeout = msecs_to_jiffies(*ival);
		break;

	case VD80_GET_TIMEOUT:             /** Get the read timeout value */
		*ival = jiffies_to_msecs(dev->timeout);
		break;

	case VD80_RESET:                   /** Reset the module re-establish connections */
		reset(dev);
		break;

	case VD80_CONNECT:                 /** Connect/Disconnect interrupts */
		tval = *ival & VD80_INTERRUPT_MASK;
		if (tval)
			tval |= (getreg(dev,VD80_GCR2) & ~VD80_INTERRUPT_MASK);

		setreg(dev,VD80_GCR2,tval);
		break;

	case VD80_GET_CONNECT:
		*ival = getreg(dev,VD80_GCR2) & VD80_INTERRUPT_MASK;
		break;

	case VD80_GET_MODULE_COUNT:
		*ival = luns_num;
		break;

	case VD80_GET_MODULE_ADDRESS:      /** Get the VME module base address (struct vd80_mod_addr_s) */
		mad = (struct vd80_mod_addr_s *) arb;
		mad->lun  = dev->map.mad.lun;
		mad->vec  = dev->map.mad.vec;
		mad->vmeb = dev->map.mad.vmeb;
		mad->slot = dev->map.mad.slot;
		break;

	/* ============================ */
	/* Clock control                */

	case VD80_SET_CLOCK:               /** Set to a Vd80 clock (Clock) */
		tval = getreg(dev,VD80_CCR) & ~VD80_CLKSOURCE_MASK;
		if (*ival == VD80_SOURCE_EXTERNAL) ireg = VD80_CLKSOURCE_EXT;
		else                               ireg = VD80_CLKSOURCE_INT;
		tval &= ~VD80_CLKSOURCE_EXT;
		tval |= ireg;
		setreg(dev,VD80_CCR,tval);
		break;

	case VD80_SET_CLOCK_DIVIDE_MODE:   /** Sub sample or clock (DivideMode) */
		tval = getreg(dev,VD80_CCR) & ~VD80_CLKDIVMODE_MASK;
		if (*ival == VD80_DIVIDE_MODE_SUBSAMPLE) ireg =  VD80_CLKDIVMODE_SUBSAMPLE;
		else                                     ireg =  VD80_CLKDIVMODE_DIVIDE;
		tval &= ~VD80_CLKDIVMODE_SUBSAMPLE;
		tval |= ireg;
		setreg(dev,VD80_CCR,tval);
		break;

	case VD80_SET_CLOCK_DIVISOR:       /** A 16 bit integer so the lowest frequency is */
		tval  = getreg(dev,VD80_CCR) & ~VD80_CLKDIV_MASK;
		tval |= (*ival << VD80_CLKDIV_SHIFT) & VD80_CLKDIV_MASK;
		setreg(dev,VD80_CCR,tval);
		break;

	case VD80_SET_CLOCK_EDGE:          /** Set to a Vd80 edge (Edge) */
		tval = getreg(dev,VD80_CCR) & ~VD80_CLKEDGE_MASK;
		if (*ival == VD80_EDGE_FALLING) ireg = VD80_CLKEDGE_FALLING;
		else                            ireg = VD80_CLKEDGE_RISING;
		tval &= ~VD80_CLKEDGE_FALLING;
		tval |= ireg;
		setreg(dev,VD80_CCR,tval);
		break;

	case VD80_SET_CLOCK_TERMINATION:   /** Set to a Vd80 termination (Termination) */
		tval = getreg(dev,VD80_CCR) & ~VD80_CLKTERM_MASK;
		if (*ival == VD80_TERMINATION_50OHM) ireg = VD80_CLKTERM_50OHM;
		else                                 ireg = VD80_CLKTERM_NONE;
		tval &= ~VD80_CLKTERM_50OHM;
		tval |= ireg;
		setreg(dev,VD80_CCR,tval);
		break;

	case VD80_GET_CLOCK:               /** (Clock) */
		tval = getreg(dev,VD80_CCR) & VD80_CLKSOURCE_MASK;
		if      (tval == VD80_CLKSOURCE_INT) *ival = VD80_SOURCE_INTERNAL;
		else if (tval == VD80_CLKSOURCE_EXT) *ival = VD80_SOURCE_EXTERNAL;
		else                                 *ival = VD80_SOURCES;
		break;

	case VD80_GET_CLOCK_DIVIDE_MODE:   /** (DivideMode) */
		tval = getreg(dev,VD80_CCR) & VD80_CLKDIVMODE_MASK;
		if      (tval == VD80_CLKDIVMODE_DIVIDE)    *ival = VD80_DIVIDE_MODE_DIVIDE;
		else if (tval == VD80_CLKDIVMODE_SUBSAMPLE) *ival = VD80_DIVIDE_MODE_SUBSAMPLE;
		else                                        *ival = VD80_DIVIDE_MODES;
		break;

	case VD80_GET_CLOCK_DIVISOR:       /** Diviser -1 -> 0 = 20KHz */
		tval = getreg(dev,VD80_CCR) & VD80_CLKDIV_MASK;
		tval >>= VD80_CLKDIV_SHIFT;
		*ival = tval;
		break;

	case VD80_GET_CLOCK_EDGE:          /** (Edge) */
		tval = getreg(dev,VD80_CCR) & VD80_CLKEDGE_FALLING;
		if      (tval == VD80_CLKEDGE_RISING)  *ival = VD80_EDGE_RISING;
		else if (tval == VD80_CLKEDGE_FALLING) *ival = VD80_EDGE_FALLING;
		else                                   *ival = VD80_EDGES;
		break;

	case VD80_GET_CLOCK_TERMINATION:   /** (Termination) */
		tval = getreg(dev,VD80_CCR) & VD80_CLKTERM_MASK;
		if      (tval == VD80_CLKTERM_NONE)  *ival = VD80_TERMINATION_NONE;
		else if (tval == VD80_CLKTERM_50OHM) *ival = VD80_TERMINATION_50OHM;
		else                                 *ival = VD80_TERMINATIONS;
		break;

	/* ============================ */
	/* Trigger control              */

	case VD80_SET_TRIGGER:             /** (Trigger) */
		tval = getreg(dev,VD80_TCR1) & ~VD80_TRIGSOURCE_MASK;
		if (*ival == VD80_SOURCE_INTERNAL) ireg = VD80_TRIGSOURCE_INT;
		else                               ireg = VD80_TRIGSOURCE_EXT;
		tval &= ~VD80_TRIGSOURCE_MASK;
		tval |= ireg;
		setreg(dev,VD80_TCR1,tval);
		break;

	case VD80_SET_TRIGGER_EDGE:        /** (Edge) */
		tval = getreg(dev,VD80_TCR1) & ~VD80_TRIGEDGE_MASK;
		if (*ival == VD80_EDGE_FALLING) ireg = VD80_TRIGEDGE_FALLING;
		else                            ireg = VD80_TRIGEDGE_RISING;
		tval &= ~VD80_TRIGEDGE_FALLING;
		tval |= ireg;
		setreg(dev,VD80_TCR1,tval);
		break;

	case VD80_SET_TRIGGER_TERMINATION: /** (Termination) */
		tval = getreg(dev,VD80_TCR1) & ~VD80_TRIG_TERM_MASK;
		if (*ival == VD80_TERMINATION_50OHM) ireg = VD80_TRIG_TERM_50OHM;
		else                                 ireg = VD80_TRIG_TERM_NONE;
		tval &= ~VD80_TRIG_TERM_50OHM;
		tval |= ireg;
		setreg(dev,VD80_TCR1,tval);
		break;

	case VD80_GET_TRIGGER:             /** (Trigger) */
		tval  = getreg(dev,VD80_TCR1) & VD80_TRIGSOURCE_MASK;
		if      (tval == VD80_TRIGSOURCE_INT)    *ival = VD80_SOURCE_INTERNAL;
		else if (tval == VD80_TRIGSOURCE_EXT)    *ival = VD80_SOURCE_EXTERNAL;
		else if (tval == VD80_TRIGSOURCE_ANALOG) *ival = VD80_SOURCE_INTERNAL;
		else                                     *ival = VD80_SOURCES;
		break;

	case VD80_GET_TRIGGER_EDGE:        /** (Edge) */
		tval = getreg(dev,VD80_TCR1) & VD80_TRIGEDGE_MASK;
		if      (tval == VD80_TRIGEDGE_RISING)  *ival = VD80_EDGE_RISING;
		else if (tval == VD80_TRIGEDGE_FALLING) *ival = VD80_EDGE_FALLING;
		else                                    *ival = VD80_EDGES;
		break;

	case VD80_GET_TRIGGER_TERMINATION: /** (Termination) */
		tval = getreg(dev,VD80_TCR1) & VD80_TRIG_TERM_MASK;
		if      (tval == VD80_TRIG_TERM_NONE)  *ival = VD80_TERMINATION_NONE;
		else if (tval == VD80_TRIG_TERM_50OHM) *ival = VD80_TERMINATION_50OHM;
		else                                   *ival = VD80_TERMINATIONS;
		break;

	case VD80_SET_ANALOGUE_TRIGGER:    /** (AnalogTrig) */
		atrg = (struct vd80_analogue_trig_s *) arb;
		for (ireg=0; ireg<VD80_CHANNELS; ireg++)
			setreg(dev,VD80_ATRIG_CHAN1+(ireg*4),0);

		ireg = atrg->channel;
		if ((ireg<1) || (ireg>VD80_CHANNELS)) {
			cc = -EACCES;
			goto out;
		}
		ireg--;

		tval = getreg(dev,VD80_TCR1) & ~VD80_TRIGSOURCE_MASK;
		tval &= ~VD80_TRIGSOURCE_MASK;
		tval |= VD80_TRIGSOURCE_ANALOG;
		setreg(dev,VD80_TCR1,tval);

		tval = ( (atrg->level   & 0xFFF) << VD80_ATRIG_LEVEL_ABOVE_SHIFT
		     |   (atrg->level   & 0xFFF) << VD80_ATRIG_LEVEL_BELOW_SHIFT
		     |   (atrg->control & 0x007) |  VD80_ATRIG_CHANGE_EDGE
		     );

		setreg(dev,VD80_ATRIG_CHAN1+(ireg*4),tval);
		break;

	case VD80_GET_ANALOGUE_TRIGGER:    /** (AnalogTrig) */
		atrg = (struct vd80_analogue_trig_s *) arb;
		ireg = atrg->channel;
		if ((ireg<1) || (ireg>VD80_CHANNELS)) {
			cc = -EACCES;
			goto out;
		}
		ireg--;

		tval = getreg(dev,VD80_ATRIG_CHAN1+(ireg*4));

		atrg->control = tval & 0x007;
		atrg->level   = (tval >> VD80_ATRIG_LEVEL_ABOVE_SHIFT) & 0xFFF;
		break;

	/* ============================ */
	/* State control                */

	case VD80_GET_STATE:               /** Returns a Vd80 state (State) */
		*ival = getreg(dev,VD80_GSR) & VD80_STATE_MASK;
		break;

	case VD80_SET_COMMAND:             /** Set to a Vd80 command (Command) */
		tval  = *ival & VD80_COMMAND_MASK;
		if (tval == VD80_COMMAND_READ) tval |= *ival & VD80_OPERANT_MASK;
		setreg(dev,VD80_GCR1,tval);

		if (tval == VD80_COMMAND_STOP)
			wait_for_idle(dev);
		break;

	case VD80_READ_ADC:                /** 16Bit ADC vaule is signed and extended to int32_t */
		if ((*ival <1 ) && (*ival >16)) {
			cc = -EACCES;
			goto out;
		}

		ireg = (*ival -1) >> 1;
		ireg = VD80_ADCR1 + (ireg * 4);
		tval = getreg(dev,ireg);
		if (*ival & 1) *ival = tval & 0xFFFF;
		else           *ival = tval >> 16;
		break;

	case VD80_READ_SAMPLE:             /** Get acquired sample buffer */
		sbuf = (struct vd80_sample_buf_s *) arb;

		/* Check that the requested post samples have been set */

		tval = getreg(dev,VD80_TCR2);
		if (tval == 0) {
			cc = -EACCES;
			goto out;
		}

		/* Force the module into the idle state */

		tval = VD80_COMMAND_SUBSTOP;
		setreg(dev,VD80_GCR1,tval);
		tval = VD80_COMMAND_STOP;
		setreg(dev,VD80_GCR1,tval);
		wait_for_idle(dev);

		/* actpostrig is the number of actual post trigger samples */
		/* shotlength is the total number of pre and post trigger samples */

		actpostrig = (getreg(dev,VD80_TSR) << VD80_ACTPOSTSAMPLES_SHIFT) & VD80_ACTPOSTSAMPLES_MASK;
		shotlength = (getreg(dev,VD80_SSR) << VD80_SHOTLEN_SHIFT) & VD80_SHOTLEN_MASK;

		sbuf->pre_trig_stat = getreg(dev,VD80_PTSR); /* 100K ticks since start */

		/* actpretrig is the actual number of pre trigger samples */

		actpretrig =  shotlength - actpostrig;
		if (sbuf->trig_position > actpretrig)     /* User asked for too many pre trigger samples */
			sbuf->trig_position = actpretrig; /* so give him what we have */

		/* samplestrt is the position in buffer to start reading from */

		samplestrt =  shotlength - (actpostrig + sbuf->trig_position); /* This has to be posative from above */

		/* sbuf->samples_read is the total number we are going to read */

		sbuf->samples_read = sbuf->buf_size_samples;
		if (samplestrt + sbuf->buf_size_samples > shotlength) /* User asked for too many post samples */
			sbuf->samples_read = (shotlength - samplestrt);

		/* Set up the readstart and readlength registers */

		tval = samplestrt / 32;             /* Start Address divided by 32 */
		tval = (tval << VD80_READSTART_SHIFT) & VD80_READSTART_MASK;
		setreg(dev,VD80_MCR1,tval);

		sbuf->trig_position = actpretrig - (tval*32); /* The real actual trigger position in buffer */

		if (sbuf->samples_read < 32)
			tval = 0;                          /* Reads 32 samples */
		else
			tval = (sbuf->samples_read - 1) / 32;   /* Divide by 32 */

		tval = (tval << VD80_READLEN_SHIFT) & VD80_READLEN_MASK;
		setreg(dev,VD80_MCR2,tval);

		/* Start the read for the given channel */

		tval = VD80_COMMAND_READ | (((sbuf->channel -1)<<VD80_OPERANT_SHIFT) & VD80_OPERANT_MASK);
		setreg(dev,VD80_GCR1,tval);

		if (dev->deb_lvl > VD80_DEBUG_BUF) {
			printk("vd80: actpostrig: %d\n",actpostrig);
			printk("vd80: shotlength: %d\n",shotlength);
			printk("vd80: actpretrig: %d\n",actpretrig);
			printk("vd80: samplestrt: %d\n",samplestrt);

			printk("vd80: sbuf->samples_read : %d\n",sbuf->samples_read );
			printk("vd80: sbuf->trig_position: %d\n",sbuf->trig_position);
			printk("vd80: sbuf->pre_trig_stat: %d\n",sbuf->pre_trig_stat);
		}

		build_dma_desc(dev,
			       (uintptr_t) sbuf->sample_buf,
			       sbuf->samples_read*sizeof(short),
			       &dma_desc);

		if (vme_do_dma(&dma_desc)) {

			if (dev->deb_lvl > VD80_DEBUG_BUF)
				printk("vd80: vme_do_dma: Failed\n");

			cc = -EACCES;
			goto out;
		}

		break;

	case VD80_SET_POSTSAMPLES:         /** Set the number of post samples you want */
		tval  = getreg(dev,VD80_PTCR);
		tval |= VD80_PSCNTCLRONSTART;
		tval &= ~(VD80_PSCNTCLKONSMPL | VD80_PSREGCLRONREAD);
		setreg(dev,VD80_PTCR,tval);

		if (*ival) tval = (*ival-1) / 32;
		else       tval = 0;
		setreg(dev,VD80_TCR2,tval);
		break;

	case VD80_GET_POSTSAMPLES:         /** Get the number of post samples you set */
		tval = getreg(dev,VD80_TCR2);
		*ival = (tval + 1) * 32;
		break;

	case VD80_GET_TRIGGER_CONFIG:      /** Get Trig delay and min pre trig samples */
		tcon = (struct vd80_trig_config_s *) arb;
		tval = getreg(dev,VD80_TCR1);
		if (tval & VD80_TRIGOUT_DELAY)
			tcon->trig_delay = getreg(dev,VD80_TCR3);
		else
			tcon->trig_delay = 0;

		ireg   = getreg(dev,VD80_PTCR);
		ireg  &= VD80_PRESAMPLESMIN_MASK;
		ireg >>= VD80_PRESAMPLESMIN_SHIFT;
		ireg  *= 32;
		tcon->min_pre_trig = ireg;
		break;

	case VD80_SET_TRIGGER_CONFIG:      /** Set Trig delay and min pre trig samples */
		tcon = (struct vd80_trig_config_s *) arb;

		tval = getreg(dev,VD80_TCR1);
		if (tcon->trig_delay) {
			tval |= VD80_TRIGOUT_DELAY;
			setreg(dev,VD80_TCR1,tval);
			setreg(dev,VD80_TCR3,tcon->trig_delay);
		} else {
			tval &=~VD80_TRIGOUT_DELAY;
			setreg(dev,VD80_TCR1,tval);
			setreg(dev,VD80_TCR3,0);
		}

		tval = getreg(dev,VD80_PTCR) & ~VD80_PRESAMPLESMIN_MASK;
		if (tcon->min_pre_trig) {
			ireg   = tcon->min_pre_trig/32;
			ireg <<= VD80_PRESAMPLESMIN_SHIFT;
			ireg  &= VD80_PRESAMPLESMIN_MASK;
			tval |= ireg;
		}
		tval |= VD80_PSCNTCLRONSTART;
		tval &= ~(VD80_PSCNTCLKONSMPL | VD80_PSREGCLRONREAD);
		setreg(dev,VD80_PTCR,tval);
		break;

	case VD80_RAW_READ:                /** Raw direct read from module address space (struct vd80_riob_s) */
		riob = (struct vd80_riob_s *) arb;
		icnt = riob->bsize/sizeof(uint32_t);
		ioff = riob->boffs/sizeof(uint32_t);
		if ((ibuf = kmalloc((icnt * sizeof(uint32_t)), GFP_KERNEL)) == NULL) {
			cc = -ENOMEM;
			goto out;
		}
		if (raw_read(dev,icnt,ioff,ibuf) != icnt) {
			cc = -EACCES;
			kfree(ival);
			goto out;
		}
		if (copy_to_user(riob->buffer, ibuf, icnt * sizeof(uint32_t))) {
			kfree(ibuf);
			cc = -EACCES;
			goto out;
		}
		kfree(ibuf);
		riob->bsize = icnt*sizeof(uint32_t);
		riob->boffs = ioff*sizeof(uint32_t);
		break;

	case VD80_RAW_WRITE:               /** Raw direct write to module address space (struct vd80_riob_s) */
		riob = (struct vd80_riob_s *) arb;
		icnt = riob->bsize/sizeof(uint32_t);
		ioff = riob->boffs/sizeof(uint32_t);
		if ((ibuf = kmalloc((icnt * sizeof(uint32_t)), GFP_KERNEL)) == NULL) {
			cc = -ENOMEM;
			goto out;
		}
		if (copy_from_user(ibuf, riob->buffer, icnt * sizeof(uint32_t))) {
			kfree(ival);
			cc = -EACCES;
			goto out;
		}
		if (raw_write(dev,icnt,ioff,ibuf) != icnt) {
			cc = -EACCES;
			kfree(ibuf);
			goto out;
		}
		kfree(ibuf);
		riob->bsize = icnt*sizeof(uint32_t);
		riob->boffs = ioff*sizeof(uint32_t);
		break;

	default:
		cc = -ENOENT;
		goto out;
		break;
	}

	if (dev->deb_lvl)
		debug_ioctl(dev->deb_lvl, iodr, iosz, ionr, arb, DEBUG_FROM_DRIVER);

	if ((iodr & _IOC_READ) && (tval = copy_to_user((void *) arg, arb, iosz))) {
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

static irqreturn_t vd80_irq(void *arg)
{
	struct vd80_device_s     *dev = arg;
	uint32_t                 *vad = dev->map.vad;
	uint32_t                 isrc;

	isrc = IHRd32(&vad[VD80_GSR/4]) >> VD80_GSR_INTERRUPT_SHIFT;
	isrc = isrc & VD80_INTERRUPT_MASK;
	if (!isrc)
		return IRQ_NONE;

	IHWr32(VD80_INTCLR,&vad[VD80_GCR1/4]);

	wa.ibuf.src = isrc;
	wa.ibuf.cnt++;
	wa.ibuf.lun = luns[dev->dev_idx];
	wake_up(&wa.queue);

	return IRQ_HANDLED;
}

/*
 * =====================================================
 * Install
 * =====================================================
 */

int vd80_install(void)
{
	int i, cc;

	printk("VD80:Installing driver for %d luns\n",luns_num);

	if (luns_num <= 0 || luns_num > MAX_DEVICES) {
		printk("VD80:Fatal:No logical units defined.\n");
		return -EACCES;
	}
	wa.devcnt = luns_num;

	if (vecs_num != luns_num) {
		printk("VD80:Fatal:Missing interrupt vector.\n");
		return -EACCES;
	}

	if (vmeb_num != luns_num) {
		printk("VD80:Fatal:Missing VME base address.\n");
		return -EACCES;
	}

	if (slot_num != luns_num) {
		printk("VD80:Fatal:Missing VME slot number.\n");
		return -EACCES;
	}

	init_waitqueue_head(&wa.queue);

	for (i=0; i<luns_num; i++) {

		struct vd80_device_s   *dev = &wa.devs[i];
		struct vd80_map_s      *map = &dev->map;
		struct vd80_mod_addr_s *mad = &map->mad;

		mad->lun  = luns[i];
		mad->vec  = vecs[i];
		mad->vmeb = vmeb[i];
		mad->slot = slot[i];

		mutex_init(&dev->mutex);

		if (init_device(dev) < 0) {
			printk("VD80:lun:%d Not installed\n",mad->lun);
			continue;
		}
		map->bus_error_handler = set_berr_handler(mad->vmeb);

		vme_intset(mad->vec, (int (*)(void *)) vd80_irq, dev, 0);

		printk("VD80:Installed:lun:%d vec:0x%X vme:0x%X -> map:0x%p\n",
		       mad->lun,mad->vec,mad->vmeb,map->vad);

		dev->dev_idx = i;

		reset(dev);
	}

	/* Register driver */

	cc = register_chrdev(vd80_major, vd80_major_name, &vd80_fops);
	if (cc < 0) {
		printk("VD80:Fatal:Error from register_chrdev [%d]\n",cc);
		return cc;
	}
	if (vd80_major == 0)
		vd80_major = cc;       /* dynamic */

	return 0;
}

/*
 * =====================================================
 * Uninstall the driver
 * =====================================================
 */

void vd80_uninstall(void)
{
	int i;

	for (i=0; i<wa.devcnt; i++) {
		unregister_module(&wa.devs[i]);
	}
	unregister_chrdev(vd80_major, vd80_major_name);
	printk("VD80:driver uninstalled\n");
}

/*
 * =====================================================
 * Open
 * =====================================================
 */

int vd80_open(struct inode *inode, struct file *filp)
{
	int lun;
	struct vd80_device_s *dev;

	lun = MINOR(inode->i_rdev);
	if (lun >= MAX_DEVICES) {
		printk("VD80:Open:Illegal or uninstalled device:%d\n",lun);
		return -ENODEV;
	}
	dev = &wa.devs[lun];
	filp->private_data = dev;
	return 0;
}

/*
 * =====================================================
 * Close
 * =====================================================
 */

int vd80_close(struct inode *inode, struct file *filp)
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

ssize_t vd80_read(struct file * filp, char *buf, size_t count, loff_t * f_pos)
{
	struct vd80_device_s *dev;

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
	if (wcnt > sizeof(struct vd80_int_buf_s))
		wcnt = sizeof(struct vd80_int_buf_s);

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

long vd80_ioctl(struct file *filp, uint32_t cmd, unsigned long arg)
{
	int res;
	struct vd80_device_s *dev;

	dev = filp->private_data;
	mutex_lock(&dev->mutex);
	get_clr_bus_err_cnt();
	res = __vd80_ioctl(filp, cmd, arg);
	mutex_unlock(&dev->mutex);

	return res;
}

/* ==================== */

struct file_operations vd80_fops = {
	.read = vd80_read,
	.unlocked_ioctl = vd80_ioctl,
	.open = vd80_open,
	.release = vd80_close,
};

/* ==================== */

module_init(vd80_install);
module_exit(vd80_uninstall);
