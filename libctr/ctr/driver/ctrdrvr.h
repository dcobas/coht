/* ********************************************************************************************* */
/*                                                                                               */
/* CTR (Controls Timing Receiver PMC) Public include file containing driver structures, that     */
/* are used by client tasks.                                                                     */
/*                                                                                               */
/* Julian Lewis 19th May 2003                                                                    */
/*                                                                                               */
/* ********************************************************************************************* */

#ifndef CTRDRVR
#define CTRDRVR

#include "ctrhard.h"   /* Hardware interface for expert and diagnostic clients */
#include "hptdc.h"     /* HPTDC (Time to Digital Convertor) definitions */

/* Maximum number of simultaneous clients for driver */

#define CtrDrvrCLIENT_CONTEXTS 16

/* Maximum number of CTR modules on one host processor */

#ifdef CTR_PMC
#define CtrDrvrMODULE_CONTEXTS 16
#else
#define CtrDrvrMODULE_CONTEXTS 16
#endif

/* *********************************************************************************** */
/* In some rare cases, it is necessary to distinguish between CTR device types.        */
/* This can control the open and install routines so that more than one device type    */
/* can be used at the sam time on a single host.                                       */

typedef enum {
   CtrDrvrDeviceANY,    /* Any CTR device, don't care which */
   CtrDrvrDeviceCTRI,   /* CTR PCI type */
   CtrDrvrDeviceCTRP,   /* CTR PMC type */
   CtrDrvrDeviceCTRV,   /* CTR VME type */
   CtrDrvrDeviceCTRE,   /* CTR VME modified for Beam Energy Safe Beam Parameters */
   CtrDrvrDEVICES
 } CtrDrvrDevice;

/* *********************************************************************************** */
/* All client access to the ctrp timing receivers is through the concept of the timing */
/* object, as per the equipment module classes PTIM, and CTIM. Event frame layouts are */
/* usually hidden from normal clients. They are available in the ctrphard.h interface. */

#define CtrDrvrCONNECTIONS 512

typedef enum {
   CtrDrvrConnectionClassHARD, /* Used when connecting to CTR hardware interrupt  */
   CtrDrvrConnectionClassCTIM, /* Used when connecting to incomming timing on GMT */
   CtrDrvrConnectionClassPTIM  /* Used when connecting to a local PTIM timing     */
 } CtrDrvrConnectionClass;

typedef struct {
   unsigned int            Module;       /* The module 1..n */
   CtrDrvrConnectionClass  EqpClass;     /* Incomming CTIM or local PTIM timing */
   unsigned int            EqpNum;       /* Either a MASK or PTIM or CTIM object  */
 } CtrDrvrConnection;

typedef struct {
   unsigned int Size;
   unsigned int Pid[CtrDrvrCLIENT_CONTEXTS];
 } CtrDrvrClientList;

typedef struct {
   unsigned int      Pid;
   unsigned int      Size;
   CtrDrvrConnection Connections[CtrDrvrCONNECTIONS];
 } CtrDrvrClientConnections;

typedef struct {
   CtrDrvrConnection Connection;       /* PTIM, CTIM or HARD object */
   unsigned int      TriggerNumber;    /* Trigger number 1..n */
   unsigned int      InterruptNumber;  /* 0,1..8 (Counter) 9..n Hardware */
   unsigned int      Ctim;             /* Ctim trigger */
   CtrDrvrEventFrame Frame;            /* Triggering event frame */
   CtrDrvrCTime      TriggerTime;      /* Time counter loaded */
   CtrDrvrCTime      StartTime;        /* Time counter was started */
   CtrDrvrCTime      OnZeroTime;       /* Time of interrupt */
 } CtrDrvrReadBuf;

typedef struct {
   unsigned int      TriggerNumber;    /* Trigger number 0..n 0=>First in connection */
   CtrDrvrConnection Connection;       /* Connection for trigger */
   unsigned int      Payload;          /* Used when simulating CTIMs */
 } CtrDrvrWriteBuf;

/* ******************************************************* */
/* Direct access to triggers and counter configurations    */
/* stored in the CTR ram.                                  */

typedef struct {
   unsigned int                TriggerNumber;   /* 1..n    */
   unsigned int                EqpNum;          /* Object  */
   CtrDrvrConnectionClass      EqpClass;        /* Class   */
   CtrDrvrTrigger              Trigger;         /* Trigger */
   CtrDrvrCounterConfiguration Config;          /* Counter */
 } CtrDrvrAction;

/* ******************************************************* */
/* CTIM object bindings                                    */

#define CtrDrvrCtimOBJECTS 2048

typedef struct {
   unsigned int      EqpNum;
   CtrDrvrEventFrame Frame;
 } CtrDrvrCtimBinding;

typedef struct {
   unsigned short     Size;
   CtrDrvrCtimBinding Objects[CtrDrvrCtimOBJECTS];
 } CtrDrvrCtimObjects;

/* ******************************************************* */
/* PTIM object bindings                                    */

#define CtrDrvrPtimOBJECTS 2048

typedef struct {
   unsigned int   EqpNum;
   unsigned char  ModuleIndex;
   unsigned char  Counter;
   unsigned short Size;
   unsigned short StartIndex;
 } CtrDrvrPtimBinding;

typedef struct {
   unsigned short     Size;
   CtrDrvrPtimBinding Objects[CtrDrvrPtimOBJECTS];
 } CtrDrvrPtimObjects;

