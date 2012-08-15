/* ********************************************************** */
/*                                                            */
/* ACDX (AC-Dipole X3-Servo) Driver code.                     */
/*                                                            */
/* A basic driver to load the user FPGA and access registers  */
/* in the BAR1 PCI address space.                             */
/*                                                            */
/* As it is extremley dangerous to access these register when */
/* there is high energy beam in the LHC, a simple locking     */
/* mechanism based on the callers PID is implemented.         */
/*                                                            */
/* July 2008 CERN AB/CO/HT Julian.Lewis@cern.ch               */
/*                                                            */
/* ********************************************************** */

#include <EmulateLynxOs.h>
#include <DrvrSpec.h>

/* These next defines are needed just here for the emulation */

#define file LynxFile
#define sel LynxSel
#define enable restore

#include <acdxdrvr.h>   /* Public driver interface            */
#include <acdxdrvrP.h>  /* Private driver structures          */

#ifndef COMPILE_TIME
#define COMPILE_TIME 0
#endif

/*========================================================================*/
/* Define prototypes and forward references for local driver routines.    */
/*========================================================================*/

static void LongCopy(volatile unsigned long *dst, volatile unsigned long *src, unsigned long size);

static void DebugIoctl(AcdxDrvrControlFunction cm, char *arg);

static void SetEndian();

static unsigned char  BitReverse(unsigned char x);

/* ========================= */
/* Driver entry points       */
/* ========================= */

irqreturn_t IntrHandler();
int         AcdxDrvrOpen();
int         AcdxDrvrClose();
int         AcdxDrvrRead();
int         AcdxDrvrWrite();
int         AcdxDrvrSelect();
char       *AcdxDrvrInstall();
int         AcdxDrvrUninstall();
int         AcdxDrvrIoctl();

#define EIEIO
#define SYNC

extern void disable_intr();
extern void enable_intr();

AcdxDrvrWorkingArea *Wa = NULL;

/*========================================================================*/
/* Cancel a timeout in a safe way                                         */
/*========================================================================*/

static void CancelTimeout(int *t) {

unsigned long ps = 0;
int v;

#ifdef EMULATE_LYNXOS_ON_LINUX
   ps = 0;
#endif

   disable(ps);
   {
      if ((v = *t)) {
	 *t = 0;
	 cancel_timeout(v);
      }
   }
   restore(ps);
}

/*========================================================================*/
/* Timer timeout during FPGA loading                                      */
/*========================================================================*/

static int ResetTimeout(AcdxDrvrModuleContext *mcon) {

   mcon->Timer = 0;
   ssignal(&(mcon->Semaphore));
   return 0;
}

/*========================================================================*/
/* Reverse bits in a char for the FPGA                                    */
/*========================================================================*/

static unsigned char  BitReverse(unsigned char x) {

   x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
   x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
   x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));

   return x;
}

/*========================================================================*/
/* 32 Bit long access for ACDX structure to/from hardware copy             */
/*========================================================================*/

static void LongCopy(volatile unsigned long *dst,
		     volatile unsigned long *src,
			      unsigned long size) {
int i, sb;

   sb = size/sizeof(unsigned long);
   for (i=0; i<sb; i++) dst[i] = src[i];
}

/* =========================================================================================== */
/* Print Debug message                                                                         */
/* =========================================================================================== */

char *ioctl_names[AcdxDrvrLAST_IOCTL] = {

   "GET_LOCK_KEY",
   "LOCK",
   "UNLOCK",
   "SET_SW_DEBUG",
   "GET_SW_DEBUG",
   "RAW_READ",
   "RAW_WRITE",
   "SET_FUNCTION_PARAMS",
   "GET_FUNCTION_PARAMS",
   "GET_VERSION",
   "GET_TEMPERATURE",
   "SET_STATUS_CONTROL",
   "GET_STATUS_CONTROL",
   "OPEN_USER_FPGA",
   "WRITE_FPGA_CHUNK",
   "CLOSE_USER_FPGA" };

/* =========================================================================================== */

static void DebugIoctl(AcdxDrvrControlFunction cm, char *arg) {
int i;
char *iocname;

   if (cm < AcdxDrvrLAST_IOCTL) {
      iocname = ioctl_names[(int) cm];
      if (arg) {
	 i = (int) *arg;
	 cprintf("AcdxDrvr: Debug: Called with IOCTL: %s Arg: %d\n",iocname, i);
      } else {
	 cprintf("AcdxDrvr: Debug: Called with IOCTL: %s Arg: NULL\n",iocname);
      }
      return;
   }
   cprintf("AcdxDrvr: Debug: ERROR: Illegal IOCTL number: %d\n",(int) cm);
}

