/* ==================================================================== */
/* Implement the timing library over the CTR familly of timing receiver */
/* cards, PMC, PCI, and VME.                                            */
/* Julian Lewis May 2004                                                */
/* ==================================================================== */

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sched.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <tgm/tgm.h>
#include <tgm/TgmDefP.h>
#include <tgv/tgv.h>
#include <TimLib.h>

/**
 * When sending messages to the TGM daemon timlib has
 * to yeild the CPU to let the daemon run. This was
 * done using sleep(1) which was far too long and was
 * causing RT problems on multiple attaches. No timlib
 * yeild the CPU for 10us.
 */

void yeild(void)
{
	unsigned int cc, usecs = 10;
	cc = usleep(usecs);
	return;
}

#include "./ctr/CtrLib.c"
#if !defined(__i386__) && !defined(__x86_64__)
/* No support any more for TG8 on MEN A20 and on PC */
#include "./tg8cps/Tg8CpsLib.c"
#include "./tg8sps/Tg8SpsLib.c"
#endif
#include "./netwrk/NetWrkLib.c"

typedef struct {
   TimLibError   (*Connect)();
   TimLibError   (*DisConnect)();
   TimLibError   (*Queue)();
   TimLibError   (*Wait)();
   TimLibError   (*Set)();
   TimLibError   (*Get)();
   TimLibError   (*Simulate)();
   TimLibError   (*RemoteControl)();
   TimLibError   (*GetRemote)();
   TimLibError   (*GetTime)();
   TimLibError   (*GetTelegram)();
   unsigned long (*GetInstalledModuleCount)();
   TimLibError   (*GetPtimObject)();
   TimLibError   (*GetCtimObject)();
   TimLibError   (*GetHandle)();
   TimLibError   (*CreateCtimObject)();
   TimLibError   (*CreatePtimObject)();
   TimLibError   (*GetCableId)();

   TimLibStatus  (*GetStatus)();
   TimLibError   (*GetModuleVersion)();

   TimLibError   (*GetAllPtimObjects)();
   TimLibError   (*GetAllCtimObjects)();
   unsigned long (*GetQueueSize)();

   char*         (*GetSpecificInfo)();

   TimLibError   (*FdConnect)();
   TimLibError   (*FdQueue)();
   TimLibError   (*FdWait)();

   TimLibError   (*GetModuleStats)();
   TimLibError   (*SetPllLocking)();

   TimLibError   (*ConnectPayload)();

 } TimLibRoutines;

static TimLibRoutines routines;

/* ==================================================================== */
/* Local routine to initialize one real device                          */