/* ***************************************************** */
/* Module descriptor                                     */

#ifdef CTR_PCI
typedef enum {
   CtrDrvrModuleTypeCTR,    /* Controls Timing Receiver */
   CtrDrvrModuleTypePLX     /* Uninitialized Plx9030 PCI chip */
 } CtrDrvrModuleType;

typedef struct {
   CtrDrvrModuleType ModuleType;
   unsigned int      PciSlot;
   unsigned int      ModuleNumber;
   unsigned int      DeviceId;
   unsigned int      VendorId;
   unsigned long     MemoryMap;
   unsigned long     LocalMap;
 } CtrDrvrModuleAddress;
#endif

#ifdef CTR_VME
typedef struct {
   unsigned int   *VMEAddress;         /* Base address for main logig A24,D32 */
   unsigned short *JTGAddress;         /* Base address for       JTAG A16,D16 */
   unsigned int    InterruptVector;    /* Interrupt vector number */
   unsigned int    InterruptLevel;     /* Interrupt level (2 usually) */
   unsigned int   *CopyAddress;        /* Copy of VME address */
 } CtrDrvrModuleAddress;
#endif

/* ***************************************************** */
/* The compilation dates in UTC time for components.     */

typedef enum {
   CtrDrvrHardwareTypeNONE,
   CtrDrvrHardwareTypeCTRP,
   CtrDrvrHardwareTypeCTRI,
   CtrDrvrHardwareTypeCTRV
 } CtrDrvrHardwareType;

#define CtrDrvrHardwareTYPES 4

typedef struct {
   unsigned int        VhdlVersion;  /* VHDL Compile date */
   unsigned int        DrvrVersion;  /* Drvr Compile date */
   CtrDrvrHardwareType HardwareType; /* Hardware type of module */
 } CtrDrvrVersion;

/* ***************************************************** */
/* HPTDC IO buffer                                       */

typedef struct {
   unsigned int        Size;   /* Number of HPTDC regs   */
   unsigned int        Pflg;   /* Parity flag            */
   unsigned int        Rflg;   /* Reset state flag       */
   HptdcCmd            Cmd;    /* Command                */
   HptdcRegisterVlaue  *Wreg;  /* Write  to HPTDC buffer */
   HptdcRegisterVlaue  *Rreg;  /* Read from HPTDC buffer */
 } CtrDrvrHptdcIoBuf;

/* ***************************************************** */
/* Counter configuration IO buffer                       */

typedef struct {
   CtrDrvrCounter Counter;
   CtrDrvrCounterConfiguration Config;
 } CtrDrvrCounterConfigurationBuf;

/* ***************************************************** */
/* Counter history IO buffer                             */

typedef struct {
   CtrDrvrCounter Counter;
   CtrDrvrCounterHistory History;
 } CtrDrvrCounterHistoryBuf;

/* ***************************************************** */
/* Counter remote command IO buffer                      */

typedef struct {
   CtrDrvrCounter Counter;
   CtrDrvrRemote Remote;
 } CtrdrvrRemoteCommandBuf;

/* ***************************************************** */
/* Counter output mask IO buffer                         */

typedef enum {
   CtrDrvrPolarityTTL,
   CtrDrvrPolarityTTL_BAR
 } CtrDrvrPolarity;

typedef struct {
   CtrDrvrCounter Counter;
   CtrDrvrCounterMask Mask;
   CtrDrvrPolarity Polarity;
 } CtrDrvrCounterMaskBuf;

/* ***************************************************** */
/* Telegram IO buffer                                    */

typedef struct {
   CtrDrvrMachine Machine;
   CtrDrvrTgm Telegram;
 } CtrDrvrTgmBuf;

/* ***************************************************** */
/* Raw IO                                                */

typedef struct {
   unsigned int Size;       /* Number int to read/write */
   unsigned int Offset;     /* Long offset address space */
   unsigned int *UserArray; /* Callers data area for  IO */
 } CtrDrvrRawIoBlock;

/* ***************************************************** */
/* Reception errors                                      */

typedef struct {
   unsigned int LastReset;     /* UTC Second of last reset */
   unsigned int PartityErrs;   /* Number of parity errors since last reset */
   unsigned int SyncErrs;      /* Number of frame synchronization errors since last reset */
   unsigned int TotalErrs;     /* Total number of IO errors since last reset */
   unsigned int CodeViolErrs;  /* Number of code violations since last reset */
   unsigned int QueueErrs;     /* Number of input Queue overflows since last reset */
 } CtrDrvrReceptionErrors;

/* ***************************************************** */
/* Board chip identity                                   */

typedef struct {
   unsigned int IdLSL;         /* ID Chip value Least Sig 32-bits */
   unsigned int IdMSL;         /* ID Chip value Most  Sig 32-bits */
 } CtrDrvrBoardId;

/* ***************************************************** */
/* Very special ISR debug code                           */

#define CtrDrvrDEBUG_ISR 0xFFFF

/* ***************************************************** */
/* Define the IOCTLs                                     */

