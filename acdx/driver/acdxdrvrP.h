/* ********************************************************************************************* */
/* ACDipole X3 Servo module driver                                                               */
/* Private definitions for driver code only                                                      */
/* Julian Lewis AB/CO/HT 15th July 2008                                                          */
/* ********************************************************************************************* */

#include <acdxdrvr.h>

#ifndef ACDX_PRIVATE
#define ACDX_PRIVATE

#define VENDOR_ID 0x1303
#define DEVICE_ID 0x0030

/* ==================================================== */
/* The acdx memory map                                  */
/* ==================================================== */

#define AcdxDrvrTOTAL_POINTS 1048576
#define AcdxDrvrPOINTS_PER_SECOND 250000
#define AcdxDrvrDURATION ( (float) AcdxDrvrTOTAL_POINTS / (float) AcdxDrvrPOINTS_PER_SECOND )
#define AcdxDrvrMAP_LONGS 0x80

typedef struct {
   unsigned long FPGAVersion;
   unsigned long StatusControl;
   unsigned long Test;
   unsigned long Temperature;
   unsigned long TempWarning;
   unsigned long TempFailure;
   unsigned long SRAMAddr;
   unsigned long SRAMData;
   unsigned long PLLClockCtl;
   unsigned long PLDTimerCtl;
   unsigned long PLLIntrface;
   unsigned long ADCChanEnbl;
   unsigned long DACChanEnbl;
   unsigned long Spare01;
   unsigned long ZBTSRAMAddr;
   unsigned long ZBTSRAMData;
   unsigned long ADAmpEnabl;
   unsigned long DACControls;
   unsigned long ADCControls;
   unsigned long P16DIOReg;
   unsigned long P16DIOCtlReg;
   unsigned long FrontPanlDIO;
   unsigned long Spare02;
   unsigned long TrigCtlAD;
   unsigned long TrigCtlDAC;
   unsigned long ADDecimatn;
   unsigned long DACDecimatn;
   unsigned long PktHeadrDAC;
   unsigned long PktHeadrAD;
   unsigned long SoftAlert;
   unsigned long AlertCtlReg;
   unsigned long AlertEnabls;

   unsigned long ADGainErCf01;
   unsigned long ADGainErCf02;
   unsigned long ADGainErCf03;
   unsigned long ADGainErCf04;
   unsigned long ADGainErCf05;
   unsigned long ADGainErCf06;
   unsigned long ADGainErCf07;
   unsigned long ADGainErCf08;
   unsigned long ADGainErCf09;
   unsigned long ADGainErCf10;
   unsigned long ADGainErCf11;
   unsigned long ADGainErCf12;
   unsigned long ADGainErCf13;
   unsigned long ADGainErCf14;
   unsigned long ADGainErCf15;
   unsigned long ADGainErCf16;

   unsigned long ADOffsErCf01;
   unsigned long ADOffsErCf02;
   unsigned long ADOffsErCf03;
   unsigned long ADOffsErCf04;
   unsigned long ADOffsErCf05;
   unsigned long ADOffsErCf06;
   unsigned long ADOffsErCf07;
   unsigned long ADOffsErCf08;
   unsigned long ADOffsErCf09;
   unsigned long ADOffsErCf10;
   unsigned long ADOffsErCf11;
   unsigned long ADOffsErCf12;
   unsigned long ADOffsErCf13;
   unsigned long ADOffsErCf14;
   unsigned long ADOffsErCf15;
   unsigned long ADOffsErCf16;

   unsigned long DACGainErCf01;
   unsigned long DACGainErCf02;
   unsigned long DACGainErCf03;
   unsigned long DACGainErCf04;
   unsigned long DACGainErCf05;
   unsigned long DACGainErCf06;
   unsigned long DACGainErCf07;
   unsigned long DACGainErCf08;
   unsigned long DACGainErCf09;
   unsigned long DACGainErCf10;
   unsigned long DACGainErCf11;
   unsigned long DACGainErCf12;
   unsigned long DACGainErCf13;
   unsigned long DACGainErCf14;
   unsigned long DACGainErCf15;
   unsigned long DACGainErCf16;

   unsigned long DACOffsErCf01;
   unsigned long DACOffsErCf02;
   unsigned long DACOffsErCf03;
   unsigned long DACOffsErCf04;
   unsigned long DACOffsErCf05;
   unsigned long DACOffsErCf06;
   unsigned long DACOffsErCf07;
   unsigned long DACOffsErCf08;
   unsigned long DACOffsErCf09;
   unsigned long DACOffsErCf10;
   unsigned long DACOffsErCf11;
   unsigned long DACOffsErCf12;
   unsigned long DACOffsErCf13;
   unsigned long DACOffsErCf14;
   unsigned long DACOffsErCf15;
   unsigned long DACOffsErCf16;

   unsigned long Spare03;
   unsigned long Spare04;
   unsigned long Spare05;
   unsigned long Spare06;

   unsigned long SinFrequency;
   unsigned long SinAmplitude;
   unsigned long SinRiseTime;
   unsigned long SinFlTopTime;
   unsigned long SinFallTime;

   unsigned long Spare07;
   unsigned long Spare08;
   unsigned long Spare09;
   unsigned long Spare10;
   unsigned long Spare11;
   unsigned long Spare12;
   unsigned long Spare13;

   unsigned long ADGain00;
   unsigned long ADGain01;
   unsigned long ADGain02;
   unsigned long ADGain03;
   unsigned long ADGain04;
   unsigned long ADGain05;
   unsigned long ADGain06;
   unsigned long ADGain07;
   unsigned long ADGain08;
   unsigned long ADGain09;
   unsigned long ADGain10;
   unsigned long ADGain11;

   unsigned long Spare14;
   unsigned long Spare15;
   unsigned long Spare16;
   unsigned long Spare17;
 } AcdxDrvrRegs;

