/*
 * ==================================================================== 
 * Implement the timing library over the SPS-TG8 timing receiver.  
 * Julian Lewis Nov 2004 
 * ==================================================================== 
 */

#include <sys/shm.h>		/* Shared memory */
#include <sys/ioctl.h>
#ifndef __68k__
#include <sys/select.h>
#endif
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "../TimLib.h"
#include <tgm/tgm.h>

extern int timlib_debug;    /* 1/Print stuff, 2/Wipe memory */
extern int timlib_delay;    /* Input delay when not zero */
extern int timlib_enable;   /* Enable modules */
extern int timlib_real_utc; /* Do not adjust the UTC time */
/*
 * General note: I can't include the SPS TG8 driver definitions in the same 
 * compilation unit because they clash with those of the CPS TG8 driver. So 
 * I will define what I need here using symbols starting with SpsTg8.  
 */

#ifndef UBCD
#define UBCD(bcd) ((bcd&0xF)+10*(bcd>>4))
#endif

#define SpsTg8DrvrMODULES  4
#define SpsTg8DrvrCLIENTS_PER_MODULE 8
#define SpsTg8DrvrGROUPS 24
#define SpsTg8DrvrACTIONS 256

typedef enum {
   SpsTg8DrvrDEBUG_ON,
   SpsTg8DrvrDEBUG_OFF,
   SpsTg8DrvrGET_DRI_VERSION,
   SpsTg8DrvrGET_FIRMWARE_VERSION,
   SpsTg8DrvrSET_TIME_OUT,
   SpsTg8DrvrSET_SSC_HEADER,
   SpsTg8DrvrENABLE_SYNC,
   SpsTg8DrvrDISABLE_SYNC,
   SpsTg8DrvrENABLE_MODULE,
   SpsTg8DrvrDISABLE_MODULE,
   SpsTg8DrvrRESET_MODULE,
   SpsTg8DrvrINSTALL_MODULE,
   SpsTg8DrvrGET_CONFIGURATION,
   SpsTg8DrvrSIMULATE_PULSE,
   SpsTg8DrvrUSER_TABLE,
   SpsTg8DrvrRECORDING_TABLE,
   SpsTg8DrvrINTERRUPT_TABLE,
   SpsTg8DrvrHISTORY_TABLE,
   SpsTg8DrvrCLOCK_TABLE,
   SpsTg8DrvrAUX_TABLE,
   SpsTg8DrvrSC_INFO,
   SpsTg8DrvrTELEGRAM,
   SpsTg8DrvrTRACE_FIRMWARE,
   SpsTg8DrvrDATE_TIME,
   SpsTg8DrvrGET_RAW_STATUS,
   SpsTg8DrvrGET_STATUS,
   SpsTg8DrvrGET_DPRAM,
   SpsTg8DrvrRELOAD_FIRMWARE,
   SpsTg8DrvrRELOAD_ACTIONS,
   SpsTg8DrvrON_CLOSE,
   SpsTg8DrvrSET_SIGNAL,
   SpsTg8DrvrGET_SIGNAL,
   SpsTg8DrvrGET_QUEUE_LENGTH,
   SpsTg8DrvrPURGE_QUEUE,
   SpsTg8DrvrSET_ACTION_STATE,
   SpsTg8DrvrCLEAR_ACTION,
   SpsTg8DrvrWAIT_EVENT,
   SpsTg8DrvrFILTER_EVENT,
   SpsTg8DrvrGET_PPM_LINE,
   SpsTg8DrvrSET_OBJECT,
   SpsTg8DrvrREMOVE_OBJECT,
   SpsTg8DrvrGET_OBJECT,
   SpsTg8DrvrCONNECT,
   SpsTg8DrvrOBJECT_PARAM,
   SpsTg8DrvrOBJECTS_LIST,
   SpsTg8DrvrTEST_DPRAM,
   SpsTg8DrvrCARD_TEST,
} SpsTg8Drvr;

typedef struct {
   unsigned long Frame;
   unsigned short Control;
   unsigned short Delay;
} SpsTg8DrvrAction;

typedef struct {
   unsigned long Row;
   unsigned long Cnt;
} SpsTg8DrvrActionAddress;

typedef struct {
   unsigned long Row;
   unsigned long Cnt;
   SpsTg8DrvrAction Table[SpsTg8DrvrACTIONS];
} SpsTg8DrvrActionTable;

typedef struct {
   unsigned char RcvErr;	/* Value of the reception error register */
   unsigned char OvwrAct;	/* Which action was overwitten (if not 0x00) */
   unsigned char Act;		/* Which action was triggered and fired */
   unsigned char Sem;		/* Semaphore to solve the read/write conflicts */
} SpsTg8IntAction;

typedef struct {
   unsigned long Frame;		/* The trigger event frame */
   unsigned int Sc;		/* The Super Cycle number */
   unsigned int Occ;		/* Occurence time from the start of the last S-Cycle */
   unsigned int Out;		/* Output time from the start of the last S-Cycle */
   SpsTg8IntAction Ext;		/* Extra data concerning the interrupted action */
} SpsTg8Interrupt;

typedef struct {
   unsigned int Id;		/* Timing object identifier */
   unsigned long Frame;		/* Declared timing event (wildcards allowed) */
   SpsTg8Interrupt Inter;	/* Interrupt information */
   unsigned short Alarms;	/* The firmware alarms bit mask ORed with the driver alarms bit mask */
   unsigned char FwStat;	/* Firmware status */
   unsigned char DevMask;	/* Target devices bitmask */
} SpsTg8DrvrEvent;

typedef struct {
  unsigned int ScNumber;        /* The Super Cycle number */
  unsigned int ScLength;        /* The last completed S-Cycle length */
  unsigned int ScTime;          /* The current value of the S-Cycle time */
  unsigned int ScCounter;       /* Number of SC received since the last Reset */
} SpsTg8SuperCycleInfo;

typedef struct {
   int Machine;			/* Acc. machine number */
   unsigned short Data[SpsTg8DrvrGROUPS];	/* Telegram buffer */
} SpsTg8DrvrTelegram;		/* SPS driver telegram */

typedef struct {
  unsigned char  aYear,      /* The date */
		 aMonth,
		 aDay,
		 aSpare1;    /* 32-bits alignment is required */
  unsigned char  aRcvErr,    /* Value of the reception error register */
		 aHour,      /* The time */
		 aMinute,
		 aSecond;
  unsigned short aMilliSecond,
		 aMsDrift;   /* Drift between ms clock and second clock */
} SpsTg8DrvrDateTime;

typedef struct {
  SpsTg8DrvrDateTime Dt;     /* RcvErr, date, time */
  unsigned int       ScTime; /* Time in the S-Cycle */
  unsigned int       Epc;    /* Exception PC */
  unsigned short     Evo;    /* Exception Vector */
  unsigned short     Hw;     /* Hardware status */
  unsigned short     Fw;     /* Firmware status (RESERVED, NOT USED) */
  unsigned short     Cc;     /* Completion code */
  unsigned int       Am;     /* Alarms bitmask */
} SpsTg8DrvrStatusBlock;

typedef struct {
  int Status;
  int Alarms;
} SpsTg8Status;

typedef struct {
   unsigned long Event;
   unsigned long Matches;
} SpsTg8DrvrFilter;

/*
 * CTIM object bindings 
 */

#define SpsTg8DrvrCtimOBJECTS 1024

typedef struct {
   unsigned long EqpNum;
   unsigned long Frame;
} SpsTg8DrvrCtimBinding;

typedef struct {
   unsigned short Size;
   SpsTg8DrvrCtimBinding Objects[SpsTg8DrvrCtimOBJECTS];
} SpsTg8DrvrCtimObjects;

/*
 * PTIM object bindings 
 */

#define SpsTg8DrvrPtimOBJECTS 256

typedef struct {
   unsigned long EqpNum;
   unsigned char ModuleIndex;
   unsigned char Counter;
   unsigned short Size;
   unsigned short StartIndex;
} SpsTg8DrvrPtimBinding;