typedef enum {

   /* Standard IOCTL Commands for timing users and developers */

   CtrDrvrSET_SW_DEBUG,           /* 00 Set driver debug mode */
   CtrDrvrGET_SW_DEBUG,

   CtrDrvrGET_VERSION,            /* 01 Get version date */

   CtrDrvrSET_TIMEOUT,            /* 02 Set the read timeout value */
   CtrDrvrGET_TIMEOUT,            /* 03 Get the read timeout value */

   CtrDrvrSET_QUEUE_FLAG,         /* 04 Set queuing capabiulities on off */
   CtrDrvrGET_QUEUE_FLAG,         /* 05 1=Q_off 0=Q_on */
   CtrDrvrGET_QUEUE_SIZE,         /* 06 Number of events on queue */
   CtrDrvrGET_QUEUE_OVERFLOW,     /* 07 Number of missed events */

   CtrDrvrGET_MODULE_DESCRIPTOR,  /* 08 Get the current Module descriptor */
   CtrDrvrSET_MODULE,             /* 09 Select the module to work with */
   CtrDrvrGET_MODULE,             /* 10 Which module am I working with */
   CtrDrvrGET_MODULE_COUNT,       /* 11 The number of installed modules */

   CtrDrvrRESET,                  /* 12 Reset the module, re-establish connections */
   CtrDrvrENABLE,                 /* 13 Enable CTR module event reception */
   CtrDrvrGET_STATUS,             /* 14 Read module status */

   CtrDrvrGET_INPUT_DELAY,        /* 15 Get input delay in 25ns ticks */
   CtrDrvrSET_INPUT_DELAY,        /* 16 Set input delay in 25ns ticks */

   CtrDrvrGET_CLIENT_LIST,        /* 17 Get the list of driver clients */

   CtrDrvrCONNECT,                /* 18 Connect to an object interrupt */
   CtrDrvrDISCONNECT,             /* 19 Disconnect from an object interrupt */
   CtrDrvrGET_CLIENT_CONNECTIONS, /* 20 Get the list of a client connections on module */

   CtrDrvrSET_UTC,                /* 21 Set Universal Coordinated Time for next PPS tick */
   CtrDrvrGET_UTC,                /* 22 Latch and read the current UTC time */

   CtrDrvrGET_CABLE_ID,           /* 23 Cables telegram ID */

   CtrDrvrGET_ACTION,             /* 24 Low level direct access to CTR RAM tables */
   CtrDrvrSET_ACTION,             /* 25 Set may not modify the bus interrupt settings */

   CtrDrvrCREATE_CTIM_OBJECT,     /* 26 Create a new CTIM timing object */
   CtrDrvrDESTROY_CTIM_OBJECT,    /* 27 Destroy a CTIM timing object */
   CtrDrvrLIST_CTIM_OBJECTS,      /* 28 Returns a list of created CTIM objects */
   CtrDrvrCHANGE_CTIM_FRAME,      /* 29 Change the frame of an existing CTIM object */

   CtrDrvrCREATE_PTIM_OBJECT,     /* 30 Create a new PTIM timing object */
   CtrDrvrDESTROY_PTIM_OBJECT,    /* 31 Destroy a PTIM timing object */
   CtrDrvrLIST_PTIM_OBJECTS,      /* 32 List PTIM timing objects */
   CtrDrvrGET_PTIM_BINDING,       /* 33 Search for a PTIM object binding */

   CtrDrvrGET_OUT_MASK,           /* 34 Counter output routing mask */
   CtrDrvrSET_OUT_MASK,           /* 35 Counter output routing mask */
   CtrDrvrGET_COUNTER_HISTORY,    /* 36 One deep history of counter */

   CtrDrvrGET_REMOTE,             /* 37 Counter Remote/Local status */
   CtrDrvrSET_REMOTE,             /* 38 Counter Remote/Local status */
   CtrDrvrREMOTE,                 /* 39 Remote control counter */

   CtrDrvrGET_CONFIG,             /* 40 Get a counter configuration */
   CtrDrvrSET_CONFIG,             /* 41 Set a counter configuration */

   CtrDrvrGET_PLL,                /* 42 Get phase locked loop parameters */
   CtrDrvrSET_PLL,                /* 43 Set phase locked loop parameters */
   CtrDrvrSET_PLL_ASYNC_PERIOD,   /* 44 Set PLL asynchronous period */
   CtrDrvrGET_PLL_ASYNC_PERIOD,   /* 45 Get PLL asynchronous period */

   CtrDrvrREAD_TELEGRAM,          /* 46 Read telegrams from CTR */
   CtrDrvrREAD_EVENT_HISTORY,     /* 47 Read incomming event history */

   /* ============================================================ */
   /* Hardware specialists IOCTL Commands to maintain and diagnose */
   /* the chips on the CTR board. Not for normal timing users.     */

   CtrDrvrJTAG_OPEN,              /* 48 Open JTAG interface to the Xilinx FPGA */
   CtrDrvrJTAG_READ_BYTE,         /* 49 Read back uploaded VHDL bit stream byte */
   CtrDrvrJTAG_WRITE_BYTE,        /* 50 Write compiled VHDL bit stream byte */
   CtrDrvrJTAG_CLOSE,             /* 51 Close JTAG interface */

   CtrDrvrHPTDC_OPEN,             /* 52 Open HPTDC JTAG interface */
   CtrDrvrHPTDC_IO,               /* 53 Perform HPTDC IO operation */
   CtrDrvrHPTDC_CLOSE,            /* 54 Close HPTDC JTAG interface */

   CtrDrvrRAW_READ,               /* 55 Raw read  access to mapped card for debug */
   CtrDrvrRAW_WRITE,              /* 56 Raw write access to mapped card for debug */

   CtrDrvrGET_RECEPTION_ERRORS,   /* 57 Timing fram reception error status */
   CtrDrvrGET_IO_STATUS,          /* 58 Status of module inputs */
   CtrDrvrGET_IDENTITY,           /* 59 Identity of board from ID chip */

   CtrDrvrSET_DEBUG_HISTORY,      /* 60 All events get logged in event history */
   CtrDrvrSET_BRUTAL_PLL,         /* 61 Control how UTC PLL relocks */
   CtrDrvrGET_MODULE_STATS,       /* 62 Get module statistics */

   CtrDrvrSET_CABLE_ID,           /* 63 Needed when no ID events sent */

   CtrDrvrIOCTL_64,               /* 64 Spare */
   CtrDrvrIOCTL_65,               /* 65 Spare */
   CtrDrvrIOCTL_66,               /* 66 Spare */
   CtrDrvrIOCTL_67,               /* 67 Spare */
   CtrDrvrIOCTL_68,               /* 68 Spare */
   CtrDrvrIOCTL_69,               /* 69 Spare */

   /* ============================================================ */
   /* Module specific IOCTL commands, Can't be used in a library!! */

#ifdef CTR_VME
   CtrDrvrGET_OUTPUT_BYTE,        /* 57 VME P2 output byte number */
   CtrDrvrSET_OUTPUT_BYTE,        /* 58 VME P2 output byte number */
#endif

#ifdef CTR_PCI
   CtrDrvrSET_MODULE_BY_SLOT,     /* 57 Select the module to work with by slot ID */
   CtrDrvrGET_MODULE_SLOT,        /* 58 Get the slot ID of the selected module */

   CtrDrvrREMAP,                  /* 59 Remap BAR2 after a config change */

   CtrDrvr93LC56B_EEPROM_OPEN,    /* 60 Open the PLX9030 configuration EEPROM 93LC56B for write */
   CtrDrvr93LC56B_EEPROM_READ,    /* 61 Read from the EEPROM 93LC56B the PLX9030 configuration */
   CtrDrvr93LC56B_EEPROM_WRITE,   /* 62 Write to the EEPROM 93LC56B a new PLX9030 configuration */
   CtrDrvr93LC56B_EEPROM_ERASE,   /* 63 Erase the EEPROM 93LC56B, deletes PLX9030 configuration */
   CtrDrvr93LC56B_EEPROM_CLOSE,   /* 64 Close EEPROM 93LC56B and load new PLX9030 configuration */

   CtrDrvrPLX9030_RECONFIGURE,    /* 65 Load EEPROM configuration into the PLX9030 */

   CtrDrvrPLX9030_CONFIG_OPEN,    /* 66 Open the PLX9030 configuration */
   CtrDrvrPLX9030_CONFIG_READ,    /* 67 Read the PLX9030 configuration registers */
   CtrDrvrPLX9030_CONFIG_WRITE,   /* 68 Write to PLX9030 configuration registers (Experts only) */
   CtrDrvrPLX9030_CONFIG_CLOSE,   /* 69 Close the PLX9030 configuration */

   CtrDrvrPLX9030_LOCAL_OPEN,     /* 70 Open the PLX9030 local configuration */
   CtrDrvrPLX9030_LOCAL_READ,     /* 71 Read the PLX9030 local configuration registers */
   CtrDrvrPLX9030_LOCAL_WRITE,    /* 72 Write the PLX9030 local configuration registers (Experts only) */
   CtrDrvrPLX9030_LOCAL_CLOSE,    /* 73 Close the PLX9030 local configuration */
#endif

   CtrDrvrLAST_IOCTL

 } CtrDrvrControlFunction;
