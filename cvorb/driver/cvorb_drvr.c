/**
 * cvorgdrv.c
 *
 * Linux Device Driver for the CVORG board at CERN.
 *
 * Copyright (c) 2012 Samuel Iglesias Gonsalvez <siglesia@cern.ch>
 *		      Michel Arruat <michel.arruat@cern.ch>
 * Released under the GPL v2. (and only v2, not any later version)
 */

#include <linux/kernel.h>
#include <linux/fs.h>
#include <vmebus.h>
#include <linux/slab.h>
#include <linux/module.h>

#include <cvorb.h>
#include <cvorb_priv.h>

MODULE_AUTHOR ("Samuel Iglesias Gonsalvez <siglesia@cern.ch>,"
		"Michel Arruat <michel.arruat@cern.ch>");
MODULE_DESCRIPTION("Linux Device Driver for the CVORB board at CERN.");
MODULE_VERSION(GIT_VERSION);
MODULE_LICENSE("GPL");

/* module parameters                                                  */
/*====================================================================*/
/* Initialized with -1 because it's used to check bad configuration,  */
/* like two times the same lun etc...                                 */
static int lun[CVORB_MAX_BOARDS] = {[0 ... CVORB_MAX_BOARDS - 1] = -1 };
static unsigned int num_lun = 0;
module_param_array(lun, int, &num_lun, S_IRUGO);
MODULE_PARM_DESC(lun, "Logical Unit Number");

static int base_address[CVORB_MAX_BOARDS];
static unsigned int num_base_address = 0;
module_param_array(base_address, int, &num_base_address, S_IRUGO);
MODULE_PARM_DESC(base_address, "VME Base Address for each CVORB");

/* Devices data                                                       */
/*====================================================================*/
/* keeps the number of cvorb boards successfully installed */
static unsigned int cvorb_installed = 0;
/* Needed to create the sysfs files */
static struct class *cvorb_class = NULL;
/* */
static dev_t cvorb_devno;

/*====================================================================*/

static int __devinit cvorb_match(struct device *pdev, unsigned int ndev)
{
	if ((ndev >= num_lun) || (ndev >= num_base_address)) {
		//Not enough number of arguments.
		return 0;
	}

	return 1;
}

static int __devinit cvorb_probe(struct device *pdev, unsigned int ndev)
{
	int j;
	int ret;
	struct cvorb_dev *cvorb;

	/* Check if the lun is not already in used */
	for (j = 0; j < cvorb_installed; j++) {
		if (lun[j] == lun[ndev])
			return -EBUSY;
	}
	/* Allocate the software representation of the cvorb board */
	cvorb = (struct cvorb_dev *) kzalloc(sizeof(struct cvorb_dev),
					 GFP_KERNEL);
	if (cvorb == NULL) {
		return -ENOMEM;
	}
	/* Set VME 32bits address and Lun */
	cvorb->vme_base = base_address[ndev];
	cvorb->lun = lun[ndev];
	printk(KERN_INFO PFX "Installing board with lun %d"
		"and vme address 0x%08x. Board %d/%d\n",
		cvorb->lun, cvorb->vme_base, ndev, num_lun);
	ret = cvorb_install(cvorb_class, pdev, cvorb_devno, cvorb);
	if (ret) {
		printk(KERN_INFO PFX
		       "board with lun %d cannot be installed\n",
		       lun[ndev]);
		return ret;
	}
	printk(KERN_INFO PFX "board with lun %d installed\n", lun[ndev]);
	cvorb_installed++;

	return 0;
}

static int __devexit cvorb_remove(struct device *pdev, unsigned int ndev)
{
	struct cvorb_dev *cvorb = dev_get_drvdata(pdev);

	printk(KERN_INFO PFX "uninstalling board with lun %d.\n",
	       cvorb->lun);
	cvorb_uninstall(cvorb_class, cvorb, cvorb_devno);
	cvorb_installed--;

	return 0;
}

static struct vme_driver cvorb_driver = {
	.match = cvorb_match,
	.probe = cvorb_probe,
	.remove = __devexit_p(cvorb_remove),
	.driver = {
		   .name = DRIVER_NAME},
};

static int __init cvorb_init_module(void)
{
	int error, i;

	/* Check Parameters */
	if (num_lun > CVORB_MAX_BOARDS) {
		printk(KERN_ERR PFX
		       "Number of boards to install exceed the limit (%d)\n",
		       CVORB_MAX_BOARDS);
		return -EINVAL;
	}
	/*
	 * Check if the difference between base addresses
	 *  is more that the board size
	 */
	for (i = 1; i < num_lun; ++i) {
		if ((base_address[i] - base_address[i - 1]) <
		    CVORB_WINDOW_LENGTH) {
			printk(KERN_ERR PFX
			       "base address are not correct(doesn't respect \n");
			return -EINVAL;
		}
	}
	/* create device class */
	cvorb_class = class_create(THIS_MODULE, "cvorb");
	if (IS_ERR(cvorb_class)) {
		printk(KERN_ERR PFX "Failed to create cvorb class\n");
		error = PTR_ERR(cvorb_class);
		/* nothing to clean, just returns the error */
		return error;
	}

	/* Get a range of minor numbers (starting with 0) to work with */
	error = alloc_chrdev_region(&cvorb_devno, 0, num_lun, DRIVER_NAME);
	if (error < 0) {
		printk(KERN_ERR PFX "Failed to allocate chrdev region\n");
		goto alloc_chrdev_region_failed;
	}

	/* some sysfs initialization before instantiating the devices */
	error = cvorb_sysfs_init_module();
	if (error) {
		printk(KERN_ERR PFX
		       "Failed initializing sysfs at module installation.(-ENOMEM) \n");
		goto alloc_chrdev_region_failed;
	}
	error = vme_register_driver(&cvorb_driver, num_lun);
	if (error) {
		printk(KERN_ERR PFX
		       "Could not register vme cvorb driver \n");
		goto alloc_chrdev_region_failed;
	}
	return 0;

alloc_chrdev_region_failed:
	class_destroy(cvorb_class);

	return error;
}

static void __exit cvorb_exit_module(void)
{
	dev_t devno = MKDEV(MAJOR(cvorb_devno), 0);

	vme_unregister_driver(&cvorb_driver);
	unregister_chrdev_region(devno, num_lun);
	/* some sysfs cleaning before leaving the module */
	cvorb_sysfs_exit_module();
	class_destroy(cvorb_class);
}

module_init(cvorb_init_module);
module_exit(cvorb_exit_module);
