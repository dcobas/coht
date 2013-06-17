/* ===================================================== */
/* Driver specific definitions header file               */
/* Julian Lewis June 2008                                */
/* ===================================================== */

#ifndef DRVR_SPEC_H
#define DRVR_SPEC_H

#include <acdxdrvr.h>
#include <acdxdrvrP.h>

/* Linux Module exported symbols */
#define LynxOsMAJOR_NAME "acdx"
#define LynxOsSUPPORTED_DEVICE "AC-Dipole X3-Servo"
#define LynxOsDESCRIPTION "Emulate LynxOs 4.0 over DRM Under Linux 2.4 or later"
#define LynxOsAUTHOR "Julian Lewis AB/CO/HT CERN"
#define LynxOsLICENSE "GPL"

/* Dynamic major device allocation when set to zero */
#define LynxOsMAJOR 0

/* Info table */
#define LynxOsINFO_TABLE_SIZE sizeof(AcdxDrvrInfoTable);

/* The maximum number of supported devices */
#define LynxOsMAX_DEVICE_COUNT AcdxDrvrMODULE_CONTEXTS

#define LynxOsALLOCATION_SIZE sizeof(AcdxDrvrFpgaChunk)

#endif
