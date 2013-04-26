/* ===================================================== */
/* Driver specific definitions header file               */
/* Julian Lewis April 2009                               */
/* ===================================================== */

#ifndef DRVR_SPEC_H
#define DRVR_SPEC_H

#include "ctrdrvrP.h"

/* Linux Module exported symbols */
#ifdef CTR_PCI
#define LynxOsMAJOR_NAME "ctrp"
#define LynxOsSUPPORTED_DEVICE "CTR-PMC, CTR-PCI timing modules"
#define LynxOsDESCRIPTION "CTRP/I 3/4 channel timing PCI module driver"
#else
#define LynxOsMAJOR_NAME "ctrv"
#define LynxOsSUPPORTED_DEVICE "CTR-VME timing modules"
#define LynxOsDESCRIPTION "CTRV 8 channel timing VME module driver"
#endif

#define LynxOsAUTHOR "Julian Lewis BE/CO/HT CERN"
#define LynxOsLICENSE "GPL"

/* Dynamic major device allocation when set to zero */
#define LynxOsMAJOR 0

/* Info table */
#define LynxOsINFO_TABLE_SIZE sizeof(CtrDrvrInfoTable);

/* The maximum number of supported devices */
#define LynxOsMAX_DEVICE_COUNT CtrDrvrMODULE_CONTEXTS

#define LynxOsALLOCATION_SIZE sizeof(CtrDrvrPtimObjects)

#endif