static TimLibError InitDevice(int fd, TimLibDevice device) {

   switch (device) {

      case TimLibDevice_CTR:
	 routines.Connect                 = CtrLibConnect;
	 routines.DisConnect              = CtrLibDisConnect;
	 routines.Queue                   = CtrLibQueue;
	 routines.Wait                    = CtrLibWait;
	 routines.Set                     = CtrLibSet;
	 routines.Get                     = CtrLibGet;
	 routines.Simulate                = CtrLibSimulate;
	 routines.RemoteControl           = CtrLibRemoteControl;
	 routines.GetRemote               = CtrLibGetRemote;
	 routines.GetTime                 = CtrLibGetTime;
	 routines.GetTelegram             = CtrLibGetTelegram;
	 routines.GetInstalledModuleCount = CtrLibGetInstalledModuleCount;
	 routines.GetPtimObject           = CtrLibGetPtimObject;
	 routines.GetCtimObject           = CtrLibGetCtimObject;
	 routines.GetHandle               = CtrLibGetHandle;
	 routines.CreateCtimObject        = CtrLibCreateCtimObject;
	 routines.CreatePtimObject        = CtrLibCreatePtimObject;
	 routines.GetCableId              = CtrLibGetCableId;

	 routines.GetStatus               = CtrLibGetStatus;
	 routines.GetModuleVersion        = CtrLibGetModuleVersion;
	 routines.GetAllPtimObjects       = CtrLibGetAllPtimObjects;
	 routines.GetAllCtimObjects       = CtrLibGetAllCtimObjects;
	 routines.GetQueueSize            = CtrLibGetQueueSize;
	 routines.GetSpecificInfo         = CtrLibGetSpecificInfo;

	 routines.FdConnect               = CtrLibFdConnect;
	 routines.FdQueue                 = CtrLibFdQueue;
	 routines.FdWait                  = CtrLibFdWait;

	 routines.GetModuleStats          = CtrLibGetModuleStats;
	 routines.SetPllLocking           = CtrLibSetPllLocking;

	 routines.ConnectPayload          = CtrLibConnectPayload;

	 if (fd) return (TimLibError) CtrLibFdInitialize(device);
		 return               CtrLibInitialize(device);

#if !defined(__i386__) && !defined(__x86_64__)
      case TimLibDevice_TG8_CPS:
	 routines.Connect                 = Tg8CpsLibConnect;
	 routines.DisConnect              = Tg8CpsLibDisConnect;
	 routines.Queue                   = Tg8CpsLibQueue;
	 routines.Wait                    = Tg8CpsLibWait;
	 routines.Set                     = Tg8CpsLibSet;
	 routines.Get                     = Tg8CpsLibGet;
	 routines.Simulate                = Tg8CpsLibSimulate;
	 routines.RemoteControl           = Tg8CpsLibRemoteControl;
	 routines.GetRemote               = Tg8CpsLibGetRemote;
	 routines.GetTime                 = Tg8CpsLibGetTime;
	 routines.GetTelegram             = Tg8CpsLibGetTelegram;
	 routines.GetInstalledModuleCount = Tg8CpsLibGetInstalledModuleCount;
	 routines.GetPtimObject           = Tg8CpsLibGetPtimObject;
	 routines.GetCtimObject           = Tg8CpsLibGetCtimObject;
	 routines.GetHandle               = Tg8CpsLibGetHandle;
	 routines.CreateCtimObject        = Tg8CpsLibCreateCtimObject;
	 routines.CreatePtimObject        = Tg8CpsLibCreatePtimObject;
	 routines.GetCableId              = Tg8CpsLibGetCableId;

	 routines.GetStatus               = Tg8CpsLibGetStatus;
	 routines.GetModuleVersion        = Tg8CpsLibGetModuleVersion;
	 routines.GetAllPtimObjects       = Tg8CpsLibGetAllPtimObjects;
	 routines.GetAllCtimObjects       = Tg8CpsLibGetAllCtimObjects;
	 routines.GetQueueSize            = Tg8CpsLibGetQueueSize;
	 routines.GetSpecificInfo         = Tg8CpsLibGetSpecificInfo;

	 routines.FdConnect               = Tg8CpsLibFdConnect;
	 routines.FdQueue                 = Tg8CpsLibFdQueue;
	 routines.FdWait                  = Tg8CpsLibFdWait;

	 routines.GetModuleStats          = Tg8CpsLibGetModuleStats;
	 routines.SetPllLocking           = Tg8CpsLibSetPllLocking;

	 routines.ConnectPayload          = Tg8CpsLibConnectPayload;

	 if (fd) return (TimLibError) Tg8CpsLibFdInitialize(device);
		 return               Tg8CpsLibInitialize(device);

      case TimLibDevice_TG8_SPS:
	 routines.Connect                 = Tg8SpsLibConnect;
	 routines.DisConnect              = Tg8SpsLibDisConnect;
	 routines.Queue                   = Tg8SpsLibQueue;
	 routines.Wait                    = Tg8SpsLibWait;
	 routines.Set                     = Tg8SpsLibSet;
	 routines.Get                     = Tg8SpsLibGet;
	 routines.Simulate                = Tg8SpsLibSimulate;
	 routines.RemoteControl           = Tg8SpsLibRemoteControl;
	 routines.GetRemote               = Tg8SpsLibGetRemote;
	 routines.GetTime                 = Tg8SpsLibGetTime;
	 routines.GetTelegram             = Tg8SpsLibGetTelegram;
	 routines.GetInstalledModuleCount = Tg8SpsLibGetInstalledModuleCount;
	 routines.GetPtimObject           = Tg8SpsLibGetPtimObject;
	 routines.GetCtimObject           = Tg8SpsLibGetCtimObject;
	 routines.GetHandle               = Tg8SpsLibGetHandle;
	 routines.CreateCtimObject        = Tg8SpsLibCreateCtimObject;
	 routines.CreatePtimObject        = Tg8SpsLibCreatePtimObject;
	 routines.GetCableId              = Tg8SpsLibGetCableId;

	 routines.GetStatus               = Tg8SpsLibGetStatus;
	 routines.GetModuleVersion        = Tg8SpsLibGetModuleVersion;
	 routines.GetAllPtimObjects       = Tg8SpsLibGetAllPtimObjects;
	 routines.GetAllCtimObjects       = Tg8SpsLibGetAllCtimObjects;
	 routines.GetQueueSize            = Tg8SpsLibGetQueueSize;
	 routines.GetSpecificInfo         = Tg8SpsLibGetSpecificInfo;

	 routines.FdConnect               = Tg8SpsLibFdConnect;
	 routines.FdQueue                 = Tg8SpsLibFdQueue;
	 routines.FdWait                  = Tg8SpsLibFdWait;

	 routines.GetModuleStats          = Tg8SpsLibGetModuleStats;
	 routines.SetPllLocking           = Tg8SpsLibSetPllLocking;

	 routines.ConnectPayload          = Tg8SpsLibConnectPayload;

	 if (fd) return (TimLibError) Tg8SpsLibFdInitialize(device);
		 return               Tg8SpsLibInitialize(device);
#endif

      case TimLibDevice_NETWORK:
	 routines.Connect                 = NetWrkLibConnect;
	 routines.DisConnect              = NetWrkLibDisConnect;
	 routines.Queue                   = NetWrkLibQueue;
	 routines.Wait                    = NetWrkLibWait;
	 routines.Set                     = NetWrkLibSet;
	 routines.Get                     = NetWrkLibGet;
	 routines.Simulate                = NetWrkLibSimulate;
	 routines.RemoteControl           = NetWrkLibRemoteControl;
	 routines.GetRemote               = NetWrkLibGetRemote;
	 routines.GetTime                 = NetWrkLibGetTime;
	 routines.GetTelegram             = NetWrkLibGetTelegram;
	 routines.GetInstalledModuleCount = NetWrkLibGetInstalledModuleCount;
	 routines.GetPtimObject           = NetWrkLibGetPtimObject;
	 routines.GetCtimObject           = NetWrkLibGetCtimObject;
	 routines.GetHandle               = NetWrkLibGetHandle;
	 routines.CreateCtimObject        = NetWrkLibCreateCtimObject;
	 routines.CreatePtimObject        = NetWrkLibCreatePtimObject;
	 routines.GetCableId              = NetWrkLibGetCableId;

	 routines.GetStatus               = NetWrkLibGetStatus;
	 routines.GetModuleVersion        = NetWrkLibGetModuleVersion;
	 routines.GetAllPtimObjects       = NetWrkLibGetAllPtimObjects;
	 routines.GetAllCtimObjects       = NetWrkLibGetAllCtimObjects;
	 routines.GetQueueSize            = NetWrkLibGetQueueSize;
	 routines.GetSpecificInfo         = NetWrkLibGetSpecificInfo;

	 routines.FdConnect               = NetWrkLibFdConnect;
	 routines.FdQueue                 = NetWrkLibFdQueue;
	 routines.FdWait                  = NetWrkLibFdWait;

	 routines.ConnectPayload          = NetWrkLibConnectPayload;

	 if (fd) return (TimLibError) NetWrkLibFdInitialize(device);
		 return               NetWrkLibInitialize(device);

      default: break;
   }
   return TimLibErrorINIT;
}

