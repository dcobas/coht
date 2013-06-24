/**
 * =================================================
 * Implement driver to do basic VME IO
 * Julian Lewis BE/CO/HT Tue 19th October 2010
 */

#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "vmebus.h"
#include "vmeio.h"

#ifdef ENCORE_DAL
#include "ctc_regs.c"
#endif

static char *gendata = "encore_gendata:""siglesia|Samuel Iglesias Gonsalvez|2012-02-15 10:29:49.901731|libencore-29-gdff0228";


#define PFX DRIVER_NAME ": "

MODULE_AUTHOR("Samuel Iglesias Gonsalvez");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("encore-generated ctc driver");
MODULE_SUPPORTED_DEVICE("ctc");

/* vcector parameters, one entry per lun */

static unsigned int lun_num;
static long lun[DRV_MAX_DEVICES];
module_param_array(lun, long, &lun_num, S_IRUGO);
MODULE_PARM_DESC(lun, "Logical unit numbers");

static unsigned int base_address1_num;
static long base_address1[DRV_MAX_DEVICES];
module_param_array(base_address1, long, &base_address1_num, S_IRUGO);
MODULE_PARM_DESC(base_address1, "First map base addresses");

static unsigned int base_address2_num;
static long base_address2[DRV_MAX_DEVICES];
module_param_array(base_address2, long, &base_address2_num, S_IRUGO);
MODULE_PARM_DESC(base_address2, "Second map base addresses");

static unsigned int vector_num;
static long vector[DRV_MAX_DEVICES];
module_param_array(vector, long, &vector_num, S_IRUGO);
MODULE_PARM_DESC(vector, "Interrupt vectors");

/* parameters common to all luns */

static long level;
module_param(level, long, S_IRUGO);
MODULE_PARM_DESC(level, "Interrupt levels");

static long am1;
module_param(am1, long, S_IRUGO);
MODULE_PARM_DESC(am1, "First VME address modifier");

static long am2;
module_param(am2, long, S_IRUGO);
MODULE_PARM_DESC(am2, "Second VME address modifier");

static long data_width1;
module_param(data_width1, long, S_IRUGO);
MODULE_PARM_DESC(data_width1, "First data width 1,2,4,8 bytes");

static long data_width2;
module_param(data_width2, long, S_IRUGO);
MODULE_PARM_DESC(data_width2, "Second data width 1,2,4,8 bytes");

static long size1;
module_param(size1, long, S_IRUGO);
MODULE_PARM_DESC(size1, "First mapping size in bytes");

static long size2;
module_param(size2, long, S_IRUGO);
MODULE_PARM_DESC(size2, "Second mapping size in bytes");

static long isrc;
module_param(isrc, long, S_IRUGO);
MODULE_PARM_DESC(isrc, "Location of interrupt source reg in base_address1");

/*
 * vmeio device descriptor:
 *	maps[max_maps]		array of VME mappings
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
	struct vme_mapping	maps[MAX_MAPS];

	int			vector;
	int			level;
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
static dev_t vmeio_major;
struct file_operations vmeio_fops;

int minor_ok(long num)
{
	if (num < 0 || num >= DRV_MAX_DEVICES) {
		printk(PFX "minor:%d ", (int) num);
		printk("BAD not in [0..%d]\n", DRV_MAX_DEVICES - 1);
		return 0;
	}
	return 1;
}

static irqreturn_t vmeio_irq(void *arg)
{
	struct vmeio_device *dev = arg;
	long data;

	if (dev->isr_source_address) {
		unsigned long data_width = dev->maps[0].data_width;
		if (data_width == 4)
			data = ioread32be(dev->isr_source_address);
		else if (data_width == 2)
			data = ioread16be(dev->isr_source_address);
		else
			data = ioread8(dev->isr_source_address);
		dev->isr_source_mask = data;
	}
	dev->icnt++;
	wake_up(&dev->queue);
	return IRQ_HANDLED;
}

int register_isr(struct vmeio_device *dev, unsigned vector, unsigned level)
{
	int err;

	err = vme_request_irq(vector, (int (*)(void *)) vmeio_irq, dev, DRIVER_NAME);
	dev->isrfl = !(err < 0);
	printk(PFX "ISR:Level:0x%X Vector:0x%X:%s\n",
		level, vector,
		(err < 0) ? "ERROR:NotRegistered" : "OK:Registered");
	return err;
}

void register_int_source(struct vmeio_device *dev, void *map, unsigned offset)
{
	dev->isr_source_address = dev->maps[0].kernel_va + dev->isrc;
	/* printk(KERN_INFO PFX "SourceRegister:0x%p\n", dev->isr_source_address);
	*/
}

