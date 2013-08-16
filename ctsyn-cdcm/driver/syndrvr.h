/* ************************************************************************ */
/* CTG (Controls Timing Generator) device driver.  SYN Version.             */
/* Julian Lewis 15th/August/2003                                            */
/* ************************************************************************ */

#ifndef SYNDRVR
#define SYNDRVR

/* Modules and clients maximum counts */

#define SynDrvrCLIENT_CONTEXTS 8
#define SynDrvrMODULE_CONTEXTS 1

/* -------------------------------------- */
/* Default setup values                   */

#define SynDrvrMAX_OUTPUT_DELAY      8000
#define SynDrvrMIN_OUTPUT_DELAY     -8000
#define SynDrvrDEFAULT_OUTPUT_DELAY -4000

#define SynDrvrMAX_SYNC_PERIOD       14400
#define SynDrvrMIN_SYNC_PERIOD       200
#define SynDrvrDEFAULT_SYNC_PERIOD   1200

#define SynDrvrDEFAULT_ASYNC_PERIOD  7.62939453125
#define SynDrvrDEFAULT_PHASE         650
#define SynDrvrDEFAULT_NUMBER_AVS    100
#define SynDrvrDEFAULT_KP            14705231
#define SynDrvrDEFAULT_KI            326717

/* --------------------------------------------------------------------- */
/* Module address has two base addresess in it one for X1 and one for X2 */

typedef struct {
	unsigned short *VMEAddress;	/* Base address for main logig A24,D32 */
	unsigned short *JTGAddress;	/* Base address for       JTAG A16,D16 */
	unsigned int InterruptVector;	/* Interrupt vector number */
	unsigned int InterruptLevel;	/* Interrupt level (2 usually) */
	unsigned short *CopyAddress;	/* Copy of VME address */
} SynDrvrModuleAddress;

/* --------------------------------------------------*/
/* Driver and VHDL bit stream compilation dates UTC. */

typedef struct {
	unsigned int VhdlVersion;	/* VHDL Compile date */
	unsigned int DrvrVersion;	/* Drvr Compile date */
} SynDrvrVersion;

/* ----------------------------------- */
/* Names and values of the status bits */

typedef enum {
	SynDrvrStatus10_MHZ_MISSING = 0x1,
	SynDrvrStatusPPS_MISSING = 0x2,
	SynDrvrStatusPPS_UNSTABLE = 0x4,
	SynDrvrStatusPPS_PHASE_ERROR = 0x8,
	SynDrvrStatusOUTPUTS_ENABLED = 0x10,

	SynDrvrStatusBUS_FAULT = 0x20,
	SynDrvrStatusFLASH_OPEN = 0x40,

	SynDrvrStatusSTATAE = 7
} SynDrvrStatus;

#define SynDrvrMscStatusMASK 0x1F

/* -------------------------------------- */
/* The driver Connect and Read structures */

#define SynDrvrCONNECTIONS 6

/* Names and values of the interrupt source bits */
/* that can be connected to.                     */

typedef enum {
	SynDrvrConMask10_MHZ_MISSING = 0x1,
	SynDrvrConMaskPPS_MISSING = 0x2,
	SynDrvrConMaskPPS_UNSTABLE = 0x4,
	SynDrvrConMaskPPS_PHASE_ERROR = 0x8,
	SynDrvrConMaskINTERNAL_PPS = 0x10,
	SynDrvrConMaskPLL_ITERATION = 0x20,
	SynDrvrConMaskGPS_PPS = 0x40,
	SynDrvrConMaskSYNC = 0x80,
	SynDrvrConMaskSOURCES = 8
} SynDrvrConMask;

#define SynDrvrConMaskMASK 0xFF

typedef struct {
	unsigned int Module;	/* The module number 1..n     */
	unsigned short ConMask;	/* Connection mask bits */
} SynDrvrConnection;

typedef struct {
	unsigned int Size;
	unsigned int Pid[SynDrvrCLIENT_CONTEXTS];
} SynDrvrClientList;

typedef struct {
	unsigned int Pid;
	unsigned int Size;
	SynDrvrConnection Connections[SynDrvrCONNECTIONS];
} SynDrvrClientConnections;

typedef struct {
	SynDrvrConnection Connection;
	unsigned short Status;
} SynDrvrReadBuf;

/* -------------------------------------- */
/* Pulse per second status buffer         */

typedef struct {
	unsigned int PpsPeriod;	/* External PPS period */
	unsigned int PpsPhase;	/* External PPS phase with PPS */
} SynDrvrPpsBuf;

