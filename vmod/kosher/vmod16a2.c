#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/uaccess.h>	/* copy_*_user */

#include "vmod16a2.h"
#include "lunargs.h"
#include "modulbus_register.h"

#define DRIVER_NAME	"vmod16a2"
#define PFX		DRIVER_NAME ": "

/* The One And Only Device (OAOD) */
static dev_t devno;
static struct cdev cdev;

/* module config tables */
static struct vmod_devices 	config;

static int ioctl(struct inode *inode, 
			struct file *fp, 
			unsigned op, 
			unsigned long arg)
{
	struct vmod16a2_select 		sel, *selp;
	struct vmod16a2_state 		*statep = fp->private_data;
	struct vmod16a2_registers	*regp;
	struct vmod16a2_dev		*modp;
	u16				value, tmp;

	switch (op) {

	case VMOD16A2_IOCPUT:

		/* sanity check user params */
		if (!access_ok(VERIFY_READ, arg, sizeof(int)))
			return -EINVAL;
		if (copy_from_user(&value, (int*)arg, sizeof(int)) != 0)
			return -EINVAL;
		if (!statep->selected)
			return -EINVAL;

		/* just do it */
		modp = &(modules[lun_to_dev[statep->lun]]);
		regp = (void*)modp->address;
		if (modp->is_big_endian)
			tmp = cpu_to_be16(value);
		else
			tmp = value;
		if (statep->channel == 0) {
#ifdef DEBUG
			printk(KERN_INFO "writing %x = %d to addr 0x%p\n",
				value, value, &(regp->dac0in));
#endif /* DEBUG */
			iowrite16(tmp, &(regp->dac0in));
			iowrite16(tmp, &(regp->ldac0));
		} else if (statep->channel == 1) {
#ifdef DEBUG
			printk(KERN_INFO "writing %x = %d to addr 0x%p\n",
				value, value, &(regp->dac1in));
#endif /* DEBUG */
			iowrite16(tmp, &(regp->dac1in));
			iowrite16(tmp, &(regp->ldac1));
		} 
		return 0;
		break;

	default:
		return -ENOTTY;
	}
	return -ENOTTY;
}

/* @brief file operations for this driver */
struct file_operations vmod16a2_fops = {
	.owner =    THIS_MODULE,
	.ioctl =    ioctl,
	.open =     open,
	.release =  release,
};


/* @brief fill in the vmod16a2 tables with configuration data

   Config data is passed through a vector luns[] of length num*4, 
   containing triple of lun,board_number,slot_number values
   describing
   	- device LUN
	- carrier LUN
	- carrier slot
   This routine scans this vector and stores its information in the
   modules vector, storing the number of modules to be
   handled in configured_modules.

   It also constructcs a reverse lookup table lun_to_dev useful to
   index modules by lun.

   @return 0 on success
   @return -1 on failure (incorrect number of parameters)
*/

static int check_luns_args(char *luns[], int num)
{
	int i, idx;
	const int ppl = VMOD16A2_PARAMS_PER_LUN;	

	if (num < 0 || num >= ppl * VMOD16A2_MAX_MODULES || num % ppl != 0) {
		printk(KERN_ERR "Incorrect number %d of lun parameters\n", num);
		return -1;
	}

	printk(KERN_INFO "Configuring %d vmod16a2 devices\n", num/ppl);
	for (i = 0, idx=0; idx < num; i++, idx += ppl) {
		int   lun     = simple_strtol(luns[idx], NULL, 10);
		char *cname   = luns[idx+1];
		int   carrier = simple_strtol(luns[idx+2], NULL, 10);
		int   slot    = simple_strtol(luns[idx+3], NULL, 10);
		struct carrier_as as, *asp = &as;

		get_address_space = carrier_as_entry(cname);
		if (get_address_space == NULL) {
			printk(KERN_ERR "no carrier %s registered\n", cname);
			return -1;
		}
		if (get_address_space(asp, carrier, slot, 1)) {
			printk(KERN_ERR "no address space for module<%d>: " 
				"carrier %s lun = %d position %d\n",
				lun, cname, carrier, slot);
			continue;
		}

		modules[i].lun		= lun;
		modules[i].cname	= cname;
		modules[i].carrier	= carrier;
		modules[i].slot		= slot;
		modules[i].address 	= asp->address;
		modules[i].is_big_endian = asp->is_big_endian;
		lun_to_dev[lun] = i;

		printk(KERN_INFO "    module<%d>: lun = %d "
				"carrier = %d slot = %d addr = 0x%lx,"
				" %s endian\n",
				i, lun, carrier, slot, asp->address,
				asp->is_big_endian? "big" : "little" );
	}
	configured_modules = i;
	return 0;
}

/* module initialization and cleanup */
static int __init init(void)
{
	int err;

	printk(KERN_INFO "Initializing vmod16a2 driver with %d luns\n", 
				num/VMOD16A2_PARAMS_PER_LUN);
	err = check_luns_args(luns, num);
	if (err != 0)
		return -1;

	err = alloc_chrdev_region(&devno, 0, VMOD16A2_MAX_MODULES, "vmod16a2");
	if (err != 0) goto fail_chrdev;
	printk("Allocated devno %0x\n",devno); 

	cdev_init(&cdev, &vmod16a2_fops);
	cdev.owner = THIS_MODULE;
	err = cdev_add(&cdev, devno, 1);
	if (err) goto fail_cdev;
	printk(KERN_INFO "Added cdev with err = %d\n", err);

	return 0;

fail_cdev:	unregister_chrdev_region(devno, VMOD16A2_MAX_MODULES);
fail_chrdev:	return -1;
}

static void __exit exit(void)
{
	cdev_del(&cdev);
	unregister_chrdev_region(devno, VMOD16A2_MAX_MODULES);
}

module_init(init);
module_exit(exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Juan David Gonzalez Cobas <dcobas@cern.ch>");