/* ========================================================== */
/* This driver works on platforms that have different endians */
/* so this routine finds out which one we are.                */
/* ========================================================== */

static void SetEndian() {

static long int str[2] = { 0x41424344, 0x0 };

   if (strcmp ((char *) &str[0], "ABCD") == 0)
      Wa->Endian = AcdxDrvrEndianBIG;
   else
      Wa->Endian = AcdxDrvrEndianLITTLE;
}

/*=========================================================== */
/* Check Driver lock                                          */
/*=========================================================== */

static unsigned long TestLock(AcdxDrvrModuleContext *mcon,
			      AcdxDrvrClientContext *ccon) {

   if (mcon->LockKey) {
      if (mcon->LockKey != ccon->Pid) return 0;
   }
   return ccon->Pid;
}

/*=========================================================== */
/* Raw IO                                                     */
/* The correct PLX9030 endian settings must have been made    */
/*=========================================================== */

static int RawIo(AcdxDrvrModuleContext *mcon,
		 AcdxDrvrRawIoBlock    *riob,
		 unsigned long         flag,
		 unsigned long         debg) {

volatile unsigned long    *mmap; /* Module Memory map */
int                        rval; /* Return value */
unsigned long              ps;   /* Processor status word */
int                        i, j;
unsigned long             *uary;
char                      *iod;

   ps = 0;

   mmap = (unsigned long *) mcon->Map;
   uary = riob->UserArray;
   rval = OK;

   if (flag)  iod = "Write"; else iod = "Read";

   i = j = 0;
   if (!recoset()) {         /* Catch bus errors */

      for (i=0; i<riob->Size; i++) {
	 j = riob->Offset+i;
	 if (flag) {
	    mmap[j] = uary[i];
	    if (debg >= 2) cprintf("RawIo: %s: %d:0x%x\n",iod,j,(int) uary[i]);
	 } else {
	    uary[i] = mmap[j];
	    if (debg >= 2) cprintf("RawIo: %s: %d:0x%x\n",iod,j,(int) uary[i]);
	 }
	 EIEIO;
	 SYNC;
      }
   } else {
      disable(ps);

      kkprintf("AcdxDrvr: BUS-ERROR: Module:%d Addr:%x Dir:%s Data:%d\n",
	       (int) mcon->ModuleIndex+1,(int) &(mmap[j]),iod,(int) uary[i]);

      pseterr(ENXIO);        /* No such device or address */
      rval = SYSERR;
      enable(ps);
   }
   noreco();                 /* Remove local bus trap */
   riob->Size = i+1;
   return rval;
}

/* ========================================================== */
/* The ISR                                                    */
/* ========================================================== */

irqreturn_t IntrHandler(AcdxDrvrModuleContext *mcon) {
   return IRQ_NONE;
}

/*========================================================================*/
/* OPEN                                                                   */
/*========================================================================*/

int AcdxDrvrOpen(wa, dnm, flp)
AcdxDrvrWorkingArea * wa;        /* Working area */
int dnm;                         /* Device number */
struct file * flp; {             /* File pointer */

int cnum;                        /* Client number */
AcdxDrvrClientContext * ccon;    /* Client context */

   /* We allow one client per minor device, we use the minor device */
   /* number as an index into the client contexts array. */

   cnum = minor(flp->dev) -1;
   if ((cnum < 0) || (cnum >= AcdxDrvrCLIENT_CONTEXTS)) {

      /* EFAULT = "Bad address" */

      pseterr(EFAULT);
      return SYSERR;
   }
   ccon = &(wa->ClientContexts[cnum]);

   /* If already open by someone else, give a permission denied error */

   if (ccon->InUse) {

      /* This next error is normal */

      /* EBUSY = "Resource busy" */

      pseterr(EBUSY);           /* File descriptor already open */
      return SYSERR;
   }

   /* Initialize a client context */

   bzero((void *) ccon, sizeof(AcdxDrvrClientContext));
   ccon->ClientIndex = cnum;
   ccon->InUse       = 1;
   ccon->Pid         = getpid();
   return OK;
}

/*========================================================================*/
/* CLOSE                                                                  */
/*========================================================================*/

