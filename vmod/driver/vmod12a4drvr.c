#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/uaccess.h>	/* copy_*_user */
#include "modulbus_register.h"
#include "lunargs.h"
#include "vmod12a4.h"


#define	DRIVER_NAME	"vmod12a4"
#define	PFX		DRIVER_NAME ": "

/* The One And Only Device (OAOD) */
static struct cdev	cdev;
static dev_t		devno;

/* module config tables */
static struct vmod_devices	config;

static int vmod12a4_offsets[VMOD_12A4_CHANNELS] = {
	VMOD_12A4_CHANNEL0,
	VMOD_12A4_CHANNEL1,
	VMOD_12A4_CHANNEL2,
	VMOD_12A4_CHANNEL3,
};

/** 
 * @brief allocate a struct vmod12a4_state on a
 * per-open-file basis, which stores the last lun,channel pair selected
 */
static int vmod12a4_open(struct inode *ino, struct file *filp)
{
	unsigned int lun = iminor(ino);
	unsigned int idx = lun_to_index(&config, lun);

	if (idx < 0) {
		printk(KERN_ERR PFX "could not open, bad lun %d\n", lun);
		return -ENODEV;
	}
	filp->private_data = &config.module[idx];
	return 0;
}

/**
 *  @brief release the struct vmod12a4_select struct */
static int vmod12a4_release(struct inode *ino, struct file *filp)
{
	return 0;
}

static int do_iocput(struct file *fp, struct vmod12a4_output *argp)
{
	/* get lun, channel and value to output */
	struct vmod_dev	*dev = fp->private_data;
	unsigned int		channel = argp->channel;
	u16			value = argp->value;
	void __iomem		*addr;

	if (channel >= VMOD_12A4_CHANNELS) {
		printk(KERN_ERR PFX "invalid channel %d in ioctl\n", channel);
		return -EINVAL;
		
	}

	/* determine channel register address and write */
	addr = (void __iomem *)(dev->address + vmod12a4_offsets[channel]);
	iowrite16be(value, addr);
	return 0;
}

/* @brief file operations for this driver */
static long vmod12a4_ioctl(struct file *fp,
			unsigned op, 
			unsigned long arg)
{
	struct vmod12a4_output	myarg, *myargp = &myarg;

	switch (op) {

	case VMOD12A4_IOCPUT:
		if (copy_from_user(myargp, (const void __user *)arg, sizeof(myarg)) != 0)
			return -EINVAL;
		return do_iocput(fp, myargp);
		break;

	default:
		return -ENOTTY;
	}
	return -ENOTTY;
}

static struct file_operations vmod12a4_fops = {
	.owner =    THIS_MODULE,
	.unlocked_ioctl =    vmod12a4_ioctl,
	.open =     vmod12a4_open,
	.release =  vmod12a4_release,
};

/* module initialization and cleanup */
static int __init vmod12a4_init(void)
{
	int err;

	printk(KERN_INFO PFX "reading parameters\n");

	err = read_params(DRIVER_NAME, &config);
	if (err != 0)
		return -1;
	printk(KERN_INFO PFX 
		"initialized driver for %d (max %d) cards\n",
		config.num_modules, VMOD_MAX_BOARDS);

	err = alloc_chrdev_region(&devno, 0, VMOD12A4_MAX_MODULES, DRIVER_NAME);
	if (err != 0) 
		goto fail_chrdev;
	printk(KERN_INFO PFX "allocated device %d\n", MAJOR(devno)); 

	cdev_init(&cdev, &vmod12a4_fops);
	cdev.owner = THIS_MODULE;
	err = cdev_add(&cdev, devno, VMOD12A4_MAX_MODULES);
	if (err) {
		printk(KERN_ERR PFX 
		"failed to create chardev %d with err %d\n",
			MAJOR(devno), err);
		goto fail_cdev;
	}

	return 0;

fail_cdev:	unregister_chrdev_region(devno, VMOD12A4_MAX_MODULES);
fail_chrdev:	return -1;
}

static void __exit vmod12a4_exit(void)
{
	cdev_del(&cdev);
	unregister_chrdev_region(devno, VMOD12A4_MAX_MODULES);
}


module_init(vmod12a4_init);
module_exit(vmod12a4_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Juan David Gonzalez Cobas <dcobas@cern.ch>");
