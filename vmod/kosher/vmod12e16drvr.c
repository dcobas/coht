/**
 * @file vmod12e16.c
 *
 * @brief Driver for VMOD12E16 ADC mezzanine on VMOD/MOD-PCI MODULBUS
 * carrier
 *
 * Copyright (c) 2009 CERN
 * @author Juan David Gonzalez Cobas <dcobas@cern.ch>
 *
 * @section license_sec License
 * Released under the GPL v2. (and only v2, not any later version)
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/semaphore.h>
#include "modulbus_register.h"
#include "lunargs.h"
#include "vmod12e16.h"

#define	DRIVER_NAME		"vmod12e16"
#define PFX			DRIVER_NAME ": "
#define	VMOD12E16_MAX_MODULES	VMOD_MAX_BOARDS

/* The One And Only Device (OAOD) */
dev_t		devno;
struct cdev	cdev;

struct vmod12e16_dev {
	struct vmod_dev		*config;
	struct semaphore	sem;
};

/** configuration parameters from module params */
struct vmod_devices	config;
struct vmod12e16_dev	device_list[VMOD12E16_MAX_MODULES];

static int vmod12e16_open(struct inode *ino, struct file  *filp)
{
	unsigned int lun = iminor(ino);
	unsigned int idx = lun_to_index(&config, lun);

	if (idx < 0) {
		printk(KERN_ERR PFX "could not open, bad lun %d\n", lun);
		return -ENODEV;
	}
	filp->private_data = &device_list[idx];
	return 0;
}

static int vmod12e16_release(struct inode *ino, struct file  *filp)
{
	return 0;
}

static u16 ioread(void *addr, int be)
{
	u16 val = ioread16(addr);
	printk(KERN_INFO PFX "Reading from address %p\n", addr);
	if (be)
		return be16_to_cpu(val);
	else
		return val;
}

static void iowrite(u16 val, void *addr, int be)
{
	u16 tmp;

	if (be)
		tmp = cpu_to_be16(val);
	else
		tmp = val;
	printk(KERN_INFO PFX "Writing 0x%x to address %p\n", tmp, addr);
	iowrite16(tmp, addr);
}

static int do_conversion(struct file *filp,
			struct vmod12e16_conversion *conversion)
{
	struct vmod12e16_dev *dev = filp->private_data;
	struct vmod12e16_registers *regs = (void*)dev->config->address;
	int be = dev->config->is_big_endian;

	int channel = conversion->channel;
	int ampli   = conversion->amplification;
	int us_elapsed;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	/*
	 * explicitly disable interrupt mode
	 *
	 * WARNING: omitting this step leads to random
	 * bus errors when doing ADC conversion in polling mode
	 */
	iowrite(VMOD_12E16_ADC_INTERRUPT_MASK, &regs->interrupt, be);

	/* specify channel and amplification */
	if ((ampli & ~((1<<2)-1)) || channel & ~((1<<4)-1))
		return -EINVAL;
	iowrite((ampli<<4) | channel, &regs->control, be);

	/* wait at most the manufacturer-supplied max time */
	us_elapsed = 0;
	while (us_elapsed < VMOD_12E16_MAX_CONVERSION_TIME) {
		udelay(VMOD_12E16_CONVERSION_TIME);
		if ((ioread(&regs->ready, be) & VMOD_12E16_RDY_BIT) == 0) {
			conversion->data = ioread(&regs->data, be) & VMOD_12E16_ADC_DATA_MASK;
			udelay(VMOD_12E16_CONVERSION_TIME);
			return 0;
		}
		us_elapsed += VMOD_12E16_CONVERSION_TIME;
	}

	/* timeout */
	up(&dev->sem);
	return -ETIME;
}

static int vmod12e16_ioctl(struct inode *ino,
		    struct file *filp,
		    unsigned int cmd,
		    unsigned long arg)
{
	struct vmod12e16_conversion cnv, *cnvp = &cnv;
	int err;

	switch (cmd) {

	case VMOD12E16_IOCCONVERT:
		if (copy_from_user(cnvp, (void*)arg, sizeof(cnv)))
			return -EINVAL;
		if ((err = do_conversion(filp, cnvp)) != 0)
			return err;
		if (copy_to_user((void*)arg, cnvp, sizeof(cnv)))
			return -EINVAL;

		return 0;
		break;

	default:
		return -ENOTTY;
		break;
	}
	return 0;
}

struct file_operations fops = {
	.owner =    THIS_MODULE,
	.ioctl =    vmod12e16_ioctl,
	.open =     vmod12e16_open,
	.release =  vmod12e16_release,
};

/* module initialization and cleanup */
static int __init init(void)
{
	int i, err;

	printk(KERN_INFO PFX "reading parameters\n");
	err = read_params(DRIVER_NAME, &config);
	if (err != 0)
		return -1;
	printk(KERN_INFO PFX
		"initializing driver for %d (max %d) cards\n",
		config.num_modules, VMOD_MAX_BOARDS);

	for (i = 0; i < config.num_modules; i++)
		init_MUTEX(&device_list[i].sem);

	err = alloc_chrdev_region(&devno, 0, VMOD12E16_MAX_MODULES, DRIVER_NAME);
	if (err != 0)
		goto fail_chrdev;
	printk(KERN_INFO PFX "allocated device %d\n", MAJOR(devno));

	cdev_init(&cdev, &fops);
	cdev.owner = THIS_MODULE;
	if (cdev_add(&cdev, devno, VMOD12E16_MAX_MODULES) != 0) {
		printk(KERN_ERR PFX
			"failed to create chardev %d with err %d\n",
				MAJOR(devno), err);
		goto fail_cdev;
	}
	return 0;

fail_cdev:	
	unregister_chrdev_region(devno, VMOD12E16_MAX_MODULES);
fail_chrdev:	
	return -1;
}

static void __exit exit(void)
{
	cdev_del(&cdev);
	unregister_chrdev_region(devno, VMOD12E16_MAX_MODULES);
}


module_init(init);
module_exit(exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Juan David Gonzalez Cobas <dcobas@cern.ch>");