/* ==================================================================== */
/* Little routines to try to handle "0" module numbers in the best way. */

static unsigned long CblIds[TimLibMODULES];
static unsigned long CblVal = 0;

unsigned long GetCableIds() {

int i;

   if (CblVal == 0) {
      bzero((void *) CblIds, sizeof(unsigned long) * TimLibMODULES);
      CblVal = TimLibGetInstalledModuleCount();
      if (CblVal > TimLibMODULES) CblVal = TimLibMODULES;

      for (i=0; i<CblVal; i++) {
	 if (TimLibGetCableId(i+1,&CblIds[i]) != TimLibErrorSUCCESS) {
	    CblVal = 0;
	    break;
	 }
      }
   }
   return CblVal;
}

/* =============================================== */

unsigned long GetModuleForCtim(unsigned long ctim) {

int i;
unsigned long cblid;

   if (GetCableIds()) {
      cblid = TgvFirstCableIdForMember(ctim);
      while (cblid != TgvCABLE_ID_NONE) {
	 for (i=0; i<CblVal; i++) {
	    if (cblid == CblIds[i]) return i+1;
	 }
	 cblid = TgvNextCableIdForMember();
      }
   }
   return 0;
}

/* =============================================== */

unsigned long GetModuleForMch(TgmMachine mch) {

int i;
unsigned long cblid;

   if (GetCableIds()) {
      cblid = TgvFirstCableIdForMachine(TgvTgmToTgvMachine(mch));
      while (cblid != TgvCABLE_ID_NONE) {
	 for (i=0; i<CblVal; i++) {
	    if (cblid == CblIds[i]) return i+1;
	 }
	 cblid = TgvNextCableIdForMachine();
      }
   }
   return 0;
}

/* =============================================== */

unsigned long TimLibGetModuleForMch(TgmMachine mch) {
   return GetModuleForMch(mch);
}

/* ==================================================================== */
/* This routine could have been hidden from the user of the Timing lib, */
/* however, in some circumstances, the initialization can take several  */
/* minutes to complete. Hence I have decided to make an initialization  */
/* routine publicly available, and force users to call it.              */
/* This routine performs the following initialization functions...      */
/*    1) Opens a connection to the driver                               */
/*    2) Checks the Firmware/VHDL version against the latest revision   */
/*       Some EProms/FPGAs may need updating, this takes a while.       */
/*    3) Load all relavent CTIM and PTIM definitions if needed.         */
/* This is the CTR implementation, so device is irrelevant here.        */

/* These control behaviour of TimLibInit */

int timlib_debug     = 0;   /* 1/Print stuff, 2/Wipe memory */
int timlib_delay     = 0;   /* Input delay when not zero */
int timlib_enable    = 0;   /* Enable modules */
int timlib_jtag      = 0;   /* Check VHDL versions */
int timlib_ctims     = 300; /* For CPS Tg8, max size of CTIMS to allocate   */
int timlib_oby1_8    = 0;   /* CTRV Only: VME P2: 8 Output bytes for modules 1 to 8 */
int timlib_oby9_16   = 0;   /* CTRV Only: VME P2: 8 Output bytes for modules 9 to 16 */
int timlib_real_utc  = 0;   /* Frig UTC to PS TG8 if zero */
int timlib_allow_old = 0;   /* Don't allow old style SPS headers */

static int libinitialized = 0;

TimLibError TimLibInitialize(TimLibDevice device) { /* Initialize hardware/software */
TimLibDevice fdev, ldev, dev;
TimLibError err;

   if (libinitialized == 0) {

      if (device == TimLibDevice_ANY) {
	 fdev = TimLibDevice_CTR;
	 ldev = TimLibDevice_NETWORK;
      } else {
	 fdev = device;
	 ldev = device;
      }

      for (dev=fdev; dev<=ldev; dev++) {
	 err = InitDevice(0,dev);
	 if (err == TimLibErrorSUCCESS) {
	    libinitialized = 1;
	    return err;
	 }
      }
   } else return TimLibErrorSUCCESS;

   return TimLibErrorINIT;
}

/* ====== */
/* ==================================================================== */
/* For multi-threaded applications, each thread needs its own filedesc. */
/* This initialize will return a new file descriptor each time it is    */
/* called. Today the drivers support a maximum of 16 file descriptors   */
/* so the resource is limited and should be used economically; always   */
/* call "close(fd)" when the descriptor is no longer needed. In case of */
/* errors, a zero is returned.                                          */

int TimLibFdInitialize(TimLibDevice device) { /* Initialize hardware/software */
TimLibDevice fdev, ldev, dev;
int fd;

   if (device == TimLibDevice_ANY) {
      fdev = TimLibDevice_CTR;
      ldev = TimLibDevice_NETWORK;
   } else {
      fdev = device;
      ldev = device;
   }

   for (dev=fdev; dev<=ldev; dev++) {
      fd = (int) InitDevice(1,dev);
      if (fd) {
	 libinitialized = 1;
	 return fd;
      }
   }
   return 0;
}

/* ==================================================================== */
/* Get the version, returns a string containing date time of compiled.  */

char *TimLibGetVersion() {
static char version[32];

   sprintf(version,"%s %s",__DATE__,__TIME__);
   return version;
}

/* ==================================================================== */
/* Get VHDL/Firmware version of all modules, and the correct version.   */

TimLibError TimLibGetModuleVersion(TimLibModuleVersion *tver) {

   return routines.GetModuleVersion(tver);
}

/* ==================================================================== */
/* Get the status of a module and its device type.                      */