int AcdxDrvrClose(wa, flp)
AcdxDrvrWorkingArea * wa;    /* Working area */
struct file * flp; {         /* File pointer */

int cnum;                    /* Client number  */
AcdxDrvrClientContext *ccon; /* Client context */
AcdxDrvrModuleContext *mcon; /* Module context */

   /* We allow one client per minor device, we use the minor device */
   /* number as an index into the client contexts array.            */

   cnum = minor(flp->dev) -1;
   if ((cnum >= 0) && (cnum < AcdxDrvrCLIENT_CONTEXTS)) {

      ccon = &(Wa->ClientContexts[cnum]);

      if (ccon->DebugOn)
	 cprintf("AcdxDrvrClose: Close client: %d for Pid: %d\n",
		 (int) ccon->ClientIndex,
		 (int) ccon->Pid);

      if (ccon->InUse == 0)
	 cprintf("AcdxDrvrClose: Error bad client context: %d  Pid: %d Not in use\n",
		 (int) ccon->ClientIndex,
		 (int) ccon->Pid);

      mcon = &(wa->ModuleContexts[ccon->ModuleIndex]);   /* Default module selected */
      if (mcon->LockKey == ccon->Pid) mcon->LockKey = 0; /* Unlock driver */

      ccon->InUse = 0; /* Free the context */

      return(OK);

   } else {

      cprintf("AcdxDrvrClose: Error bad client context: %d\n",cnum);

      /* EFAULT = "Bad address" */

      pseterr(EFAULT);
      return SYSERR;
   }
}

/*========================================================================*/
/* READ                                                                   */
/*========================================================================*/

int AcdxDrvrRead(wa, flp, u_buf, cnt)
AcdxDrvrWorkingArea * wa;       /* Working area */
struct file * flp;              /* File pointer */
char * u_buf;                   /* Users buffer */
int cnt; {                      /* Byte count in buffer */

   pseterr(EINTR);
   return 0;
}

/*========================================================================*/
/* WRITE                                                                  */
/*========================================================================*/

int AcdxDrvrWrite(wa, flp, u_buf, cnt)
AcdxDrvrWorkingArea * wa;      /* Working area */
struct file * flp;             /* File pointer */
char * u_buf;                  /* Users buffer */
int cnt; {                     /* Byte count in buffer */

   pseterr(EINTR);
   return 0;
}

/*========================================================================*/
/* SELECT                                                                 */
/*========================================================================*/

int AcdxDrvrSelect(wa, flp, wch, ffs)
AcdxDrvrWorkingArea * wa;       /* Working area */
struct file * flp;              /* File pointer */
int wch;                        /* Read/Write direction */
struct sel * ffs; {             /* Selection structurs */

   pseterr(EACCES);
   return SYSERR;
}

/*========================================================================*/
/* INSTALL  LynxOS Version 4 with Full DRM support                        */
/* Assumes that all Acdx modules are brothers                              */
/*========================================================================*/

