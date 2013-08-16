/* ************************************************************************ */
/* PRIVATE driver structures, info, module and client contexts etc.         */
/* Julian Lewis Thu 31st July 2003                                          */
/* ************************************************************************ */

#ifndef SYNDRVR_P
#define SYNDRVR_P

#include <syndrvr.h>

/* ************************************************************************ */
/* Hardware description                                                     */
/* ************************************************************************ */

/* ----------------------------- */
/* Interrupt registers (IRQ)     */

typedef struct {
	unsigned short Source;	/* Interrupt source register */
	unsigned short Enable;	/* Interrupt Enable          */
	unsigned short Setup;	/* Interrupt setup register  */
} SynDrvrIrq;

/* ----------------------------- */
/* Module status & control MSC   */

typedef struct {
	unsigned short Control;	/* Control function register */
	unsigned short Status;	/* Status register */
} SynDrvrMsc;

/* Control function codes */

typedef enum {
	SynDrvrMscControlENABLE_OUTPUT = 0x01,	/* Enable output clocks, and resync PLL */
	SynDrvrMscControlRESYNC_PPS = 0x02,	/* Resync PPS with external PPS */

	SynDrvrMscControlCONTROLS = 2
} SynDrvrMscControl;

/* ----------------------------- */

typedef struct {
	unsigned short Upr;
	unsigned short Lwr;
} SynDrvrLong;

/* ----------------------------- */

typedef struct {
	SynDrvrLong PpsPeriod;	/* External PPS period */
	SynDrvrLong PpsPhase;	/* External PPS phase with PPS */
} SynDrvrPps;

/* ----------------------------- */

typedef struct {
	SynDrvrLong PllError;	/* RO: Pll phase error */
	SynDrvrLong PllIntegrator;	/* RW: Pll Integrator */
	unsigned short PllDac;	/* RO: Value applied to DAC */
	SynDrvrLong PllLastItLen;	/* RO: Last iteration length */
	SynDrvrLong PllPhase;	/* RW: Pll phase */
	SynDrvrLong PllNumAverage;	/* RW: Numeric average */
	SynDrvrLong PllKP;	/* RW: Constant of proportionality */
	SynDrvrLong PllKI;	/* RW: Constant of integration */
} SyndrvrPll;

/* ----------------------------- */
/* CT-Sync memory map            */

typedef struct {
	SynDrvrIrq Irq;		/* Interrupt registers */
	SynDrvrMsc Msc;		/* Status & control registers */
	SynDrvrLong VhdlDate;	/* VHDL Compilation date */
	SynDrvrLong OutputDelay;	/* Output PPS delay */
	SynDrvrPps Pps;		/* One pulse per second status */
	SyndrvrPll Pll;		/* Phase locked loop status */
	unsigned short SyncPeriod;	/* Sync basic period length */
} SynDrvrMap;

/* ************************************************************************ */
/* Driver internal structures                                               */
/* ************************************************************************ */

typedef struct {
	unsigned int Modules;
	SynDrvrModuleAddress Addresses[SynDrvrMODULE_CONTEXTS];
} SynDrvrInfoTable;

/* ----------------------------------------------- */
/* Up to 32 incomming events per client are queued */

#define SynDrvrCLIENT_QUEUE_SIZE 32

typedef struct {
	unsigned int QOff;
	unsigned int Size;
	unsigned int Head;
	unsigned int Tail;
	unsigned int Missed;
	SynDrvrReadBuf Queue[SynDrvrCLIENT_QUEUE_SIZE];
} SynDrvrClientQueue;

/* ------------------ */
/* The client context */

#define SynDrvrDEFAULT_TIMEOUT 2000	/* In 10 ms ticks */

typedef struct {
	cdcm_spinlock_t lock;
	unsigned int InUse;
	unsigned int Pid;
	unsigned int ClientIndex;
	unsigned int ModuleIndex;
	unsigned int Timeout;
	int Timer;
	int Semaphore;
	SynDrvrClientQueue Queue;
} SynDrvrClientContext;

/* --------------------------------- */
/* Module side connection structures */

typedef struct {
	cdcm_spinlock_t lock;
	SynDrvrModuleAddress Address;
	unsigned int InUse;
	unsigned int ModuleIndex;
	unsigned int BusError;
	unsigned int FlashOpen;
	unsigned short EnabledOutput;
	unsigned short EnabledInterrupts;
	SynDrvrLong Phase;
	SynDrvrLong NumAverage;
	SynDrvrLong KP;
	SynDrvrLong KI;
	float PllAsPrdNs;	/* Pll local async oscillator period in ns */
	SynDrvrLong OutputDelay;
	struct vme_berr_handler *BerrHandler;	/* Address of VME bus error handler */
	unsigned int BusErrorCount;	/* Abort fatal loops with this */
	unsigned short SyncPeriod;
	SynDrvrClientContext *Clients[SynDrvrCLIENT_CONTEXTS];
	unsigned short ConMask[SynDrvrCLIENT_CONTEXTS];

} SynDrvrModuleContext;

/* ------------------- */
/* Driver Working Area */

typedef struct {
	cdcm_spinlock_t lock;
	unsigned int Modules;
	SynDrvrModuleContext ModuleContexts[SynDrvrMODULE_CONTEXTS];
	SynDrvrClientContext ClientContexts[SynDrvrCLIENT_CONTEXTS];
} SynDrvrWorkingArea;

#endif