typedef union {                                 /* BAR 1 */
   unsigned long  Array[AcdxDrvrMAP_LONGS];
   AcdxDrvrRegs   Regs;
 } AcdxDrvrMemoryMap;

typedef enum {
   AcdxDrvrCtrlBUSY = 0x01, /* RO */
   AcdxDrvrCtrlDONE = 0x02, /* RO */
   AcdxDrvrCtrlINIT = 0x04, /* RO */
   AcdxDrvrCtrlPROG = 0x10  /* WO 0 = Reset FPGA configuration */
 } AcdxDrvrCtrl;

#define FpgaData 0x12000
#define FpgaCtrl 0x12400

/* ============================================================ */
/* Endian is needed to set up the Plx9030 mapping               */
/* ============================================================ */

typedef enum {
   AcdxDrvrEndianUNKNOWN,
   AcdxDrvrEndianBIG,
   AcdxDrvrEndianLITTLE
 } AcdxDrvrEndian;

/* ============================================================ */
/* The client context                                           */
/* ============================================================ */

typedef struct {
   unsigned long InUse;
   unsigned long DebugOn;
   unsigned long Pid;
   unsigned long ClientIndex;
   unsigned long ModuleIndex;
 } AcdxDrvrClientContext;

/* ============================================================ */
/* The Module context                                           */
/* ============================================================ */

#define AcdxDrvrFORCE_UNLOCK 0x4c98abf2

typedef struct {

   drm_node_handle              Handle;            /* Handle from DRM */
   int                          LinkId;            /* IointSet Link ID */
   unsigned long                IVector;           /* Resulting interrupt vector */
   unsigned long                DeviceId;          /* ACDXP or PLX9030 */
   unsigned long                PciSlot;           /* Module geographic ID PCI Slot */
   unsigned long                ModuleIndex;       /* Which module we are */
   unsigned long                InUse;             /* Module context in use */
   unsigned long                LockKey;           /* Lock Key */
   unsigned long                FpgaOpen;          /* True when open for updating */
   int                          Timer;             /* Timer */
   int                          Semaphore;         /* Wait semaphore */
   AcdxDrvrFunctionParams       FunParms;          /* Sinwave function parameters */
   AcdxDrvrMemoryMap            *Map;              /* Pointer to hardware at BAR1 */
   unsigned long                *Local;            /* Pointer to hardware at BAR0 */

 } AcdxDrvrModuleContext;

/* ============================================================ */
/* Driver Working Area                                          */
/* ============================================================ */

typedef struct {

   AcdxDrvrEndian             Endian;
   unsigned long              Modules;
   AcdxDrvrModuleContext      ModuleContexts[AcdxDrvrMODULE_CONTEXTS];
   AcdxDrvrClientContext      ClientContexts[AcdxDrvrCLIENT_CONTEXTS];
 } AcdxDrvrWorkingArea;

/* ============================================================ */
/* Info Table                                                   */
/* ============================================================ */

typedef struct {

   int RecoverMode; /* Needed to recover "lost" modules after FPGA crash */

 } AcdxDrvrInfoTable;

#endif