typedef struct {
   unsigned short Size;
   SpsTg8DrvrPtimBinding Objects[SpsTg8DrvrPtimOBJECTS];
} SpsTg8DrvrPtimObjects;

static SpsTg8DrvrCtimObjects *ctims = NULL;
static SpsTg8DrvrPtimObjects *ptims = NULL;
static int QFlag = 0;
static int TmOut = 20000000;
static SpsTg8DrvrActionTable *atabs = NULL;

/*
 * Control word bits Clock/CLK Disable/DIS Start/STR Counter/CNT Output/OUT Bus/BUS
 */

#define CW_BIT_MASK 0xC7C7

#define CW_CLK_MASK 0x0003
#define CW_DIS_MASK 0x0004
#define CW_STR_MASK 0x00C0
#define CW_CNT_MASK 0x0700
#define CW_OUT_MASK 0x4000
#define CW_BUS_MASK 0x8000

#define CW_CLK_BITN 0
#define CW_DIS_BITN 2
#define CW_STR_BITN 6
#define CW_CNT_BITN 8
#define CW_OUT_BITN 14
#define CW_BUS_BITN 15

#define CW_STR_CHAINED 0x0040
#define CW_STR_EXT1    0x0080
#define CW_CLK_EXT1    0x0002
#define CW_CLK_EXT2    0x0003

/*
 * ==================================================================== 
 * Keep copy of PTIM and CTIM equipments in shared memory so that all 
 * tasks can access them. Also keep the action tables here.  
 * ==================================================================== 
 */

TimLibError AttachMemory() {

   int shmkey, shmid;

   if (ctims == NULL) {
      shmkey = TgmGetKey("CTIMS");
      shmid = shmget(shmkey, sizeof(SpsTg8DrvrCtimObjects), IPC_CREAT | 0666);
      ctims = (SpsTg8DrvrCtimObjects *) shmat(shmid, NULL, 0);
      if ((int) ctims == -1) {
	 ctims = NULL;
	 return TimLibErrorNOMEM;
      }
   }
   if (timlib_debug)
      fprintf(stderr, "AttachMemory: CTIMS key:%d id:%d OK\n", shmkey, shmid);

   if (ptims == NULL) {
      shmkey = TgmGetKey("PTIMS");
      shmid = shmget(shmkey, sizeof(SpsTg8DrvrPtimObjects), IPC_CREAT | 0666);
      ptims = (SpsTg8DrvrPtimObjects *) shmat(shmid, NULL, 0);
      if ((int) ptims == -1) {
	 ptims = NULL;
	 return TimLibErrorNOMEM;
      }
   }
   if (timlib_debug)
      fprintf(stderr, "AttachMemory: PTIMS key:%d id:%d OK\n", shmkey, shmid);

   if (atabs == NULL) {
      shmkey = TgmGetKey("ATABS");
      shmid = shmget(shmkey, sizeof(SpsTg8DrvrActionTable) * SpsTg8DrvrMODULES, IPC_CREAT | 0666);
      atabs = (SpsTg8DrvrActionTable *) shmat(shmid, NULL, 0);
      if ((int) atabs == -1) {
	 atabs = NULL;
	 return TimLibErrorNOMEM;
      }
   }
   if (timlib_debug)
      fprintf(stderr, "AttachMemory: ATABS key:%d id:%d OK\n", shmkey, shmid);

   return TimLibErrorSUCCESS;
}

/*
 * ======================================================================== 
 * Open the SPS Tg8 driver for a module.  The SPS driver reserves sets 
 * of <CLIENTS_PER_MODULE> file nodes in /dev for up to <SpsTg8DrvrMODULES> 
 * Tg8 modules.  
 * ======================================================================== 
 */

static char fnm[32];

static int tg8s[SpsTg8DrvrMODULES] = { 0, 0, 0, 0 };	/* This global holds the SPS TG8 Driver file handle */

static int SpsTg8Open(int module) {

char *cp, *ep, ver[32];
unsigned long yr;
int i, m, fnum, fd;

   if ((module < 1) || (module > SpsTg8DrvrMODULES)) return 0;

   m = module - 1; if (tg8s[m] > 0) return tg8s[m];

   for (i = 0; i < SpsTg8DrvrCLIENTS_PER_MODULE; i++) {
      fnum = (m * SpsTg8DrvrCLIENTS_PER_MODULE) + i + 1;
      sprintf(fnm, "/dev/Tg8.%1d", fnum);
      fd = open(fnm, O_RDWR, 0);
      if (fd > 0) { tg8s[m] = fd;

	 if (timlib_debug)
	    fprintf(stderr,"SpsTg8Open: Opened driver file: %s Module: %d\n",fnm,m+1);

	 /*
	  * The CPS Tg8 driver returns a long version string <__DATE__ __TIME__> 
	  * while the SPS Tg8 driver returns a short version string <__DATE__> 
	  */

	 bzero((void *) ver, 32);
	 if (ioctl(tg8s[m], SpsTg8DrvrGET_FIRMWARE_VERSION, &ver) < 0) {
	    perror("SpsTg8Open");
	    ver[0] = 0;
	 }
	 if ((strlen(ver) > 0) && (strlen(ver) < (strlen(__DATE__) + 4))) {	/* Is it an SPS TG8 ? */
	    cp = &(ver[6]); /* Points to the year string */
	    yr = strtoul(cp,&ep,10);
	    if (yr < 2005) {
	       fprintf(stderr,"Found old SL_TG8 Firmware Version: %s Updating...\n",ver);
	       system("/usr/local/tim/reload_sl_tg8");
	       ioctl(tg8s[m], SpsTg8DrvrGET_FIRMWARE_VERSION, &ver);
	    }
	    fprintf(stderr,"Using SL_TG8 Firmware Version: %s\n",ver);
	    return tg8s[m];	/* Yes */
	 }
	 close(tg8s[m]);
	 tg8s[m] = 0;		/* No it a PS TG8 */
	 return 0;		/* No point in continuing */
      }
   }
   return 0;
}

/*
 * ======================================================================== 
 * Write an action to a module 
 * If anum is zero I make a new action if it dosn't exist already.  
 * Else anum is overwritten by clear and write.  
 * ======================================================================== 
 */

TimLibError SpsTg8WriteAction(SpsTg8DrvrAction * ap, int anum, int m) {

SpsTg8DrvrActionAddress ad;
int i, clear;

   ap->Control &= CW_BIT_MASK;  /* Remove spurious bits */

   if (anum) {			/* Clear action ? */
      clear = 1;
      atabs[m].Table[anum - 1] = *ap;
   } else {
      clear = 0;
      for (i = 0; i < SpsTg8DrvrACTIONS; i++) {
	 if (   (atabs[m].Table[i].Frame   == ap->Frame)
	     && (atabs[m].Table[i].Control == ap->Control)
	     && (atabs[m].Table[i].Delay   == ap->Delay)  ) {
	    anum = i + 1;
	    break;
	 }
      }
      if (anum == 0) {
	 for (i = 0; i < SpsTg8DrvrACTIONS; i++) {
	    if (atabs[m].Table[i].Frame == 0) {	/* Make a new action */
	       anum = i + 1;
	       atabs[m].Table[anum - 1] = *ap;
	       break;
	    }
	 }
      }
   }
   if (anum == 0) return TimLibErrorNOMEM;

   if (timlib_debug) {
      fprintf(stderr,
	      "SpsTg8WriteAction: Writing: Frame: 0x%08X Control: 0x%08X Delay: %d\n",
	      (int) ap->Frame, (int) ap->Control, (int) ap->Delay);
      fprintf(stderr, "SpsTg8WriteAction: To module: %d Action row: %d\n", (int) m + 1, (int) anum);
   }

   if (clear) {
      ad.Row = anum;
      ad.Cnt = 1;
      if (ioctl(tg8s[m], SpsTg8DrvrCLEAR_ACTION, &ad) < 0) {
	 perror("Tg8SpsLibInitialize");
	 return TimLibErrorIO;
      }
   }
   if (write(tg8s[m], ap, sizeof(SpsTg8DrvrAction)) <= 0) {
      perror("SpsTg8WriteAction");
      return TimLibErrorIO;
   }
   return TimLibErrorSUCCESS;
}