static int check_module_params(void)
{
	if (lun_num <= 0 || lun_num > DRV_MAX_DEVICES) {
		printk(PFX "Fatal:No logical units defined.\n");
		return -EACCES;
	}

	/* Vector parameters must all be the same size */

	if (vector_num != lun_num && vector_num != 0) {
		printk(PFX "Fatal:Missing interrupt vector.\n");
		return -EACCES;
	}

	if (base_address1_num != lun_num && base_address1_num != 0) {
		printk(PFX "Fatal:Missing first base address.\n");
		return -EACCES;
	}
	if (base_address2_num != lun_num && base_address2_num != 0) {
		printk(PFX "Fatal:Missing second base address.\n");
		return -EACCES;
	}
	return 0;
}

static int map(struct vme_mapping *desc,
	unsigned base_address, unsigned data_width, unsigned am, unsigned size)
{
	desc->data_width = data_width;
	desc->am = am;
	desc->vme_addru = 0;
	desc->vme_addrl = base_address;
	desc->sizeu = 0;
	desc->sizel = size;
	desc->read_prefetch_enabled = 0;

	return vme_find_mapping(desc, 1);
}

int install_device(struct vmeio_device *dev, unsigned i)
{
	memset(dev, 0, sizeof(*dev));

	dev->lun = lun[i];
	dev->debug = DEBUG;

	/* configure mmapped I/O */
	if (base_address1_num && map(&dev->maps[0], base_address1[i],
						data_width1, am1, size1)) {
		printk(KERN_ERR PFX "could not map lun:%d, first space\n",
							dev->lun);
		goto out_map1;
	}

	if (base_address2_num && map(&dev->maps[1], base_address2[i],
						data_width2, am2, size2)) {
		printk(KERN_ERR PFX "could not map lun:%d, second space\n",
							dev->lun);
		goto out_map2;
	}

	/* configure interrupt handling */
	dev->vector  = vector[i];
	dev->level  = level;
	dev->isrc = isrc;
	dev->timeout = msecs_to_jiffies(TIMEOUT);
	dev->icnt = 0;
	init_waitqueue_head(&dev->queue);

	if (dev->level && dev->vector &&
		register_isr(dev, dev->vector, dev->level) < 0) {
			printk(KERN_ERR PFX "could not register isr "
				"for vector %d, level %d\n",
				dev->vector, dev->level);
			goto out_isr;
	}
	/* This will be eventually removed */
	register_int_source(dev, dev->maps[0].kernel_va, dev->isrc);

	return 0;

out_isr:
	vme_release_mapping(&dev->maps[1], 1);
out_map2:
	vme_release_mapping(&dev->maps[0], 1);
out_map1:
	return -ENODEV;

}

int vmeio_install(void)
{
	int i, cc;

	printk(KERN_ERR PFX "%s\n", gendata);	/* ACET string */
	if ((cc = check_module_params()) != 0)
		return cc;

	for (i = 0; i < lun_num; i++) {
		if (install_device(&devices[i], i) == 0)
			continue;
		/* error, bail out */
		printk(KERN_ERR PFX
			"ERROR: lun %d not installed, quitting\n",
			devices[i].lun);
		return -1;
	}

	/* Register driver */
	cc = register_chrdev(0, DRIVER_NAME, &vmeio_fops);
	if (cc < 0) {
		printk(PFX "Fatal:Error from register_chrdev [%d]\n", cc);
		return cc;
	}
	vmeio_major = cc;
	return 0;
}

