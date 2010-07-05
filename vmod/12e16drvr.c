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
#include <linux/wait.h>
#include "modulbus_register.h"
#include "lunargs.h"
#include "12e16.h"

#define	DRIVER_NAME		"12e16"
#define PFX			DRIVER_NAME ": "
#define	VMOD12E16_MAX_MODULES	VMOD_MAX_BOARDS

/* The One And Only Device (OAOD) */
static dev_t		devno;
static struct cdev	cdev;

/** configuration parameters from module params */
static struct vmod_devices	config;
static struct vmod12e16_dev	device_list[VMOD12E16_MAX_MODULES];

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

static int ioread(void __iomem *addr, int be)
{
	unsigned int val = ioread16(addr);
	if (be)
		return be16_to_cpu(val);
	else
		return val;
}

static void iowrite(u16 val, void __iomem *addr, int be)
{
	u16 tmp;

	if (be)
		tmp = cpu_to_be16(val);
	else
		tmp = val;
	iowrite16(tmp, addr);
} 
#define TIMEOUT_JIFFIES_OR_ZERO	(VMOD_12E16_MAX_CONVERSION_TIME*HZ/1000000)
#define TIMEOUT_JIFFIES	(TIMEOUT_JIFFIES_OR_ZERO ? TIMEOUT_JIFFIES_OR_ZERO : 1)

static int ready = 0;
static u16 adc_data_register;
static int int_handler(void *source, void *extra)
{
	struct vmod12e16_dev *dev = (struct vmod12e16_dev *)source;
	struct vmod12e16_registers __iomem *regs = 
		(struct vmod12e16_registers __iomem *)dev->config->address;
	int be = dev->config->is_big_endian;

	/* acknowledge the interrupt */
	adc_data_register = ioread(&regs->data, be);
	iowrite(VMOD_12E16_ADC_INTERRUPT_MASK, &regs->interrupt, be);
	ready = 1;
#if 0
	printk(KERN_ERR PFX "handled interrupt at lun%d\n", 
		dev->config->lun);
#endif
	wake_up_interruptible(&dev->wq);
	return 0;
}

static int do_conversion(struct file *filp,
			struct vmod12e16_conversion *conversion)
{
	struct vmod12e16_dev *dev = filp->private_data;
	struct vmod12e16_registers __iomem *regs = 
		(struct vmod12e16_registers __iomem *)dev->config->address;
	int be = dev->config->is_big_endian;

	int channel = conversion->channel;
	int ampli   = conversion->amplification;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	/* explicitly enable interrupt mode */
	iowrite((u16)~VMOD_12E16_ADC_INTERRUPT_MASK, &regs->interrupt, be);

	/* specify channel and amplification */
	if ((ampli & ~((1<<2)-1)) || channel & ~((1<<4)-1))
		return -EINVAL;
	iowrite((ampli<<4) | channel, &regs->control, be);

	/* wait at most the manufacturer-supplied max time */
	wait_event_interruptible(dev->wq, ready == 1);
	if (!ready) {
		printk("missing interrupt in 12e16 read cycle\n");
	}
	ready = 0;
	if ((ioread(&regs->ready, be) & VMOD_12E16_RDY_BIT) == 0) {
		conversion->data = ioread(&regs->data, be) & VMOD_12E16_ADC_DATA_MASK;
		up(&dev->sem);
		return 0;
	} else { /* timeout or signal */
		up(&dev->sem);
		return -ETIME;
	}
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
		if (copy_from_user(cnvp, (const void __user*)arg, sizeof(cnv)))
			return -EINVAL;
		if ((err = do_conversion(filp, cnvp)) != 0)
			return err;
		if (copy_to_user((void __user *)arg, cnvp, sizeof(cnv)))
			return -EINVAL;

		return 0;
		break;

	default:
		return -ENOTTY;
		break;
	}
	return 0;
}

static struct file_operations fops = {
	.owner =    THIS_MODULE,
	.ioctl =    vmod12e16_ioctl,
	.open =     vmod12e16_open,
	.release =  vmod12e16_release,
};

static int register_module_isr(struct vmod12e16_dev *dev, isrcb_t handler)
{
	int err;
	risr_t register_isr;
	char *carrier = dev->config->carrier_name;
	int lun = dev->config->carrier_lun;
	int slot = dev->config->slot;

	register_isr = modulbus_carrier_isr_entry(carrier);
	if (register_isr == NULL) {
		printk(KERN_ERR PFX 
			"could not register irq handler for lun %d\n", 
			dev->config->lun);
		return -1;
	}
	err = register_isr(handler, (void *)dev, lun, slot);
	if (err < 0) {
		printk(KERN_ERR PFX 
			"could not register irq handler for lun %d\n",
			dev->config->lun);
		return -1;
	}
	return 0;
}

static int unregister_module_isr(struct vmod12e16_dev *dev)
{
	return register_module_isr(dev, NULL);
}

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

	/* fill in config data */
	for (i = 0; i < config.num_modules; i++) {
		struct vmod12e16_dev *dev = &device_list[i];

		dev->config = &config.module[i];
		init_MUTEX(&dev->sem);
		init_waitqueue_head(&dev->wq);
	
		if (register_module_isr(dev, int_handler) < 0)
			goto fail_chrdev;
	}

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
	int i;

	for (i = 0; i < config.num_modules; i++) {
		struct vmod12e16_dev *dev = &device_list[i];

		if (unregister_module_isr(dev) == 0) 
			continue;
		printk(KERN_ERR PFX 
			"could not unregister isr for lun %d\n",
			dev->config->lun);
	}
	cdev_del(&cdev);
	unregister_chrdev_region(devno, VMOD12E16_MAX_MODULES);
}


module_init(init);
module_exit(exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Juan David Gonzalez Cobas <dcobas@cern.ch>");