/*
 * ======================================================================== 
 * Force a write back of all actions to the Tg8s from shared memory.  
 * This can be usefull when other non TimLib application have done things 
 * to the user table, or you just want a reset.  
 * ======================================================================== 
 */

TimLibError SpsTg8WriteAllActions() {

int i, m;
TimLibError err;
SpsTg8DrvrActionAddress ad;

   ad.Row = 1;
   ad.Cnt = 255;

   for (m = 0; m < SpsTg8DrvrMODULES; m++) {
      if (tg8s[m] > 0) {
	 if (ioctl(tg8s[m], SpsTg8DrvrCLEAR_ACTION, &ad) < 0) {
	    perror("Tg8SpsLibInitialize");
	    return TimLibErrorIO;
	 }
	 for (i = 0; i < SpsTg8DrvrACTIONS; i++) {
	    if (atabs[m].Table[i].Frame == 0)
	       break;
	    err = SpsTg8WriteAction(&(atabs[m].Table[i]), i + 1, m);
	    if (err != TimLibErrorSUCCESS)
	       return err;
	 }
	 fprintf(stderr, "SpsTg8WriteAllActions: All written to Module:%d\n", m + 1);
      }
   }
   return TimLibErrorSUCCESS;
}

/*
 * ======================================================================== 
 * Wipe out all memory CTIMS/PTIMS/Actions 
 * Start with a clean slate, this can be usefull during debugging.  
 * ======================================================================== 
 */

TimLibError SpsTg8WipeMemory() {

int m;
SpsTg8DrvrActionAddress ad;

   ad.Row = 1;
   ad.Cnt = 255;

   bzero((void *) ctims, sizeof(SpsTg8DrvrCtimObjects));
   bzero((void *) ptims, sizeof(SpsTg8DrvrPtimObjects));

   for (m = 0; m < SpsTg8DrvrMODULES; m++) {
      bzero((void *) &atabs[m], sizeof(SpsTg8DrvrActionTable));
      if (tg8s[m] > 0) {
	 if (ioctl(tg8s[m], SpsTg8DrvrCLEAR_ACTION, &ad) < 0) {
	    perror("Tg8SpsLibInitialize");
	    return TimLibErrorIO;
	 }
	 fprintf(stderr, "SpsTg8WipeMemory: Module:%d Memory Wiped clean OK\n", m + 1);
      }
   }
   return TimLibErrorSUCCESS;
}

/*
 * ======================================================================== 
 * Read an action from shared memory, I never read back from the TG8 as it 
 * can return garbage in the control word. I need a real reference for what 
 * should be in the modules user table.  
 * ======================================================================== 
 */

TimLibError SpsTg8ReadAction(SpsTg8DrvrAction * ap, int anum, int m) {

   *ap = atabs[m].Table[anum - 1];
   if ((ap->Control & 0xF000) == 0) {
      if (timlib_debug) {
	 fprintf(stderr,
		 "SpsTg8ReadAction: Read: Frame: 0x%08X Control: 0x%08X Delay: %d\n",
		 (int) ap->Frame, (int) ap->Control, (int) ap->Delay);
	 fprintf(stderr, "SpsTg8ReadAction: From module: %d Action row: %d\n", (int) m + 1, (int) anum);
      }
   }
   return TimLibErrorSUCCESS;
}

/*
 * ==================================================================== 
 * This routine could have been hidden from the user of the Timing lib, 
 * however, in some circumstances, the initialization can take several 
 * minutes to complete. Hence I have decided to make an initialization 
 * routine publicly available, and force users to call it.  
 * This routine performs the following initialization functions...  
 * 1) Opens a connection to the driver 
 * 2) Checks the Firmware/VHDL version against the latest revision 
 * Some EProms/FPGAs may need updating, this takes a while.  
 * 3) Load all relavent CTIM and PTIM definitions if needed.  
 *
 * Special behaviour for the SPS TG8 version before calling this.  
 * The debug variable "timlib_debug" can be set from TimLibSetDebug 
 * When its not zero, error printing is turned on, so lots of messages. 
 * When its equal to 2, the Tg8 module is reinitialized from the shared 
 * memory segments to force its state to be as in the library.  
 * When its equal to 3, all memory is wiped clean, and all actions in 
 * the Tg8 modules are cleared.  
 */

TimLibError Tg8SpsLibInitialize(TimLibDevice device) { /* Initialize hardware/software */

int m, ok, module;
TimLibError err;

   if ((device == TimLibDevice_ANY) || (device == TimLibDevice_TG8_SPS)) {

      ok = 0;
      for (module = 1; module <= SpsTg8DrvrMODULES; module++) {
	 if (SpsTg8Open(module) != 0) {
	    ok = 1;
	    m = module - 1;
	    if ((timlib_enable) && (ioctl(tg8s[m], SpsTg8DrvrENABLE_MODULE, &timlib_enable) < 0)) {
	       close(tg8s[m]);
	       tg8s[m] = 0;
	       perror("SpsTg8LibInitialize");
	       return TimLibErrorIO;
	    }
	 }
      }
      if (ok) {
	 err = AttachMemory();
	 if (err != TimLibErrorSUCCESS) return err;
	 if (timlib_debug == 2) err = SpsTg8WriteAllActions();
	 if (err != TimLibErrorSUCCESS) return err;
	 if (timlib_debug == 3) err = SpsTg8WipeMemory();
	 return err;
      } else return TimLibErrorOPEN;
   }
   return TimLibErrorINIT;
}

/* ====== */

int Tg8SpsLibFdInitialize(TimLibDevice device) { /* Initialize hardware/software */

TimLibError er;

   if (tg8s[0] == 0) {
       er = Tg8SpsLibInitialize(device);
       if (er != TimLibErrorSUCCESS) return 0;
   }
   return tg8s[0];
}

/*
 * ==================================================================== 
 * The SL Tg8 driver returns all enabled interrupt to every client
 * so I need to throw away unwanted interrupts.
 * Get a connection made here
 */

#define MAX_CONNECTIONS 32

static unsigned long CtimCons[MAX_CONNECTIONS];
static unsigned long PtimCons[MAX_CONNECTIONS];

static int ClearConnect = 1;

int GetConnection(unsigned long equip, TimLibClass iclss) {
int i;
unsigned long *cons;

   if (ClearConnect) {

      ClearConnect = 0;

      bzero((void *) CtimCons, MAX_CONNECTIONS*sizeof(unsigned long));
      bzero((void *) PtimCons, MAX_CONNECTIONS*sizeof(unsigned long));
      return 0;
   }

   if      (iclss == TimLibClassCTIM) cons = CtimCons;
   else if (iclss == TimLibClassPTIM) cons = PtimCons;
   else    return 0;

   for (i=0; i<MAX_CONNECTIONS; i++) {
      if (cons[i] == 0) break;
      if (cons[i] == equip) return i+1;
   }
   return 0;
}

/*
 * ==================================================================== 
 * Remember the connection
 */

int SetConnection(unsigned long equip, TimLibClass iclss) {
int idx, i;
unsigned long *cons;

   if      (iclss == TimLibClassCTIM) cons = CtimCons;
   else if (iclss == TimLibClassPTIM) cons = PtimCons;
   else    return 0;

   if (equip == 0) ClearConnect = 1;

   idx = GetConnection(equip,iclss);
   if (idx != 0) return idx;            /* Already connected */

   for (i=0; i<MAX_CONNECTIONS; i++) {
      if (cons[i] == 0) {
	 cons[i] = equip;
	 return i+1;
      }
   }
   return 0;
}

