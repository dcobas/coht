/******************************************************************/
/* The prived declarations for the Tg8's selftest program         */
/* V.Kovaltsov Oct 96                                             */
/******************************************************************/

#include <tg8.h>
#include "/tmp/tg8selftestsym.c" /* ASM definitions */

/***********************/
/* Function prototypes */
/***********************/

extern void Tpu0Isr();
extern void Tpu1Isr();
extern void Tpu2Isr();
extern void Tpu3Isr();
extern void Tpu4Isr();
extern void Tpu5Isr();
extern void Tpu6Isr();
extern void Tpu7Isr();
extern void Tpu8Isr();

extern void AbortIsr();
extern void DscIsr();
extern void XrIsr();

extern void BusErrorIsr();
extern void SpuriousIsr();
extern void AddressErrorIsr();
extern void PrivViolationIsr();

void Init();
void CamInitialize();

void memset(short *m,short v,unsigned int words);

/***********************/
/* Constants           */
/***********************/

#define RAM_BTM 0x04000 /* The base address for the RAM test */
#define RAM_TOP 0x10000 /* The final address of the RAM test */

/***************/
/* Variables:: */
/***************/

McpSim * sim;    /* System Integration Module structure */
StDpm  * dpm;    /* Pointer to DPRAM structure */
Tg8Xlx * xlx;    /* XILINX 1 & 2 Counters (2 groups with 4 channels per group) */
short  * cam;    /* Content addressed memory */

McpTpu * tpu;    /* Time processor unit */
short  * tpu_parameters[16]; /* Functions params for 16 channels */

char   chr;         /* Character code used by the PrintStr */
int    busi[2],     /* Keep the bus_int condition here */
       busn;        /* Number of bus interrupts */
long   bus_int,     /* Bus interrupt counter */
       frame_int,   /* Number of incoming frames */
       tpu_int[9],  /* TPU interrupts counters */
       xl_context[15]; /* Xilinx Receiver context area */
long  *debug_p;

Tg8Event event_frame; /* Event frame code */

/*******************************************************************************/
/* The RAM data will be used as the DPRAM block in the case of the DPRAM fails */
/*******************************************************************************/

StDpm dpr;

/*****************/
/* Firmware code */
/*****************/

#include "tg8.exe.c"  /* Tg8FirmwareObject */

/**********************************************************************/
/* Macros  (General Purpose)                                          */
/**********************************************************************/

/* Reset the Micro Processor */
#define RESET  asm(" reset ")

/* Interrupts ON or OFF */
#define INTOFF asm(" movew #Tg8DISABLE_INTERRUPTS,SR ")
#define INTON  asm(" /** oriw  #Tg8ENABLE_WATCHDOG,SimProtect **/
		     movew #Tg8ENABLE_INTERRUPTS,SR ")

/* Delay for waiting of the CAM operations to be finished */
#define WAIT asm volatile (" movel d0,d0 ")

/**********************************************************************/
/* Some macros for handling the Content Addressed Memory CAM          */
/**********************************************************************/

#define CamSetCommandMode asm(" andiw #CamCOMMAND_MODE,SimDataF1 ")
#define CamSetDataMode    asm(" oriw  #CamDATA_MODE,SimDataF1 ")

#define CamClearSkipBits  *cam= CamCLEAR_SKIP_BITS
#define CamClear asm(" movel _cam,a0
		       movew #0,a0@")

/* Start matching */

#define CamMatch(wb) \
   CamSetDataMode; \
   *cam = wb[0];   \
   *cam = wb[1];   \
   *cam = wb[2];   \
   CamSetCommandMode;

/* Write an entry into the CAM */

#define CamWriteArray(adr,wb) \
   CamSetDataMode; \
   *cam = wb[0];   \
   *cam = wb[1];   \
   *cam = wb[2];   \
   CamSetCommandMode; \
   *cam = adr | CamWRITE;

/* Read an entry from the CAM */

#define CamReadArray(adr,wb) \
   CamSetCommandMode;   \
   *cam = adr | 0xF000; \
   *cam = adr | 0x7000; \
   CamSetDataMode;      \
   wb[0] = *cam;        \
   CamSetCommandMode;   \
   *cam = adr | 0xF400; \
   *cam = adr | 0x7000; \
   CamSetDataMode;      \
   wb[1] = *cam;        \
   CamSetCommandMode;   \
   *cam = adr | 0xF800; \
   *cam = adr | 0x7000; \
   CamSetDataMode;      \
   wb[2] = *cam;

/* Generate the VME Bus interrupt */

#define MakeDscInterrupt (*((short *) (Tg8VME_INTERRUPT)) = 1)

/* eof selftestP.h */



