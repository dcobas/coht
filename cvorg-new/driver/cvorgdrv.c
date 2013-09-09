/**
 * cvorgdrv.c
 *
 * Linux Device Driver for the CVORG board at CERN.
 *
 * Copyright (c) 2012 Samuel Iglesias Gonsalvez <siglesia@cern.ch>
 * 
 * Released under the GPL v2. (and only v2, not any later version)
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <vmebus.h>

#include <cvorg.h>
#include <cvorg_priv.h>

#define	DRIVER_NAME		"cvorg"
#define	PFX			DRIVER_NAME ": "

MODULE_AUTHOR("Samuel Iglesias Gonsalvez <siglesia@cern.ch>");
MODULE_DESCRIPTION("Linux Device Driver for the CVORG board at CERN.");
MODULE_VERSION("1.0");
MODULE_VERSION(GIT_VERSION);
MODULE_LICENSE("GPL");

/* module parameters */
static int lun[CVORG_MAX_BOARDS];
static unsigned int num_lun;
module_param_array(lun, int, &num_lun, S_IRUGO);
MODULE_PARM_DESC(lun, "Logical Unit Number");

static int base_address[CVORG_MAX_BOARDS];
static unsigned int num_base_address;
module_param_array(base_address, int, &num_base_address, S_IRUGO);
MODULE_PARM_DESC(base_address, "VME Base Adress for each CVORG");

static int irq[CVORG_MAX_BOARDS];
static unsigned int num_irq;
module_param_array(irq, int, &num_irq, S_IRUGO);
MODULE_PARM_DESC(irq, "IRQ vector for each CVORG");

static unsigned int cvorg_installed = 0;

/* Table to save the pointers to all cvorg devices */
struct cvorg* cvorg_table[CVORG_MAX_BOARDS];

/* Needed to create the sysfs files */
struct class *cvorg_class;
dev_t cvorg_devno;


static int __devinit cvorg_match(struct device *pdev, unsigned int ndev)
{

	if ((ndev >= num_lun) || (ndev >= num_base_address) || (ndev >= num_irq)) {
		//Not enough number of arguments.
		return 0;
	}

	return 1;
}

static int __devinit cvorg_probe(struct device *pdev, unsigned int ndev)
{
	int i = ndev;
	int j;
	int ret;

	for(j = 0; j < cvorg_installed; j++) {
		if(cvorg_table[j] == NULL)
			continue;

		if(cvorg_table[j]->lun == lun[ndev])
			return -EBUSY;	
	}

	printk(KERN_INFO PFX "Installing board with lun %d. Board %d/%d\n", 
					lun[i], i, num_lun);
	ret = cvorg_module_install(pdev, lun[i], base_address[i], irq[i], 2, cvorg_installed);	
	if (ret) {
		printk(KERN_INFO PFX "board with lun %d cannot be installed\n", lun[i]);
		return ret;
	} 
	printk(KERN_INFO PFX "board with lun %d installed\n", lun[i]);
	cvorg_installed++;

	return 0;
}

static int __devexit cvorg_remove(struct device *pdev, unsigned int ndev)
{
	struct cvorg *cvorg = dev_get_drvdata(pdev);
	
	printk(KERN_INFO PFX "uninstalling board with lun %d.\n", cvorg->lun);
	cvorg_module_uninstall(cvorg);
	cvorg_installed--;

	return 0;
}

static struct vme_driver cvorg_driver = {
        .match          = cvorg_match,
        .probe          = cvorg_probe,
        .remove         = __devexit_p(cvorg_remove),
        .driver         = {
                .name   = DRIVER_NAME,
        },
};

static int __init cvorg_sysfs_device_create(void)
{
        int error = 0;

        cvorg_class = class_create(THIS_MODULE, "cvorg");
        if (IS_ERR(cvorg_class)) {
                printk(KERN_ERR PFX "Failed to create cvorg class\n");
                error = PTR_ERR(cvorg_class);
                goto out;
        }   

        error = alloc_chrdev_region(&cvorg_devno, 0, CVORG_MAX_BOARDS, "cvorg");
        if (error) {
                printk(KERN_ERR PFX "Failed to allocate chrdev region\n");
                goto alloc_chrdev_region_failed;
        }   

	return 0;

 alloc_chrdev_region_failed:
        class_destroy(cvorg_class);
 out:
        return error;
}

static int __init cvorg_init(void)
{
	int ret;
	
	if(num_lun > CVORG_MAX_BOARDS) {
		printk(KERN_ERR PFX "given data of more boards than supported\n");
		return -EINVAL;
	}

	ret = cvorg_sysfs_device_create();
	if (ret) {
		printk(KERN_ERR PFX "cannot create the sysfs class for the device\n");
		return -EINVAL;
	}

	return vme_register_driver(&cvorg_driver, CVORG_MAX_BOARDS);
}

static void __exit cvorg_exit(void)
{
        dev_t devno = MKDEV(MAJOR(cvorg_devno), 0);

	vme_unregister_driver(&cvorg_driver);
        unregister_chrdev_region(devno, CVORG_MAX_BOARDS);
	class_destroy(cvorg_class);
}

module_init(cvorg_init);
module_exit(cvorg_exit);