/*
 * ==================================================================== 
 * Connect to an interrupt. If you are connecting to either a CTIM 
 * interrupt or to a hardware interrupt, you may need to specify on 
 * which device the interrupt should be connected. This is achieved by 
 * the module parameter. If the module is zero, the system will decide 
 * which device to use, otherwise module contains a value between 1 and 
 * the number of installed timing receiver cards. For PTIM objects the 
 * module parameter must be set to zero or the real module on which the 
 * PTIM object is implemented. On PTIM objects the module is implicit.  
 */

TimLibError Tg8SpsLibConnect(TimLibClass iclss,      /* Class of interrupt */
			     unsigned long equip,    /* Equipment or hardware mask */
			     unsigned long module) { /* For HARD or CTIM classes */
int i, m;
SpsTg8DrvrCtimBinding *ctimp;
SpsTg8DrvrPtimBinding *ptimp;
SpsTg8DrvrAction act;
SpsTg8DrvrFilter ftr;
TimLibError err;

   if (module > SpsTg8DrvrMODULES) return TimLibErrorMODULE;
   m = 0; if (module) m = module - 1;

   if (tg8s[m] <= 0) return TimLibErrorMODULE;

   if (iclss == TimLibClassCTIM) {

      if (timlib_debug)
	 fprintf(stderr, "Tg8SpsLibConnect: To Ctim:%d on Module:%d\n", (int) equip, (int) m + 1);

      for (i = 0; i < ctims->Size; i++) {
	 ctimp = &(ctims->Objects[i]);
	 if (ctimp->EqpNum == equip) {
	    act.Frame = ctimp->Frame;
	    act.Control = CW_BUS_MASK;
	    act.Delay = 0;
	    SetConnection(ctimp->EqpNum,TimLibClassCTIM);
	    err = SpsTg8WriteAction(&act, 0, m);
	    if (err != TimLibErrorSUCCESS) return err;
	    ftr.Event = act.Frame | 0x00FFFFFF;
	    if (ioctl(tg8s[m],SpsTg8DrvrFILTER_EVENT,&ftr) < 0) {
	       perror("Tg8SpsLibConnect");
	       return TimLibErrorIO;
	    }
	    return TimLibErrorSUCCESS;
	 }
      }
      return TimLibErrorCTIM;
   }

   else if (iclss == TimLibClassPTIM) {
      for (i = 0; i < ptims->Size; i++) {
	 ptimp = &(ptims->Objects[i]);
	 if (ptimp->EqpNum == equip) {
	    if (!module) m = ptimp->ModuleIndex;

	    if (timlib_debug) fprintf(stderr,
				      "Tg8SpsLibConnect: To Ptim:%d on Module:%d\n",
				      (int) equip,
				      (int) m + 1);

	    if (ptimp->ModuleIndex != m) return TimLibErrorMODULE;
	    err = SpsTg8ReadAction(&act, ptimp->StartIndex + 1, m);
	    if (err != TimLibErrorSUCCESS) return err;
	    SetConnection(ptimp->EqpNum,TimLibClassPTIM);
	    if (!(act.Control & CW_BUS_MASK)) {
	       act.Control |= CW_BUS_MASK;
	       err = SpsTg8WriteAction(&act, ptimp->StartIndex + 1, m);
	       if (err != TimLibErrorSUCCESS) return err;
	    }
	    ftr.Event = act.Frame | 0x00FFFFFF;
	    if (ioctl(tg8s[m],SpsTg8DrvrFILTER_EVENT,&ftr) < 0) {
	       perror("Tg8SpsLibConnect");
	       return TimLibErrorIO;
	    }
	    if (ftr.Matches == 0) return TimLibErrorCONNECT;
	    return TimLibErrorSUCCESS;
	 }
      }
      return TimLibErrorPTIM;
   }
   return TimLibErrorNOT_IMP;
}

/* ====== */

TimLibError Tg8SpsLibFdConnect(int         fd,         /* File descriptor */
			       TimLibClass iclss,      /* Class of interrupt */
			       unsigned long equip,    /* Equipment or hardware mask */
			       unsigned long module) { /* For HARD or CTIM classes */

   return Tg8SpsLibConnect(iclss,equip,module);
}

/* ==================================================================== */
/* Disconnect from an interrupt                                         */

TimLibError Tg8SpsLibDisConnect(TimLibClass iclss,      /* Class of interrupt */
				unsigned long equip,    /* Equipment or hardware mask */
				unsigned long module) { /* For HARD or CTIM classes */

   return TimLibErrorNOT_IMP;
}

/*
 * ==================================================================== 
 * Set queueing On or Off, and the time out value in micro seconds.  
 * A timeout value of zero means no time out, you wait for ever.  
 */

TimLibError Tg8SpsLibQueue(unsigned long qflag,   /* 0=>Queue, 1=>NoQueue */
			   unsigned long tmout) { /* 0=>No time outs */

   QFlag = qflag;
   TmOut = tmout;

   return TimLibErrorSUCCESS;
}

/* ====== */

TimLibError Tg8SpsLibFdQueue(int           fd,      /* File descriptor */
			     unsigned long qflag,   /* 0=>Queue, 1=>NoQueue */
			     unsigned long tmout) { /* 0=>No time outs */
   return Tg8SpsLibQueue(qflag,tmout);
}

/*
 * ====================================================================
 * To know if a call to wait will block, this call returns the Queue
 * size. If the size iz greater than zero a call to wait will not block
 * and return without waiting. If the qflag is set to NoQueue, zero is
 * allways returned and all calls to wait will block.
 */

unsigned long Tg8SpsLibGetQueueSize() {

unsigned long qsize;

   if (ioctl(tg8s[0], SpsTg8DrvrGET_QUEUE_LENGTH, &qsize) < 0) return 0;
   return qsize;
}

/*
 * ==================================================================== 
 * Wait for an interrupt. The parameters are all returned from the call 
 * so you can know which interrupt it was that came back. Note, when 
 * waiting for a hardware interrupt from either CTIM or from a counter, 
 * it is the CTIM or PTIM object that caused the interrupt returned.  
 * The telegram will have been read already by the high prioity task 
 * get_tgm_tg8/tg8, be aware of the race condition here, hence payload. 
 * This routine is a blocking call, it waits for interrupt or timeout.  
 * Any NULL argument is permitted, and no value will be returned.  
 *
 * Arguments: 
 * iclss: The class of the interrupt CTIM, PTIM, or hardware 
 * equip: The PTIM, CTIM equipment, or hardware mask 
 * plnum: If class is PTIM this is the PLS line number 
 * source: The hardware source of the interrupt 
 * onzero: The time of the interrupt 
 * trigger: The arrival time of the event that triggered the action 
 * start: The time the start of the counter occured 
 * ctim: The CTIM equipment number of the triggering event 
 * payload: The payload of the triggering event 
 * module: The module number 1..n of the timing receiver card 
 * missed: The number of missed events since the last wait 
 * qsize: The number of remaining interrupts on the queue 
 */