TimLibStatus TimLibGetStatus(unsigned long module, TimLibDevice *dev) {

   if (libinitialized) return routines.GetStatus(module,dev);
   return 0;
}

/* ==================================================================== */
/* Connect to an interrupt. If you are connecting to either a CTIM      */
/* interrupt or to a hardware interrupt, you may need to specify on     */
/* which device the interrupt should be connected. This is achieved by  */
/* the module parameter. If the module is zero, the system will decide  */
/* which device to use, otherwise module contains a value between 1 and */
/* the number of installed timing receiver cards. For PTIM objects the  */
/* module parameter must be set to zero or the real module on which the */
/* PTIM object is implemented. On PTIM objects the module is implicit.  */

TimLibError TimLibConnect(TimLibClass   iclss,    /* Class of interrupt */
			  unsigned long equip,    /* Equipment or hardware mask */
			  unsigned long module) { /* For HARD or CTIM classes */

#define OLD_SPS_SSC_HEADER 0x20000000
#define OLD_SPS_HEADER 0x21000000
#define ANCIENT_SPS_HEADER 0x2F000000

unsigned long frame;
unsigned long header;
TimLibStatus  sts;
TimLibDevice  dev;

   if (libinitialized) {
      if (iclss == TimLibClassCTIM) {
	 frame = TgvGetFrameForMember(equip);
	 if (frame != 0) {
	    header = frame & 0xFF000000;
	    if ( (timlib_allow_old == 0)
	    &&   ( (header == OLD_SPS_HEADER)
	    ||     (header == ANCIENT_SPS_HEADER)
	    ||     (header == OLD_SPS_SSC_HEADER) ) ) {

	       /* Can't connect to old style SPS events */

	       fprintf(stderr,"ERROR: Connecting to a deprecated SPS event: CTIM:%d Frame:0x%08X\n",
		       (int) equip, (int) frame);
	       return TimLibErrorCONNECT;
	    }
	 }
	 if (module == 0) module = GetModuleForCtim(equip);
      }
      if (iclss != TimLibClassHARDWARE) {
	 if (module) {
	    sts = TimLibGetStatus(module,&dev);
	    if ((sts & TimLibStatusENABLED) == 0) return TimLibErrorNOT_ENAB;
	 }
      }
      return routines.Connect(iclss,equip,module);
   }
   return TimLibErrorINIT;
}

/* ==================================================================== */
/* Connect to a CTIM with specified payload                             */

TimLibError TimLibConnectPayload(unsigned long ctim,        /* The CTIM ID you want to connect to */
				 unsigned long payload,     /* The 16 bit payload in a long */
				 unsigned long module) {    /* The module, or zero means don't care */

   if (libinitialized) return routines.ConnectPayload(ctim,payload,module);
   return TimLibErrorINIT;
}

/* ==================================================================== */
/* Get the module that best suits a given CTIM based on the cable ID    */

unsigned long TimLibGetModuleForCtim(unsigned long ctim) {
   return GetModuleForCtim(ctim);
}

/* ==================================================================== */
/* Disconnect from an interrupt                                         */

TimLibError TimLibDisConnect(TimLibClass   iclss,    /* Class of interrupt */
			     unsigned long equip,    /* Equipment or hardware mask */
			     unsigned long module) { /* For HARD or CTIM classes */

   if (libinitialized) return routines.DisConnect(iclss,equip,module);
   return TimLibErrorINIT;
}

/* ==================================================================== */
/* Set queueing On or Off, and the time out value in micro seconds.     */
/* A timeout value of zero means no time out, you wait for ever.        */

TimLibError TimLibQueue(unsigned long qflag,    /* 0=>Queue, 1=>NoQueue  */
			unsigned long tmout) {  /* 0=>No time outs       */

   if (libinitialized) return routines.Queue(qflag,tmout);
   return TimLibErrorINIT;
}

/* ==================================================================== */
/* To know if a call to wait will block, this call returns the Queue    */
/* size. If the size iz greater than zero a call to wait will not block */
/* and return without waiting. If the qflag is set to NoQueue, zero is  */
/* allways returned and all calls to wait will block.                   */

unsigned long TimLibGetQueueSize() {

   if (libinitialized) return routines.GetQueueSize();
   return TimLibErrorINIT;
}

/* ==================================================================== */
/* Wait for an interrupt. The parameters are all returned from the call */
/* so you can know which interrupt it was that came back. Note, when    */
/* waiting for a hardware interrupt from either CTIM or from a counter, */
/* it is the CTIM or PTIM object that caused the interrupt returned.    */
/* The telegram will have been read already by the high prioity task    */
/* get_tgm_ctr/tg8, be aware of the race condition here, hence payload. */
/* This routine is a blocking call, it waits for interrupt or timeout.  */
/* Any NULL argument  is permitted, and no value will be returned.      */

/* Arguments:                                                           */
/*    iclss:   The class of the interrupt CTIM, PTIM, or hardware       */
/*    equip:   The PTIM, CTIM equipment, or hardware mask               */
/*    plnum:   If class is PTIM this is the PLS line number             */
/*    source:  The hardware source of the interrupt                     */
/*    onzero:  The time of the interrupt                                */
/*    trigger: The arrival time of the event that triggered the action  */
/*    start:   The time the start of the counter occured                */
/*    ctim:    The CTIM equipment number of the triggering event        */
/*    payload: The payload of the triggering event                      */
/*    module:  The module number 1..n of the timing receiver card       */
/*    missed:  The number of missed events since the last wait          */
/*    qsize:   The number of remaining interrupts on the queue          */
/*    mch:     The machine to which the interrupt belongs               */