#endif

/*
 * Set up the IOCTL numbers
 */

#ifdef __linux__

#define MAGIC 'C'

#define CIO(nr)      _IO(MAGIC,nr)
#define CIOR(nr,sz)  _IOR(MAGIC,nr,sz)
#define CIOW(nr,sz)  _IOW(MAGIC,nr,sz)
#define CIOWR(nr,sz) _IOWR(MAGIC,nr,sz)

#define CtrIoctlSET_SW_DEBUG                   CIOWR(CtrDrvrSET_SW_DEBUG           ,unsigned long)
#define CtrIoctlGET_SW_DEBUG                   CIOWR(CtrDrvrGET_SW_DEBUG           ,unsigned long)
#define CtrIoctlGET_VERSION                    CIOWR(CtrDrvrGET_VERSION            ,CtrDrvrVersion)
#define CtrIoctlSET_TIMEOUT                    CIOWR(CtrDrvrSET_TIMEOUT            ,unsigned long)
#define CtrIoctlGET_TIMEOUT                    CIOWR(CtrDrvrGET_TIMEOUT            ,unsigned long)
#define CtrIoctlSET_QUEUE_FLAG                 CIOWR(CtrDrvrSET_QUEUE_FLAG         ,unsigned long)
#define CtrIoctlGET_QUEUE_FLAG                 CIOWR(CtrDrvrGET_QUEUE_FLAG         ,unsigned long)
#define CtrIoctlGET_QUEUE_SIZE                 CIOWR(CtrDrvrGET_QUEUE_SIZE         ,unsigned long)
#define CtrIoctlGET_QUEUE_OVERFLOW             CIOWR(CtrDrvrGET_QUEUE_OVERFLOW     ,unsigned long)
#define CtrIoctlGET_MODULE_DESCRIPTOR          CIOWR(CtrDrvrGET_MODULE_DESCRIPTOR  ,CtrDrvrModuleAddress)
#define CtrIoctlSET_MODULE                     CIOWR(CtrDrvrSET_MODULE             ,unsigned long)
#define CtrIoctlGET_MODULE                     CIOWR(CtrDrvrGET_MODULE             ,unsigned long)
#define CtrIoctlGET_MODULE_COUNT               CIOWR(CtrDrvrGET_MODULE_COUNT       ,unsigned long)
#define CtrIoctlRESET                          CIOWR(CtrDrvrRESET                  ,unsigned long)
#define CtrIoctlENABLE                         CIOWR(CtrDrvrENABLE                 ,unsigned long)
#define CtrIoctlGET_STATUS                     CIOWR(CtrDrvrGET_STATUS             ,unsigned long)
#define CtrIoctlGET_INPUT_DELAY                CIOWR(CtrDrvrGET_INPUT_DELAY        ,unsigned long)
#define CtrIoctlSET_INPUT_DELAY                CIOWR(CtrDrvrSET_INPUT_DELAY        ,unsigned long)
#define CtrIoctlGET_CLIENT_LIST                CIOWR(CtrDrvrGET_CLIENT_LIST        ,CtrDrvrClientList)
#define CtrIoctlCONNECT                        CIOWR(CtrDrvrCONNECT                ,CtrDrvrConnection)
#define CtrIoctlDISCONNECT                     CIOWR(CtrDrvrDISCONNECT             ,CtrDrvrConnection)
#define CtrIoctlGET_CLIENT_CONNECTIONS         CIOWR(CtrDrvrGET_CLIENT_CONNECTIONS ,CtrDrvrClientConnections)
#define CtrIoctlSET_UTC                        CIOWR(CtrDrvrSET_UTC                ,CtrDrvrCTime)
#define CtrIoctlGET_UTC                        CIOWR(CtrDrvrGET_UTC                ,CtrDrvrCTime)
#define CtrIoctlGET_CABLE_ID                   CIOWR(CtrDrvrGET_CABLE_ID           ,unsigned long)
#define CtrIoctlGET_ACTION                     CIOWR(CtrDrvrGET_ACTION             ,CtrDrvrAction)
#define CtrIoctlSET_ACTION                     CIOWR(CtrDrvrSET_ACTION             ,CtrDrvrAction)
#define CtrIoctlCREATE_CTIM_OBJECT             CIOWR(CtrDrvrCREATE_CTIM_OBJECT     ,CtrDrvrCtimBinding)
#define CtrIoctlDESTROY_CTIM_OBJECT            CIOWR(CtrDrvrDESTROY_CTIM_OBJECT    ,CtrDrvrCtimBinding)
#define CtrIoctlLIST_CTIM_OBJECTS              CIOWR(CtrDrvrLIST_CTIM_OBJECTS      ,CtrDrvrCtimObjects) // too big
#define CtrIoctlCHANGE_CTIM_FRAME              CIOWR(CtrDrvrCHANGE_CTIM_FRAME      ,CtrDrvrCtimBinding)
#define CtrIoctlCREATE_PTIM_OBJECT             CIOWR(CtrDrvrCREATE_PTIM_OBJECT     ,CtrDrvrPtimBinding)
#define CtrIoctlDESTROY_PTIM_OBJECT            CIOWR(CtrDrvrDESTROY_PTIM_OBJECT    ,CtrDrvrPtimBinding)
#define CtrIoctlLIST_PTIM_OBJECTS              CIOWR(CtrDrvrLIST_PTIM_OBJECTS      ,CtrDrvrPtimObjects)
#define CtrIoctlGET_PTIM_BINDING               CIOWR(CtrDrvrGET_PTIM_BINDING       ,CtrDrvrPtimBinding)
#define CtrIoctlGET_OUT_MASK                   CIOWR(CtrDrvrGET_OUT_MASK           ,CtrDrvrCounterMaskBuf)
#define CtrIoctlSET_OUT_MASK                   CIOWR(CtrDrvrSET_OUT_MASK           ,CtrDrvrCounterMaskBuf)
#define CtrIoctlGET_COUNTER_HISTORY            CIOWR(CtrDrvrGET_COUNTER_HISTORY    ,CtrDrvrCounterHistoryBuf)
#define CtrIoctlGET_REMOTE                     CIOWR(CtrDrvrGET_REMOTE             ,CtrdrvrRemoteCommandBuf)
#define CtrIoctlSET_REMOTE                     CIOWR(CtrDrvrSET_REMOTE             ,CtrdrvrRemoteCommandBuf)
#define CtrIoctlREMOTE                         CIOWR(CtrDrvrREMOTE                 ,CtrdrvrRemoteCommandBuf)
#define CtrIoctlGET_CONFIG                     CIOWR(CtrDrvrGET_CONFIG             ,CtrDrvrCounterConfigurationBuf)
#define CtrIoctlSET_CONFIG                     CIOWR(CtrDrvrSET_CONFIG             ,CtrDrvrCounterConfigurationBuf)
#define CtrIoctlGET_PLL                        CIOWR(CtrDrvrGET_PLL                ,CtrDrvrPll)
#define CtrIoctlSET_PLL                        CIOWR(CtrDrvrSET_PLL                ,CtrDrvrPll)
#define CtrIoctlSET_PLL_ASYNC_PERIOD           CIOWR(CtrDrvrSET_PLL_ASYNC_PERIOD   ,CtrDrvrPllAsyncPeriodNs)
#define CtrIoctlGET_PLL_ASYNC_PERIOD           CIOWR(CtrDrvrGET_PLL_ASYNC_PERIOD   ,CtrDrvrPllAsyncPeriodNs)
#define CtrIoctlREAD_TELEGRAM                  CIOWR(CtrDrvrREAD_TELEGRAM          ,CtrDrvrTgmBuf)
#define CtrIoctlREAD_EVENT_HISTORY             CIOWR(CtrDrvrREAD_EVENT_HISTORY     ,CtrDrvrEventHistory) // too big
#define CtrIoctlJTAG_OPEN                      CIOWR(CtrDrvrJTAG_OPEN              ,unsigned long)
#define CtrIoctlJTAG_READ_BYTE                 CIOWR(CtrDrvrJTAG_READ_BYTE         ,unsigned long)
#define CtrIoctlJTAG_WRITE_BYTE                CIOWR(CtrDrvrJTAG_WRITE_BYTE        ,unsigned long)
#define CtrIoctlJTAG_CLOSE                     CIOWR(CtrDrvrJTAG_CLOSE             ,unsigned long)
#define CtrIoctlHPTDC_OPEN                     CIOWR(CtrDrvrHPTDC_OPEN             ,unsigned long)
#define CtrIoctlHPTDC_IO                       CIOWR(CtrDrvrHPTDC_IO               ,CtrDrvrHptdcIoBuf)
#define CtrIoctlHPTDC_CLOSE                    CIOWR(CtrDrvrHPTDC_CLOSE            ,unsigned long)
#define CtrIoctlRAW_READ                       CIOWR(CtrDrvrRAW_READ               ,CtrDrvrRawIoBlock)
#define CtrIoctlRAW_WRITE                      CIOWR(CtrDrvrRAW_WRITE              ,CtrDrvrRawIoBlock)
#define CtrIoctlGET_RECEPTION_ERRORS           CIOWR(CtrDrvrGET_RECEPTION_ERRORS   ,CtrDrvrReceptionErrors)
#define CtrIoctlGET_IO_STATUS                  CIOWR(CtrDrvrGET_IO_STATUS          ,unsigned long)
#define CtrIoctlGET_IDENTITY                   CIOWR(CtrDrvrGET_IDENTITY           ,CtrDrvrBoardId)
#define CtrIoctlSET_DEBUG_HISTORY              CIOWR(CtrDrvrSET_DEBUG_HISTORY      ,unsigned long)
#define CtrIoctlSET_BRUTAL_PLL                 CIOWR(CtrDrvrSET_BRUTAL_PLL         ,unsigned long)
#define CtrIoctlGET_MODULE_STATS               CIOWR(CtrDrvrGET_MODULE_STATS       ,CtrDrvrModuleStats)
#define CtrIoctlSET_CABLE_ID                   CIOWR(CtrDrvrSET_CABLE_ID           ,unsigned long)
#define CtrIoctlIOCTL_64                       CIOWR(CtrDrvrIOCTL_64               ,unsigned long)
#define CtrIoctlIOCTL_65                       CIOWR(CtrDrvrIOCTL_65               ,unsigned long)
#define CtrIoctlIOCTL_66                       CIOWR(CtrDrvrIOCTL_66               ,unsigned long)
#define CtrIoctlIOCTL_67                       CIOWR(CtrDrvrIOCTL_67               ,unsigned long)
#define CtrIoctlIOCTL_68                       CIOWR(CtrDrvrIOCTL_68               ,unsigned long)
#define CtrIoctlIOCTL_69                       CIOWR(CtrDrvrIOCTL_69               ,unsigned long)