TimLibError Tg8SpsLibWait(TimLibClass * iclass,     /* Class of interrupt */
			  unsigned long *equip,     /* PTIM CTIM or hardware mask */
			  unsigned long *plnum,     /* Ptim line number 1..n or 0 */
			  TimLibHardware * source,  /* Hardware source of interrupt */
			  TimLibTime * onzero,      /* Time of interrupt/output */
			  TimLibTime * trigger,     /* Time of counters load */
			  TimLibTime * start,       /* Time of counters start */
			  unsigned long *ctim,      /* CTIM trigger equipment ID */
			  unsigned long *payload,   /* Payload of trigger event */
			  unsigned long *module,    /* Module that interrupted */
			  unsigned long *missed,    /* Number of missed interrupts */
			  unsigned long *qsize,     /* Remaining interrupts on queue */
			  unsigned long *machine) { /* Tgm Machine */

SpsTg8DrvrEvent rbuf, orbuf;
SpsTg8DrvrAction *actp;
SpsTg8DrvrActionTable atab;
SpsTg8DrvrPtimBinding *ptimp;
SpsTg8DrvrCtimBinding *ctimp;

fd_set rdset;
struct timeval tv;
struct timeval *ptv;
struct timezone tz;
int i, m, cnt, cc, found;

TimLibClass xclass = TimLibClassHARDWARE;
unsigned long xequip = 0;
unsigned long xplnum = 0;
TimLibHardware xsource = TimLibHardwareCTIM;
TimLibTime xonzero = { 0, 0, 0, TgmMACHINE_NONE };
TimLibTime xtrigger = { 0, 0, 0, TgmMACHINE_NONE };
TimLibTime xstart = { 0, 0, 0, TgmMACHINE_NONE };
unsigned long xctim = 0;
unsigned long xpayload = 0;
unsigned long xmodule = 0;
unsigned long xmissed = 0;
unsigned long xqsize = 0;
unsigned long xmachine = 0;
TimLibError err;

   cnt = 0;
   FD_ZERO(&rdset);
   for (m = 0; m < SpsTg8DrvrMODULES; m++) {
      if (tg8s[m] > 0) {
	 if (QFlag) ioctl(tg8s[m], SpsTg8DrvrPURGE_QUEUE, NULL);
	 FD_SET(tg8s[m], &rdset);
	 cnt++;
      }
   }
   if (cnt == 0) return TimLibErrorINIT;

   if (TmOut) {
      ptv = &tv;
      tv.tv_sec = TmOut / 1000000;
      tv.tv_usec = TmOut % 1000000;
   } else
      ptv = NULL;

   tz.tz_minuteswest = 0;
   tz.tz_dsttime = 0;

   while(1) {

      cc  = select(cnt, &rdset, NULL, NULL, ptv);
      if (cc  == 0) return TimLibErrorTIMEOUT;

      xmodule = 0;
      for (m = 0; m < SpsTg8DrvrMODULES; m++) {
	 if ((tg8s[m] > 0) && (FD_ISSET(tg8s[m], &rdset))) {
	    xmodule = m + 1;
	    if (timlib_debug) fprintf(stderr,"Tg8SpsLibWait: Select data on module: %d\n",m+1);
	 }
	 break;
      }
      if (xmodule == 0) return TimLibErrorTIMEOUT;

      bcopy((void *) &rbuf, (void *) &orbuf, sizeof(SpsTg8DrvrEvent));
      bzero((void *) &rbuf, sizeof(SpsTg8DrvrEvent));
      cc = read(tg8s[m], &rbuf, sizeof(SpsTg8DrvrEvent));
      err = TimLibGetTime(1,&xonzero);
      if (cc <= 0) {
	 perror("Tg8SpsLibWait");
	 return TimLibErrorIO;

      } else {

	 if (timlib_debug) {
	    fprintf(stderr,"Tg8SpsLibWait: Testing: Frm:0x%08X Out:%05d Occ:%05d\n",
		    (int) rbuf.Inter.Frame,
		    (int) rbuf.Inter.Out,
		    (int) rbuf.Inter.Occ);
	 }

#ifdef THROW_AWAY_REPEATS
	 if ((rbuf.Inter.Frame == orbuf.Inter.Frame)
	 &&  (rbuf.Inter.Out   == orbuf.Inter.Out)
	 &&  (rbuf.Inter.Occ   == orbuf.Inter.Occ)) continue;
#endif
	 ioctl(tg8s[m], SpsTg8DrvrGET_QUEUE_LENGTH, &xqsize);
	 xmissed = 0;
	 if (xqsize >= 128) xmissed = 1;

	 bzero((void *) &atab, sizeof(SpsTg8DrvrActionTable));
	 atab.Row = rbuf.Inter.Ext.Act;
	 atab.Cnt = 1;
	 actp = &(atab.Table[0]);
	 if (ioctl(tg8s[m], SpsTg8DrvrUSER_TABLE, &atab) < 0) {
	    perror("Tg8SpsLibWait");
	    return TimLibErrorIO;
	 }
	 actp->Control &= CW_BIT_MASK;
	 if (actp->Control == CW_BUS_MASK) xclass = TimLibClassCTIM;
	 else                              xclass = TimLibClassPTIM;

	 if (timlib_debug) {
	    fprintf(stderr,"Tg8SpsLibWait: Control Word:0x%08X: Hence class:%d\n",
		   (int) actp->Control,
		   (int) xclass);
	 }

	 xmachine = (rbuf.Inter.Frame & 0xF0000000) >> 28;
	 xmachine = TgvTgvToTgmMachine(xmachine);

	 xpayload = rbuf.Inter.Frame & 0xFFFF;

	 xonzero.CTrain = rbuf.Inter.Out;
	 xonzero.Machine = xmachine;

	 xstart = xtrigger = xonzero;
	 xtrigger.CTrain = rbuf.Inter.Occ;

	 found = 0;
	 for (i = 0; i < ctims->Size; i++) {
	    ctimp = &(ctims->Objects[i]);
	    if ((ctimp->Frame     & 0xFFFF0000)
	    ==  (rbuf.Inter.Frame & 0xFFFF0000)) {
	       xequip = xctim = ctimp->EqpNum;
	       xsource = TimLibHardwareCTIM;
	       found = 1;
	       break;
	    }
	 }
	 if (!found) continue;

	 if (xclass == TimLibClassCTIM) {
	    if (!GetConnection(ctimp->EqpNum,TimLibClassCTIM)) {
	       continue;
	    }
	 } else if (xclass == TimLibClassPTIM) {
	    found = 0;
	    for (i = 0; i < ptims->Size; i++) {
	       ptimp = &(ptims->Objects[i]);
	       if ((atab.Row >= ptimp->StartIndex) && (atab.Row < ptimp->StartIndex + ptimp->Size + 1)) {
		  found = 1;
		  xequip = ptimp->EqpNum;
		  xmodule = ptimp->ModuleIndex + 1;
		  xplnum = 1;
		  xsource = ptimp->Counter;
		  break;
	       }
	    }
	    if (!found) continue;
	    if (!GetConnection(ptimp->EqpNum,TimLibClassPTIM)) continue; /* Not for me discard interrupt */
	 }
      }
      break;
   }

   if (iclass)  *iclass  = xclass;
   if (equip)   *equip   = xequip;
   if (plnum)   *plnum   = xplnum;
   if (source)  *source  = xsource;
   if (onzero)  *onzero  = xonzero;
   if (trigger) *trigger = xtrigger;
   if (start)   *start   = xstart;
   if (ctim)    *ctim    = xctim;
   if (payload) *payload = xpayload;
   if (module)  *module  = xmodule;
   if (missed)  *missed  = xmissed;
   if (qsize)   *qsize   = xqsize;
   if (machine) *machine = xmachine;

   return TimLibErrorSUCCESS;
}

/* ====== */

TimLibError Tg8SpsLibFdWait(int fd,                   /* File descriptor */
			    TimLibClass * iclass,     /* Class of interrupt */
			    unsigned long *equip,     /* PTIM CTIM or hardware mask */
			    unsigned long *plnum,     /* Ptim line number 1..n or 0 */
			    TimLibHardware * source,  /* Hardware source of interrupt */
			    TimLibTime * onzero,      /* Time of interrupt/output */
			    TimLibTime * trigger,     /* Time of counters load */
			    TimLibTime * start,       /* Time of counters start */
			    unsigned long *ctim,      /* CTIM trigger equipment ID */
			    unsigned long *payload,   /* Payload of trigger event */
			    unsigned long *module,    /* Module that interrupted */
			    unsigned long *missed,    /* Number of missed interrupts */
			    unsigned long *qsize,     /* Remaining interrupts on queue */
			    unsigned long *machine) { /* Tgm Machine */

   return Tg8SpsLibWait(iclass,equip,plnum,source,onzero,trigger,start,ctim,payload,module,missed,qsize,machine);
}

/*
 * ==================================================================== 
 * Set the Ccv of a PTIM equipment. Note neither the counter number nor 
 * the trigger condition can be changed.  
 */