TimLibError TimLibWait(TimLibClass    *iclss,   /* Class of interrupt */
		       unsigned long  *equip,   /* PTIM CTIM or hardware mask */
		       unsigned long  *plnum,   /* Ptim line number 1..n or 0 */
		       TimLibHardware *source,  /* Hardware source of interrupt */
		       TimLibTime     *onzero,  /* Time of interrupt/output */
		       TimLibTime     *trigger, /* Time of counters load */
		       TimLibTime     *start,   /* Time of counters start */
		       unsigned long  *ctim,    /* CTIM trigger equipment ID */
		       unsigned long  *payload, /* Payload of trigger event */
		       unsigned long  *module,  /* Module that interrupted */
		       unsigned long  *missed,  /* Number of missed interrupts */
		       unsigned long  *qsize,   /* Remaining interrupts on queue */
		       TgmMachine     *mch) {   /* Corresponding TgmMachine */


   if (libinitialized) {

      return routines.Wait(iclss,
			   equip,
			   plnum,
			   source,
			   onzero,
			   trigger,
			   start,
			   ctim,
			   payload,
			   module,
			   missed,
			   qsize,
			   mch);
   }
   return TimLibErrorINIT;
}

/* ==================================================================== */
/* Set the Ccv of a PTIM equipment. Note neither the counter number nor */
/* the trigger condition can be changed.                                */

TimLibError TimLibSet(unsigned long ptim,    /* PTIM to write to */
		      unsigned long plnum,   /* Ptim line number 1..n or 0 */
		      unsigned long grnum,   /* Tgm group number or Zero */
		      unsigned long grval,   /* Group value if num not zero */
		      TimLibCcvMask ccvm,    /* Which values to write */
		      TimLibCcv     *ccv) {  /* Current control value */

   if (libinitialized) return routines.Set(ptim,plnum,grnum,grval,ccvm,ccv);
   return TimLibErrorINIT;
}

/* ==================================================================== */
/* Get the Ccv of a PTIM equipment.                                     */

TimLibError TimLibGet(unsigned long ptim,
		      unsigned long plnum,   /* Ptim line number 1..n or 0 */
		      unsigned long grnum,
		      unsigned long grval,
		      TimLibCcvMask *ccvm,   /* Valid fields in ccv */
		      TimLibCcv     *ccv) {

   if (libinitialized) return routines.Get(ptim,plnum,grnum,grval,ccvm,ccv);
   return TimLibErrorINIT;
}

/* ==================================================================== */
/* By writing to the driver this call simulates an interrupt for the    */
/* connected clients. Also it can be used as a way of synchronizing     */
/* processes, this is especially important in Linux systems where the   */
/* schedular is not preemptive.                                         */

/* Arguments:                                                                     */
/*    iclss:   Class of interrupt to simulate, PTIM, CTIM or Hardware             */
/*    equip:   Equipment number for PTIM or CTIM, hardware mask for Hardware      */
/*    module:  When class is CTIM or Hardware, the module number is used          */
/*    machine: Telegram ID is used for PTIM interrupts if grnum is not zero       */
/*    grnum:   If zero, no telegram checking, else the PTIM triggers group number */
/*    grval:   The telegram group value for the PTIM trigger                      */

TimLibError TimLibSimulate(TimLibClass   iclss,
			   unsigned long equip,
			   unsigned long module,
			   TgmMachine    machine,
			   unsigned long grnum,
			   unsigned long grval) {

   if (libinitialized) return routines.Simulate(iclss,equip,module,machine,grnum,grval);
   return TimLibErrorINIT;
}

/* ==================================================================== */
/* Set a counter under full remote control (IE under DSC tasks control) */
/* This feature permits you to do what you like with counters even if   */
/* there is no timing cable attached. With this you can drive stepper   */
/* motors, wire scanners or whatever. No PTIM or CTIM is involved, the  */
/* configuration is loaded directly by the application. Note that when  */
/* the argument remflg is set to 1, the counter can not be written to   */
/* by incomming triggers so all PTIM objects using the counter stop     */
/* overwriting the counter configuration and are effectivley disabled.  */
/* Setting the remflg 0 permits PTIM triggers to write to the counter   */
/* configuration, the write block is removed. Also note that in some    */
/* cases it is useful to perform remote actions, such as remoteSTOP,    */
/* even if the remflg is set to zero. The remflg simply blocks PTIM     */
/* overwrites, the counter configuration can still be accessed !        */

TimLibError TimLibRemoteControl(unsigned long remflg, /* 0 = Normal, 1 = Remote */
				unsigned long module, /* Module or zero */
				unsigned long cntr,   /* 1..8 counter number */
				TimLibRemote  rcmd,   /* Command */
				TimLibCcvMask ccvm,   /* Fields to be set */
				TimLibCcv     *ccv) { /* Value to load in counter */

   if (libinitialized) return routines.RemoteControl(remflg,module,cntr,rcmd,ccvm,ccv);
   return TimLibErrorINIT;
}

/* ==================================================================== */
/* Get a counters remote configuration                                  */

TimLibError TimLibGetRemote(unsigned long module,
			    unsigned long cntr,
			    unsigned long *remflg,
			    TimLibCcvMask *ccvm,
			    TimLibCcv     *ccv) {

   if (libinitialized) return routines.GetRemote(module,cntr,remflg,ccvm,ccv);
   return TimLibErrorINIT;
}

/* ==================================================================== */
/* Read the instantaneous value of the time in UTC. The module parameter*/
/* can be set to zero in which case the system decideds which module to */
/* read the time from, otherwise it can be set to a value between 1 and */
/* the number of installed modules.                                     */

TimLibError TimLibGetTime(unsigned long module, /* Module number to read from */
			  TimLibTime    *utc) { /* Returned time value */


   if (libinitialized) return routines.GetTime(module,utc);
   return TimLibErrorINIT;
}

/* ==================================================================== */
/* Read a machines telegram from a timing receiver. The module can be   */
/* either zero, in which case the system decides which device to use,   */
/* or it can be explicitly set between 1 and the number of installed    */
/* modules. The telegram object returned has an opaque structure and    */
/* can only be decoded through the Tgm library routine .....            */