void unregister_module(struct vmeio_device *dev)
{
	if (dev->vector)
		vme_free_irq(dev->vector);
	if (dev->maps[0].kernel_va)
		vme_release_mapping(&dev->maps[0], 1);
	if (dev->maps[1].kernel_va)
		vme_release_mapping(&dev->maps[1], 1);
}

void vmeio_uninstall(void)
{
	int i;

	for (i = 0; i < lun_num; i++) {
		unregister_module(&devices[i]);
	}
	unregister_chrdev(vmeio_major, DRIVER_NAME);
}

/* file operations */

int vmeio_open(struct inode *inode, struct file *filp)
{
	long minor;
	int i;

	minor = MINOR(inode->i_rdev);
	if (!minor_ok(minor))
		return -EACCES;
	for (i = 0; i < lun_num; i++) {
		if (devices[i].lun == minor) {
			filp->private_data = &devices[i];
			return 0;
		}
	}
	return -ENODEV;
}

int vmeio_close(struct inode *inode, struct file *filp)
{
	long num;

	num = MINOR(inode->i_rdev);
	if (!minor_ok(num))
		return -EACCES;
	filp->private_data = NULL;

	return 0;
}

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
	if (!minor_ok(minor))
		return -EACCES;
	dev = filp->private_data;

	if (dev->debug) {
		printk(PFX "read:count:%zd minor:%d\n",
		       count, (int) minor);
		if (dev->debug > 1) {
			printk(PFX "read:timout:%d\n", dev->timeout);
		}
	}

	if (count < sizeof(rbuf)) {
		if (dev->debug) {
			printk(PFX "read:Access error buffer too small\n");
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
		printk(PFX "wait_event:returned:%d\n", cc);
	}

	if (cc == -ERESTARTSYS) {
		printk(PFX "vmeio_read:interrupted by signal\n");
		return cc;
	}
	if (cc == 0 && dev->timeout)
		return -ETIME;	/* Timer expired */
	if (cc < 0)
		return cc;	/* Error */

	rbuf.logical_unit = dev->lun;
	rbuf.interrupt_mask = dev->isr_source_mask;
	rbuf.interrupt_count = dev->icnt;

	cc = copy_to_user(buf, &rbuf, sizeof(rbuf));
	if (cc != 0) {
		printk(PFX "Can't copy to user space:cc=%d\n", cc);
		return -EACCES;
	}
	return sizeof(struct vmeio_read_buf_s);
}

ssize_t vmeio_write(struct file * filp, const char *buf, size_t count,
		    loff_t * f_pos)
{
	long minor;
	struct vmeio_device *dev;
	struct inode *inode;
	int cc, mask;

	inode = filp->f_dentry->d_inode;
	minor = MINOR(inode->i_rdev);
	if (!minor_ok(minor))
		return -EACCES;
	dev = filp->private_data;

	if (count >= sizeof(int)) {
		cc = copy_from_user(&mask, buf, sizeof(int));
		if (cc != 0) {
			printk(PFX "write:Error:%d could not copy from user\n", cc);
			return -EACCES;
		}
	}

	if (dev->debug) {
		printk(PFX "write:count:%zd minor:%d mask:0x%X\n",
		       count, (int) minor, mask);
	}

	dev->isr_source_mask = mask;
	dev->icnt++;
	wake_up(&dev->queue);
	return sizeof(int);
}

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

	printk(PFX "debug_ioctl:ionr:%d", ionr);
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
	*version = COMPILE_TIME;
}

static void vmeio_set_timeout(struct vmeio_device *dev, int *timeout)
{
	dev->timeout = msecs_to_jiffies(*timeout);
}

static void vmeio_get_timeout(struct vmeio_device *dev, int *timeout)
{
	*timeout = jiffies_to_msecs(dev->timeout);
}

static int get_mapping(struct vmeio_device *dev, struct vmeio_get_mapping *mapping)
{
	int mapnum = mapping->mapnum-1;

	if (!(mapnum >=0 && mapnum < MAX_MAPS))
		return -EINVAL;
	memcpy(&mapping->map, &dev->maps[mapnum], sizeof(mapping->map));
	return 0;
}

