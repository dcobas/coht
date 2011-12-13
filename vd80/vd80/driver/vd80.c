/**
 * Main module, built by including all sources
 * Julian Lewis BE/CO/HT Dec 2011
 */

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

#include <mod_pars_build_tree.c>
#include <skelvme.c>
#include <skelpci.c>
#include <skelcar.c>

#include <skeldrvr.c>