/* -------------------------------------- */
/* Phase locked loop status buffer        */

typedef struct {
	long PllError;		/* Pll phase error */
	long PllIntegrator;	/* Pll Integrator */
	short PllDac;		/* Value applied to DAC */
	unsigned int PllLastItLen;	/* Last iteration length */
	unsigned int PllPhase;	/* Pll phase */
	unsigned int PllNumAverage;	/* Numeric average */
	unsigned int PllKP;	/* Constant of proportionality */
	unsigned int PllKI;	/* Constant of integration */
	float PllAsPrdNs;	/* Pll local async oscillator period in ns */
} SynDrvrPllBuf;

/* -------------------------------------- */
/* Raw Input Output transfer block        */

typedef struct {
	unsigned int Offset;	/* Short Index into mmap */
	unsigned short Data;	/* Callers data area for  IO */
} SynDrvrRawIoBlock;

/* =========================================================== */

#define IOCTL_MAGIC 'Y'

#define SYN_IO(nr)              _IO(IOCTL_MAGIC, nr)
#define SYN_IOR(nr,sz)         _IOR(IOCTL_MAGIC, nr, sz)
#define SYN_IOW(nr,sz)         _IOW(IOCTL_MAGIC, nr, sz)
#define SYN_IOWR(nr,sz)       _IOWR(IOCTL_MAGIC, nr, sz)

/* ------------------------ */
/* Define the IOCTL numbers */

typedef enum {

	SynNumGET_VERSION,	/* Get driver and VHDL versions */
	SynNumSET_TIMEOUT,	/* Set the read timeout value */
	SynNumGET_TIMEOUT,	/* Get the read timeout value */
	SynNumSET_QUEUE_FLAG,	/* Set queuing capabiulities on off */
	SynNumGET_QUEUE_FLAG,	/* 1=Q_off 0=Q_on */
	SynNumGET_QUEUE_SIZE,	/* Number of events on queue */
	SynNumGET_QUEUE_OVERFLOW,	/* Number of missed events */
	SynNumSET_MODULE,	/* Select the module to work with */
	SynNumGET_MODULE,	/* Which module am I working with */
	SynNumGET_MODULE_COUNT,	/* The number of installed modules */
	SynNumENABLE_OUTPUTS,	/* Enable output clocks and resync PLL */
	SynNumRESYNC_PPS,	/* Resync counters with external PPS */
	SynNumSET_OUTPUT_DELAY,	/* Set the output delay in 40MHz ticks */
	SynNumGET_OUTPUT_DELAY,	/* Get the output delay in 40MHz ticks */
	SynNumSET_SYNC_PERIOD,	/* Set the basic period length */
	SynNumGET_SYNC_PERIOD,	/* Get the basic period length */
	SynNumGET_PLL,		/* Get the PLL data */
	SynNumSET_PLL_PHASE,	/* Set the PLL phase offest */
	SynNumSET_PLL_NUM_AVERAGE,	/* Set the PLL numeric average */
	SynNumSET_PLL_KP,	/* Set the PLL constant of proportioality */
	SynNumSET_PLL_KI,	/* Set the PLL constant of integration */
	SynNumSET_PLL_AP,	/* Set the PLL asynchronous period in ns */
	SynNumSET_PLL_INTG,	/* Set the PLL integrator */
	SynNumGET_PPS,		/* Get external PPS data */
	SynNumRESET,		/* Reset driver */
	SynNumGET_STATUS,	/* Read module status */
	SynNumGET_CLIENT_LIST,	/* Get the list of driver clients */
	SynNumCONNECT,		/* Connect to and object interrupt */
	SynNumGET_CLIENT_CONNECTIONS,	/* Get the list of a client connections on module */
	SynNumJTAG_OPEN,	/* Open JTAG interface */
	SynNumJTAG_READ_BYTE,	/* Read back uploaded VHDL bit stream */
	SynNumJTAG_WRITE_BYTE,	/* Upload a new compiled VHDL bit stream */
	SynNumJTAG_CLOSE,	/* Close JTAG interface */
	SynNumGET_MODULE_ADDRESS,	/* Get the VME module base address */
	SynNumRAW_READ,		/* Raw read  access to card for debug */
	SynNumRAW_WRITE		/* Raw write access to card for debug */
} SynNumControlFunction;


