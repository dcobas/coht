/**
 * Main module, built by including all sources
 * Julian Lewis BE/CO/HT Dec 2011
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/vmalloc.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/page-flags.h>
#include <linux/pagemap.h>
#include <linux/time.h>
#include <linux/delay.h>

/**
 * InsLib structures, definitions
 * and kernel routines. All depends
 * on this.
 */

#include <config_data.h>
#include <libinstkernel.h>
#include <libinstkernel.c>

/**
 * Driver definitions
 */

#include <skeldrvrP.h>
#include <skel.h>

/**
 * This is the VD80 driver
 * User definitions, ioctl numbers and structures
 * Ioctl names and the driver callbacks
 */

#include <skeluser.h>
#include <skeluser_ioctl.h>
#include <vd80Names.c>
#include <vd80Drvr.c>

// #define CONFIG_BUS_PCI
// #define CONFIG_BUS_CAR

#define CONFIG_BUS_VME

#include <mod_pars_build_tree.c>
#include <skelvme.c>
#include <skelpci.c>
#include <skelcar.c>

#include <skeldrvr.c>