TimLibError Tg8SpsLibSet(unsigned long ptim,	/* PTIM to write to */
			 unsigned long plnum,	/* Ptim line number 1..n or 0 */
			 unsigned long grnum,	/* Tgm group number or Zero */
			 unsigned long grval,	/* Group value if num not zero */
			 TimLibCcvMask ccvm,	/* Which values to write */
			 TimLibCcv * ccv) {     /* Current control value */

int i, m, msk, cntr, found;
SpsTg8DrvrPtimBinding *ptimp;
SpsTg8DrvrAction act;
TimLibError err;

   found = 0;
   for (i = 0; i < ptims->Size; i++) {
      ptimp = &(ptims->Objects[i]);
      if (ptimp->EqpNum == ptim) {
	 m = ptimp->ModuleIndex;
	 if (tg8s[m] <= 0) return TimLibErrorMODULE;
	 cntr = ptimp->Counter & 0x7;
	 err = SpsTg8ReadAction(&act, ptimp->StartIndex + 1, m);
	 if (err != TimLibErrorSUCCESS) return err;
	 found = 1;
	 break;
      }
   }
   if (!found) return TimLibErrorPTIM;

   if ((grnum) || (plnum > 1)) return TimLibErrorPPM;    /* No PPM allowed */

   msk = 1;
   do {
      if (ccvm & msk) {
	 act.Control &= ~CW_CNT_MASK;
	 act.Control |= cntr << CW_CNT_BITN;
	 switch ((TimLibCcvMask) msk) {
	 case TimLibCcvMaskENABLE:
	    if (ccv->Enable & TimLibEnableOUT) {
	       act.Control |=  CW_OUT_MASK;
	       act.Control &= ~CW_DIS_MASK;
	    } else
	       act.Control |=  CW_DIS_MASK;
	    break;

	 case TimLibCcvMaskSTART:
	    if (ccv->Start == TimLibStartNORMAL) {
	       act.Control &= ~CW_STR_MASK;
	    } else if (ccv->Start == TimLibStartCHAINED) {
	       act.Control &= ~CW_STR_MASK;
	       act.Control |=  CW_STR_CHAINED;
	    } else if (ccv->Start == TimLibStartEXT1) {
	       act.Control &= ~CW_STR_MASK;
	       act.Control |=  CW_STR_EXT1;
	    } else {
	       return TimLibErrorSTART;
	    }
	    break;

	 case TimLibCcvMaskCLOCK:
	    if (ccv->Clock == TimLibClock1KHZ) {
	       act.Control &= ~CW_CLK_MASK;
	    } else if (ccv->Clock == TimLibClockEXT1) {
	       act.Control &= ~CW_CLK_MASK;
	       act.Control |=  CW_CLK_EXT1;
	    } else if (ccv->Clock == TimLibClockEXT2) {
	       act.Control &= ~CW_CLK_MASK;
	       act.Control |=  CW_CLK_EXT2;
	    } else
	       return TimLibErrorCLOCK;
	    break;

	 case TimLibCcvMaskCTIM:
	    found = 0;
	    if (ccv->Ctim == 0) ccv->Ctim = ctims->Objects[0].EqpNum;
	    for (i = 0; i < ctims->Size; i++) {
	       if (ctims->Objects[i].EqpNum == ccv->Ctim) {
		  act.Frame = ctims->Objects[i].Frame;
		  act.Control = CW_OUT_MASK | (cntr << CW_CNT_BITN);
		  act.Delay = 1;
		  found = 1;
		  break;
	       }
	    }
	    if (!found)
	       return TimLibErrorCTIM;
	    break;

	 case TimLibCcvMaskPAYLOAD:
	    act.Frame &= 0xFFFF0000;
	    act.Frame |= (ccv->Payload & 0xFFFF);
	    break;

	 case TimLibCcvMaskDELAY:
	    act.Delay = ccv->Delay;
	    break;

	 case TimLibCcvMaskMODE:
	 case TimLibCcvMaskOMASK:
	 case TimLibCcvMaskPWIDTH:
	 case TimLibCcvMaskPOLARITY:
	    return TimLibErrorNOT_IMP;

	 case TimLibCcvMaskMACHINE:
	 case TimLibCcvMaskGRNUM:
	 case TimLibCcvMaskGRVAL:
	    return TimLibErrorPPM;

	 default:
	    break;
	 }
      }
      msk <<= 1;
   }
   while (msk & TimLibCcvMaskBITS);

   return SpsTg8WriteAction(&act, ptimp->StartIndex + 1, m);
}

/*
 * ==================================================================== 
 * Get the Ccv of a PTIM equipment.  
 */

TimLibError Tg8SpsLibGet(unsigned long ptim,
			 unsigned long plnum, /* Ptim line number 1..n or 0 */
			 unsigned long grnum,
			 unsigned long grval,
			 TimLibCcvMask *ccvm, /* Valid fields in ccv */
			 TimLibCcv *ccv) {

int i, m, found;
SpsTg8DrvrPtimBinding *ptimp;
SpsTg8DrvrAction act;
TimLibError err;

   bzero((void *) ccv, sizeof(TimLibCcv));
   if ((grnum) || (plnum > 1))
      return TimLibErrorPPM;

   found = 0;
   for (i = 0; i < ptims->Size; i++) {
      ptimp = &(ptims->Objects[i]);
      if (ptimp->EqpNum == ptim) {
	 m = ptimp->ModuleIndex;
	 if (tg8s[m] <= 0) return TimLibErrorMODULE;
	 err = SpsTg8ReadAction(&act, ptimp->StartIndex + 1, m);
	 if (err != TimLibErrorSUCCESS)
	    return err;
	 found = 1;
	 break;
      }
   }
   if (!found)
      return TimLibErrorPTIM;

   *ccvm = TimLibCcvMaskENABLE
	 | TimLibCcvMaskSTART
	 | TimLibCcvMaskCLOCK
	 | TimLibCcvMaskDELAY
	 | TimLibCcvMaskCTIM
	 | TimLibCcvMaskPAYLOAD;

						   ccv->Enable  = TimLibEnableNOOUT;
   if ((act.Control & CW_DIS_MASK) == 0)           ccv->Enable |= TimLibEnableOUT;
   if  (act.Control & CW_BUS_MASK)                 ccv->Enable |= TimLibEnableBUS;

   if (act.Control & CW_STR_CHAINED)               ccv->Start   = TimLibStartCHAINED;
   if (act.Control & CW_STR_EXT1)                  ccv->Start   = TimLibStartEXT1;

   if ((act.Control & CW_CLK_MASK) == 0)           ccv->Clock   = TimLibClock1KHZ;
   if ((act.Control & CW_CLK_MASK) == CW_CLK_EXT1) ccv->Clock   = TimLibClockEXT1;
   if ((act.Control & CW_CLK_MASK) == CW_CLK_EXT2) ccv->Clock   = TimLibClockEXT2;

   ccv->Delay = act.Delay;
   ccv->Payload = act.Frame & 0xFFFF;

   found = 0;
   for (i = 0; i < ctims->Size; i++) {
      if ((0xFFFF0000 & ctims->Objects[i].Frame) == (0xFFFF0000 & act.Frame)) {
	 ccv->Ctim = ctims->Objects[i].EqpNum;
	 found = 1;
	 break;
      }
   }
   if (!found) return TimLibErrorCTIM;

   return TimLibErrorSUCCESS;
}

/*
 * ==================================================================== 
 * By writing to the driver this call simulates an interrupt for the 
 * connected clients. Also it can be used as a way of synchronizing 
 * processes, this is especially important in Linux systems where the 
 * schedular is not preemptive.  
 *
 * Arguments: 
 * xclass: Class of interrupt to simulate, PTIM, CTIM or Hardware 
 * equip: Equipment number for PTIM or CTIM, hardware mask for Hardware 
 * module: When class is CTIM or Hardware, the module number is used 
 * machine: Telegram ID is used for PTIM interrupts if grnum is not zero 
 * grnum: If zero, no telegram checking, else the PTIM triggers group number 
 * grval: The telegram group value for the PTIM trigger 
 */

