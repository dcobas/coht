/**
 * Main module, built by including all sources
 * Julian Lewis BE/CO/HT Dec 2011
 */

/**
 * InsLib structure definitions and kernel routines. All depends on these
 * config_data.h contains the structures to build the installation tree
 * libinstkernel.c implements the tree walk routines
 */

#include <config_data.h>
#include <libinstkernel.c>

/**
 * Driver definitions
 * skeldrvrP.h contains non exported Private driver structure definitions
 * skel.h is used by the skel driver implementor and user space libraries
 */

#include <skeldrvrP.h>
#include <skel.h>

/**
 * This is the VD80 driver
 * skeluser.h contains user definitions used by skel to build the driver
 * vd80Names.c contains an array of ioctl names used in debug logic
 * vd80Drvr.c contains the skel callback implementations
 */

#include <skeluser.h>
#include <skeluser_ioctl.h>
#include <vd80Names.c>
#include <vd80Drvr.c>

/**
 * Trigger the BUS logic you want by defining
 * its type here. This pulls out the corresponding
 * implementation.
 *
 * #define CONFIG_BUS_PCI
 * #define CONFIG_BUS_CAR
 * #define CONFIG_BUS_VME
 *
 */

#define CONFIG_BUS_VME

/**
 * Implement corresponding module parameters according to the BUS
 * Implement the corresponding BUS logic VME/PCI/Car
 */

#include <mod_pars_build_tree.c>
#include <skelvme.c>
#include <skelpci.c>
#include <skelcar.c>

/**
 * Finally the skel driver that calls back the user code
 */

#include <skeldrvr.c>
