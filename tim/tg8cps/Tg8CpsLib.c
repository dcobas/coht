/* ==================================================================== */
/* Implement the timing library over the CTR familly of timing receiver */
/* cards, PMC, PCI, and VME.                                            */
/* Julian Lewis May 2004                                                */
/* ==================================================================== */

/* ==================================================================== */
/* 27th/April/2005 Julian did these modifications                       */
/*    Suppressed unused variable errors in the Set routine              */
/*    Added plnum parameter to the Get routine                          */
/*    Added GetStatus routine                                           */
/*    Added GetAllPtimObjects routine                                   */
/* 20th/June/2005                                                       */
/*    Forced use of CheckClass at CTIM_PTIM_BOUNDARY                    */
/* 04th/July/2005 Julian                                                */
/*    Enable module code added: Tg8CpsLibInitialize                     */
/* 24th/August/2005 Julian                                              */
/*    Added full CTIM support so now can use timload to set up a TG8    */
/*    Handle Enable option in initialize                                */
/*    Make the read status function correctly                           */
/*    Bugs in GetCtim corrected                                         */
/* 31st/August/2005 Julian                                              */
/*    Payload logic had to be changed in Wait. Now uses event history   */
/* 19st/April/2006 Jean-Claude                                          */
/*    Change CTIM_PTIM_BOUNDARY from 2000 to 2500                       */
/* 20th back to 2000 again, its a long story ! JL                       */

#include <time.h>

#if defined(__powerpc__)

#include <tg8/Tg8DrvrApi.h>

#define MAX_OB_LIST 1024

static Tg8IoBlock iob;
static Tg8DrvrActionTable tab;
/* static int objects[MAX_OB_LIST]; */

#define UBCD(bcd) ((bcd&0xF)+10*(bcd>>4))

extern int timlib_debug;  /* 1/Print stuff, 2/Wipe memory */
extern int timlib_delay;  /* Input delay when not zero */
extern int timlib_enable; /* Enable modules */
extern int timlib_ctims;  /* Size of CTIM table to be created */

/* ==================================================================== */
/* Open a Tg8Cps Driver file handler                                    */

static int tg8cps = 0;		/* This global holds the Tg8Cps Driver file handler */

static int Tg8CpsOpen() {
char fnm[32];
int i;

   if (tg8cps) return tg8cps;
   for (i = 1; i <= Tg8DrvrDEVNUM; i++) {
      sprintf(fnm, "/dev/Tg8.%1d", i);
      if ((tg8cps = open(fnm, O_RDWR, 0)) > 0) return (tg8cps);
   }
   return (0);
}

/* ====== */

static int Tg8CpsFdOpen() {
char fnm[32];
int i, fd;

   for (i = 1; i <= Tg8DrvrDEVNUM; i++) {
      sprintf(fnm, "/dev/Tg8.%1d", i);
      if ((fd = open(fnm, O_RDWR, 0)) > 0) return fd;
   }
   return (0);
}

/* ==================================================================== */
/* This is horrible, PTIM and CTIM equipment numbers exist in the same  */
/* enumeration space so equipment numbers less than the following value */
/* are assumed to be CTIMs, while greater or equal are PTIMs !!         */

#define CTIM_PTIM_BOUNDARY 2000