TimLibError Tg8SpsLibSimulate(TimLibClass xclass,
		  unsigned long equip,
		  unsigned long module,
		  TgmMachine machine,
		  unsigned long grnum,
		  unsigned long grval) {

   return TimLibErrorNOT_IMP;
}

/*
 * ==================================================================== 
 * On the SPS Tg8 I can at least output a pulse.  
 */

TimLibError Tg8SpsLibRemoteControl(unsigned long remflg, /* 0 = Normal, 1 = Remote */
				   unsigned long module, /* Module or zero */
				   unsigned long cntr,   /* 1..8 counter number */
				   TimLibRemote rcmd,    /* Command */
				   TimLibCcvMask ccvm,   /* Fields to be set */
				   TimLibCcv * ccv) {    /* Value to load in counter */

int msk, m;

   if (rcmd == TimLibRemoteOUT) {
      msk = (1 << cntr) & 0xFF;
      m = 0;
      if (module)
	 m = module - 1;
      if (tg8s[m] > 0) {
	 if (ioctl(tg8s[m], SpsTg8DrvrSIMULATE_PULSE, &msk) < 0) {
	    perror("Tg8SpsLibRemoteControl");
	    return TimLibErrorIO;
	 }
	 if (timlib_debug)
	    fprintf(stderr, "Tg8SpsLibRemoteControl: Output pulse on Counter:%d Module:%d\n", (int) cntr, (int) module);
      }
   }
   return TimLibErrorNO_REMOTE;
}

/*
 * ====================================================================
 * Get a counters remote configuration
 */

TimLibError Tg8SpsLibGetRemote(unsigned long module,
			       unsigned long cntr,
			       unsigned long *remflg,
			       TimLibCcvMask *ccvm,
			       TimLibCcv     *ccv) {

   return TimLibErrorNOT_IMP;
}

/*
 * ==================================================================== 
 * As the SPS TG8 is unable for the present to respond to the UTC time 
 * event codes B5 & B6, I will just read the system time.  
 */

#define Tg8NS_IN_SEC 1000000000    /* Ns in one second 10^9 */
#define Tg8NS_ONE_MS 1000000       /* Ns in two millisecond 1 x 10^6 */

TimLibError Tg8SpsLibGetTime(unsigned long module,	/* Module number to read from */
			     TimLibTime * utc) {        /* Returned time value */

SpsTg8DrvrDateTime dt;
SpsTg8SuperCycleInfo sc;
struct tm mtm;
time_t tod;
int m;

   m = 0; if (module) m = module - 1;
   if (tg8s[m] <= 0) return TimLibErrorMODULE;
   if (ioctl(tg8s[m],SpsTg8DrvrDATE_TIME,&dt) < 0) return TimLibErrorIO;

   mtm.tm_sec   = UBCD(dt.aSecond);
   mtm.tm_min   = UBCD(dt.aMinute);
   mtm.tm_hour  = UBCD(dt.aHour);
   mtm.tm_mday  = UBCD(dt.aDay);
   mtm.tm_mon   = UBCD(dt.aMonth) - 1;
   mtm.tm_year  = UBCD(dt.aYear) + 100;
   mtm.tm_isdst = 0;

   tod = TgmMkGmtTime(&mtm);

   utc->Second  = (unsigned long) tod;
   utc->Nano    = dt.aMilliSecond * Tg8NS_ONE_MS;
   utc->CTrain  = 0;
   utc->Machine = TgmSPS;

   if (ioctl(tg8s[m],SpsTg8DrvrSC_INFO,&sc) >=0) utc->CTrain = sc.ScTime;

#ifdef CTR_TIME_ADJUST_TG8

   if (!timlib_real_utc) return TimLibErrorSUCCESS;

   if (utc->Nano >= Tg8NS_ONE_MS) utc->Nano -= Tg8NS_ONE_MS;
   else {
      utc->Second--;
      utc->Nano += (Tg8NS_IN_SEC - Tg8NS_ONE_MS);
   }

#endif

   return TimLibErrorSUCCESS;
}

/*
 * ==================================================================== 
 * Read a machines telegram from a timing receiver. The module can be 
 * either zero, in which case the system decides which device to use, 
 * or it can be explicitly set between 1 and the number of installed 
 * modules. The telegram object returned has an opaque structure and 
 * can only be decoded through the Tgm library routine .....  
 *
 * unsigned long grval = TgmGetGroupValueFromTelegram(TgmMachine machine, 
 * unsigned long grnum, 
 * TgmTelegram *telegram) 
 *
 * WARNING: The only task that should call this routine will be, get_tgm_lib, 
 * all other, LOWER PRIORITY tasks must NEVER call this routine, instead they 
 * should call the telegram library directly like this ...  
 *
 * TgmTelegram telegram; 
 * if (TgmGetTelegram(machine, index, offset, &telegram) == TgmNOT_IMP) { ...  
 * For more information on this function see the Tgm library man pages.  
 */

TimLibError Tg8SpsLibGetTelegram(unsigned long module,
				 TgmMachine machine,
				 TgmPTelegram * telegram) {

SpsTg8DrvrTelegram spstgm;
int i, m;

   m = 0;
   if (module) m = module - 1;
   if (tg8s[m] <= 0) return TimLibErrorMODULE;

   spstgm.Machine = TgvTgmToTgvMachine(machine);
   if (ioctl(tg8s[m], SpsTg8DrvrTELEGRAM, &spstgm) < 0) {
      perror("Tg8SpsLibGetTelegram");
      return TimLibErrorIO;
   }
   bzero((void *) telegram, sizeof(TgmTelegram));
   telegram->Machine = machine;
   telegram->Size = TgmLastGroupNumber(machine);
   for (i = 0; i < telegram->Size; i++)
      telegram->Groups[i] = spstgm.Data[i];

   return TimLibErrorSUCCESS;
}

/*
 * ==================================================================== 
 * Lets you know how many installed modules there are on this host.  
 */

unsigned long Tg8SpsLibGetInstalledModuleCount() {

unsigned long cnt = 0;
int m;

   for (m = 0; m < SpsTg8DrvrMODULES; m++) if (tg8s[m] > 0) cnt++;
   return cnt;
}

/*
 * ==================================================================== 
 * Get the description of a given PTIM equipment. The dimension returns 
 * the PPM dimension, counter and module are obvious.  
 */

TimLibError Tg8SpsLibGetPtimObject(unsigned long ptim,	/* PTIM equipment number */
				   unsigned long *module,
				   unsigned long *counter,
				   unsigned long *dimension) {
int i;

   for (i = 0; i < ptims->Size; i++) {
      if (ptims->Objects[i].EqpNum == ptim) {
	 *module = ptims->Objects[i].ModuleIndex + 1;
	 *counter = ptims->Objects[i].Counter;
	 *dimension = ptims->Objects[i].Size;
	 return TimLibErrorSUCCESS;
      }
   }
   return TimLibErrorPTIM;
}

/*
 * ==================================================================== 
 * Get the event code corresponding to a given CTIM equipment number.  
 */

TimLibError Tg8SpsLibGetCtimObject(unsigned long ctim,	/* CTIM equipment number */
				   unsigned long *eventcode) {

int i;

   for (i = 0; i < ctims->Size; i++) {
      if (ctims->Objects[i].EqpNum == ctim) {
	 *eventcode = ctims->Objects[i].Frame;
	 return TimLibErrorSUCCESS;
      }
   }
   return TimLibErrorCTIM;
}

/*
 * ==================================================================== 
 * In some cases when running a GUI under Linux, say, a file handle to 
 * put in a "select" is needed so that one can wait on multiple file 
 * handles simultaneously. This routine returns such a handle suitable 
 * to check for waiting interrupts. Do not read directly from it, but 
 * call the wait routine. The queue flag must be on for this to work !! 
 */