char * AcdxDrvrInstall(info)
AcdxDrvrInfoTable * info; {      /* Driver info table */

AcdxDrvrWorkingArea *wa;
drm_node_handle handle;
AcdxDrvrModuleContext *mcon;
unsigned long vadr;
int modix, mid, cc;

int cmd;

   /* Allocate the driver working area. */

   wa = (AcdxDrvrWorkingArea *) sysbrk(sizeof(AcdxDrvrWorkingArea));
   if (!wa) {
      cprintf("AcdxDrvrInstall: NOT ENOUGH MEMORY(WorkingArea)\n");
      pseterr(ENOMEM);
      return (char *) SYSERR;
   }
   bzero((void *) wa,sizeof(AcdxDrvrWorkingArea));  /* Clear working area */
   Wa = wa;                                        /* Global working area pointer */

   SetEndian(); /* Set Big/Little endian flag in driver working area */

   for (modix=0; modix<AcdxDrvrMODULE_CONTEXTS; modix++) {
      cc = drm_get_handle(PCI_BUSLAYER,VENDOR_ID,DEVICE_ID,&handle);
      if (cc) {
	 if (modix) break;
	 sysfree((void *) wa,sizeof(AcdxDrvrWorkingArea)); Wa = NULL;
	 cprintf("AcdxDrvrInstall: No ACDX device found: No hardware: Fatal\n");
	 pseterr(ENODEV);
	 return (char *) SYSERR;
      }

      mcon = &(wa->ModuleContexts[modix]);
      if (mcon->InUse == 0) {
	 drm_device_read(handle,PCI_RESID_DEVNO,0,0,&mid);
	 mcon->PciSlot     = mid;
	 mcon->ModuleIndex = modix;
	 mcon->Handle      = handle;
	 mcon->InUse       = 1;
	 mcon->DeviceId    = DEVICE_ID;
      }

      /* Ensure using memory mapped I/O */

      drm_device_read(handle,  PCI_RESID_REGS, 1, 0, &cmd);
      cmd |= 2;
      drm_device_write(handle, PCI_RESID_REGS, 1, 0, &cmd);

      vadr = (int) NULL;
      cc = drm_map_resource(handle,PCI_RESID_BAR0,&vadr);
      if ((cc) || (!vadr)) {
	 cprintf("AcdxDrvrInstall: Can't map memory (BAR0) for ACDX module: %d\n",modix+1);
	 return((char *) SYSERR);
      }
      mcon->Local = (unsigned long *) vadr;
      cprintf("AcdxDrvrInstall: BAR0 is mapped to address: 0x%08X\n",(int) vadr);

      vadr = (int) NULL;
      cc = drm_map_resource(handle,PCI_RESID_BAR1,&vadr);
      if ((cc) || (!vadr)) {
	 cprintf("AcdxDrvrInstall: Can't map memory (BAR1) for ACDX module: %d\n",modix+1);
	 return((char *) SYSERR);
      }
      mcon->Map = (AcdxDrvrMemoryMap *) vadr;
      cprintf("AcdxDrvrInstall: BAR1 is mapped to address: 0x%08X\n",(int) vadr);

      wa->Modules = modix+1;

      cprintf("AcdxDrvrInstall: Installed: ACDX ModuleNumber: %d In Slot: %d OK\n",
	      (int) modix+1, (int) mid);
   }

   cprintf("AcdxDrvrInstall: %d Modules installed: OK\n",(int) wa->Modules);
   return (char *) wa;
}

/*========================================================================*/
/* Uninstall                                                              */
/*========================================================================*/

int AcdxDrvrUninstall(wa)
AcdxDrvrWorkingArea * wa; {     /* Drivers working area pointer */

int i;
AcdxDrvrModuleContext *mcon;

   for (i=0; i<AcdxDrvrMODULE_CONTEXTS; i++) {
      mcon = &(wa->ModuleContexts[i]);
      if (mcon->InUse) {
	 drm_unmap_resource(mcon->Handle,PCI_RESID_BAR1);
	 drm_free_handle(mcon->Handle);
	 bzero((void *) mcon, sizeof(AcdxDrvrModuleContext));
      }
   }

   sysfree((void *) wa,sizeof(AcdxDrvrWorkingArea)); Wa = NULL;
   cprintf("AcdxDrvr: Driver: UnInstalled\n");
   return OK;
}

/*========================================================================*/
/* IOCTL                                                                  */
/*========================================================================*/