/* unsigned long grval = TgmGetGroupValueFromTelegram(TgmMachine    machine,   */
/*                                                    unsigned long grnum,     */
/*                                                    TgmTelegram   *telegram) */

/* WARNING: The only task that should call this routine will be, get_tgm_lib,  */
/* all other, LOWER PRIORITY tasks must NEVER call this routine, instead they  */
/* should call the telegram library directly like this ...                     */

/* WARNING: If the calling task is not get_tgm_lib, then calling this routine  */
/* will actually be re-routed and the equivalent TGM call will be made instead */

/* TgmTelegram telegram;                                                       */
/*                                                                             */
/* if (TgmGetTelegram(machine, index, offset, &telegram) == TgmSUCCESS) { ...  */
/*                                                                             */
/* For more information on this function see the Tgm library man pages.        */

int TimLibClient = 0;
static int attached = 0;

TimLibError TimLibGetTelegram(unsigned long module,
			      TgmMachine    machine,
			      TgmTelegram   *telegram) {

int msk;

   if (TimLibClient) {
      if (libinitialized) {
	 if (module == 0) module = GetModuleForMch(machine);
	 return routines.GetTelegram(module,machine,(TgmPTelegram *) telegram);
      }
      return TimLibErrorINIT;
   }

   msk = 1 << (int) machine;
   if ((msk & attached) == 0) {
      if (TgmAttach(machine,TgmTELEGRAM) == TgmSUCCESS) {
	 attached |= msk;
	 yeild();
      } else return TimLibErrorINIT;
   }

   if (TgmGetTelegram(machine,TgmCURRENT,0,telegram) == TgmSUCCESS)
      return TimLibErrorSUCCESS;
   return TimLibErrorIO;
}

/* ==================================================================== */
/* Convert a TimLibError into a string. The returned char pointer is    */
/* either NULL if the supplied error is out of range, or it points to a */
/* static string contained on the library routines heap. You must copy  */
/* the string if you need to be sure it is not overwritten. Obviously   */
/* this routine is therfore not thread safe, but you don't need to free */
/* allocated memory thus avoiding potential memory leaks.               */

char *TimLibErrorToString(TimLibError error) {

static char *estr[TimLibERRORS] = {
"No Error ",
"Invalid hardware setting for this module type ",
"Invalid start setting for this module type ",
"Invalid mode setting for this module type ",
"Invalid clock setting for this module type ",
"Invalid pulse width setting for this module type ",
"Invalid delay value setting for this module type ",
"That remote command is not available on this module type ",
"No module with that number is installed ",
"Invalid counter/channel number for this module",
"No such PTIM equipment number defined ",
"No such CTIM equipment number defined ",
"That operation has been blocked ",
"PPM is not supported on this device type",
"No such telegram ID ",
"No such group for that PTIM object",
"Library not initialized or get_tgm_tim not running",
"Can not establish a connection to the hardware, OPEN failed ",
"Can not connect to the specified interrupt ",
"There are no connections to wait for ",
"Timeout occured in wait ",
"The queue flag is set 1=off, however queueing is needed ",
"Gereral IO error, see errno for more details ",
"That feature is not implemented on this device",
"That equipment exists already",
"Out of resource space or memory",
"The module is not enabled",
"Couldn't find a cycle for that time stamp",
"Cycle string did not specify an actual cycle" };

static char result[TimLibErrorSTRING_SIZE];
char *cp;

   if ((error < 0) || (error >= TimLibERRORS)) cp = "No such error number";
   else                                        cp = estr[(int) error];
   bzero((void *) result, TimLibErrorSTRING_SIZE);
   strcpy(result,cp);
   return result;
}

/* ==================================================================== */
/* Lets you know how many installed modules there are on this host.     */

unsigned long TimLibGetInstalledModuleCount() {

   if (libinitialized) return routines.GetInstalledModuleCount();
   return 0;
}

/* ==================================================================== */
/* Get the description of a given PTIM equipment. The dimension returns */
/* the PPM dimension, counter and module are obvious.                   */

TimLibError TimLibGetPtimObject(unsigned long ptim, /* PTIM equipment number */
				unsigned long *module,
				unsigned long *counter,
				unsigned long *dimension) {

   if (libinitialized) return routines.GetPtimObject(ptim,module,counter,dimension);
   return TimLibErrorINIT;
}

/* ==================================================================== */
/* Get the list of all defined PTIM objects                             */

TimLibError TimLibGetAllPtimObjects(unsigned long *ptimlist,  /* List of ptim equipments */
				    unsigned long *psize,     /* Number of ptims in list */
				    unsigned long size) {     /* Max size of list */

   if (libinitialized) return routines.GetAllPtimObjects(ptimlist,psize,size);
   return TimLibErrorINIT;
}

/* ==================================================================== */
/* Get the list of all defined CTIM objects                             */

TimLibError TimLibGetAllCtimObjects(unsigned long *ctimlist,  /* List of ctim equipments */
				    unsigned long *csize,     /* Number of ctims in list */
				    unsigned long size) {     /* Max size of list */

   if (libinitialized) return routines.GetAllCtimObjects(ctimlist,csize,size);
   return TimLibErrorINIT;
}

/* ==================================================================== */
/* Get the event code corresponding to a given CTIM equipment number.   */

TimLibError TimLibGetCtimObject(unsigned long ctim, /* CTIM equipment number */
				unsigned long *eventcode) {

   if (libinitialized) return routines.GetCtimObject(ctim,eventcode);
   return TimLibErrorINIT;
}

/* ==================================================================== */
/* In some cases when running a GUI under Linux, say, a file handle to  */
/* put in a "select" is needed so that one can wait on multiple file    */
/* handles simultaneously. This routine returns such a handle suitable  */
/* to check for waiting interrupts. Do not read directly from it, but   */
/* call the wait routine. The queue flag must be on for this to work !! */

