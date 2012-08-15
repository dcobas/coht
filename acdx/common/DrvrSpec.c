/* ===================================================== */
/* Driver specific routines and data areas.              */
/* Julian Lewis June 2008                                */
/* ===================================================== */

#include <linux/errno.h>
#include <EmulateLynxOs.h>
#include <DrvrSpec.h>

/* ===================================================== */

static AcdxDrvrInfoTable info_table;
void *LynxOsInfoTable = (void *) &info_table;

/* ===================================================== */
/* Initialize info table, returns 0 if ok                */
/* else return a Linux 2.4 Error code                    */
/* To be filled with driver specific initialization      */
/* ===================================================== */

int LynxOsInitInfoTable(int recover) {
   info_table.RecoverMode = recover;
   return 0;
}

/* ===================================================== */
/* Returns the size and direction of Ioctl IO transfers. */
/* Fill in an entry for each driver Ioctl                */
/* ===================================================== */

#define READ_FLAG 0x01 /* As per LynxOsMemoryREAD_FLAG   */
#define WRIT_FLAG 0x03 /* defined in EmulateLynxOs.h     */

int LynxOsMemoryGetSize(int cmd, int *dirp) {
int cnt, dir;

   switch ((AcdxDrvrControlFunction) cmd) {

   case AcdxDrvrGET_LOCK_KEY:           /* Get driver lock key */
      cnt = sizeof(unsigned long);
      dir = WRIT_FLAG;
      break;

   case AcdxDrvrLOCK:                   /* Lock the driver: read only */
      cnt = sizeof(unsigned long);
      dir = READ_FLAG;
      break;

   case AcdxDrvrUNLOCK:                 /* Unlock the driver */
      cnt = sizeof(unsigned long);
      dir = READ_FLAG;
      break;

   case AcdxDrvrSET_SW_DEBUG:           /* Set driver debug mode */
      cnt = sizeof(unsigned long);
      dir = READ_FLAG;
      break;

   case AcdxDrvrGET_SW_DEBUG:           /* Get driver debug level */
      cnt = sizeof(unsigned long);
      dir = WRIT_FLAG | READ_FLAG;
      break;

   case AcdxDrvrRAW_READ:               /* Raw read  access to mapped card for debug */
      cnt = sizeof(AcdxDrvrRawIoBlock);
      dir = WRIT_FLAG | READ_FLAG;
      break;

   case AcdxDrvrRAW_WRITE:              /* Raw write access to mapped card for debug */
      cnt = sizeof(AcdxDrvrRawIoBlock);
      dir = WRIT_FLAG | READ_FLAG;
      break;

   case AcdxDrvrSET_FUNCTION_PARAMS:    /* Set sin-wave function parameters */
      cnt = sizeof(AcdxDrvrFunctionParams);
      dir = WRIT_FLAG | READ_FLAG;
      break;

   case AcdxDrvrGET_FUNCTION_PARAMS:    /* Get sin-wave function parameters */
      cnt = sizeof(AcdxDrvrFunctionParams);
      dir = WRIT_FLAG | READ_FLAG;
      break;

   case AcdrDrvrGET_VERSION:            /* Get FPGA Code version and driver version */
      cnt = sizeof(AcdxDrvrVersion);
      dir = WRIT_FLAG | READ_FLAG;
      break;

   case AcdxDrvrGET_TEMPERATURE:        /* Get temperature in Centigrade */
      cnt = sizeof(AcdxDrvrTemperature);
      dir = WRIT_FLAG | READ_FLAG;
      break;

   case AcdxDrvrSET_STATUS_CONTROL:     /* Set module status control register */
      cnt = sizeof(unsigned long);
      dir = READ_FLAG;
      break;

   case AcdxDrvrGET_STATUS_CONTROL:     /* Get module status control register */
      cnt = sizeof(unsigned long);
      dir = WRIT_FLAG;
      break;

   case AcdxDrvrOPEN_USER_FPGA:         /* Opens users FPGA for write */
      cnt = sizeof(unsigned long);
      dir = READ_FLAG;
      break;

   case AcdxDrvrWRITE_FPGA_CHUNK:       /* Write a chunk of FPGA bitstream */
      cnt = sizeof(AcdxDrvrFpgaChunk);
      dir = WRIT_FLAG | READ_FLAG;
      break;

   case AcdxDrvrCLOSE_USER_FPGA:        /* Close FPGA after write */
      cnt = sizeof(unsigned long);
      dir = READ_FLAG;
      break;

   default:
      cnt = -EFAULT;
      dir = 0;
   }
   *dirp = dir;
   return cnt;
}