#ifdef CTR_VME
#define CtrIoctlGET_OUTPUT_BYTE                CIOWR(CtrDrvrGET_OUTPUT_BYTE        ,unsigned long)
#define CtrIoctlSET_OUTPUT_BYTE                CIOWR(CtrDrvrSET_OUTPUT_BYTE        ,unsigned long)
#endif

#ifdef CTR_PCI
#define CtrIoctlSET_MODULE_BY_SLOT             CIOWR(CtrDrvrSET_MODULE_BY_SLOT     ,unsigned long)
#define CtrIoctlGET_MODULE_SLOT                CIOWR(CtrDrvrGET_MODULE_SLOT        ,unsigned long)
#define CtrIoctlREMAP                          CIOWR(CtrDrvrREMAP                  ,unsigned long)
#define CtrIoctl93LC56B_EEPROM_OPEN            CIOWR(CtrDrvr93LC56B_EEPROM_OPEN    ,unsigned long)
#define CtrIoctl93LC56B_EEPROM_READ            CIOWR(CtrDrvr93LC56B_EEPROM_READ    ,CtrDrvrRawIoBlock)
#define CtrIoctl93LC56B_EEPROM_WRITE           CIOWR(CtrDrvr93LC56B_EEPROM_WRITE   ,CtrDrvrRawIoBlock)
#define CtrIoctl93LC56B_EEPROM_ERASE           CIOWR(CtrDrvr93LC56B_EEPROM_ERASE   ,unsigned long)
#define CtrIoctl93LC56B_EEPROM_CLOSE           CIOWR(CtrDrvr93LC56B_EEPROM_CLOSE   ,unsigned long)
#define CtrIoctlPLX9030_RECONFIGURE            CIOWR(CtrDrvrPLX9030_RECONFIGURE    ,unsigned long)
#define CtrIoctlPLX9030_CONFIG_OPEN            CIOWR(CtrDrvrPLX9030_CONFIG_OPEN    ,unsigned long)
#define CtrIoctlPLX9030_CONFIG_READ            CIOWR(CtrDrvrPLX9030_CONFIG_READ    ,CtrDrvrRawIoBlock)
#define CtrIoctlPLX9030_CONFIG_WRITE           CIOWR(CtrDrvrPLX9030_CONFIG_WRITE   ,CtrDrvrRawIoBlock)
#define CtrIoctlPLX9030_CONFIG_CLOSE           CIOWR(CtrDrvrPLX9030_CONFIG_CLOSE   ,unsigned long)
#define CtrIoctlPLX9030_LOCAL_OPEN             CIOWR(CtrDrvrPLX9030_LOCAL_OPEN     ,unsigned long)
#define CtrIoctlPLX9030_LOCAL_READ             CIOWR(CtrDrvrPLX9030_LOCAL_READ     ,CtrDrvrRawIoBlock)
#define CtrIoctlPLX9030_LOCAL_WRITE            CIOWR(CtrDrvrPLX9030_LOCAL_WRITE    ,CtrDrvrRawIoBlock)
#define CtrIoctlPLX9030_LOCAL_CLOSE            CIOWR(CtrDrvrPLX9030_LOCAL_CLOSE    ,unsigned long)
#endif