TimLibError TimLibGetHandle(int *fd) {

   if (libinitialized) return routines.GetHandle(fd);
   return TimLibErrorINIT;
}

/* ==================================================================== */
/* Create a new PTIM object, the CCV settings will be defaulted.        */


TimLibError TimLibCreatePtimObject(unsigned long ptim, /* PTIM equipment number */
				   unsigned long module,
				   unsigned long counter,
				   unsigned long dimension) {

   if (libinitialized) return routines.CreatePtimObject(ptim,module,counter,dimension);
   return TimLibErrorINIT;
}

/* ==================================================================== */
/* Create a new CTIM object. If a payload is to be used for this event  */
/* be sure to set the low 16-Bits to 0xFFFF                             */

TimLibError TimLibCreateCtimObject(unsigned long ctim, /* CTIM equipment number */
				   unsigned long eventcode) {

   if (libinitialized) return routines.CreateCtimObject(ctim,eventcode);
   return TimLibErrorINIT;
}

/* ==================================================================== */
/* Set the debug level                                                  */

void TimLibSetDebug(unsigned long level) { /* Zero means no debug */
   timlib_debug = level;
   if (timlib_debug) fprintf(stderr,"TimLibSetDebug:ON Level:%d\n",(int) timlib_debug);
}

/* ==================================================================== */
/* Get the cable identifier to which a given module is attached so that */
/* the correct module can be used to read telegrams. This function will */
/* be used by the program get_tgm_tim only; it is of no interest to the */
/* majority of clients because calls to ReadTelegram are diverted.      */

TimLibError TimLibGetCableId(unsigned long module,   /* The given module */
			     unsigned long *cable) { /* The cable ID */

   if (libinitialized) return routines.GetCableId(module,cable);
   return TimLibErrorINIT;
}

/* ==================================================================== */
/* Get telegram information from a time stamp.                          */
/* This routine searches the telegram history for a cycle in which your */
/* time stamp occured. It then returns the cycles time stamp in which   */
/* the time you gave occurd and its present and next tags.              */
/* Null pointers for cyclestamp, cytag and ncytag are allowed, and in   */
/* particular the routine runs faster when the tags are null.           */

TimLibError TimLibGetTgmInfo(TimLibTime    stamp,       /* Time you want telegram information about */
			     TimLibTime    *cyclestamp, /* Time of cycle for given stamp */
			     unsigned long *cytag,      /* Cycle tag */
			     unsigned long *ncytag) {   /* Next cycle tag */


TgmHistoryBuffer *his;
TgmMachine mch;
unsigned long secs, mlsc, nano;
int of, msk;

unsigned long gcytag, gncytag;

   msk = 1 << (int) stamp.Machine;
   if ((msk & attached) == 0) {
      if (TgmAttach(stamp.Machine,TgmTELEGRAM) == TgmSUCCESS) {
	 attached |= msk;
	 yeild();
      } else return TimLibErrorINIT;
   }

   for (of=0; of<BlkEVENT_HISTORY_SIZE; of++) {
      his = (TgmHistoryBuffer *) TgmGetEvent(of);
      if (his) {
	 mch = (TgmMachine) TgmHisGetMachine(his);
	 if (stamp.Machine == mch) {
	    secs = (unsigned long) TgmHisGetCycleTimeSec(his);
	    mlsc = (unsigned long) TgmHisGetCycleTimeMSc(his);
	    nano = mlsc * 1000000;
	    if (stamp.Second < secs) continue;
	    if (stamp.Second > secs) break;
	    if (stamp.Nano   < nano) continue;
	    break;
	 }
      } else return TimLibErrorNOT_FOUND;
   }
   if (of<BlkEVENT_HISTORY_SIZE) {
      if (cyclestamp) {
	 cyclestamp->Second  = secs;
	 cyclestamp->Nano    = nano;
	 cyclestamp->Machine = mch;
	 cyclestamp->CTrain  = 0;
      }
      if (cytag) {
	 gcytag= (unsigned long) TgmGetGroupNumber(mch,"CYTAG");
	 if (gcytag) *cytag  = (unsigned long) TgmHisGetTelegram(his,gcytag);
	 else        *cytag  = 0;
      }
      if (ncytag) {
	 gncytag = (unsigned long) TgmGetGroupNumber(mch,"NCYTAG");
	 if (gncytag) *ncytag = (unsigned long) TgmHisGetTelegram(his,gncytag);
	 else         *ncytag = 0;
      }
      return TimLibErrorSUCCESS;
   }
   return TimLibErrorNOT_FOUND;
}

/* ==================================================================== */
/* Get a group value from the telegram for given cycle stamp.           */

TimLibError TimLibGetGroupValueFromStamp(TimLibTime    stamp, /* Cycle stamp  */
					 unsigned long gn,    /* Group number */
					 unsigned long next,  /* Next flag    */
					 unsigned long *gv) { /* Group value  */

TgmHistoryBuffer his, res, *rp;
unsigned long msk, g, v;

   msk = 1 << (int) stamp.Machine;
   if ((msk & attached) == 0) {
      if (TgmAttach(stamp.Machine,TgmTELEGRAM) == TgmSUCCESS) {
	 attached |= msk;
	 yeild();
      } else return TimLibErrorINIT;
   }

   TgmHisClear(&his);
   TgmHisSetMachine(&his,stamp.Machine);
   TgmHisSetCycleTimeSec(&his,stamp.Second);
   TgmHisSetCycleTimeMSc(&his,stamp.Nano/1000000);

   rp = &res;
   if (TgmHisGetHistory(&his,1,&rp)) {
      if (gv) {
	 if (next) g = TgmGetNextGroupNumber(stamp.Machine,gn);
	 else      g = gn;
	 if (g == 0) return TimLibErrorGROUP;
	 v = TgmHisGetTelegram(rp,g);
	 if (gv) *gv = v;
      }
      return TimLibErrorSUCCESS;
   }
   return TimLibErrorNOT_FOUND;
}

