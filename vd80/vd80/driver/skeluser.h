/* =========================================================== */
/* Define here the resources you want to reserve for driver    */
/* working area. Max clients, modules etc.                     */
/* Julian Lewis Tue 16th Dec 2008 AB/CO/HT                     */
/* =========================================================== */

#ifndef SKELUSER
#define SKELUSER

#include <vd80Drvr.h>

/* =========================================================== */
/* Modify these constants to control the number of client and  */
/* module contexts.                                            */

#define SkelDrvrCLIENT_CONTEXTS 32
#define SkelDrvrMODULE_CONTEXTS 16

/* =========================================================== */
/* How much space do you want to reserve in the module context */
/* to keep copies of registers. These are useful in emulation  */
/* and to restore settings after a hardware reset.             */

#define SkelDrvrREGISTERS 256

/* =========================================================== */
/* Define how many interrupt sources you want and the size of  */
/* the queue you need to store them for each client.           */

#define SkelDrvrINTERRUPTS 32
#define SkelDrvrQUEUE_SIZE 32
#define SkelDrvrCONNECTIONS 16

/* =========================================================== */
/* Define default timeout values, in 10ms ticks                */

#define SkelDrvrDEFAULT_CLIENT_TIMEOUT 2000
#define SkelDrvrDEFAULT_MODULE_TIMEOUT 10

/* =========================================================== */
/* Extra definitions that allow build_drvr to build the driver */
/* tree from module parameters.                                */

#define SkelDrvrNAME "VD80"
#define SkelDrvrCOMMENT "ADC sampler card from INCA"
#define SkelDrvrAUTHOR "Julian Lewis BE/CO/HT"

#define SkelDrvrLEVEL 2
#define SkelDrvrINTERRUPT_COMMENT ""
#define SkelDrvrMODULE_COMMENT ""
#define SkelDrvrMODULE_NAME ""
#define SkelDrvrMODULE_ADDRESS_COMMENT ""

#define SkelDrvrAMD1 0x2F
#define SkelDrvrAMD2 0x39

#define SkelDrvrWIN1 0x80000
#define SkelDrvrWIN2 0x80000

#define SkelDrvrDWIDTH1 32
#define SkelDrvrDWIDTH2 32

#define SkelDrvrFREE1 0
#define SkelDrvrFREE2 0

#define SkelDrvrADDR_COMMENT_1 "VME Auto detect"
#define SkelDrvrADDR_COMMENT_2 "Base address"

#endif
