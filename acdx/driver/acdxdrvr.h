/* ********************************************************************************************* */
/*                                                                                               */
/* ********************************************************************************************* */

#ifndef ACDXDRVR
#define ACDXDRVR

/* Maximum number of simultaneous clients for driver */

#define AcdxDrvrCLIENT_CONTEXTS 16

/* Maximum number of ACDX modules on one host processor */

#define AcdxDrvrMODULE_CONTEXTS 1

/* ***************************************************** */
/* Module descriptor                                     */

typedef enum {
   AcdxDrvrModuleTypeACDX,    /* Controls Timing Receiver */
   AcdxDrvrModuleTypePLX     /* Uninitialized Plx9030 PCI chip */
 } AcdxDrvrModuleType;

typedef struct {
   AcdxDrvrModuleType ModuleType;
   unsigned long     PciSlot;
   unsigned long     ModuleNumber;
   unsigned long     DeviceId;
   unsigned long     VendorId;
   unsigned long     MemoryMap;
   unsigned long     LocalMap;
 } AcdxDrvrModuleAddress;


typedef struct {
   unsigned long Size;      /* Number long to read/write */
   unsigned long Offset;    /* Long offset address space */
   unsigned long *UserArray;/* Callers data area for  IO */
 } AcdxDrvrRawIoBlock;

#define AcdxDrvrMIN_FREQ 2700   /* Hertz */
#define AcdxDrvrMAX_FREQ 4000   /* Hertz */
#define AcdxDrvrMIN_AMPL 0      /* Milli volts */
#define AcdxDrvrMAX_AMPL 10000  /* Milli volts */
#define AcdxDrvrMIN_TIME 200    /* Milli seconds */

typedef struct {
   unsigned long  SinFrequency;  /* In Hertz 2500..3500 */
   unsigned long  SinAmplitude;  /* In milli volts 0..10000 */
   unsigned long  SinRiseTime;   /* In milli seconds 200..available */
   unsigned long  SinFlTopTime;  /* In milli seconds 200..available */
   unsigned long  SinFallTime;   /* In milli seconds 200..available */
 } AcdxDrvrFunctionParams;

typedef struct {
   unsigned long FpgaVersion;   /* FPGA Version number */
   unsigned long DrvrVersion;   /* Driver Version */
 } AcdxDrvrVersion;

#define AcdxDrvrCHUNK_SIZE 1024

typedef struct {
   unsigned long Size;                      /* Number of bytes to load */
   unsigned char Chunk[AcdxDrvrCHUNK_SIZE]; /* Chunk of code for FPGA */
 } AcdxDrvrFpgaChunk;

typedef struct {
   unsigned long Temperature;
   unsigned long TempWarning;
   unsigned long TempFailure;
 } AcdxDrvrTemperature;

typedef enum {

   AcdxDrvrStatusControlRESET_USER_FPGA    = 0x000001, /* RW 1=Reset users FPGA */
   AcdxDrvrStatusControlADC_RUN            = 0x000002, /* RW Allows data capture. 1=run 0=off (Default) */
   AcdxDrvrStatusControlINHIB_EXT_TRIG     = 0x000004, /* RW 1=Inhibit external trigger */
   AcdxDrvrStatusControlSOFTWARE_TRIG      = 0x000008, /* RW 1=Auto reset */

   AcdxDrvrStatusControlAMPLIFIER_CTRL     = 0x001000, /* RO From AB/BT plc */
   AcdxDrvrStatusControlAMPLIFIER_STAT     = 0x002000, /* RW To   AB/BT plc */
   AcdxDrvrStatusControlARM                = 0x008000, /* RW Arm, trigger allowed */

   AcdxDrvrStatusControlDAC_RUN            = 0x010000, /* RW Allows data capture. 1=run 0=off (Delault) (DIO Out) */
   AcdxDrvrStatusControlRUNNING            = 0x040000, /* RO 1=Waveform being generated */
   AcdxDrvrStatusControlAQN_RUN            = 0x080000, /* RO 1=Acquisition in progress */

   AcdxDrvrStatusControlDCM_LOCKED         = 0x100000,  /* RO 1=DCM is locked */
   AcdxDrvrStatusControlONE_MIN_INHIB      = 0x200000,  /* RO Trigger inhibit 1 Min */
   AcdxDrvrStatusControlPLL_LOCKED         = 0x400000   /* PLL is Locked */

 } AcdxDrvrStatusControl;

/* WR (writeable) and RD (readable) bit masks */

#define AcdxDrvrStatusControlWR_BIT_MASK 0x01A00F
#define AcdxDrvrStatusControlRD_BIT_MASK 0x7BB00F

/* ***************************************************** */
/* Define the IOCTLs                                     */

typedef enum {

   /* Standard IOCTL Commands for timing users and developers */

   AcdxDrvrGET_LOCK_KEY,           /* Get driver lock key */
   AcdxDrvrLOCK,                   /* Lock the driver, read only */
   AcdxDrvrUNLOCK,                 /* Unlock the driver */

   AcdxDrvrSET_SW_DEBUG,           /* Set driver debug level */
   AcdxDrvrGET_SW_DEBUG,           /* Get driver debug level */

   AcdxDrvrRAW_READ,               /* Raw read  access to mapped card for debug */
   AcdxDrvrRAW_WRITE,              /* Raw write access to mapped card for debug */

   AcdxDrvrSET_FUNCTION_PARAMS,    /* Set sin-wave function parameters */
   AcdxDrvrGET_FUNCTION_PARAMS,    /* Get sin-wave function parameters */

   AcdrDrvrGET_VERSION,            /* Get FPGA Code version and driver version */
   AcdxDrvrGET_TEMPERATURE,        /* Get temperature in Centigrade */

   AcdxDrvrSET_STATUS_CONTROL,     /* Set module status control register */
   AcdxDrvrGET_STATUS_CONTROL,     /* Get module status control register */

   AcdxDrvrOPEN_USER_FPGA,         /* Opens users FPGA for write */
   AcdxDrvrWRITE_FPGA_CHUNK,       /* Write a chunk of FPGA bitstream */
   AcdxDrvrCLOSE_USER_FPGA,        /* Close FPGA after write */

   AcdxDrvrLAST_IOCTL

 } AcdxDrvrControlFunction;

#endif