/* ==================================================================== */
/* Convert a cycle Id string like CPS.USER.SFTPRO into a slot index.    */
/* No abbreviations are supported, seperators are dots. Strict Syntax.  */
/* The result is slix, the slot index ranging from [0..(GroupSize-1)].  */
/* No change is made to slix on error always check return before using. */

TimLibError TimLibStringToSlot(char          *cyid,     /* Cycle ID string */
			       unsigned long *slix) {   /* Resulting slot index */

TgmLineNameTable ltab;
char *cp, mch[TgmNAME_SIZE], grp[TgmNAME_SIZE], lnm[TgmNAME_SIZE];
int i, m;

   cp = cyid;
   if (cp == NULL) return TimLibErrorIO;

   i = 0;
   while ((*cp) && (*cp != '.'))
      mch[i++] = *cp++;
   mch[i] = 0; cp++;

   for (m=0; m<TgmMACHINES; m++) {
      if (strcmp(mch,TgmGetMachineName(m)) == 0) {
	 i=0;
	 while ((*cp) && (*cp != '.'))
	    grp[i++] = *cp++;
	 grp[i] = 0; cp++;
	 if (TgmGetLineNameTable(m,grp,&ltab) == TgmSUCCESS) {
	    i = 0;
	    while ((*cp) && (*cp != '.'))
	      lnm[i++] = *cp++;
	    lnm[i] = 0; cp++;
	    for (i=0; i<ltab.Size; i++) {
	       if (strcmp(lnm,ltab.Table[i].Name)==0) {
		  *slix = i;
		  return TimLibErrorSUCCESS;
	       }
	    }
	    return TimLibErrorSTRING;
	 }
	 return TimLibErrorSTRING;
      }
   }
   return TimLibErrorSTRING;
}

/* ==================================================================== */
/* Special IO routine for CTR only, Get CTR LEMO values                 */

TimLibError TimLibGetIoStatus(unsigned long module,
			      TimLibLemo *input) {

   return CtrLibGetIoStatus(module, input);
}

/* ==================================================================== */
/* Special IO routine for CTR only, Set Counter Output LEMO values      */

TimLibError TimLibSetOutputs(unsigned long module,
			     TimLibLemo output,     /* Output values */
			     TimLibLemo mask) {     /* Which ones you eant to set */

   return CtrLibSetOutputs(module, output, mask);
}

/* ==================================================================== */
/* Get specific information string                                      */
/* There is some very specific module dependent status information.     */
/* This routine returns a free format human readable string containing  */
/* specific status information that may help diagnosing problems for a  */
/* timing receiver module. A null pointer can be returned if the module */
/* is either dead or not installed.                                     */

char *TimLibGetSpecificInfo(unsigned long module) { /* The given module */

   if (libinitialized) return routines.GetSpecificInfo(module);
   return NULL;
}

/* ==================================================================== */
/* For multi-threaded applications, each thread needs its own filedesc. */
/* This is just like the usual connect except you must supply a fd that */
/* is open and was obtained from FdInitialize. Should you supply a fd   */
/* with a bad value the driver will check if it belongs to your process */
/* and if it dose it will use it. This may cause unpredictable results. */

TimLibError TimLibFdConnect(int         fd,       /* File descriptor */
			  TimLibClass   iclss,    /* Class of interrupt */
			  unsigned long equip,    /* Equipment or hardware mask */
			  unsigned long module) { /* For HARD or CTIM classes */

   return routines.FdConnect(fd,iclss,equip,module);
}

/* ==================================================================== */
/* For multi-threaded applications, each thread needs its own filedesc. */

TimLibError TimLibFdQueue(int         fd,        /* File descriptor */
			unsigned long qflag,     /* 0=>Queue, 1=>NoQueue  */
			unsigned long tmout) {   /* 0=>No time outs       */

   return routines.FdQueue(fd,qflag,tmout);
}

/* ==================================================================== */
/* For multi-threaded applications, each thread needs its own filedesc. */

TimLibError TimLibFdWait(int          fd,        /* File descriptor */
		       TimLibClass    *iclss,    /* Class of interrupt */
		       unsigned long  *equip,    /* PTIM CTIM or hardware mask */
		       unsigned long  *plnum,    /* Ptim line number 1..n or 0 */
		       TimLibHardware *source,   /* Hardware source of interrupt */
		       TimLibTime     *onzero,   /* Time of interrupt/output */
		       TimLibTime     *trigger,  /* Time of counters load */
		       TimLibTime     *start,    /* Time of counters start */
		       unsigned long  *ctim,     /* CTIM trigger equipment ID */
		       unsigned long  *payload,  /* Payload of trigger event */
		       unsigned long  *module,   /* Module that interrupted */
		       unsigned long  *missed,   /* Number of missed interrupts */
		       unsigned long  *qsize,    /* Remaining interrupts on queue */
		       TgmMachine     *mch) {    /* Corresponding TgmMachine */

   return routines.FdWait(fd,iclss,equip,plnum,source,onzero,trigger,start,
			  ctim,payload,module,missed,qsize,mch);
}

/* ==================================================================== */
/* Get the module statistics for the given module                       */

TimLibError TimLibGetModuleStats(unsigned long module,
				 TimLibModuleStats *stats) {

   return routines.GetModuleStats(module,stats);
}

/* ==================================================================== */
/* Control how the PLL locks after synchronization loss                 */

TimLibError TimLibSetPllLocking(unsigned long module,
				unsigned long lockflag) { /* 1=> Brutal, else Slow */

   return routines.SetPllLocking(module,lockflag);
}