static int do_raw_dma(struct vmeio_dma_op *request)
{
	struct vme_dma dma_desc;
	unsigned int bu, bl;
	int cc;
	unsigned int haddr;

	bl = (unsigned long)request->buffer;
	bu = 0;

	memset(&dma_desc, 0, sizeof(dma_desc));

	dma_desc.dir = request->direction;
	dma_desc.novmeinc = 0;
	dma_desc.length = request->byte_length;

	dma_desc.ctrl.pci_block_size = VME_DMA_BSIZE_4096;
	dma_desc.ctrl.pci_backoff_time = VME_DMA_BACKOFF_0;
	dma_desc.ctrl.vme_block_size = VME_DMA_BSIZE_4096;
	dma_desc.ctrl.vme_backoff_time = VME_DMA_BACKOFF_0;

	dma_desc.dst.data_width = request->data_width;
	dma_desc.dst.am = request->am;
	dma_desc.src.data_width = request->data_width;
	dma_desc.src.am = request->am;

	haddr = request->address;

	if (request->direction == VME_DMA_TO_DEVICE) {
		dma_desc.src.addrl = bl;
		dma_desc.src.addru = bu;
		dma_desc.dst.addrl = haddr;
	} else {
		dma_desc.src.addrl = haddr;
		dma_desc.dst.addrl = bl;
		dma_desc.dst.addru = bu;
	}

	if ((cc = vme_do_dma(&dma_desc)) < 0)
		return cc;

	return 0;
}

static int raw_dma(struct vmeio_device *dev,
	struct vmeio_riob *riob, enum vme_dma_dir direction)
{
	struct vme_mapping *map = &dev->maps[riob->mapnum];
	struct vmeio_dma_op req;

	req.am = map->am;
	req.data_width = map->data_width;
	if (riob->data_width != 0)
		req.data_width = riob->data_width;
	req.address = map->vme_addrl + riob->offset;
	req.byte_length = riob->wsize * req.data_width/8;
	req.buffer = riob->buffer;
	req.direction = direction;

	return do_raw_dma(&req);
}

union vmeio_word {
	unsigned int	width4;
	unsigned short	width2;
	unsigned char	width1;
};

static int raw_read(struct vmeio_device *dev, struct vmeio_riob *riob)
{
	struct vme_mapping *mapx = &dev->maps[riob->mapnum-1];
	int dwidth = riob->data_width ? riob->data_width : mapx->data_width;
	int byte_dwidth = dwidth/8;
	int bsize = riob->wsize * byte_dwidth;
	int i, j, cc;
	char *map, *iob;

	if (bsize > vmeioMAX_BUF)
		return -E2BIG;
	iob = kmalloc(bsize, GFP_KERNEL);
	if (!iob)
		return -ENOMEM;
	if ((map = mapx->kernel_va) == NULL) {
		kfree(iob);
		return -ENODEV;
	}
	if (dev->debug > 1) {
		printk("RAW:READ:win:%d map:0x%p offs:0x%X amd:0x%2x dwd:%d words:%d\n",
		     riob->mapnum, mapx->kernel_va, riob->offset,
		     mapx->am, dwidth, riob->wsize);
	}

	for (i = 0, j = riob->offset; i < bsize; i += byte_dwidth, j += byte_dwidth) {
		union vmeio_word *dst = (void *)&iob[i];
		if (dwidth == VME_D32)
			dst->width4 = ioread32be(&map[j]);
		else if (dwidth == VME_D16)
			dst->width2 = ioread16be(&map[j]);
		else if (dwidth == VME_D8)
			dst->width1 = ioread8(&map[j]);
		else
			printk(KERN_ERR PFX "invalid data width %d\n", dwidth);
	}
	cc = copy_to_user(riob->buffer, iob, bsize);
	kfree(iob);
	if (cc)
		return -EACCES;
	return 0;
}