#else

#define CtrIoctlSET_SW_DEBUG                   CtrDrvrSET_SW_DEBUG
#define CtrIoctlGET_SW_DEBUG                   CtrDrvrGET_SW_DEBUG
#define CtrIoctlGET_VERSION                    CtrDrvrGET_VERSION
#define CtrIoctlSET_TIMEOUT                    CtrDrvrSET_TIMEOUT
#define CtrIoctlGET_TIMEOUT                    CtrDrvrGET_TIMEOUT
#define CtrIoctlSET_QUEUE_FLAG                 CtrDrvrSET_QUEUE_FLAG
#define CtrIoctlGET_QUEUE_FLAG                 CtrDrvrGET_QUEUE_FLAG
#define CtrIoctlGET_QUEUE_SIZE                 CtrDrvrGET_QUEUE_SIZE
#define CtrIoctlGET_QUEUE_OVERFLOW             CtrDrvrGET_QUEUE_OVERFLOW
#define CtrIoctlGET_MODULE_DESCRIPTOR          CtrDrvrGET_MODULE_DESCRIPTOR
#define CtrIoctlSET_MODULE                     CtrDrvrSET_MODULE
#define CtrIoctlGET_MODULE                     CtrDrvrGET_MODULE
#define CtrIoctlGET_MODULE_COUNT               CtrDrvrGET_MODULE_COUNT
#define CtrIoctlRESET                          CtrDrvrRESET
#define CtrIoctlENABLE                         CtrDrvrENABLE
#define CtrIoctlGET_STATUS                     CtrDrvrGET_STATUS
#define CtrIoctlGET_INPUT_DELAY                CtrDrvrGET_INPUT_DELAY
#define CtrIoctlSET_INPUT_DELAY                CtrDrvrSET_INPUT_DELAY
#define CtrIoctlGET_CLIENT_LIST                CtrDrvrGET_CLIENT_LIST
#define CtrIoctlCONNECT                        CtrDrvrCONNECT
#define CtrIoctlDISCONNECT                     CtrDrvrDISCONNECT
#define CtrIoctlGET_CLIENT_CONNECTIONS         CtrDrvrGET_CLIENT_CONNECTIONS
#define CtrIoctlSET_UTC                        CtrDrvrSET_UTC
#define CtrIoctlGET_UTC                        CtrDrvrGET_UTC
#define CtrIoctlGET_CABLE_ID                   CtrDrvrGET_CABLE_ID
#define CtrIoctlGET_ACTION                     CtrDrvrGET_ACTION
#define CtrIoctlSET_ACTION                     CtrDrvrSET_ACTION
#define CtrIoctlCREATE_CTIM_OBJECT             CtrDrvrCREATE_CTIM_OBJECT
#define CtrIoctlDESTROY_CTIM_OBJECT            CtrDrvrDESTROY_CTIM_OBJECT
#define CtrIoctlLIST_CTIM_OBJECTS              CtrDrvrLIST_CTIM_OBJECTS
#define CtrIoctlCHANGE_CTIM_FRAME              CtrDrvrCHANGE_CTIM_FRAME
#define CtrIoctlCREATE_PTIM_OBJECT             CtrDrvrCREATE_PTIM_OBJECT
#define CtrIoctlDESTROY_PTIM_OBJECT            CtrDrvrDESTROY_PTIM_OBJECT
#define CtrIoctlLIST_PTIM_OBJECTS              CtrDrvrLIST_PTIM_OBJECTS
#define CtrIoctlGET_PTIM_BINDING               CtrDrvrGET_PTIM_BINDING
#define CtrIoctlGET_OUT_MASK                   CtrDrvrGET_OUT_MASK
#define CtrIoctlSET_OUT_MASK                   CtrDrvrSET_OUT_MASK
#define CtrIoctlGET_COUNTER_HISTORY            CtrDrvrGET_COUNTER_HISTORY
#define CtrIoctlGET_REMOTE                     CtrDrvrGET_REMOTE
#define CtrIoctlSET_REMOTE                     CtrDrvrSET_REMOTE
#define CtrIoctlREMOTE                         CtrDrvrREMOTE
#define CtrIoctlGET_CONFIG                     CtrDrvrGET_CONFIG
#define CtrIoctlSET_CONFIG                     CtrDrvrSET_CONFIG
#define CtrIoctlGET_PLL                        CtrDrvrGET_PLL
#define CtrIoctlSET_PLL                        CtrDrvrSET_PLL
#define CtrIoctlSET_PLL_ASYNC_PERIOD           CtrDrvrSET_PLL_ASYNC_PERIOD
#define CtrIoctlGET_PLL_ASYNC_PERIOD           CtrDrvrGET_PLL_ASYNC_PERIOD
#define CtrIoctlREAD_TELEGRAM                  CtrDrvrREAD_TELEGRAM
#define CtrIoctlREAD_EVENT_HISTORY             CtrDrvrREAD_EVENT_HISTORY
#define CtrIoctlJTAG_OPEN                      CtrDrvrJTAG_OPEN
#define CtrIoctlJTAG_READ_BYTE                 CtrDrvrJTAG_READ_BYTE
#define CtrIoctlJTAG_WRITE_BYTE                CtrDrvrJTAG_WRITE_BYTE
#define CtrIoctlJTAG_CLOSE                     CtrDrvrJTAG_CLOSE
#define CtrIoctlHPTDC_OPEN                     CtrDrvrHPTDC_OPEN
#define CtrIoctlHPTDC_IO                       CtrDrvrHPTDC_IO
#define CtrIoctlHPTDC_CLOSE                    CtrDrvrHPTDC_CLOSE
#define CtrIoctlRAW_READ                       CtrDrvrRAW_READ
#define CtrIoctlRAW_WRITE                      CtrDrvrRAW_WRITE
#define CtrIoctlGET_RECEPTION_ERRORS           CtrDrvrGET_RECEPTION_ERRORS
#define CtrIoctlGET_IO_STATUS                  CtrDrvrGET_IO_STATUS
#define CtrIoctlGET_IDENTITY                   CtrDrvrGET_IDENTITY
#define CtrIoctlSET_DEBUG_HISTORY              CtrDrvrSET_DEBUG_HISTORY
#define CtrIoctlSET_BRUTAL_PLL                 CtrDrvrSET_BRUTAL_PLL
#define CtrIoctlGET_MODULE_STATS               CtrDrvrGET_MODULE_STATS
#define CtrIoctlSET_CABLE_ID                   CtrDrvrSET_CABLE_ID
#define CtrIoctlIOCTL_64                       CtrDrvrIOCTL_64
#define CtrIoctlIOCTL_65                       CtrDrvrIOCTL_65
#define CtrIoctlIOCTL_66                       CtrDrvrIOCTL_66
#define CtrIoctlIOCTL_67                       CtrDrvrIOCTL_67
#define CtrIoctlIOCTL_68                       CtrDrvrIOCTL_68
#define CtrIoctlIOCTL_69                       CtrDrvrIOCTL_69