#define SynDrvrGET_VERSION            SYN_IOR(SynNumGET_VERSION,SynDrvrVersion)
#define SynDrvrSET_TIMEOUT            SYN_IOW(SynNumSET_TIMEOUT,uint32_t)
#define SynDrvrGET_TIMEOUT            SYN_IOR(SynNumGET_TIMEOUT,uint32_t)
#define SynDrvrSET_QUEUE_FLAG         SYN_IOW(SynNumSET_QUEUE_FLAG,uint32_t)
#define SynDrvrGET_QUEUE_FLAG         SYN_IOR(SynNumGET_QUEUE_FLAG,uint32_t)
#define SynDrvrGET_QUEUE_SIZE         SYN_IOR(SynNumGET_QUEUE_SIZE,uint32_t)
#define SynDrvrGET_QUEUE_OVERFLOW     SYN_IOR(SynNumGET_QUEUE_OVERFLOW,uint32_t)
#define SynDrvrSET_MODULE             SYN_IOW(SynNumSET_MODULE,uint32_t)
#define SynDrvrGET_MODULE             SYN_IOR(SynNumGET_MODULE,uint32_t)
#define SynDrvrGET_MODULE_COUNT       SYN_IOR(SynNumGET_MODULE_COUNT,uint32_t)
#define SynDrvrENABLE_OUTPUTS         SYN_IOWR(SynNumENABLE_OUTPUTS,uint32_t)
#define SynDrvrRESYNC_PPS             SYN_IO(SynNumRESYNC_PPS)
#define SynDrvrSET_OUTPUT_DELAY       SYN_IOW(SynNumSET_OUTPUT_DELAY,uint32_t)
#define SynDrvrGET_OUTPUT_DELAY       SYN_IOR(SynNumGET_OUTPUT_DELAY,uint32_t)
#define SynDrvrSET_SYNC_PERIOD        SYN_IOW(SynNumSET_SYNC_PERIOD,uint32_t)
#define SynDrvrGET_SYNC_PERIOD        SYN_IOR(SynNumGET_SYNC_PERIOD,uint32_t)
#define SynDrvrGET_PLL                SYN_IOR(SynNumGET_PLL,SynDrvrPllBuf)
#define SynDrvrSET_PLL_PHASE          SYN_IOW(SynNumSET_PLL_PHASE,uint32_t)
#define SynDrvrSET_PLL_NUM_AVERAGE    SYN_IOW(SynNumSET_PLL_NUM_AVERAGE,uint32_t)
#define SynDrvrSET_PLL_KP             SYN_IOW(SynNumSET_PLL_KP,uint32_t)
#define SynDrvrSET_PLL_KI             SYN_IOW(SynNumSET_PLL_KI,uint32_t)
#define SynDrvrSET_PLL_AP             SYN_IOW(SynNumSET_PLL_AP,float)
#define SynDrvrSET_PLL_INTG           SYN_IOW(SynNumSET_PLL_INTG,uint32_t)
#define SynDrvrGET_PPS                SYN_IOR(SynNumGET_PPS,SynDrvrPpsBuf)
#define SynDrvrRESET                  SYN_IO(SynNumRESET)
#define SynDrvrGET_STATUS             SYN_IOR(SynNumGET_STATUS,uint32_t)
#define SynDrvrGET_CLIENT_LIST        SYN_IOR(SynNumGET_CLIENT_LIST,SynDrvrClientList)
#define SynDrvrCONNECT                SYN_IOWR(SynNumCONNECT,SynDrvrConnection)
#define SynDrvrGET_CLIENT_CONNECTIONS SYN_IOWR(SynNumGET_CLIENT_CONNECTIONS,SynDrvrClientConnections)
#define SynDrvrJTAG_OPEN              SYN_IO(SynNumJTAG_OPEN)
#define SynDrvrJTAG_READ_BYTE         SYN_IOR(SynNumJTAG_READ_BYTE,uint32_t)
#define SynDrvrJTAG_WRITE_BYTE        SYN_IOW(SynNumJTAG_WRITE_BYTE,uint32_t)
#define SynDrvrJTAG_CLOSE             SYN_IO(SynNumJTAG_CLOSE)
#define SynDrvrGET_MODULE_ADDRESS     SYN_IOWR(SynNumGET_MODULE_ADDRESS,SynDrvrModuleAddress)
#define SynDrvrRAW_READ               SYN_IOWR(SynNumRAW_READ,SynDrvrRawIoBlock)
#define SynDrvrRAW_WRITE              SYN_IOWR(SynNumRAW_WRITE,SynDrvrRawIoBlock)

#endif