static int raw_write(struct vmeio_device *dev, struct vmeio_riob *riob)
{
	struct vme_mapping *mapx = &dev->maps[riob->mapnum-1];
	int dwidth = riob->data_width ? riob->data_width : mapx->data_width;
	int byte_dwidth = dwidth/8;
	int bsize = riob->wsize * byte_dwidth;
	int i, j, cc;
	char *map, *iob;

	if (bsize > vmeioMAX_BUF)
		return -E2BIG;
	iob = kmalloc(bsize, GFP_KERNEL);
	if (!iob)
		return -ENOMEM;
	if ((map = mapx->kernel_va) == NULL) {
		kfree(iob);
		return -ENODEV;
	}

	cc = copy_from_user(iob, riob->buffer, bsize);
	if (cc < 0) {
		kfree(iob);
		return -EACCES;
	}

	if (dev->debug > 1) {
		printk("RAW:WRITE:win:%d map:0x%p ofs:0x%X amd:0x%2x dwd:%d words:%d\n",
		     riob->mapnum, mapx->kernel_va, riob->offset,
		     mapx->am, dwidth, riob->wsize);
	}

	for (i = 0, j = riob->offset; i < bsize; i += byte_dwidth, j += byte_dwidth) {
		union vmeio_word *src = (void *)&iob[i];
		if (dwidth == VME_D32)
			iowrite32be(src->width4, &map[j]);
		else if (dwidth == VME_D16)
			iowrite16be(src->width2, &map[j]);
		else if (dwidth == VME_D8)
			iowrite8(src->width1, &map[j]);
		else
			printk(KERN_ERR PFX "invalid data width %d\n", dwidth);
	}
	kfree(iob);
	return 0;
}

#ifdef ENCORE_DAL
static int *nregs = &ctc_nregs;
static struct encore_reginfo *reginfo = ctc_registers;

static void get_nregs(int *n)
{
	*n = *nregs;
}

static int get_reginfo(struct encore_reginfo **array)
{
	int cc;

	cc = copy_to_user(*array, reginfo, *nregs*sizeof(*reginfo));
	if (cc != 0)
		return -EACCES;
	else
		return 0;
}
#endif

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
	if (!minor_ok(minor))
		return -EACCES;
	dev = filp->private_data;

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

	case VMEIO_RAW_READ_DMA:
		cc = raw_dma(dev, arb, VME_DMA_FROM_DEVICE);
		if (cc < 0)
			goto out;
		break;

	case VMEIO_RAW_WRITE_DMA:
		cc = raw_dma(dev, arb, VME_DMA_TO_DEVICE);
		if (cc < 0)
			goto out;
		break;

	case VMEIO_READ_DMA:
	case VMEIO_WRITE_DMA:
		cc = do_raw_dma(arb);
		if (cc < 0)
			goto out;
		break;

	case VMEIO_RAW_READ:
		cc = raw_read(dev, arb);
		if (cc < 0)
			goto out;
		break;

	case VMEIO_RAW_WRITE:
		cc = raw_write(dev, arb);
		if (cc < 0)
			goto out;
		break;

	case VMEIO_GET_MAPPING:
		cc = get_mapping(dev, arb);
		if (cc < 0)
			goto out;
		break;

#ifdef ENCORE_DAL
	case VMEIO_GET_NREGS:
		get_nregs(arb);
		break;
	case VMEIO_GET_REGINFO:
		cc = get_reginfo(arb);
		break;
#else
	case VMEIO_GET_NREGS:
	case VMEIO_GET_REGINFO:
		cc = -ENOENT;
		break;
#endif

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

static DEFINE_MUTEX(driver_mutex);

long vmeio_ioctl32(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int res;
	struct inode *inode = filp->f_dentry->d_inode;

	mutex_lock(&driver_mutex);
	res = vmeio_ioctl(inode, filp, cmd, arg);
	mutex_unlock(&driver_mutex);
	return res;
}

struct file_operations vmeio_fops = {
	.owner = THIS_MODULE,
	.read = vmeio_read,
	.write = vmeio_write,
	.unlocked_ioctl = vmeio_ioctl32,
	.open = vmeio_open,
	.release = vmeio_close,
};


module_init(vmeio_install);
module_exit(vmeio_uninstall);