static TimLibError CheckClass(TimLibClass iclss, unsigned long equip) {

   if  (tg8cps == 0)                                                 return TimLibErrorINIT;
   if  (iclss  == TimLibClassHARDWARE)                               return TimLibErrorNOT_IMP;
   if ((iclss  == TimLibClassCTIM) && (equip >= CTIM_PTIM_BOUNDARY)) return TimLibErrorCTIM;
   if ((iclss  == TimLibClassPTIM) && (equip <  CTIM_PTIM_BOUNDARY)) return TimLibErrorPTIM;
   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Lets you know how many installed modules there are on this host.     */

unsigned long Tg8CpsLibGetInstalledModuleCount() {

unsigned long cnt = 0;
Tg8ModuleAddress *moad;
int i;

   if (tg8cps == 0) return 0;
   if (ioctl(tg8cps, Tg8DrvrGET_CONFIGURATION, &iob) < 0) return 0;
   for (i = 0; i < Tg8DrvrMODULES; i++) {
      moad = &iob.GetConfig.Addresses[i];
      if (!moad->VMEAddress) continue;
      cnt++;
   }
   return cnt;
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

TimLibError Tg8CpsLibInitialize(TimLibDevice device) { /* Initialize hardware/software */
int m, mods;

   char dver[64];
   if (tg8cps) return TimLibErrorSUCCESS;
   if (Tg8CpsOpen() == 0) {
      tg8cps = 0;
      return TimLibErrorOPEN;
   }
   if (ioctl(tg8cps, Tg8DrvrGET_DRI_VERSION, &dver) < 0) {
      close(tg8cps);
      tg8cps = 0;
      return TimLibErrorINIT;
   }  
   
   if  (strlen(dver) < (strlen(__DATE__) + 4)) {
      close(tg8cps);
      tg8cps = 0;
      return TimLibErrorINIT;
   }

   mods = Tg8CpsLibGetInstalledModuleCount();
   for (m=1; m<=mods; m++) {
      ioctl(tg8cps,Tg8DrvrSET_DEFAULT_MODULE,&m);
      if ((timlib_enable) && (ioctl(tg8cps,Tg8DrvrENABLE_MODULE,NULL) < 0)) {
	 close(tg8cps);
	 tg8cps = 0;
	 return TimLibErrorINIT;
      }
   }
   m = 1;
   ioctl(tg8cps,Tg8DrvrSET_DEFAULT_MODULE,&m);

   return TimLibErrorSUCCESS;
}

/* ====== */

int Tg8CpsLibFdInitialize(TimLibDevice device) { /* Initialize hardware/software */
TimLibError er;

   if (tg8cps == 0) {
      er = Tg8CpsLibInitialize(device);
      if (er != TimLibErrorSUCCESS) return 0;
      return tg8cps;
   }
   return Tg8CpsFdOpen();
}

/* ==================================================================== */
/* Read status from Tg8 module                                          */

TimLibStatus Tg8CpsLibGetStatus(unsigned long module, TimLibDevice *dev) {

Tg8IoBlock iob;
TimLibStatus res;
unsigned long stat;

   if (dev) *dev = TimLibDevice_TG8_CPS;

   if (module)
      if (ioctl(tg8cps, Tg8DrvrSET_DEFAULT_MODULE, &module) < 0)
	 return TimLibErrorMODULE;

   if (ioctl(tg8cps,Tg8DrvrGET_STATUS,&iob) < 0)
      return TimLibErrorIO;

   stat = iob.Status.Status;

   res = 0;
   if ((stat & Tg8DrvrMODULE_SELFTEST_ERROR) == 0) res |= TimLibStatusSELF_OK;
   if ((stat & Tg8DrvrMODULE_DPRAM_ERROR)    == 0) res |= TimLibStatusBUS_OK;
						   res |= TimLibStatusPLL_OK;

   if (ioctl(tg8cps,Tg8DrvrGET_RAW_STATUS,&iob) < 0)
      return TimLibErrorIO;

   stat = iob.RawStatus.Sb.Dt.aRcvErr;

   if ((stat &
	( XrDATA_OVERFLOW | XrPARITY_ERROR   | XrEND_SEQUENCE
	| XrMID_BIT       | XrSTART_SEQUENCE | XrMS_MISSING
	| XrMS_WATCH_DOG
	)
       ) == 0) res |= TimLibStatusGMT_OK;

   stat = iob.RawStatus.Sb.Hw;
   if (stat & Tg8HS_RECEIVER_ENABLED) res |= TimLibStatusENABLED;

   return res;
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

static int tg8cps_connected = 0;

TimLibError Tg8CpsLibConnect(TimLibClass iclss,      /* Class of interrupt */
			     unsigned long equip,    /* Equipment or hardware mask */
			     unsigned long module) { /* For HARD or CTIM classes */
TimLibError err;
Tg8DrvrObjectConnection con;

   err = CheckClass(iclss,equip); if (err) return err;

   if ((module) && (ioctl(tg8cps,Tg8DrvrSET_DEFAULT_MODULE,&module))) return TimLibErrorMODULE;

   con.Id = equip;
   con.Mask = -1;
   if (ioctl(tg8cps, Tg8DrvrCONNECT, (char *) &con) < 0) return TimLibErrorCONNECT;
   tg8cps_connected++;

   return TimLibErrorSUCCESS;
}

TimLibError Tg8CpsLibFdConnect(int           fd,       /* File handel */
			       TimLibClass   iclss,    /* Class of interrupt */
			       unsigned long equip,    /* Equipment or hardware mask */
			       unsigned long module) { /* For HARD or CTIM classes */
TimLibError err;
Tg8DrvrObjectConnection con;

   err = CheckClass(iclss,equip); if (err) return err;

   if ((module) && (ioctl(fd,Tg8DrvrSET_DEFAULT_MODULE,&module))) return TimLibErrorMODULE;

   con.Id = equip;
   con.Mask = -1;
   if (ioctl(fd, Tg8DrvrCONNECT, (char *) &con) < 0) return TimLibErrorCONNECT;

   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Disconnect from an interrupt                                         */

TimLibError Tg8CpsLibDisConnect(TimLibClass iclss,      /* Class of interrupt */
				unsigned long equip,    /* Equipment or hardware mask */
				unsigned long module) { /* For HARD or CTIM classes */
TimLibError err;
Tg8DrvrObjectConnection con;

   err = CheckClass(iclss,equip); if (err) return err;

   if ((module) && (ioctl(tg8cps,Tg8DrvrSET_DEFAULT_MODULE,&module))) return TimLibErrorMODULE;

   con.Id = equip;
   con.Mask = 0;
   if (ioctl(tg8cps, Tg8DrvrCONNECT, (char *) &con) < 0) return TimLibErrorCONNECT;
   tg8cps_connected--;

   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Set queueing On or Off, and the time out value in micro seconds.     */
/* A timeout value of zero means no time out, you wait for ever.        */

static int CpsQFlag = 0;

TimLibError Tg8CpsLibQueue(unsigned long qflag,   /* 0=>Queue, 1=>NoQueue  */
			   unsigned long tmout) { /* 0=>No time outs       */

   if (tg8cps == 0) return TimLibErrorINIT;
   CpsQFlag = qflag;
   // if (tmout) return TimLibErrorTIMEOUT;
   return TimLibErrorSUCCESS;
}

/* ====== */

TimLibError Tg8CpsLibFdQueue(int           fd,
			     unsigned long qflag,   /* 0=>Queue, 1=>NoQueue  */
			     unsigned long tmout) { /* 0=>No time outs       */

   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* To know if a call to wait will block, this call returns the Queue    */
/* size. If the size iz greater than zero a call to wait will not block */
/* and return without waiting. If the qflag is set to NoQueue, zero is  */
/* allways returned and all calls to wait will block.                   */

unsigned long Tg8CpsLibGetQueueSize() {

unsigned long qsize;

   if (CpsQFlag) return 0;
   if (ioctl(tg8cps, Tg8DrvrGET_QUEUE_LENGTH, &qsize) < 0) return 0;
   return qsize;
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

TimLibError Tg8CpsLibWait(TimLibClass    *iclss,      /* Class of interrupt */
			  unsigned long  *equip,      /* PTIM CTIM or hardware mask */
			  unsigned long  *plnum,      /* Ptim line number 1..n or 0 */
			  TimLibHardware *source,     /* Hardware source of interrupt */
			  TimLibTime     *onzero,     /* Time of interrupt/output */
			  TimLibTime     *trigger,    /* Time of counters load */
			  TimLibTime     *start,      /* Time of counters start */
			  unsigned long  *ctim,       /* CTIM trigger equipment ID */
			  unsigned long  *payload,    /* Payload of trigger event */
			  unsigned long  *modnum,     /* Module that interrupted */
			  unsigned long  *missed,     /* Number of missed interrupts */
			  unsigned long  *qsize,      /* Remaining interrupts on queue */
			  TgmMachine     *machine) {  /* Corresponding TgmMachine */
Tg8DrvrEvent ev;
Tg8DateTime dt;
int qlen, cc, i;
unsigned long mod, cnt, dim, pln, ctm, frm;
TgmMachine mch;
TimLibClass cls;
TimLibTime tld;
Tg8DrvrObjectDescriptor obd;
struct tm mtm;
time_t tod;
Tg8History *hp;
Tg8DrvrEventHistoryTable h;

   if (tg8cps           == 0) return TimLibErrorINIT;
   if (tg8cps_connected == 0) return TimLibErrorWAIT;

   if (ioctl(tg8cps, Tg8DrvrDATE_TIME, &iob) < 0) return TimLibErrorIO;
   bcopy((void *) &(iob.DateTime), (void *) &dt, sizeof(Tg8DateTime));

   if (CpsQFlag) ioctl(tg8cps, Tg8DrvrPURGE_QUEUE,NULL);
   cc = read(tg8cps, &ev, sizeof(Tg8DrvrEvent));
   if (cc <= 0) return TimLibErrorTIMEOUT;

   if (ev.Inter.iMin || ev.Inter.iSec || ev.Inter.iMs) { /* ev time available ? */
      dt.aMinute = ev.Inter.iMin;
      dt.aSecond = ev.Inter.iSec;
      dt.aMilliSecond = ev.Inter.iMs;
   }

   if (ioctl(tg8cps, Tg8DrvrGET_QUEUE_LENGTH, &iob) < 0) return TimLibErrorIO;
   qlen = iob.QueueLength;

   if (ev.Id >= CTIM_PTIM_BOUNDARY) {
      cls = TimLibClassPTIM;
      bzero((void *) &obd, sizeof(Tg8DrvrObjectDescriptor));
      obd.Object.Id = ev.Id;
      if (ioctl(tg8cps, Tg8DrvrGET_OBJECT, &obd) < 0) return TimLibErrorIO;
      pln = ev.Inter.iExt.iAct - obd.Act + 1;
      mod = obd.Object.Module;
      ctm = obd.Object.Lines[0].TriggerId;
      mch = TgvTgvToTgmMachine(TgvGetMachineForMember(ctm));
      cnt = Tg8CW_CNT_Get(obd.Object.Lines[0].Cw);
      if (!cnt) cnt = 8;
   } else {
      cls = TimLibClassCTIM;
      pln = 0;
      cnt = 0;
      dim = 0;
      mod = 1;
      mch = TgvTgvToTgmMachine(TgvGetMachineForMember(ev.Id));
      ctm = ev.Id;
   }

   mtm.tm_sec   = UBCD(dt.aSecond);
   mtm.tm_min   = UBCD(dt.aMinute);
   mtm.tm_hour  = UBCD(dt.aHour);
   mtm.tm_mday  = UBCD(dt.aDay);
   mtm.tm_mon   = UBCD(dt.aMonth) - 1;
   mtm.tm_year  = UBCD(dt.aYear) + 100;
   mtm.tm_isdst = 0;
   tod = TgmMkGmtTime(&mtm);
   tld.Second = (unsigned long) tod;
   tld.Nano = dt.aMilliSecond * 1000000;
   tld.Machine = mch;
   tld.CTrain = ev.Inter.iOcc;

   if (iclss)   *iclss = cls;
   if (equip)   *equip = ev.Id;
   if (plnum)   *plnum = pln;
   if (source)  {
      if (cls == TimLibClassCTIM) *source = TimLibHardwareCTIM;
      else                        *source = 1 << (cnt+1);
   }
   if (onzero)  *onzero = tld;
   if (trigger) *trigger = tld;
   if (start)   *start = tld;

   if (ctim)    *ctim = ctm;
   if (payload) {
      *payload = 0;

      h.Cnt = Tg8HISTORIES;
      if (ioctl(tg8cps,Tg8DrvrHISTORY_TABLE,&h) < 0) return TimLibErrorIO;
      frm = TgvGetFrameForMember(ev.Id) & 0xFFFF0000;
      for (i=0; i<h.Cnt; i++) {
	 hp = &(h.Table[i]);
	 if (frm == (hp->hEvent.Long & 0xFFFF0000)) {
	    *payload = 0x0000FFFF & hp->hEvent.Long;
	    break;
	 }
      }
   }

   if (modnum)  *modnum = mod;
   if (missed)  *missed = 0;
   if (qsize)   *qsize = qlen;
   if (machine) *machine = mch;

   return TimLibErrorSUCCESS;
}

/* ====== */

TimLibError Tg8CpsLibFdWait(int            fd,          /* File descriptor */
			    TimLibClass    *iclss,      /* Class of interrupt */
			    unsigned long  *equip,      /* PTIM CTIM or hardware mask */
			    unsigned long  *plnum,      /* Ptim line number 1..n or 0 */
			    TimLibHardware *source,     /* Hardware source of interrupt */
			    TimLibTime     *onzero,     /* Time of interrupt/output */
			    TimLibTime     *trigger,    /* Time of counters load */
			    TimLibTime     *start,      /* Time of counters start */
			    unsigned long  *ctim,       /* CTIM trigger equipment ID */
			    unsigned long  *payload,    /* Payload of trigger event */
			    unsigned long  *modnum,     /* Module that interrupted */
			    unsigned long  *missed,     /* Number of missed interrupts */
			    unsigned long  *qsize,      /* Remaining interrupts on queue */
			    TgmMachine     *machine) {  /* Corresponding TgmMachine */
Tg8DrvrEvent ev;
Tg8DateTime dt;
int qlen, cc, i;
unsigned long mod, cnt, dim, pln, ctm, frm;
TgmMachine mch;
TimLibClass cls;
TimLibTime tld;
Tg8DrvrObjectDescriptor obd;
struct tm mtm;
time_t tod;
Tg8History *hp;
Tg8DrvrEventHistoryTable h;

   if (fd == 0) return TimLibErrorINIT;

   if (ioctl(fd, Tg8DrvrDATE_TIME, &iob) < 0) return TimLibErrorIO;
   bcopy((void *) &(iob.DateTime), (void *) &dt, sizeof(Tg8DateTime));

   cc = read(fd, &ev, sizeof(Tg8DrvrEvent));
   if (cc <= 0) return TimLibErrorTIMEOUT;

   if (ev.Inter.iMin || ev.Inter.iSec || ev.Inter.iMs) { /* ev time available ? */
      dt.aMinute = ev.Inter.iMin;
      dt.aSecond = ev.Inter.iSec;
      dt.aMilliSecond = ev.Inter.iMs;
   }

   if (ioctl(fd, Tg8DrvrGET_QUEUE_LENGTH, &iob) < 0) return TimLibErrorIO;
   qlen = iob.QueueLength;

   if (ev.Id >= CTIM_PTIM_BOUNDARY) {
      cls = TimLibClassPTIM;
      bzero((void *) &obd, sizeof(Tg8DrvrObjectDescriptor));
      obd.Object.Id = ev.Id;
      if (ioctl(fd, Tg8DrvrGET_OBJECT, &obd) < 0) return TimLibErrorIO;
      pln = ev.Inter.iExt.iAct - obd.Act + 1;
      mod = obd.Object.Module;
      ctm = obd.Object.Lines[0].TriggerId;
      mch = TgvTgvToTgmMachine(TgvGetMachineForMember(ctm));
      cnt = Tg8CW_CNT_Get(obd.Object.Lines[0].Cw);
      if (!cnt) cnt = 8;
   } else {
      cls = TimLibClassCTIM;
      pln = 0;
      cnt = 0;
      dim = 0;
      mod = 1;
      mch = TgvTgvToTgmMachine(TgvGetMachineForMember(ev.Id));
      ctm = ev.Id;
   }

   mtm.tm_sec   = UBCD(dt.aSecond);
   mtm.tm_min   = UBCD(dt.aMinute);
   mtm.tm_hour  = UBCD(dt.aHour);
   mtm.tm_mday  = UBCD(dt.aDay);
   mtm.tm_mon   = UBCD(dt.aMonth) - 1;
   mtm.tm_year  = UBCD(dt.aYear) + 100;
   mtm.tm_isdst = 0;
   tod = TgmMkGmtTime(&mtm);
   tld.Second = (unsigned long) tod;
   tld.Nano = dt.aMilliSecond * 1000000;
   tld.Machine = mch;
   tld.CTrain = ev.Inter.iOcc;

   if (iclss)   *iclss = cls;
   if (equip)   *equip = ev.Id;
   if (plnum)   *plnum = pln;
   if (source)  {
      if (cls == TimLibClassCTIM) *source = TimLibHardwareCTIM;
      else                        *source = 1 << (cnt+1);
   }
   if (onzero)  *onzero = tld;
   if (trigger) *trigger = tld;
   if (start)   *start = tld;

   if (ctim)    *ctim = ctm;
   if (payload) {
      *payload = 0;

      h.Cnt = Tg8HISTORIES;
      if (ioctl(fd,Tg8DrvrHISTORY_TABLE,&h) < 0) return TimLibErrorIO;
      frm = TgvGetFrameForMember(ev.Id) & 0xFFFF0000;
      for (i=0; i<h.Cnt; i++) {
	 hp = &(h.Table[i]);
	 if (frm == (hp->hEvent.Long & 0xFFFF0000)) {
	    *payload = 0x0000FFFF & hp->hEvent.Long;
	    break;
	 }
      }
   }

   if (modnum)  *modnum = mod;
   if (missed)  *missed = 0;
   if (qsize)   *qsize = qlen;
   if (machine) *machine = mch;

   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Set the Ccv of a PTIM equipment. Note neither the counter number nor */
/* the trigger condition can be changed.                                */

TimLibError Tg8CpsLibSet(unsigned long ptim,  /* PTIM to write to */
			 unsigned long plnum, /* Ptim line number 1..n or 0 */
			 unsigned long grnum, /* Tgm group number or Zero */
			 unsigned long grval, /* Group value if num not zero */
			 TimLibCcvMask ccvm,  /* Which values to write */
			 TimLibCcv * ccv) {   /* Current control value */

int anum;

Tg8DrvrAction *act;
Tg8DrvrObjectDescriptor obd;
Tg8DrvrObjectParameter opr;

Tg8User *uac;
Tg8ActionParam *apr;

int gt, cw, st, ck, ev, md;
int i;
unsigned long msk, mod;

TgmGroupDescriptor desc;
TimLibError err;

   err = CheckClass(TimLibClassPTIM,ptim); if (err) return err;

   bzero((void *) &opr, sizeof (Tg8DrvrObjectParameter));
   apr = &(opr.Par);

   bzero((void *) &obd, sizeof(Tg8DrvrObjectDescriptor));
   obd.Object.Id = ptim;
   if (ioctl(tg8cps, Tg8DrvrGET_OBJECT, &obd) < 0) return TimLibErrorPTIM;

   if (plnum) anum = (obd.Act -1) + (plnum -1);
   else if (grnum) {
     anum = obd.Act + grval - 1;
   } else {
     anum = obd.Act - 1;
   }

   mod = obd.Object.Module;
   if (mod) {
      if (ioctl(tg8cps,Tg8DrvrSET_DEFAULT_MODULE,&mod) < 0) {
	 return TimLibErrorMODULE;
      }
   }
   bzero((void *) &iob, sizeof(Tg8IoBlock));
   iob.Range.Row = 1;
   iob.Range.Cnt = 256;
   if (ioctl (tg8cps, Tg8DrvrACTION_TABLE, &iob) < 0) return TimLibErrorIO;
   tab = iob.ActionTable;
   act = &(tab.Table[anum]);

   uac = &(act->Action);
   cw = uac->uControl;

   for (i=0; i<16; i++) {
      msk = 1 << i;
      if (ccvm & msk) {

	 bzero((void *) apr,sizeof (Tg8ActionParam));

	 switch ((TimLibCcvMask) msk) {

	    case TimLibCcvMaskENABLE:
	       apr->Sel = Tg8SEL_STATE;
	       if (ccv->Enable  & TimLibEnableOUT)   apr->Val = Tg8DO_OUTPUT;
	       if (ccv->Enable == TimLibEnableNOOUT) apr->Val = Tg8DO_NOTHING;
	    break;

	    case TimLibCcvMaskSTART:
	       st = 0;
	       if (ccv->Start == TimLibStartNORMAL)  st = Tg8CM_NORMAL;
	       if (ccv->Start == TimLibStartCHAINED) st = Tg8CM_CHAINED;
	       if (ccv->Start == TimLibStartEXT1)    st = Tg8CM_EXTERNAL_1;
	       if (ccv->Start == TimLibStartEXT2)    st = Tg8CM_EXTERNAL_2;

	       cw &= ~(Tg8CW_START_BITM << Tg8CW_START_BITN);
	       cw |= (st << Tg8CW_START_BITN);

	       apr->Sel = Tg8SEL_CW;
	       apr->Val = cw;
	       apr->Aux = uac->uDelay;
	    break;

	    case TimLibCcvMaskMODE:
	       md = 0;
	       if (ccv->Mode == TimLibModeNORMAL) {
		  if (uac->uDelay == 0) md = Tg8SEL_DIRECT;
		  else                  md = Tg8SEL_SIMPLE;
	       }
	       if (ccv->Mode == TimLibModeMULTIPLE) md = Tg8SEL_DIVIDE;
	       if (ccv->Mode == TimLibModeBURST)    md = Tg8SEL_BURST;

	       apr->Sel = md;
	    break;

	    case TimLibCcvMaskCLOCK:
	       ck = 0;
	       if (ccv->Clock == TimLibClock1KHZ)   ck = Tg8CLK_MILLISECOND;
	       if (ccv->Clock == TimLibClock10MHZ)  ck = Tg8CLK_CABLE;
	       if (ccv->Clock == TimLibClockEXT1)   ck = Tg8CLK_X1;
	       if (ccv->Clock == TimLibClockEXT2)   ck = Tg8CLK_X2;

	       apr->Sel = Tg8SEL_CLOCK;
	       apr->Val = ck;
	       apr->Aux = uac->uDelay;
	    break;

	    case TimLibCcvMaskPWIDTH:
	    break;

	    case TimLibCcvMaskDELAY:
	       apr->Sel = Tg8SEL_DELAY;
	       apr->Val = ccv->Delay;
	    break;

	    case TimLibCcvMaskOMASK:
	    break;

	    case TimLibCcvMaskPOLARITY:
	    break;

	    case TimLibCcvMaskCTIM:
	       apr->Sel = Tg8SEL_TRIGG;
	       apr->Val = ccv->Ctim;
	       apr->Aux = uac->uDelay;
	    break;

	    case TimLibCcvMaskPAYLOAD:
	       ev = TgvGetFrameForMember(ccv->Ctim);
	       ev &= 0xFFFF0000;
	       ev |= (ccv->Payload & 0xFFFF);

	       apr->Sel = Tg8SEL_CEVENT;
	       apr->Val = ev;
	       apr->Aux = uac->uDelay;
	    break;

	    case TimLibCcvMaskMACHINE:
	    case TimLibCcvMaskGRNUM:
	    case TimLibCcvMaskGRVAL:

	       apr->Sel  = Tg8SEL_GATE;
	       apr->Mach = ccv->Machine;
	       apr->Gn   = ccv->GrNum;
	       apr->Val  = ccv->GrVal;
	       if (TgmGetGroupDescriptor(ccv->Machine,ccv->GrNum,&desc) == TgmSUCCESS) {
		  if (desc.Type == TgmBIT_PATTERN)   gt = Tg8GT_BIT;
		  if (desc.Type == TgmNUMERIC_VALUE) gt = Tg8GT_NUM;
		  if (desc.Type == TgmEXCLUSIVE)     gt = Tg8GT_EXC;

		  apr->Aux = gt;

		  break;
	       }
	    return TimLibErrorGROUP;

	    default:
	    break;
	 }
      }
   }

   if (apr->Sel) {
      opr.Id = ptim;
      opr.Line = plnum;
      if (ioctl(tg8cps,Tg8DrvrMOD_OBJECT_PARAM,&opr) < 0) return TimLibErrorIO;
   }
   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Get the Ccv of a PTIM equipment.                                     */

TimLibError Tg8CpsLibGet(unsigned long ptim,
			 unsigned long plnum,
			 unsigned long grnum,
			 unsigned long grval,
			 TimLibCcvMask * ccvm,    /* Valid fields in ccv */
			 TimLibCcv * ccv) {

int anum;
Tg8DrvrAction *act;
Tg8DrvrObjectDescriptor obd;

Tg8User *uac;
Tg8Gate *gte;

int tg, gn, gv, cw, st, ck, rs, ds, ev, ti, md;
int i, ch;
unsigned long msk, mod;
TimLibError err;

   err = CheckClass(TimLibClassPTIM,ptim); if (err) return err;

   bzero((void *) &obd, sizeof(Tg8DrvrObjectDescriptor));
   obd.Object.Id = ptim;
   if (ioctl(tg8cps, Tg8DrvrGET_OBJECT, &obd) < 0) return TimLibErrorPTIM;

   if (plnum) anum = (obd.Act -1) + (plnum -1);
   else if (grnum) {
     anum = obd.Act + grval - 1;
   } else {
     anum = obd.Act - 1;
   }

   mod = obd.Object.Module;
   if (mod) {
      if (ioctl(tg8cps,Tg8DrvrSET_DEFAULT_MODULE,&mod) < 0) {
	 return TimLibErrorMODULE;
      }
   }
   bzero((void *) &iob, sizeof(Tg8IoBlock));
   iob.Range.Row = 1;
   iob.Range.Cnt = 256;
   if (ioctl (tg8cps, Tg8DrvrACTION_TABLE, &iob) < 0) return TimLibErrorIO;
   tab = iob.ActionTable;
   act = &(tab.Table[anum]);

   *ccvm = TimLibCcvMaskBITS;

   uac = &(act->Action);
   gte = &(act->Gate);

   tg = (int) ((gte->Machine) >> 4);    /* Tgm Machine */
   gn = (int)  (gte->GroupNum);
   gv = (int)  (gte->GroupVal);

   cw = uac->uControl;
   ch = (cw >> Tg8CW_CNT_BITN)   & Tg8CW_CNT_BITM; if (ch == 0) ch = 8;
   st = (cw >> Tg8CW_START_BITN) & Tg8CW_START_BITM;
   ck = (cw >> Tg8CW_CLOCK_BITN) & Tg8CW_CLOCK_BITM;
   rs = (cw >> Tg8CW_INT_BITN)   & Tg8CW_INT_BITM;
   ds = (cw >> Tg8CW_STATE_BITN) & Tg8CW_STATE_BITM;
   md = (cw >> Tg8CW_MODE_BITN)  & Tg8CW_MODE_BITM;

   ev = (int) uac->uEvent.Long;
   ti = act->TriggerId;

   for (i=0; i<16; i++) {
      msk = 1 << i;
      if (TimLibCcvMaskBITS & msk) {
	 switch ((TimLibCcvMask) msk) {

	    case TimLibCcvMaskENABLE:
	       ccv->Enable = TimLibEnableNOOUT;
	       if (rs & Tg8DO_OUTPUT)        ccv->Enable |= TimLibEnableOUT;
	       if (rs & Tg8DO_INTERRUPT)     ccv->Enable |= TimLibEnableBUS;
	       if (ds == 1)                  ccv->Enable  = TimLibEnableNOOUT;
	    break;

	    case TimLibCcvMaskSTART:
	       if (st == Tg8CM_NORMAL)       ccv->Start = TimLibStartNORMAL;
	       if (st == Tg8CM_CHAINED)      ccv->Start = TimLibStartCHAINED;
	       if (st == Tg8CM_EXTERNAL_1)   ccv->Start = TimLibStartEXT1;
	       if (st == Tg8CM_EXTERNAL_2)   ccv->Start = TimLibStartEXT2;
	    break;

	    case TimLibCcvMaskMODE:
	       if (md == Tg8MODE_NONE)       ccv->Mode = TimLibModeNORMAL;
	       if (md == Tg8MODE_DIRECT)     ccv->Mode = TimLibModeNORMAL;
	       if (md == Tg8MODE_DIVIDE)     ccv->Mode = TimLibModeMULTIPLE;
	       if (md == Tg8MODE_BURST)      ccv->Mode = TimLibModeBURST;
	    break;

	    case TimLibCcvMaskCLOCK:
	       if (ck == Tg8CLK_MILLISECOND) ccv->Clock = TimLibClock1KHZ;
	       if (ck == Tg8CLK_CABLE)       ccv->Clock = TimLibClock10MHZ;
	       if (ck == Tg8CLK_X1)          ccv->Clock = TimLibClockEXT1;
	       if (ck == Tg8CLK_X2)          ccv->Clock = TimLibClockEXT2;
	    break;

	    case TimLibCcvMaskPWIDTH:
	       ccv->PulsWidth = 40;
	    break;

	    case TimLibCcvMaskDELAY:
	       ccv->Delay = uac->uDelay;
	    break;

	    case TimLibCcvMaskOMASK:
	       ccv->OutputMask = 1 << ch;
	    break;

	    case TimLibCcvMaskPOLARITY:
	       ccv->Polarity = TimLibPolarityTTL_BAR;
	    break;

	    case TimLibCcvMaskCTIM:
	       ccv->Ctim = ti;
	    break;

	    case TimLibCcvMaskPAYLOAD:
	       ccv->Payload = ev & 0xFFFF;
	    break;

	    case TimLibCcvMaskMACHINE:
	       ccv->Machine = tg;
	    break;

	    case TimLibCcvMaskGRNUM:
	       ccv->GrNum = gn;
	    break;

	    case TimLibCcvMaskGRVAL:
	       ccv->GrVal = gv;
	    break;

	    default:
	    break;
	 }
      }
   }
   return TimLibErrorSUCCESS;
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

TimLibError Tg8CpsLibSimulate(TimLibClass iclss,
			      unsigned long equip,
			      unsigned long module,
			      TgmMachine machine,
			      unsigned long grnum,
			      unsigned long grval) {

// int cc;

   if (tg8cps == 0) return TimLibErrorINIT;

   if (module) {
      if (ioctl(tg8cps, Tg8DrvrSET_DEFAULT_MODULE, &module)) return TimLibErrorMODULE;
   }
   //   iob.SimPulseMask = (1 << (LOW(dtr->address1)-1));
   //   if (ioctl(tg8,Tg8DrvrSIMULATE_PULSE,&iob) < 0) return TimLibErrorIO;

   return TimLibErrorSUCCESS;
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

TimLibError Tg8CpsLibRemoteControl(unsigned long remflg, /* 0 = Normal, 1 = Remote */
				   unsigned long module, /* Module or zero */
				   unsigned long cntr,   /* 1..8 counter number */
				   TimLibRemote rcmd,    /* Command */
				   TimLibCcvMask ccvm,   /* Fields to be set */
				   TimLibCcv * ccv) {    /* Value to load in counter */

   return TimLibErrorNOT_IMP;
}

/* ==================================================================== */
/* Get a counters remote configuration                                  */

TimLibError Tg8CpsLibGetRemote(unsigned long module,
			       unsigned long cntr,
			       unsigned long *remflg,
			       TimLibCcvMask *ccvm,
			       TimLibCcv     *ccv) {

   return TimLibErrorNOT_IMP;
}

/* ==================================================================== */
/* Read the instantaneous value of the time in UTC. The module parameter*/
/* can be set to zero in which case the system decideds which module to */
/* read the time from, otherwise it can be set to a value between 1 and */
/* the number of installed modules.                                     */

TimLibError Tg8CpsLibGetTime(unsigned long module, /* Module number to read from */
			     TimLibTime *utc) {    /* Returned time value */
Tg8DateTime *dt;
Tg8DrvrRawStatus *rwst;
Tg8ModuleAddress *moad;
struct tm mtm;
time_t tod;

   if (tg8cps == 0) return TimLibErrorINIT;
   if (module) if (ioctl(tg8cps, Tg8DrvrSET_DEFAULT_MODULE, &module)) return TimLibErrorMODULE;
   if (ioctl(tg8cps, Tg8DrvrDATE_TIME, &iob) < 0) return TimLibErrorIO;
   dt = &iob.DateTime;

   mtm.tm_sec   = UBCD(dt->aSecond);
   mtm.tm_min   = UBCD(dt->aMinute);
   mtm.tm_hour  = UBCD(dt->aHour);
   mtm.tm_mday  = UBCD(dt->aDay);
   mtm.tm_mon   = UBCD(dt->aMonth) - 1;
   mtm.tm_year  = UBCD(dt->aYear) + 100;
   mtm.tm_isdst = 0;
   tod = TgmMkGmtTime(&mtm);
   utc->Second = (unsigned long) tod;
   utc->Nano = dt->aMilliSecond * 1000000;

   if (ioctl(tg8cps, Tg8DrvrGET_CONFIGURATION, &iob) < 0) return TimLibErrorIO;
   if (module) moad = &iob.GetConfig.Addresses[module - 1];
   else        moad = &iob.GetConfig.Addresses[0];
   utc->Machine = moad->Machine;

   if (ioctl(tg8cps, Tg8DrvrGET_RAW_STATUS, &iob) < 0) return TimLibErrorIO;
   rwst = &iob.RawStatus;
   utc->CTrain = rwst->Sb.ScTime;

   return TimLibErrorSUCCESS;
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

/* TgmTelegram telegram;                                                       */
/*                                                                             */
/* if (TgmGetTelegram(machine, index, offset, &telegram) == TgmSUCCESS) { ...  */
/*                                                                             */
/* For more information on this function see the Tgm library man pages.        */

TimLibError Tg8CpsLibGetTelegram(unsigned long module,
				 TgmMachine    machine,
				 TgmPTelegram  *telegram) {

Tg8DrvrTelegramBlock tbk;
int i;

   if (tg8cps == 0) return TimLibErrorINIT;
   if (module) {
      if (ioctl(tg8cps, Tg8DrvrSET_DEFAULT_MODULE, &module)) return TimLibErrorMODULE;
   }
   //tbk.Machine = machine;
   tbk.Machine = TgvTgmToMtgvMachine(machine);
   if (ioctl(tg8cps, Tg8DrvrTELEGRAM, &tbk) < 0) return TimLibErrorIO;
   telegram->Size = TgmLastGroupNumber(machine);
   telegram->Machine = machine;
   for (i = 0; i < telegram->Size; i++) {
      TgmSetGroupValueInTelegram(i + 1, (long) tbk.Data[i], (TgmTelegram *) telegram);
   }

   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Get the description of a given PTIM equipment. The dimension returns */
/* the PPM dimension, counter and module are obvious.                   */

TimLibError Tg8CpsLibGetPtimObject(unsigned long ptim,	/* PTIM equipment number */
				   unsigned long *module,
				   unsigned long *counter,
				   unsigned long *dimension) {
int i, cw, ch;
Tg8DrvrObjectDescriptor obd;
Tg8ObjectLine oln;
Tg8DrvrBindMtgEvent *eb;

   if (tg8cps == 0) return TimLibErrorINIT;

   bzero((void *) &iob, sizeof(Tg8IoBlock));
   iob.BindEvents.Length = Tg8BINDINGS;
   if (ioctl(tg8cps, Tg8DrvrGET_BINDINGS, &iob) < 0) return TimLibErrorIO;

   for (i = 0; i < iob.BindEvents.Length; i++) {
      eb = &(iob.BindEvents.Table[i]);
      if (ptim == eb->Id) return TimLibErrorPTIM;
   }

   bzero((void *) &obd, sizeof(Tg8DrvrObjectDescriptor));
   obd.Object.Id = ptim;
   if (ioctl(tg8cps, Tg8DrvrGET_OBJECT, &obd) < 0) return TimLibErrorPTIM;

   oln = obd.Object.Lines[0];
   cw = oln.Cw;
   ch = Tg8CW_CNT_Get(cw);

   //ch = (cw >> Tg8CW_CNT_BITN) & Tg8CW_CNT_BITM;

   if (ch == 0) ch = 8;

   if (module)    *module = obd.Object.Module;
   if (counter)   *counter = ch;
   if (dimension) *dimension = obd.Dim;

   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Get the event code corresponding to a given CTIM equipment number.   */

TimLibError Tg8CpsLibGetCtimObject(unsigned long ctim,	/* CTIM equipment number */
				   unsigned long *eventcode) {

Tg8DrvrBindMtgEvent *eb;
int i;

   if (tg8cps == 0) return TimLibErrorINIT;
   bzero((void *) &iob, sizeof(Tg8IoBlock));
   iob.BindEvents.Length = Tg8BINDINGS;
   if (ioctl(tg8cps, Tg8DrvrGET_BINDINGS, &iob) < 0) return TimLibErrorIO;

   for (i = 0; i < iob.BindEvents.Length; i++) {
      eb = &(iob.BindEvents.Table[i]);
      if (ctim == eb->Id) {
	 if (eventcode) *eventcode = (unsigned long) eb->Event.Long;
	 return TimLibErrorSUCCESS;
      }
   }
   return TimLibErrorCTIM;
}

/* ==================================================================== */
/* In some cases when running a GUI under Linux, say, a file handle to  */
/* put in a "select" is needed so that one can wait on multiple file    */
/* handles simultaneously. This routine returns such a handle suitable  */
/* to check for waiting interrupts. Do not read directly from it, but   */
/* call the wait routine. The queue flag must be on for this to work !! */

TimLibError Tg8CpsLibGetHandle(int *fd) {

   if (tg8cps == 0) return TimLibErrorINIT;
   *fd = tg8cps;
   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Create a new PTIM object, the CCV settings will be defaulted.        */

TimLibError Tg8CpsLibCreatePtimObject(unsigned long ptim,	/* PTIM equipment number */
				      unsigned long module,
				      unsigned long counter,
				      unsigned long dimension) {
Tg8DrvrObject ob;
/* Tg8DrvrObjectsList obl; */
Tg8ObjectLine *olp;
int str, chn, clk, i /* , j, nid, maxobs */ ;
TimLibError err;

   err = CheckClass(TimLibClassPTIM,ptim); if (err) return err;

   bzero((void *) &ob, sizeof(Tg8DrvrObject));
   ob.Id     = (unsigned int) ptim;
   ob.Module = (unsigned char) module;
   ob.Dim    = (unsigned char) dimension;
   chn       = counter;

   /* edit the actions with some initial values which will be overwritten by TimLibSet */

   for (i = 0; i < dimension; i++) {
      olp = &(ob.Lines[i]);	/* get action */
      olp->TriggerId = 200;	/* set RPLS as load */
      olp->Cw = (Tg8DO_OUTPUT << Tg8CW_INT_BITN);	/* produce output */
      str = Tg8CM_NORMAL;
      Tg8CW_START_Set(olp->Cw, str);	/* set start mode */
      clk = Tg8CLK_MILLISECOND;
      Tg8CW_CLOCK_Set(olp->Cw, clk);	/* set clock */
      Tg8CW_CNT_Set(olp->Cw, chn);	/* set channel */
      olp->Delay = 1;		/* set delay */
   }

   if (ioctl(tg8cps, Tg8DrvrCREATE_OBJECT, &ob) < 0) return TimLibErrorIO;

   /* bzero ((void *) &iob, sizeof (Tg8IoBlock));
      iob.ObjectsList.Length = MAX_OB_LIST;
      if (ioctl (tg8cps, Tg8DrvrOBJECTS_LIST, &iob) < 0) return TimLibErrorIO;

      for (i = 0; i < iob.ObjectsList.Length; i++) {
      nid = iob.ObjectsList.Id[i];
      bzero ((void *) &obl, sizeof (Tg8DrvrObjectsList));
      obl.Length = Tg8DrvrOBJECTS;
      obl.Id[0] = nid;
      if (ioctl (tg8cps, Tg8DrvrGET_MEMBER_IDS, &obl) < 0) return TimLibErrorIO;

      if (nid < 0) {
	 if (nid != obl.Id[0])
	 for (j = 0; j < obl.Length; j++)
	 objects[maxobs++] = obl.Id[j];
      } else if (nid > 999)
	 objects[maxobs++] = nid;
      } */

   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Create a new CTIM object. If a payload is to be used for this event  */
/* be sure to set the low 16-Bits to 0xFFFF                             */

TimLibError Tg8CpsLibCreateCtimObject(unsigned long ctim,	/* CTIM equipment number */
				      unsigned long eventcode) {

Tg8DrvrBindMtgEvent *eb, *ebn;
Tg8DrvrEventBinding binds;
int i;

   if ((ctim == 0) || (ctim > CTIM_PTIM_BOUNDARY))
      return TimLibErrorCTIM;

   bzero((void *) &binds, sizeof(Tg8DrvrEventBinding));
   binds.Length = Tg8BINDINGS;
   if (ioctl(tg8cps,Tg8DrvrGET_BINDINGS,&binds) < 0) return TimLibErrorIO;

   if (binds.Length) {
      ebn = NULL;
      for (i=0; i<binds.Length; i++) {
	 eb = &(binds.Table[i]);
	 if ((ebn == NULL) && (eb->Event.Long == 0)) ebn = eb;
	 else if (eb->Id == ctim) {
	    if (eb->Event.Long == (long) eventcode) return TimLibErrorEXISTS;

	    /* Change the event code */

	    eb->Event.Long = (long) eventcode;
	    if (ioctl(tg8cps,Tg8DrvrBIND_MTG_EVENTS,&binds) < 0)
	       return TimLibErrorIO;
	    return TimLibErrorSUCCESS;
	 }
      }

      /* It didn't exist so create a new CTIM in the empty hole */

      if (ebn) {
	 ebn->Id = ctim;
	 ebn->Event.Long = (long) eventcode;
	 if (ioctl(tg8cps,Tg8DrvrBIND_MTG_EVENTS,&binds) < 0)
	    return TimLibErrorIO;
	 return TimLibErrorSUCCESS;
      }

      return TimLibErrorNOMEM; /* Can't extend binding table after created */
   }

   /* No bindings; so create a new binding table for the first time */

   binds.Length = timlib_ctims; /* Default value set in TimLib.c */
   eb = &(binds.Table[0]);
   eb->Id = ctim;
   eb->Event.Long = (long) eventcode;
   if (ioctl(tg8cps,Tg8DrvrBIND_MTG_EVENTS,&binds) < 0)
      return TimLibErrorIO;

   return TimLibErrorSUCCESS;
}

/*
 * ==================================================================== 
 * Get the cable identifier to which a given module is attached so that
 * the correct module can be used to read telegrams. This function will
 * be used by the program get_tgm_tim only; it is of no interest to the
 * majority of clients because calls to ReadTelegram are diverted.
 */

TimLibError Tg8CpsLibGetCableId(unsigned long module,	/* The given module */
				unsigned long *cable) { /* The cable ID */

Tg8ModuleAddress *moad;
int i;

   if (tg8cps == 0) return TimLibErrorINIT;
   if ((module < 1) || (module > Tg8DrvrMODULES)) return TimLibErrorMODULE;
   if (ioctl(tg8cps, Tg8DrvrGET_CONFIGURATION, &iob) < 0) return TimLibErrorIO;
   i = module - 1;
   moad = &iob.GetConfig.Addresses[i];
   if (!moad->VMEAddress) return TimLibErrorMODULE;
   *cable = moad->CableId;
   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Get the list of all defined PTIM objects                             */

#define MAX_LIST_SIZE 1024

TimLibError Tg8CpsLibGetAllPtimObjects(unsigned long *ptimlist,  /* List of ptim equipments */
				       unsigned long *psize,     /* Number of ptims in list */
				       unsigned long size) {     /* Max size of list */

Tg8IoBlock iob;
int i, ix;

   bzero ((void *) &iob, sizeof (Tg8IoBlock));
   if (size > MAX_LIST_SIZE) size = MAX_LIST_SIZE;
   iob.ObjectsList.Length = size;
   if (ioctl (tg8cps, Tg8DrvrOBJECTS_LIST, &iob) < 0) return TimLibErrorIO;
   for (ix=0,i=0; i<iob.ObjectsList.Length; i++) {
      if (ix >= size) break;
      if (iob.ObjectsList.Id[i] >= CTIM_PTIM_BOUNDARY) {
	 ptimlist[ix++] = iob.ObjectsList.Id[i];
      }
   }
   if (psize) *psize = ix;
   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Get the list of all defined CTIM objects                             */

TimLibError Tg8CpsLibGetAllCtimObjects(unsigned long *ctimlist,  /* List of ctim equipments */
				       unsigned long *csize,     /* Number of ctims in list */
				       unsigned long size) {     /* Max size of list */

Tg8DrvrBindMtgEvent *eb;
Tg8DrvrEventBinding binds;
int i;

   if (size > Tg8BINDINGS) size = Tg8BINDINGS;
   binds.Length = size;
   if (ioctl(tg8cps,Tg8DrvrGET_BINDINGS,&binds) < 0) return TimLibErrorIO;
   for (i=0; i<binds.Length; i++) {
      eb = &(binds.Table[i]);
      ctimlist[i] = eb->Id;
      *csize = i+1;
   }
   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Get VHDL/Firmware version of all modules, and the correct version.   */

static char *mnm[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

TimLibError Tg8CpsLibGetModuleVersion(TimLibModuleVersion *tver) {

struct tm stoy;
time_t toy;
char dver[64], *cp, *ep;
int i, cnt, m;

   if (tver == NULL) return TimLibErrorNOMEM;
   bzero((void *) tver, sizeof(TimLibModuleVersion));

   tver->ModTyp = TimLibModuleTypeCPS_TG8;

   if (ioctl(tg8cps, Tg8DrvrGET_DRI_VERSION, &dver) < 0) return TimLibErrorIO;

   /* Decode: mmm dd yyyy hh:mm:ss */
   /*         01234567890123456789 */

   bzero((void *) &stoy, sizeof(struct tm));

   for (i=0; i<12; i++) {
      if (strncmp(dver,mnm[i],3)==0) {
	 stoy.tm_mon = i;
	 break;
      }
   }

   cp = &dver[4];
   stoy.tm_mday = strtoul(cp,&ep,10);
   cp = &dver[7];
   stoy.tm_year = strtoul(cp,&ep,10) - 1900;
   cp = &dver[12];
   stoy.tm_hour = strtoul(cp,&ep,10);
   cp = &dver[15];
   stoy.tm_min = strtoul(cp,&ep,10);
   cp = &dver[18];
   stoy.tm_sec = strtoul(cp,&ep,10);

   toy = mktime(&stoy);
   tver->DrvVer = toy;

   cnt = Tg8CpsLibGetInstalledModuleCount();
   for (m=1; m<=cnt; m++) {
      if (ioctl(tg8cps, Tg8DrvrSET_DEFAULT_MODULE, &m)) return TimLibErrorMODULE;
      if (ioctl(tg8cps, Tg8DrvrGET_FIRMWARE_VERSION, &dver) < 0) return TimLibErrorIO;

      bzero((void *) &stoy, sizeof(struct tm));

      for (i=0; i<12; i++) {
	 if (strncmp(dver,mnm[i],3)==0) {
	    stoy.tm_mon = i;
	    break;
	 }
      }

      cp = &dver[4];
      stoy.tm_mday = strtoul(cp,&ep,10);
      cp = &dver[7];
      stoy.tm_year = strtoul(cp,&ep,10) - 1900;

      toy = mktime(&stoy);
      tver->ModVer[m-1] = toy;
   }
   tver->CorVer = toy;

   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Get specific information string                                      */
/* There is some very specific module dependent status information.     */
/* This routine returns a free format human readable string containing  */
/* specific status information that may help diagnosing problems for a  */
/* timing receiver module. A null pointer can be returned if the module */
/* is either dead or not installed.                                     */

/* Software status */

#define SSBITS 11

static char *sson[SSBITS] = {
   "Enb",        "FwLoad",     "AcLd",   "FwRun",
   "FwEr",       "HwEr",       "ExtClk", "TimeOut",
   "SelfTestIn", "SelfTestEr", "DpramEr"
 };

static char *ssof[SSBITS] = {
   "Dis",        "NoRply", "NoAc",     "",
   "",           "HwOK",   "10MhzClk", "",
   "NoSelfTest", "",       ""
 };

/* Xylinx status */

#define XSBITS 8

char *xson[XSBITS] = {
   "DtOverER", "ParityER", "EndSqER",       "MidBitER",
   "StrtSqER", "XrDisb",   "MSecMissingER", "WatchDogER"
 };

char *xsof[XSBITS] = {
   "", "", "", "", "", "XrEnab", "MSecOK", ""
 };

/* Hardware status */

#define HSBITS 16

char *hson[HSBITS] = {
   "IntCh1", "IntCh2", "IntCh3",       "IntCh4",
   "IntCh5", "IntCh6", "IntCh7",       "IntCh8",
   "IntDPm", "RecEnb", "WaitDownLoad", "",
   "",       "",       "ExClk",        "SelfTestER"
 };

char *hsof[HSBITS] = {
   "", "", "", "", "", "", "", "",
   "", "RecDis", "", "XRER", "X1ER", "X2ER", "IntClk", ""
 };

/* Alarm status */

#define ASBITS 16

char *ason[ASBITS] = {

   "",                         /* 01/0x01    Not used */
   "LostIntIA",                /* 02/0x02    The immediate action's interrupt was lost */

   /* This often simply means the start or clock was missing, it occurs */
   /* when the counter is reloaded before it has made an output. Hence  */
   /* this alarm often occurs in normal operation, and is not serious.  */

   "LostOutPt",                /* 03/0x04    The counter pulse was lost */

   "UserQFull",                /* 04/0x08    The UT processes queue is full */
   "OverWork",                 /* 05/0x10    Too many Machine/Group combinations */
   "TrainCut",                 /* 06/0x20    Train was terminated by the next trigger */
   "NoMemMltP",                /* 07/0x40    No space for multipulse list */

   /* The firmware did not have enough time to launch all actions and  */
   /* process all Aqn TPU interrups before the ms tick arrived. Usualy */
   /* not soo bad, its only the aqn that is wrong.                     */

   "OverLoad",                 /* 08/0x80    The UT process is not completed before the next ms */

   "ImActMovd",                /* 09/0x100   The immediate interrupts where moved */
   "XRecError",                /* 10/0x200   XILINX Receiver error detected */
   "ImActQFl",                 /* 11/0x400   Immediate actions queue overflowed */
   "MBoxBusy",                 /* 12/0x800   The mailbox busy, but there is the new request */
   "BadCntInt",                /* 13/0x1000  A counter gives the VME interrupt without the appropriate data in the Interrupt Table. */
   "DrvrSlowI",                /* 14/0x2000  Interrupt info was lost due to the driver is too busy */
   "BadSwitch",                /* 15/0x4000  The switch is in bad position */

   /* The firmware recalculates a checksum continuously to monitor    */
   /* itself, if the checksum check fails, the firmware is corrupted  */
   /* interrupts are switched off, and the mail box routines remain   */
   /* alive for memory examination purposes. Experts only. This next  */
   /* alarm thus is very bad, hardware or software design error.      */

   "BadChkSum",                /* 16/0x8000  Very bad ie 68332 Memory corrupted fatal */
 };

char *asof[ASBITS] = {
   "", "", "", "",
   "", "", "", "",
   "", "", "", "",
   "", "", "", ""
 };

/* ===================== */
/* Build a status string */
/* ===================== */

static char ststr[256];

char *BuildStStr (int st, char **on, char **of, int bits) {

int bn, bv;
char bs[16];

   bzero ((void *) ststr, 256);

   for (bn = 0; bn < bits; bn++) {
      bv = 1 << bn;
      if (st & bv)
	 sprintf (bs, "+%s ", on[bn]);
      else
	 sprintf (bs, "-%s ", of[bn]);
      strcat (ststr, bs);
   }
   return (ststr);
}

/* ==================================================================== */

char *Tg8CpsLibGetSpecificInfo(unsigned long module) { /* The given module */

#define RESLN 1024
#define TMPLN 512
static char res[RESLN];

Tg8DrvrRawStatus raw;
Tg8IoBlock iob;
unsigned long stat;
char tmp[TMPLN];

   if (module)
      if (ioctl(tg8cps, Tg8DrvrSET_DEFAULT_MODULE, &module) < 0)
	 return NULL;

   if (ioctl(tg8cps,Tg8DrvrGET_STATUS,&iob) < 0)
      return NULL;

   sprintf(res,"\nFirmware Parameters Module:%d\n",(int) module);

   stat = iob.Status.Status;
   BuildStStr (stat, sson, ssof, SSBITS);
   sprintf(tmp,"Software: %04X: %s\n", (int) stat, ststr);
   strcat(res,tmp);

   stat = iob.Status.Alarms;
   BuildStStr(stat, ason, asof, ASBITS);
   sprintf(tmp,"FwAlarms: %04X: %s\n", (int) stat, ststr);
   strcat(res,tmp);

   if (ioctl(tg8cps,Tg8DrvrGET_RAW_STATUS,&raw) < 0) return res;

   stat = raw.Sb.Hw;
   BuildStStr(stat, hson, hsof, HSBITS);
   sprintf(tmp,"Hardware: %04X: %s\n", (int) stat, ststr);
   strcat(res,tmp);

   stat = raw.Sb.Dt.aRcvErr;
   BuildStStr (stat, xson, xsof, XSBITS);
   sprintf(tmp,"XylnxRec: %04X: %s\n", (int) stat, ststr);
   strcat(res,tmp);

   stat = raw.Res.FaultType;
   if (stat) {
      sprintf(tmp,"SelfTest: %04X: Failed self test\n", (int) stat);
      strcat(res,tmp);
   }

   return res;
}

/* ==================================================================== */
/* Get the module statistics for the given module                       */

TimLibError Tg8CpsLibGetModuleStats(unsigned long module,
				    TimLibModuleStats *stats) {
   return TimLibErrorNOT_IMP;
}

/* ==================================================================== */
/* Control how the PLL locks after synchronization loss                 */

TimLibError Tg8CpsLibSetPllLocking(unsigned long module,
				   unsigned long lockflag) { /* 0=> Brutal, else Slow */
   return TimLibErrorNOT_IMP;
}

/* ==================================================================== */

TimLibError Tg8CpsLibConnectPayload(unsigned long ctim,        /* The CTIM ID you want to connect to */
				    unsigned long payload,     /* The 16 bit payload in a long */
				    unsigned long module) {    /* The module, or zero means don't care */
   return TimLibErrorNOT_IMP;
}

/* ==================================================================== */
/* If its not on a power PC, then there is no VME, and no TG8. So just  */
/* return Not Implemented everywhere!!                                  */
/* ==================================================================== */

#else
TimLibError Tg8CpsLibInitialize(TimLibDevice device) { return TimLibErrorNOT_IMP; }
int         Tg8CpsLibFdInitialize(TimLibDevice device) { return 0; }
TimLibError Tg8CpsLibConnect(TimLibClass iclss,
			     unsigned long equip,
			     unsigned long module) { return TimLibErrorNOT_IMP; }
TimLibError Tg8CpsLibFdConnect(int fd,
			       TimLibClass iclss,
			       unsigned long equip,
			       unsigned long module) { return TimLibErrorNOT_IMP; }
TimLibError Tg8CpsLibDisConnect(TimLibClass iclss,
				unsigned long equip,
				unsigned long module) { return TimLibErrorNOT_IMP; }
TimLibError Tg8CpsLibQueue(unsigned long qflag,
			   unsigned long tmout) { return TimLibErrorNOT_IMP; }
TimLibError Tg8CpsLibFdQueue(int fd,
			     unsigned long qflag,
			     unsigned long tmout) { return TimLibErrorNOT_IMP; }
unsigned long Tg8CpsLibGetQueueSize() { return 0; }
TimLibError Tg8CpsLibWait(TimLibClass * iclss,
			  unsigned long *equip,
			  unsigned long *plnum,
			  TimLibHardware * source,
			  TimLibTime * onzero,
			  TimLibTime * trigger,
			  TimLibTime * start,
			  unsigned long *ctim,
			  unsigned long *payload,
			  unsigned long *module,
			  unsigned long *missed,
			  unsigned long *qsize) { return TimLibErrorNOT_IMP; }
TimLibError Tg8CpsLibFdWait(int fd,
			    TimLibClass * iclss,
			    unsigned long *equip,
			    unsigned long *plnum,
			    TimLibHardware * source,
			    TimLibTime * onzero,
			    TimLibTime * trigger,
			    TimLibTime * start,
			    unsigned long *ctim,
			    unsigned long *payload,
			    unsigned long *module,
			    unsigned long *missed,
			    unsigned long *qsize) { return TimLibErrorNOT_IMP; }
TimLibError Tg8CpsLibSet(unsigned long ptim,
			 unsigned long plnum,
			 unsigned long grnum,
			 unsigned long grval,
			 TimLibCcvMask ccvm,
			 TimLibCcv * ccv) { return TimLibErrorNOT_IMP; }
TimLibError Tg8CpsLibGet(unsigned long ptim,
			 unsigned long plnum,
			 unsigned long grnum,
			 unsigned long grval,
			 TimLibCcvMask * ccvm,
			 TimLibCcv * ccv) { return TimLibErrorNOT_IMP; }
TimLibError Tg8CpsLibSimulate(TimLibClass iclss,
			      unsigned long equip,
			      unsigned long module,
			      TgmMachine machine,
			      unsigned long grnum,
			      unsigned long grval) { return TimLibErrorNOT_IMP; }
TimLibError Tg8CpsLibRemoteControl(unsigned long remflg,
				   unsigned long module,
				   unsigned long cntr,
				   TimLibRemote rcmd,
				   TimLibCcvMask ccvm,
				   TimLibCcv * ccv) { return TimLibErrorNOT_IMP; }
TimLibError Tg8CpsLibGetRemote(unsigned long module,
			       unsigned long cntr,
			       unsigned long *remflg,
			       TimLibCcvMask *ccvm,
			       TimLibCcv     *ccv) { return TimLibErrorNOT_IMP; }

TimLibError Tg8CpsLibGetTime(unsigned long module,
			     TimLibTime * utc) { return TimLibErrorNOT_IMP; }
TimLibError Tg8CpsLibGetTelegram(unsigned long module,
				 TgmMachine machine,
				 TgmTelegram * telegram) { return TimLibErrorNOT_IMP; }
TimLibError Tg8CpsLibGetPtimObject(unsigned long ptim,
				   unsigned long *module,
				   unsigned long *counter,
				   unsigned long *dimension) { return TimLibErrorNOT_IMP; }
TimLibError Tg8CpsLibGetCtimObject(unsigned long ctim,
				   unsigned long *eventcode) { return TimLibErrorNOT_IMP; }
TimLibError Tg8CpsLibGetHandle(int *fd) { return TimLibErrorNOT_IMP; }
TimLibError Tg8CpsLibCreatePtimObject(unsigned long ptim,
				      unsigned long module,
				      unsigned long counter,
				      unsigned long dimension) { return TimLibErrorNOT_IMP; }
TimLibError Tg8CpsLibCreateCtimObject(unsigned long ctim,
				      unsigned long eventcode) { return TimLibErrorNOT_IMP; }
unsigned long Tg8CpsLibGetInstalledModuleCount() { return 0; }
TimLibError Tg8CpsLibGetCableId(unsigned long module,
				unsigned long *cable) { return TimLibErrorNOT_IMP; }

TimLibError Tg8CpsLibGetAllPtimObjects(unsigned long *ptimlist,  /* List of ptim equipments */
				    unsigned long *ptims,     /* Number of ptims in list */
				    unsigned long size) {     /* Max size of list */

   return TimLibErrorNOT_IMP;
}

TimLibError Tg8CpsLibGetAllCtimObjects(unsigned long *ctimlist,  /* List of ctim equipments */
				    unsigned long *ctims,     /* Number of ctims in list */
				    unsigned long size) {     /* Max size of list */

   return TimLibErrorNOT_IMP;
}

TimLibStatus Tg8CpsLibGetStatus(unsigned long module, TimLibDevice *dev) {

   return TimLibErrorNOT_IMP;
}

TimLibError Tg8CpsLibGetModuleVersion(TimLibModuleVersion *version) {

   return TimLibErrorNOT_IMP;
}

char *Tg8CpsLibGetSpecificInfo(unsigned long module) { /* The given module */

   return NULL;
}

TimLibError Tg8CpsLibGetModuleStats(unsigned long module,
				    TimLibModuleStats *stats) {
   return TimLibErrorNOT_IMP;
}

TimLibError Tg8CpsLibSetPllLocking(unsigned long module,
				   unsigned long lockflag) { /* 0=> Brutal, else Slow */
   return TimLibErrorNOT_IMP;
}

TimLibError Tg8CpsLibConnectPayload(unsigned long ctim,        /* The CTIM ID you want to connect to */
				    unsigned long payload,     /* The 16 bit payload in a long */
				    unsigned long module) {    /* The module, or zero means don't care */
   return TimLibErrorNOT_IMP;
}
#endif