#ifdef CTR_VME
#define CtrIoctlGET_OUTPUT_BYTE                CtrDrvrGET_OUTPUT_BYTE
#define CtrIoctlSET_OUTPUT_BYTE                CtrDrvrSET_OUTPUT_BYTE
#endif

#ifdef CTR_PCI
#define CtrIoctlSET_MODULE_BY_SLOT             CtrDrvrSET_MODULE_BY_SLOT
#define CtrIoctlGET_MODULE_SLOT                CtrDrvrGET_MODULE_SLOT
#define CtrIoctlREMAP                          CtrDrvrREMAP
#define CtrIoctl93LC56B_EEPROM_OPEN            CtrDrvr93LC56B_EEPROM_OPEN
#define CtrIoctl93LC56B_EEPROM_READ            CtrDrvr93LC56B_EEPROM_READ
#define CtrIoctl93LC56B_EEPROM_WRITE           CtrDrvr93LC56B_EEPROM_WRITE
#define CtrIoctl93LC56B_EEPROM_ERASE           CtrDrvr93LC56B_EEPROM_ERASE
#define CtrIoctl93LC56B_EEPROM_CLOSE           CtrDrvr93LC56B_EEPROM_CLOSE
#define CtrIoctlPLX9030_RECONFIGURE            CtrDrvrPLX9030_RECONFIGURE
#define CtrIoctlPLX9030_CONFIG_OPEN            CtrDrvrPLX9030_CONFIG_OPEN
#define CtrIoctlPLX9030_CONFIG_READ            CtrDrvrPLX9030_CONFIG_READ
#define CtrIoctlPLX9030_CONFIG_WRITE           CtrDrvrPLX9030_CONFIG_WRITE
#define CtrIoctlPLX9030_CONFIG_CLOSE           CtrDrvrPLX9030_CONFIG_CLOSE
#define CtrIoctlPLX9030_LOCAL_OPEN             CtrDrvrPLX9030_LOCAL_OPEN
#define CtrIoctlPLX9030_LOCAL_READ             CtrDrvrPLX9030_LOCAL_READ
#define CtrIoctlPLX9030_LOCAL_WRITE            CtrDrvrPLX9030_LOCAL_WRITE
#define CtrIoctlPLX9030_LOCAL_CLOSE            CtrDrvrPLX9030_LOCAL_CLOSE
#endif

#endif