TimLibError Tg8SpsLibGetHandle(int *fd) {

   if (tg8s[0] <= 0) return TimLibErrorINIT;
   *fd = tg8s[0];
   return TimLibErrorSUCCESS;
}

/*
 * ==================================================================== 
 * Create a new PTIM object, the CCV settings will be defaulted.  
 */

TimLibError Tg8SpsLibCreatePtimObject(unsigned long ptim,	/* PTIM equipment number */
				      unsigned long module,
				      unsigned long counter,
				      unsigned long dimension) {
SpsTg8DrvrAction act;
int m, i, anum, cntr;

   if ((module < 1) || (module > SpsTg8DrvrMODULES)) return TimLibErrorMODULE;
   m = module - 1;
   if (tg8s[m] <= 0) return TimLibErrorMODULE;
   if ((counter < 1) || (counter > 8)) return TimLibErrorCOUNTER;

   if (dimension != 1) return TimLibErrorPPM;

   for (i = 0; i < ptims->Size; i++) {
      if (ptims->Objects[i].EqpNum == ptim) {
	 return TimLibErrorEXISTS;
      }
   }
   anum = 0;
   for (i = 0; i < SpsTg8DrvrACTIONS; i++) {
      if (atabs[m].Table[i].Frame == 0) {
	 anum = i + 1;
	 break;
      }
   }
   if (anum == 0) return TimLibErrorNOMEM;

   i = ptims->Size;
   if (i < SpsTg8DrvrPtimOBJECTS) {
      ptims->Objects[i].EqpNum = ptim;
      ptims->Objects[i].ModuleIndex = m;
      ptims->Objects[i].Counter = counter;
      ptims->Objects[i].Size = 1;
      ptims->Objects[i].StartIndex = anum - 1;
      ptims->Size++;
   }

   /*
    * Need to do this next step to be sure that space is reserved in the atab 
    */

   for (i = 0; i < dimension; i++) {
      cntr = counter & 0x7;
      act.Frame = 0x240AFFFF;   /* SPS Start Basic Period */
      act.Control = (cntr << 8) | 0x4000;
      act.Delay = 1;
   }

   return SpsTg8WriteAction(&act, anum, m);
}

/*
 * ==================================================================== 
 * Create a new CTIM object. If a payload is to be used for this event 
 * be sure to set the low 16-Bits to 0xFFFF 
 */

TimLibError Tg8SpsLibCreateCtimObject(unsigned long ctim,	/* CTIM equipment number */
				      unsigned long eventcode) {
int i;

   for (i = 0; i < ctims->Size; i++) {
      if (ctims->Objects[i].EqpNum == ctim) {
	 if (ctims->Objects[i].Frame  == eventcode) {
	    return TimLibErrorEXISTS;
	 } else {
	    ctims->Objects[i].Frame = eventcode;
	    return TimLibErrorSUCCESS;
	 }
      }
   }
   i = ctims->Size;
   if (ctims->Size < SpsTg8DrvrCtimOBJECTS) {
      ctims->Objects[i].EqpNum = ctim;
      ctims->Objects[i].Frame = eventcode;
      ctims->Size++;
      return TimLibErrorSUCCESS;
   }
   return TimLibErrorNOMEM;
}

/*
 * ==================================================================== 
 * Get the cable identifier to which a given module is attached so that
 * the correct module can be used to read telegrams. This function will
 * be used by the program get_tgm_tim only; it is of no interest to the
 * majority of clients because calls to ReadTelegram are diverted.
 */

TimLibError Tg8SpsLibGetCableId(unsigned long module,   /* The given module */
				unsigned long *cable) { /* The cable ID */

   *cable = TgvFirstCableIdForMember(500);    /* SPS Cable */
   return TimLibErrorSUCCESS;
}

/*
 * ==================================================================== 
 * Get Status from module.
 * See SPS version of tg8Hardw.h if you need to understand this code
 */

TimLibStatus Tg8SpsLibGetStatus(unsigned long module,
				TimLibDevice *dev)    {

TimLibStatus res;
int m, stat;
SpsTg8DrvrStatusBlock *sb;
SpsTg8Status st;
unsigned char iob[512];

   sb = (SpsTg8DrvrStatusBlock *) iob;
   *dev = TimLibDevice_TG8_SPS;
   m = 0;
   if (module) m = module - 1;

   res = TimLibStatusENABLED
       | TimLibStatusSELF_OK
       | TimLibStatusBUS_OK
       | TimLibStatusPLL_OK;

   ioctl(tg8s[m],SpsTg8DrvrGET_STATUS,&st);

   if  ((st.Status & 1     ) == 0)  res &= ~TimLibStatusENABLED;
   if (((st.Status & 0x60  ) != 0)
   ||  ((st.Status & 0x10  ) == 0)) res &= ~TimLibStatusSELF_OK;
   if  ((st.Status & 0x400 ) != 0)  res &= ~TimLibStatusPLL_OK;
   if  ((st.Alarms & 0x1803) != 0)  res &= ~TimLibStatusBUS_OK;

   ioctl(tg8s[m],SpsTg8DrvrGET_RAW_STATUS,sb);
   stat = sb->Dt.aRcvErr;
   stat &= ~0x20; stat &= 0xFF;
   if (stat == 0) res |= TimLibStatusGMT_OK;

   return res;
}

/*
 * ==================================================================== 
 * Get the list of all defined PTIMS
 */

TimLibError Tg8SpsLibGetAllPtimObjects(unsigned long *ptimlist,  /* List of ptim equipments */
				       unsigned long *psize,     /* Number of ptims in list */
				       unsigned long size) {     /* Max size of list */
SpsTg8DrvrPtimBinding *pb;
int i;

   if (ptims) {
      for (i=0; i<ptims->Size; i++) {
	 if (i>=size) break;
	 pb = &(ptims->Objects[i]);
	 ptimlist[i] = pb->EqpNum;
	 *psize = i+1;
      }
   } else *psize = 0;
   return TimLibErrorSUCCESS;
}

/*
 * ==================================================================== 
 * Get the list of all defined CTIMS
 */

TimLibError Tg8SpsLibGetAllCtimObjects(unsigned long *ctimlist,  /* List of ctim equipments */
				       unsigned long *csize,     /* Number of ctims in list */
				       unsigned long size) {     /* Max size of list */
int i;

   if (ctims) {
      for (i=0; i<ctims->Size; i++) {
	 if (i>=size) break;
	 ctimlist[i] = ctims->Objects[i].EqpNum;
	 *csize = i+1;
      }
   } else *csize = 0;
   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Get VHDL/Firmware version of all modules, and the correct version.   */

TimLibError Tg8SpsLibGetModuleVersion(TimLibModuleVersion *version) {

   return TimLibErrorNOT_IMP;

}

/* ==================================================================== */
/* Get specific information string                                      */
/* There is some very specific module dependent status information.     */
/* This routine returns a free format human readable string containing  */
/* specific status information that may help diagnosing problems for a  */
/* timing receiver module. A null pointer can be returned if the module */
/* is either dead or not installed.                                     */

char *Tg8SpsLibGetSpecificInfo(unsigned long module) { /* The given module */

   return NULL;
}

/* ==================================================================== */
/* Get the module statistics for the given module                       */

TimLibError Tg8SpsLibGetModuleStats(unsigned long module,
				    TimLibModuleStats *stats) {
   return TimLibErrorNOT_IMP;
}

/* ==================================================================== */
/* Control how the PLL locks after synchronization loss                 */

TimLibError Tg8SpsLibSetPllLocking(unsigned long module,
				   unsigned long lockflag) { /* 0=> Brutal, else Slow */
   return TimLibErrorNOT_IMP;
}

/* ==================================================================== */

TimLibError Tg8SpsLibConnectPayload(unsigned long ctim,        /* The CTIM ID you want to connect to */
				    unsigned long payload,     /* The 16 bit payload in a long */
				    unsigned long module) {    /* The module, or zero means don't care */
   return TimLibErrorNOT_IMP;
}