int AcdxDrvrIoctl(wa, flp, cm, arg)
AcdxDrvrWorkingArea * wa;       /* Working area */
struct file * flp;             /* File pointer */
AcdxDrvrControlFunction cm;     /* IOCTL command */
char * arg; {                  /* Data for the IOCTL */

volatile AcdxDrvrMemoryMap *mmap;
volatile unsigned long     *lmap;
AcdxDrvrClientContext      *ccon;
AcdxDrvrModuleContext      *mcon;
AcdxDrvrRawIoBlock         *riob;
AcdxDrvrFunctionParams     *funp;
AcdxDrvrFpgaChunk          *chnk;
AcdxDrvrVersion            *vers;
AcdxDrvrTemperature        *tepr;

int i, pid, maxt;

int cnum;                 /* Client number */
long lav, *lap;           /* Long Value pointed to by Arg */
unsigned short sav;       /* Short argument and for Jtag IO */
int rcnt, wcnt;           /* Readable, Writable byte counts at arg address */

   /* Check argument contains a valid address for reading or writing. */
   /* We can not allow bus errors to occur inside the driver due to   */
   /* the caller providing a garbage address in "arg". So if arg is   */
   /* not null set "rcnt" and "wcnt" to contain the byte counts which */
   /* can be read or written to without error. */

   if (arg != NULL) {
      rcnt = rbounds((int)arg);       /* Number of readable bytes without error */
      wcnt = wbounds((int)arg);       /* Number of writable bytes without error */
      if (rcnt < sizeof(long)) {      /* We at least need to read one long */
	 pid = getpid();
	 cprintf("AcdxDrvrIoctl:Illegal arg-pntr:0x%X ReadCnt:%d(%d) Pid:%d Cmd:%d\n",
		 (int) arg,rcnt,sizeof(long),pid,(int) cm);
	 pseterr(EINVAL);        /* Invalid argument */
	 return SYSERR;
      }
      lav = *((long *) arg);       /* Long argument value */
      lap =   (long *) arg ;       /* Long argument pointer */
   } else {
      rcnt = 0; wcnt = 0; lav = 0; lap = NULL; /* Null arg = zero read/write counts */
   }
   sav = lav;                      /* Short argument value */

   /* We allow one client per minor device, we use the minor device */
   /* number as an index into the client contexts array. */

   cnum = minor(flp->dev) -1;
   if ((cnum < 0) || (cnum >= AcdxDrvrCLIENT_CONTEXTS)) {
      pseterr(ENODEV);          /* No such device */
      return SYSERR;
   }

   /* We can't control a file which is not open. */

   ccon = &(wa->ClientContexts[cnum]);
   if (ccon->InUse == 0) {
      cprintf("AcdxDrvrIoctl: DEVICE %2d IS NOT OPEN\n",cnum+1);
      pseterr(EBADF);           /* Bad file number */
      return SYSERR;
   }

   /* Only the pid that opened the driver is allowed to call an ioctl */

#if 0
   pid = getpid();
   if (pid != ccon->Pid) {
      cprintf("AcdxDrvrIoctl: Spurious IOCTL call:%d by PID:%d for PID:%d on FD:%d\n",
	      (int) cm,
	      (int) pid,
	      (int) ccon->Pid,
	      (int) cnum);
      pseterr(EBADF);           /* Bad file number */
      return SYSERR;
   }
#endif

   /* Set up some useful module pointers */

   mcon = &(wa->ModuleContexts[ccon->ModuleIndex]); /* Default module selected */
   mmap = (AcdxDrvrMemoryMap *) mcon->Map;

   if (mcon->InUse == 0) {
      cprintf("AcdxDrvrIoctl: No hardware installed\n");
      pseterr(ENODEV);          /* No such device */
      return SYSERR;
   }

   if (ccon->DebugOn) DebugIoctl(cm,arg);   /* Print debug message */

   /*************************************/
   /* Decode callers command and do it. */
   /*************************************/

   switch (cm) {

      case AcdxDrvrGET_LOCK_KEY:           /* Get driver lock key */
	 if (lap) {
	    *lap = mcon->LockKey;
	    return OK;
	 }
      break;

      case AcdxDrvrLOCK:                   /* Lock the driver, read only */
	 if (mcon->LockKey == 0)
	    mcon->LockKey = ccon->Pid;
	 if (mcon->LockKey == ccon->Pid)
	    return OK;
      break;

      case AcdxDrvrUNLOCK:                 /* Unlock the driver */
	 if (lap) {
	    if (lav == AcdxDrvrFORCE_UNLOCK) {
	       mcon->LockKey = 0;
	       return OK;
	    }
	 }
	 if (TestLock(mcon,ccon) == 0) {
	    pseterr(ENOLCK);
	    return SYSERR;
	 }  else {
	    mcon->LockKey = 0;
	    return OK;
	 }
      break;

      case AcdxDrvrSET_SW_DEBUG:           /* Set driver debug level */
	 if (lap) {
	    if (lav) ccon->DebugOn = lav;
	    else     ccon->DebugOn = 0;
	    return OK;
	 }
      break;

      case AcdxDrvrGET_SW_DEBUG:           /* Get driver debug level */
	 if (lap) {
	    *lap = ccon->DebugOn;
	    return OK;
	 }
      break;

      case AcdxDrvrRAW_READ:               /* Raw read  access to card for debug */
	 if (wcnt >= sizeof(AcdxDrvrRawIoBlock)) {
	    riob = (AcdxDrvrRawIoBlock *) arg;
	    if ((riob->UserArray != NULL)
	    &&  (wcnt > riob->Size * sizeof(unsigned long))) {
	       return RawIo(mcon,riob,0,ccon->DebugOn);
	    }
	 }
      break;

      case AcdxDrvrRAW_WRITE:              /* Raw write access to card for debug */
	 if (rcnt >= sizeof(AcdxDrvrRawIoBlock)) {
	    riob = (AcdxDrvrRawIoBlock *) arg;
	    if ((riob->UserArray != NULL)
	    &&  (rcnt > riob->Size * sizeof(unsigned long))) {
	       if (TestLock(mcon,ccon) == 0) {
		  pseterr(ENOLCK);
		  return SYSERR;
	       }
	       return RawIo(mcon,riob,1,ccon->DebugOn);
	    }
	 }
      break;

      case AcdxDrvrSET_FUNCTION_PARAMS:    /* Set sin-wave function parameters */
	 if (rcnt >= sizeof(AcdxDrvrFunctionParams)) {
	    funp = (AcdxDrvrFunctionParams *) arg;

	    if ((funp->SinFrequency < AcdxDrvrMIN_FREQ)
	    ||  (funp->SinFrequency > AcdxDrvrMAX_FREQ)) break;

	    if ((funp->SinAmplitude < AcdxDrvrMIN_AMPL)
	    ||  (funp->SinAmplitude > AcdxDrvrMAX_AMPL)) break;

	    if ((funp->SinRiseTime  < AcdxDrvrMIN_TIME)
	    ||  (funp->SinFlTopTime < AcdxDrvrMIN_TIME)
	    ||  (funp->SinFallTime  < AcdxDrvrMIN_TIME)) break;

	    maxt = (funp->SinRiseTime + funp->SinFlTopTime + funp->SinFallTime) / 1000;
	    if (maxt > AcdxDrvrDURATION) break;

	    if (TestLock(mcon,ccon) == 0) {
	       pseterr(ENOLCK);
	       return SYSERR;
	    }
	    LongCopy((volatile unsigned long *) &(mcon->FunParms),
		     (volatile unsigned long *) funp,
		     sizeof(AcdxDrvrFunctionParams));

	    mmap->Regs.SinFrequency = funp->SinFrequency;   /* Hertz */
	    mmap->Regs.SinAmplitude = funp->SinAmplitude;   /* Milli volts */
	    mmap->Regs.SinRiseTime  = funp->SinRiseTime;    /* Milli seconds */
	    mmap->Regs.SinFlTopTime = funp->SinFlTopTime;   /* Milli seconds */
	    mmap->Regs.SinFallTime  = funp->SinFallTime;    /* Milli seconds */
	    return OK;
	 }
      break;

      case AcdxDrvrGET_FUNCTION_PARAMS:    /* Get sin-wave function parameters */
	 if (wcnt >= sizeof(AcdxDrvrFunctionParams)) {
	    funp = (AcdxDrvrFunctionParams *) arg;
	    funp->SinFrequency =  mmap->Regs.SinFrequency;  /* Hertz */
	    funp->SinAmplitude =  mmap->Regs.SinAmplitude;  /* Milli volts */
	    funp->SinRiseTime  =  mmap->Regs.SinRiseTime;   /* Milli seconds */
	    funp->SinFlTopTime =  mmap->Regs.SinFlTopTime;  /* Milli seconds */
	    funp->SinFallTime  =  mmap->Regs.SinFallTime;   /* Milli seconds */
	    return OK;
	 }
      break;

      case AcdrDrvrGET_VERSION:            /* Get FPGA Code version and driver version */
	 if (rcnt >= sizeof(AcdxDrvrVersion)) {
	    vers = (AcdxDrvrVersion *) arg;
	    vers->DrvrVersion = COMPILE_TIME;
	    vers->FpgaVersion = mmap->Regs.FPGAVersion;
	    return OK;
	 }
      break;

      case AcdxDrvrGET_TEMPERATURE:        /* Get temperature in Centigrade */
	 if (rcnt >= sizeof(AcdxDrvrTemperature)) {
	    tepr = (AcdxDrvrTemperature *) arg;
	    tepr->Temperature = mmap->Regs.Temperature;
	    tepr->TempWarning = mmap->Regs.TempWarning;
	    tepr->TempFailure = mmap->Regs.TempFailure;
	    return OK;
	 }
      break;

      case AcdxDrvrSET_STATUS_CONTROL:     /* Set module status control register */

	 if (TestLock(mcon,ccon) == 0) {
	    pseterr(ENOLCK);
	    return SYSERR;
	 }
	 if (lap) {
	    mmap->Regs.StatusControl = lav;
	    return OK;
	 }
      break;

      case AcdxDrvrGET_STATUS_CONTROL:     /* Get module status control register */
	 if (lap) {
	    *lap = mmap->Regs.StatusControl;
	    return OK;
	 }
      break;

      case AcdxDrvrOPEN_USER_FPGA:         /* Opens users FPGA for write */

	 if (TestLock(mcon,ccon) == 0) {
	    pseterr(ENOLCK);
	    return SYSERR;
	 }

	 lmap = mcon->Local;
	 lmap[FpgaCtrl] = 0;

	 maxt = 0;
	 while ( ((lmap[FpgaCtrl] & AcdxDrvrCtrlINIT))
	 &&      (maxt < 2000) ) {
	    sreset(&(mcon->Semaphore));
	    mcon->Timer = timeout(ResetTimeout, (char *) mcon, 2);
	    swait(&(mcon->Semaphore), SEM_SIGABORT);
	    maxt += 10;
	 }
	 if (mcon->Timer) CancelTimeout(&mcon->Timer);

	 if (maxt >= 2000) {
	    pseterr(ETIME);
	    return SYSERR;
	 }

	 lmap[FpgaCtrl] = AcdxDrvrCtrlPROG;

	 maxt = 0;
	 while ( ((lmap[FpgaCtrl] & AcdxDrvrCtrlINIT) == 0)
	 &&      (maxt < 2000) ) {
	    sreset(&(mcon->Semaphore));
	    mcon->Timer = timeout(ResetTimeout, (char *) mcon, 2);
	    swait(&(mcon->Semaphore), SEM_SIGABORT);
	    maxt += 10;
	 }
	 if (mcon->Timer) CancelTimeout(&mcon->Timer);

	 if (maxt >= 2000) {
	    pseterr(ETIME);
	    return SYSERR;
	 }

	 sreset(&(mcon->Semaphore));
	 mcon->Timer = timeout(ResetTimeout, (char *) mcon, 1);
	 swait(&(mcon->Semaphore), SEM_SIGABORT);

	 if (mcon->Timer) CancelTimeout(&mcon->Timer);

	 if ((lmap[FpgaCtrl] & AcdxDrvrCtrlDONE) == 0) {
	    mcon->FpgaOpen = 1;
	    return OK;
	 }
      break;

      case AcdxDrvrWRITE_FPGA_CHUNK:       /* Write a chunk of FPGA bitstream */
	 if (TestLock(mcon,ccon) == 0) {
	    pseterr(ENOLCK);
	    return SYSERR;
	 }
	 if (mcon->FpgaOpen) {
	    lmap = mcon->Local;
	    chnk = (AcdxDrvrFpgaChunk *) arg;
	    if (chnk->Size > AcdxDrvrCHUNK_SIZE) break;
	    for (i=0; i<chnk->Size; i++) {
	       lmap[FpgaData] = (unsigned long) BitReverse(chnk->Chunk[i]);
	    }
	    if ((lmap[FpgaCtrl] & AcdxDrvrCtrlINIT) == 0) {
	       pseterr(EPROTO);
	       return SYSERR;
	    }
	    return OK;
	 }
      break;

      case AcdxDrvrCLOSE_USER_FPGA:        /* Close FPGA after write */
	 if (TestLock(mcon,ccon) == 0) {
	    pseterr(ENOLCK);
	    return SYSERR;
	 }
	 lmap = mcon->Local;
	 sreset(&(mcon->Semaphore));
	 mcon->Timer = timeout(ResetTimeout, (char *) mcon, 2000);
	 swait(&(mcon->Semaphore), SEM_SIGABORT);
	 if (mcon->Timer) CancelTimeout(&mcon->Timer);
	 mcon->FpgaOpen = 0;
	 if (lmap[FpgaCtrl] & AcdxDrvrCtrlDONE) return OK;
	 pseterr(EPROTO);
	 return SYSERR;
      break;

      default: break;
   }

   /* EINVAL = "Invalid argument" */

   pseterr(EINVAL);                           /* Caller error */
   return SYSERR;
}

/*************************************************************/
/* Dynamic loading information for driver install routine.   */
/*************************************************************/

struct dldd entry_points = {
   AcdxDrvrOpen, AcdxDrvrClose,
   AcdxDrvrRead, AcdxDrvrWrite,
   AcdxDrvrSelect,
   AcdxDrvrIoctl,
   AcdxDrvrInstall, AcdxDrvrUninstall
};
