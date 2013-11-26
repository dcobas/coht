/* ==================================================================== */
/* Implement the timing library over the CTR familly of timing receiver */
/* cards, PMC, PCI, and VME.                                            */
/* Julian Lewis May 2004                                                */
/* ==================================================================== */

#define CTR_VME

#include <sys/ipc.h>
#include <sys/sem.h>

#include <ctrdrvr.h>
#include <errno.h>

#include <errno.h>
extern int errno;

extern double round(double x);

extern int timlib_debug;
extern int timlib_delay;
extern int timlib_enable;
extern int timlib_jtag;
extern int timlib_oby1_8;
extern int timlib_oby9_16;
extern int timlib_real_utc;

/**
 * Local routines needed to protect open/close
 */

#define LOCK_LIB_KEY 13426587

static int lock_lib()
{
	key_t key;
	int semid;

	struct sembuf sops[2];
	int nsops, cc;

	key = LOCK_LIB_KEY;

	semid = semget(key, 1, 0666);
	if (semid < 0) {

		semid = semget(key, 1, 0666 | IPC_CREAT);
		if (semid < 0)
			return -1;

		sops[0].sem_num = 0;
		sops[0].sem_op = 0;
		sops[0].sem_flg = SEM_UNDO;

		cc = semop(semid, sops, 1);
		if (cc < 0) {
			semctl(semid,0,IPC_RMID);
			return -1;
		}
	}

	nsops = 2;

	sops[0].sem_num = 0;
	sops[0].sem_op = 0;
	sops[0].sem_flg = SEM_UNDO;

	sops[1].sem_num = 0;
	sops[1].sem_op = 1;
	sops[1].sem_flg = SEM_UNDO | IPC_NOWAIT;

	cc = semop(semid, sops, nsops);
	if (cc < 0)
		return cc;

	return 0;
}

static int unlock_lib()
{
	key_t key;
	int semid;

	struct sembuf sops[2];
	int nsops, cc;

	key = LOCK_LIB_KEY;

	semid = semget(key, 1, 0666);
	if (semid <= 0)
		return -1;

	nsops = 1;

	sops[0].sem_num = 0;
	sops[0].sem_op = -1;
	sops[0].sem_flg = SEM_UNDO | IPC_NOWAIT;

	cc = semop(semid, sops, nsops);
	if (cc < 0)
		return cc;

	return 0;
}

/* ==================================================================== */
/* Open a Ctr Driver file handel.                                       */

CtrDrvrDevice ctr_device = CtrDrvrDeviceANY;
char *devnames[CtrDrvrDEVICES] = {"ctr", "ctri", "ctrp", "ctrv", "ctre" };

static int ctr = 0; /* This global holds the CTR Driver file handle */

static int CtrOpen() {

char fnm[32];
int  i;

   if (ctr) return ctr;
   lock_lib();
   for (i = 1; i <= CtrDrvrCLIENT_CONTEXTS; i++) {
      sprintf(fnm,"/dev/%s.%1d",devnames[ctr_device],i);
      if ((ctr = open(fnm,O_RDWR,0)) > 0) break;
   }
   unlock_lib();
   if (ctr < 0) ctr = 0;
   return ctr;
}

/* ========= */

static int CtrFdOpen() {

char fnm[32];
int  i, fd;

   lock_lib();
   for (i = 1; i <= CtrDrvrCLIENT_CONTEXTS; i++) {
      sprintf(fnm,"/dev/%s.%1d",devnames[ctr_device],i);
      if ((fd = open(fnm,O_RDWR,0)) > 0) return fd;
   }
   unlock_lib();
   if (ctr < 0) ctr = 0;
   return ctr;
}

/* ==================================================================== */
/* Convert HPTDC ticks to nano seconds.                                 */

static unsigned long CtrHptdcToNano(unsigned long hptdc) {
double fns;
unsigned long ins;

   fns = ((double) hptdc) / 2.56;
   ins = (unsigned long) fns;
   return ins;
}

/* ==================================================================== */
/* Lets you know how many installed modules there are on this host.     */

unsigned long CtrLibGetInstalledModuleCount() {

uint32_t cnt;

   if (ctr == 0) return 0;
   if (ioctl(ctr,CtrIoctlGET_MODULE_COUNT,&cnt) < 0) return 0;
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

TimLibError CtrLibInitialize(TimLibDevice device) { /* Initialize hardware/software */
char cmd[64];
int mods;
unsigned long p2c, oby; /* VME P2Connector output byte */
uint32_t m, debug, delay, enable;

   if ((device == TimLibDevice_ANY)
   ||  (device == TimLibDevice_CTR)) {
      if (!ctr) {
	 if (CtrOpen() == 0) {
	    ctr = 0;
	    return TimLibErrorOPEN;
	 }
      }

      if (timlib_jtag) {
	 sprintf(cmd,"echo \"(Xjtag) (load 1) q\" | /usr/local/bin/ctrtest");
	 fprintf(stdout,"CtrLibInitialize:Execute:%s\n",cmd);
	 system(cmd);
      }
      mods = CtrLibGetInstalledModuleCount();
      for (m=1; m<=mods; m++) {
	 ioctl(ctr,CtrIoctlSET_MODULE,&m);

	 debug  = timlib_debug;
	 delay  = timlib_delay;
	 enable = timlib_enable;

	 if (timlib_debug)  ioctl(ctr,CtrIoctlSET_SW_DEBUG,   &debug);
	 if (timlib_delay)  ioctl(ctr,CtrIoctlSET_INPUT_DELAY,&delay);
	 if (timlib_enable) ioctl(ctr,CtrIoctlENABLE,         &enable);

	 if (m == 1) p2c = timlib_oby1_8;
	 if (m == 9) p2c = timlib_oby9_16;
	 oby = 0xF & p2c;
	 if (oby) {
	    sprintf(cmd,"echo \"(mo %d) (oby %d) q\" | /usr/local/bin/ctrvtest",m,(int) oby);
	    fprintf(stdout,"CtrLibInitialize:Execute:%s\n",cmd);
	    system(cmd);
	 }
	 p2c = p2c >> 4;

      }
      m = 1;
      ioctl(ctr,CtrIoctlSET_MODULE,&m);

      return TimLibErrorSUCCESS;
   }
   return TimLibErrorINIT;
}

/* ====== */

int CtrLibFdInitialize(TimLibDevice device) { /* Initialize hardware/software */

TimLibError er;

   if (ctr == 0) {
      er = CtrLibInitialize(device);
      if (er != TimLibErrorSUCCESS) return 0;
      return ctr;
   }
   return CtrFdOpen();
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

static int ctr_connected = 0;

TimLibError CtrLibConnect(TimLibClass   iclss,    /* Class of interrupt */
			  unsigned long equip,    /* Equipment or hardware mask */
			  unsigned long module) { /* For HARD or CTIM classes */

CtrDrvrConnection con;

   if (ctr == 0) return TimLibErrorINIT;

   con.EqpClass = (CtrDrvrConnectionClass) iclss;
   con.EqpNum   = equip;
   con.Module   = module;

   if (con.EqpNum == 0) {
      if (ioctl(ctr,CtrIoctlDISCONNECT,&con) < 0) return TimLibErrorIO;
      ctr_connected = 0;
      return TimLibErrorSUCCESS;
   }
   if (ioctl(ctr,CtrIoctlCONNECT,&con) < 0) return TimLibErrorCONNECT;

   ctr_connected++;

   return TimLibErrorSUCCESS;
}

/* ====== */

TimLibError CtrLibFdConnect(int           fd,       /* File descriptor */
			    TimLibClass   iclss,    /* Class of interrupt */
			    unsigned long equip,    /* Equipment or hardware mask */
			    unsigned long module) { /* For HARD or CTIM classes */

CtrDrvrConnection con;

   if (fd == 0) return TimLibErrorINIT;

   con.EqpClass = (CtrDrvrConnectionClass) iclss;
   con.EqpNum   = equip;
   con.Module   = module;

   if (con.EqpNum == 0) {
      if (ioctl(fd,CtrDrvrDISCONNECT,&con) < 0) return TimLibErrorIO;
      ctr_connected = 0;
      return TimLibErrorSUCCESS;
   }
   if (ioctl(fd,CtrDrvrCONNECT,&con) < 0) return TimLibErrorCONNECT;

   return TimLibErrorSUCCESS;
}

/* =============================================== */

TimLibError CtrLibConnectPayload(unsigned long ctim,        /* The CTIM ID you want to connect to */
				 unsigned long payload,     /* The 16 bit payload in a long */
				 unsigned long module) {    /* The module, or zero means don't care */
CtrDrvrAction act;
CtrDrvrCtimObjects ctimo;
CtrDrvrTrigger *trg;
TimLibError ter;
int i ,j;

   trg = &(act.Trigger);

   if (module) {
      uint32_t m = module;
      if (ioctl(ctr,CtrIoctlSET_MODULE,&m) < 0)
	 return TimLibErrorMODULE;
   }
   if (ioctl(ctr,CtrIoctlLIST_CTIM_OBJECTS,&ctimo) < 0) return TimLibErrorIO;
   for (i=0; i<ctimo.Size; i++) {
      if (ctimo.Objects[i].EqpNum == ctim) {

	 ter = CtrLibConnect(TimLibClassCTIM,ctim,module);
	 if (ter != TimLibErrorSUCCESS) return ter;

	 for (j=1; j<=CtrDrvrRamTableSIZE; j++) {
	    act.TriggerNumber = j;
	    if (ioctl(ctr,CtrIoctlGET_ACTION,&act) < 0) return TimLibErrorIO;

	    if (act.EqpClass == CtrDrvrConnectionClassCTIM) {
	       if (trg->Ctim == ctim) {
		  trg->Frame.Long = ((ctimo.Objects[i].Frame.Long & 0xFFFF0000)
				  |  (payload & 0x0000FFFF));

		  if (ioctl(ctr,CtrIoctlSET_ACTION,&act) < 0) return TimLibErrorIO;
		  else                                       return TimLibErrorSUCCESS;
	       }
	    }
	    else if (act.EqpClass == CtrDrvrConnectionClassPTIM) continue;
	    else                                                 break;
	 }
	 break;
      }
   }
   return TimLibErrorCTIM;
}

/* ==================================================================== */
/* Disconnect from an interrupt                                         */

TimLibError CtrLibDisConnect(TimLibClass   iclss,    /* Class of interrupt */
			     unsigned long equip,    /* Equipment or hardware mask */
			     unsigned long module) { /* For HARD or CTIM classes */

CtrDrvrConnection con;

   if (ctr == 0) return TimLibErrorINIT;

   con.EqpClass = (CtrDrvrConnectionClass) iclss;
   con.EqpNum   = equip;
   con.Module   = module;

   if (ioctl(ctr,CtrIoctlDISCONNECT,&con) < 0) return TimLibErrorCONNECT;

   if (con.EqpNum == 0)   ctr_connected = 0;
   if (ctr_connected > 0) ctr_connected--;

   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Set queueing On or Off, and the time out value in micro seconds.     */
/* A timeout value of zero means no time out, you wait for ever.        */

TimLibError CtrLibQueue(unsigned long qflag,    /* 0=>Queue, 1=>NoQueue  */
			unsigned long tmout) {  /* 0=>No time outs       */

   uint32_t qf = qflag;
   uint32_t tm = tmout;

   if (ctr == 0) return TimLibErrorINIT;

   if (ioctl(ctr,CtrIoctlSET_TIMEOUT, &tm)    < 0) return TimLibErrorIO;
   if (ioctl(ctr,CtrIoctlSET_QUEUE_FLAG, &qf) < 0) return TimLibErrorIO;

   return TimLibErrorSUCCESS;
}

/* ====== */

TimLibError CtrLibFdQueue(int           fd,      /* File descriptor       */
			  unsigned long qflag,   /* 0=>Queue, 1=>NoQueue  */
			  unsigned long tmout) { /* 0=>No time outs       */

   uint32_t tm = tmout;
   uint32_t qf = qflag;

   if (fd == 0) return TimLibErrorINIT;

   if (ioctl(fd,CtrDrvrSET_TIMEOUT,&tm)    < 0) return TimLibErrorIO;
   if (ioctl(fd,CtrDrvrSET_QUEUE_FLAG,&qf) < 0) return TimLibErrorIO;

   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* To know if a call to wait will block, this call returns the Queue    */
/* size. If the size iz greater than zero a call to wait will not block */
/* and return without waiting. If the qflag is set to NoQueue, zero is  */
/* allways returned and all calls to wait will block.                   */

unsigned long CtrLibGetQueueSize() {

   uint32_t qflag;
   uint32_t qsize;

   if (ioctl(ctr,CtrIoctlGET_QUEUE_FLAG,&qflag) < 0) return 0;
   if (qflag) return 0;

   if (ioctl(ctr,CtrIoctlGET_QUEUE_SIZE,&qsize) < 0) return 0;

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

TimLibError CtrLibWait(TimLibClass    *iclss,      /* Class of interrupt */
		       unsigned long  *equip,      /* PTIM CTIM or hardware mask */
		       unsigned long  *plnum,      /* Ptim line number 1..n or 0 */
		       TimLibHardware *source,     /* Hardware source of interrupt */
		       TimLibTime     *onzero,     /* Time of interrupt/output */
		       TimLibTime     *trigger,    /* Time of counters load */
		       TimLibTime     *start,      /* Time of counters start */
		       unsigned long  *ctim,       /* CTIM trigger equipment ID */
		       unsigned long  *payload,    /* Payload of trigger event */
		       unsigned long  *module,     /* Module that interrupted */
		       unsigned long  *missed,     /* Number of missed interrupts */
		       unsigned long  *qsize,      /* Remaining interrupts on queue */
		       TgmMachine     *machine) {  /* Corresponding TgmMachine */

static uint32_t    cbl = 0;	/* FIXME: this is not reentrant! */
CtrDrvrReadBuf     rbf;
CtrDrvrPtimBinding ob;
CtrDrvrAction      act;
TgmMachine         mch;

   uint32_t miss, qs;

   if (ctr == 0) return TimLibErrorINIT;
   if (ctr_connected == 0) return TimLibErrorWAIT;

   while (1) {
      if (read(ctr,&rbf,sizeof(CtrDrvrReadBuf)) <= 0) return TimLibErrorTIMEOUT;
      if (rbf.Connection.EqpClass == CtrDrvrConnectionClassPTIM) {
         uint32_t m = rbf.Connection.Module;
	 ioctl(ctr,CtrIoctlSET_MODULE,&m);
	 act.TriggerNumber = rbf.TriggerNumber;
	 if (ioctl(ctr,CtrIoctlGET_ACTION,&act) < 0) return TimLibErrorIO;
	 if ((act.Config.OnZero & CtrDrvrCounterOnZeroOUT) == 0) continue;
      }
      break;
   }
   
   if (cbl == 0) ioctl(ctr,CtrIoctlGET_CABLE_ID,&cbl);
   mch = TgvTgvToTgmMachine(TgvFirstMachineForCableId(cbl));

   if (iclss)   *iclss   = rbf.Connection.EqpClass;
   if (equip)   *equip   = rbf.Connection.EqpNum;
   if (source)  *source  = rbf.InterruptNumber;
   if (module)  *module  = rbf.Connection.Module;
   if (machine) {
      if (*iclss != TimLibClassHARDWARE) *machine = TgvTgvToTgmMachine(TgvGetMachineForMember(rbf.Ctim));
      else                               *machine = mch;
   }
   if (ctim)    *ctim    = rbf.Ctim;

   if (payload) *payload = rbf.Frame.Struct.Value;

   if (plnum) {
      if (rbf.Connection.EqpClass == CtrDrvrConnectionClassPTIM) {
	 ob.EqpNum = rbf.Connection.EqpNum;
	 ioctl(ctr,CtrIoctlGET_PTIM_BINDING,&ob);
	 *plnum  = rbf.TriggerNumber - ob.StartIndex;
      } else *plnum = 0;
   }
   if (missed) {
	ioctl(ctr,CtrIoctlGET_QUEUE_OVERFLOW,&miss);
	*missed = miss;
   }
   if (qsize) {
	ioctl(ctr,CtrIoctlGET_QUEUE_SIZE,&qs);
	*qsize = qs;
   }

   if (onzero) {
      onzero->Machine = mch;
      onzero->CTrain  = rbf.OnZeroTime.CTrain;
      onzero->Second  = rbf.OnZeroTime.Time.Second;
      onzero->Nano    = CtrHptdcToNano(rbf.OnZeroTime.Time.TicksHPTDC);
   }

   if (trigger) {
      trigger->Machine = mch;
      trigger->CTrain  = rbf.TriggerTime.CTrain;
      trigger->Second  = rbf.TriggerTime.Time.Second;
      trigger->Nano    = CtrHptdcToNano(rbf.TriggerTime.Time.TicksHPTDC);
   }
   if (start) {
      start->Machine = mch;
      start->CTrain  = rbf.StartTime.CTrain;
      start->Second  = rbf.StartTime.Time.Second;
      start->Nano    = CtrHptdcToNano(rbf.StartTime.Time.TicksHPTDC);
   }
   return TimLibErrorSUCCESS;
}

/* ====== */

TimLibError CtrLibFdWait(int            fd,          /* File descriptor */
			 TimLibClass    *iclss,      /* Class of interrupt */
			 unsigned long  *equip,      /* PTIM CTIM or hardware mask */
			 unsigned long  *plnum,      /* Ptim line number 1..n or 0 */
			 TimLibHardware *source,     /* Hardware source of interrupt */
			 TimLibTime     *onzero,     /* Time of interrupt/output */
			 TimLibTime     *trigger,    /* Time of counters load */
			 TimLibTime     *start,      /* Time of counters start */
			 unsigned long  *ctim,       /* CTIM trigger equipment ID */
			 unsigned long  *payload,    /* Payload of trigger event */
			 unsigned long  *module,     /* Module that interrupted */
			 unsigned long  *missed,     /* Number of missed interrupts */
			 unsigned long  *qsize,      /* Remaining interrupts on queue */
			 TgmMachine     *machine) {  /* Corresponding TgmMachine */

static uint32_t    cbl = 0;	/* FIXME: this is not reentrant */
CtrDrvrReadBuf     rbf;
CtrDrvrPtimBinding ob;
CtrDrvrAction      act;
TgmMachine         mch;

   uint32_t miss, qs;

   if (fd == 0) return TimLibErrorINIT;

   while (1) {
      if (read(fd,&rbf,sizeof(CtrDrvrReadBuf)) <= 0) return TimLibErrorTIMEOUT;
      if (rbf.Connection.EqpClass == CtrDrvrConnectionClassPTIM) {
         uint32_t m = rbf.Connection.Module;
	 ioctl(fd,CtrDrvrSET_MODULE,&m);
	 act.TriggerNumber = rbf.TriggerNumber;
	 if (ioctl(fd,CtrDrvrGET_ACTION,&act) < 0) return TimLibErrorIO;
	 if ((act.Config.OnZero & CtrDrvrCounterOnZeroOUT) == 0) continue;
      }
      break;
   }
   
   if (cbl == 0) ioctl(fd,CtrDrvrGET_CABLE_ID,&cbl);
   mch = TgvTgvToTgmMachine(TgvFirstMachineForCableId(cbl));

   if (iclss)   *iclss   = rbf.Connection.EqpClass;
   if (equip)   *equip   = rbf.Connection.EqpNum;
   if (source)  *source  = rbf.InterruptNumber;
   if (module)  *module  = rbf.Connection.Module;
   if (machine) {
      if (*iclss != TimLibClassHARDWARE) *machine = TgvTgvToTgmMachine(TgvGetMachineForMember(rbf.Ctim));
      else                               *machine = mch;
   }
   if (ctim)    *ctim    = rbf.Ctim;

   if (payload) *payload = rbf.Frame.Struct.Value;

   if (plnum) {
      if (rbf.Connection.EqpClass == CtrDrvrConnectionClassPTIM) {
	 ob.EqpNum = rbf.Connection.EqpNum;
	 ioctl(fd,CtrDrvrGET_PTIM_BINDING,&ob);
	 *plnum  = rbf.TriggerNumber - ob.StartIndex;
      } else *plnum = 0;
   }
   if (missed) {
	ioctl(fd,CtrDrvrGET_QUEUE_OVERFLOW,&miss);
	*missed = miss;
   }
   if (qsize) {
	ioctl(fd,CtrDrvrGET_QUEUE_SIZE,&qs);
	*qsize = qs;
   }

   if (onzero) {
      onzero->Machine = mch;
      onzero->CTrain  = rbf.OnZeroTime.CTrain;
      onzero->Second  = rbf.OnZeroTime.Time.Second;
      onzero->Nano    = CtrHptdcToNano(rbf.OnZeroTime.Time.TicksHPTDC);
   }

   if (trigger) {
      trigger->Machine = mch;
      trigger->CTrain  = rbf.TriggerTime.CTrain;
      trigger->Second  = rbf.TriggerTime.Time.Second;
      trigger->Nano    = CtrHptdcToNano(rbf.TriggerTime.Time.TicksHPTDC);
   }
   if (start) {
      start->Machine = mch;
      start->CTrain  = rbf.StartTime.CTrain;
      start->Second  = rbf.StartTime.Time.Second;
      start->Nano    = CtrHptdcToNano(rbf.StartTime.Time.TicksHPTDC);
   }
   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Set the Ccv of a PTIM equipment. Note neither the counter number nor */
/* the trigger condition can be changed.                                */

TimLibError CtrLibSet(unsigned long ptim,    /* PTIM to write to */
		      unsigned long plnum,   /* Ptim line number 1..n or 0 */
		      unsigned long grnum,   /* Tgm group number or Zero */
		      unsigned long grval,   /* Group value if num not zero */
		      TimLibCcvMask ccvm,    /* Which values to write */
		      TimLibCcv     *ccv) {  /* Current control value */

int                          anum, msk, i, found;
uint32_t		     module;
CtrDrvrAction                act;
CtrDrvrTrigger              *trg;
CtrDrvrCounterConfiguration *cnf;
CtrDrvrTgmGroup             *grp;
CtrDrvrPtimBinding           ob;
CtrDrvrCounterMaskBuf        cmsb;
TgmGroupDescriptor           desc;
CtrDrvrCtimObjects           ctimo;

   if (ctr == 0) return TimLibErrorINIT;

   ob.EqpNum = ptim;
   if (ioctl(ctr,CtrIoctlGET_PTIM_BINDING,&ob) < 0) return TimLibErrorPTIM;
   anum = ob.StartIndex +1;

   module = ob.ModuleIndex +1;
   ioctl(ctr,CtrIoctlSET_MODULE,&module);

   trg = &(act.Trigger);
   cnf = &(act.Config);
   grp = &(trg->Group);

   act.TriggerNumber = anum;
   if (ioctl(ctr,CtrIoctlGET_ACTION,&act) < 0) return TimLibErrorIO;

   if (plnum) {
      anum += plnum -1;
      act.TriggerNumber = anum;

      if (anum > ob.StartIndex + ob.Size) return TimLibErrorGROUP;

      if (ioctl(ctr,CtrIoctlGET_ACTION,&act) < 0) return TimLibErrorIO;

   } else if (grnum) {
      do {
	 if ((trg->Group.GroupNumber == grnum)
	 &&  (trg->Group.GroupValue  == grval)) break;

	 if (anum++ >= ob.StartIndex + ob.Size) return TimLibErrorGROUP;

	 act.TriggerNumber = anum;
	 if (ioctl(ctr,CtrIoctlGET_ACTION,&act) < 0) return TimLibErrorGROUP;
      } while (1);
   }
   trg->Counter = ob.Counter;

   msk = 1;
   do {
      if (ccvm & msk) {
	 switch ((TimLibCcvMask) msk) {
	    case TimLibCcvMaskENABLE:
	       if (ccv->Enable & TimLibEnableOUT) cnf->OnZero |= CtrDrvrCounterOnZeroOUT;
	       else                               cnf->OnZero &= ~CtrDrvrCounterOnZeroOUT;
	       break;

	    case TimLibCcvMaskSTART:
	       if (ccv->Start >= TimLibSTARTS) return TimLibErrorSTART;
	       cnf->Start = ccv->Start;
	       break;

	    case TimLibCcvMaskMODE:
	       if (ccv->Mode >= TimLibMODES) return TimLibErrorMODE;
	       cnf->Mode = ccv->Mode;
	       break;

	    case TimLibCcvMaskCLOCK:
	       if (ccv->Clock >= TimLibCLOCKS) return TimLibErrorCLOCK;
	       cnf->Clock = ccv->Clock;
	       break;

	    case TimLibCcvMaskPWIDTH:
	       if ((ccv->PulsWidth < 0)
	       ||  (ccv->PulsWidth > CtrDrvrCounterConfigPULSE_WIDTH_MASK))
		  return TimLibErrorPWIDTH;
	       cnf->PulsWidth = ccv->PulsWidth;
	       break;

	    case TimLibCcvMaskDELAY:
	       cnf->Delay = ccv->Delay;
	       break;

	    case TimLibCcvMaskOMASK:
	       cmsb.Counter = ob.Counter;
	       if (ioctl(ctr,CtrIoctlGET_OUT_MASK,&cmsb) < 0) return TimLibErrorIO;
	       cmsb.Mask = ccv->OutputMask;
	       if (ioctl(ctr,CtrIoctlSET_OUT_MASK,&cmsb) < 0) return TimLibErrorIO;
	       break;

	    case TimLibCcvMaskPOLARITY:
	       cmsb.Counter = ob.Counter;
	       if (ioctl(ctr,CtrIoctlGET_OUT_MASK,&cmsb) < 0) return TimLibErrorIO;
	       cmsb.Polarity = ccv->Polarity;
	       if (ioctl(ctr,CtrIoctlSET_OUT_MASK,&cmsb) < 0) return TimLibErrorIO;
	       break;

	    case TimLibCcvMaskCTIM:
	       found = 0;
	       if (ioctl(ctr,CtrIoctlLIST_CTIM_OBJECTS,&ctimo) < 0) return TimLibErrorIO;
	       if (ccv->Ctim == 0) {
		  if (ctimo.Size) {
		     trg->Ctim  = ctimo.Objects[0].EqpNum;
		     trg->Frame = ctimo.Objects[0].Frame;
		  } else {
		     trg->Ctim       = 100;
		     trg->Frame.Long = 0x34fe0000;
		  }
		  break;
	       }
	       for (i=0; i<ctimo.Size; i++) {
		  if (ctimo.Objects[i].EqpNum == ccv->Ctim) {
		     trg->Ctim  = ccv->Ctim;
		     trg->Frame = ctimo.Objects[i].Frame;
		     found = 1;
		     break;
		  }
	       }
	       if (!found) return TimLibErrorCTIM;
	       break;

	    case TimLibCcvMaskPAYLOAD:
	       trg->Frame.Struct.Value = ccv->Payload;
	       break;

	    case TimLibCcvMaskMACHINE:
	       trg->Machine = TgvTgmToTgvMachine(ccv->Machine);
	       break;

	    case TimLibCcvMaskGRNUM:
	       grp->GroupNumber = ccv->GrNum;
	       trg->TriggerCondition = CtrDrvrTriggerConditionNO_CHECK;
	       if (ccv->GrNum) {
		  if (TgmGetGroupDescriptor(TgvTgvToTgmMachine(trg->Machine),ccv->GrNum,&desc) != TgmSUCCESS)
		     return TimLibErrorGROUP;
		  if (desc.Type == TgmEXCLUSIVE)
		     trg->TriggerCondition = CtrDrvrTriggerConditionEQUALITY;
		  else if (desc.Type == TgmBIT_PATTERN)
		     trg->TriggerCondition = CtrDrvrTriggerConditionAND;
	       }
	       break;

	    case TimLibCcvMaskGRVAL:
	       grp->GroupValue = ccv->GrVal;
	    break;

	    default: break;
	 }
      }
      msk <<= 1;
   } while (msk & TimLibCcvMaskBITS);

   act.TriggerNumber = anum;
   if (ioctl(ctr,CtrIoctlSET_ACTION,&act) < 0) return TimLibErrorIO;

   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Get the Ccv of a PTIM equipment.                                     */

TimLibError CtrLibGet(unsigned long ptim,
		      unsigned long plnum,   /* Ptim line number 1..n or 0 */
		      unsigned long grnum,
		      unsigned long grval,
		      TimLibCcvMask *ccvm,  /* Valid fields in ccv */
		      TimLibCcv     *ccv) {

int                          anum;
uint32_t		     module;
CtrDrvrAction                act;
CtrDrvrTrigger              *trg;
CtrDrvrCounterConfiguration *cnf;
CtrDrvrTgmGroup             *grp;
CtrDrvrPtimBinding           ob;
CtrDrvrCounterMaskBuf        cmsb;

   if (ctr == 0) return TimLibErrorINIT;

   ob.EqpNum = ptim;
   if (ioctl(ctr,CtrIoctlGET_PTIM_BINDING,&ob) < 0) return TimLibErrorPTIM;
   anum = ob.StartIndex +1;

   module = ob.ModuleIndex +1;
   ioctl(ctr,CtrIoctlSET_MODULE,&module);

   act.TriggerNumber = anum;
   trg = &(act.Trigger);
   cnf = &(act.Config);
   grp = &(trg->Group);

   if (ioctl(ctr,CtrIoctlGET_ACTION,&act) < 0) return TimLibErrorIO;

   /* If the Tgm group number is not zero, search for the PTIM action */

   if (plnum) {
      anum += plnum -1;
      act.TriggerNumber = anum;

      if (anum > ob.StartIndex + ob.Size) return TimLibErrorGROUP;

      if (ioctl(ctr,CtrIoctlGET_ACTION,&act) < 0) return TimLibErrorIO;

   } else if (grnum) {
      do {
	 if ((trg->Group.GroupNumber == grnum)
	 &&  (trg->Group.GroupValue  == grval)) break;

	 if (anum++ >= ob.StartIndex + ob.Size) return TimLibErrorGROUP;

	 act.TriggerNumber = anum;
	 if (ioctl(ctr,CtrIoctlGET_ACTION,&act) < 0) return TimLibErrorIO;
      } while (1);
   }

   ccv->Enable = TimLibEnableNOOUT;
   if (cnf->OnZero & CtrDrvrCounterOnZeroOUT) ccv->Enable |= TimLibEnableOUT;
   if (cnf->OnZero & CtrDrvrCounterOnZeroBUS) ccv->Enable |= TimLibEnableBUS;

   *ccvm          = TimLibCcvMaskBITS;
   ccv->Start     = cnf->Start;
   ccv->Mode      = cnf->Mode;
   ccv->Clock     = cnf->Clock;
   ccv->PulsWidth = cnf->PulsWidth;
   ccv->Delay     = cnf->Delay;
   ccv->Ctim      = trg->Ctim;
   ccv->Payload   = trg->Frame.Struct.Value & 0xFFFF;
   ccv->Machine   = TgvTgvToTgmMachine(trg->Machine);
   if (trg->TriggerCondition == CtrDrvrTriggerConditionNO_CHECK) ccv->GrNum = 0;
   else                                                          ccv->GrNum = grp->GroupNumber;
   ccv->GrVal     = grp->GroupValue;

   cmsb.Counter = ob.Counter;
   if (ioctl(ctr,CtrIoctlGET_OUT_MASK,&cmsb) < 0) return TimLibErrorIO;

   ccv->OutputMask = cmsb.Mask;
   ccv->Polarity   = cmsb.Polarity;

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

TimLibError CtrLibSimulate(TimLibClass   iclss,
			     unsigned long equip,
			     unsigned long module,
			     TgmMachine    machine,
			     unsigned long grnum,
			     unsigned long grval) {

int                          cc, anum;
CtrDrvrPtimBinding           ob;
CtrDrvrWriteBuf              wbf;
CtrDrvrAction                act;
CtrDrvrTrigger              *trg;
CtrDrvrCounterConfiguration *cnf;
CtrDrvrTgmGroup             *grp;

   if (ctr == 0) return TimLibErrorINIT;

   trg = &(act.Trigger);
   cnf = &(act.Config);
   grp = &(trg->Group);

   switch (iclss) {

      case CtrDrvrConnectionClassHARD:
      case CtrDrvrConnectionClassCTIM:
	 wbf.TriggerNumber       = 0;
	 wbf.Connection.Module   = module;
	 wbf.Connection.EqpNum   = equip;
	 wbf.Connection.EqpClass = iclss;
	 wbf.Payload             = grval; /* CTIM payload */
	 cc = write(ctr,&wbf,sizeof(CtrDrvrWriteBuf));
	 if (cc > 0) return TimLibErrorSUCCESS;
      break;

      case CtrDrvrConnectionClassPTIM:
	 ob.EqpNum = equip;
	 if (ioctl(ctr,CtrIoctlGET_PTIM_BINDING,&ob) < 0) return TimLibErrorPTIM;

	 if (grnum) anum = grval + ob.StartIndex -1; else anum = 0;

	 wbf.TriggerNumber       = anum;
	 wbf.Connection.Module   = ob.ModuleIndex + 1;
	 wbf.Connection.EqpNum   = ob.EqpNum;
	 wbf.Connection.EqpClass = CtrDrvrConnectionClassPTIM;

	 cc = write(ctr,&wbf,sizeof(CtrDrvrWriteBuf));
	 if (cc > 0) return TimLibErrorSUCCESS;
      break;

      default: break;
   }
   return TimLibErrorIO;
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

TimLibError CtrLibRemoteControl(unsigned long remflg, /* 0 = Normal, 1 = Remote */
				unsigned long module, /* The module or zero */
				unsigned long cntr,   /* 1..8 counter number */
				TimLibRemote  rcmd,   /* Command */
				TimLibCcvMask ccvm,   /* Fields to be set */
				TimLibCcv     *ccv) { /* Value to load in counter */

unsigned long                  msk;
CtrDrvrCounterConfiguration    *cnf;
CtrDrvrCounterConfigurationBuf cnfb;
CtrdrvrRemoteCommandBuf        crmb;
CtrDrvrCounterMaskBuf          cmbf;

   uint32_t m = module;

   if (ctr == 0) return TimLibErrorINIT;

   ioctl(ctr,CtrIoctlSET_MODULE,&m);

   crmb.Remote = remflg;
   crmb.Counter = cntr;

   if (ioctl(ctr,CtrIoctlSET_REMOTE,&crmb) < 0) return TimLibErrorIO;

   msk = ccvm & ~TimLibCcvMaskOMASK;
   if (ccvm & TimLibCcvMaskBITS) {
      cnfb.Counter = cntr;
      if (ioctl(ctr,CtrIoctlGET_CONFIG,&cnfb) < 0) return TimLibErrorIO;
      cnf = &cnfb.Config;
      if (ccvm & TimLibCcvMaskSTART ) cnf->Start     = ccv->Start;
      if (ccvm & TimLibCcvMaskMODE  ) cnf->Mode      = ccv->Mode;
      if (ccvm & TimLibCcvMaskCLOCK ) cnf->Clock     = ccv->Clock;
      if (ccvm & TimLibCcvMaskDELAY ) cnf->Delay     = ccv->Delay;
      if (ccvm & TimLibCcvMaskPWIDTH) cnf->PulsWidth = ccv->PulsWidth;
      if (ccvm & TimLibCcvMaskENABLE) {
	 cnf->OnZero = 0;
	 if (ccv->Enable & TimLibEnableOUT) cnf->OnZero |= CtrDrvrCounterOnZeroOUT;
	 if (ccv->Enable & TimLibEnableBUS) cnf->OnZero |= CtrDrvrCounterOnZeroBUS;
      }

      if (ioctl(ctr,CtrIoctlSET_CONFIG,&cnfb) < 0) return TimLibErrorIO;
   }

   if (ccvm & (TimLibCcvMaskPOLARITY | TimLibCcvMaskOMASK)) {
      cmbf.Counter = cntr;
      if (ioctl(ctr,CtrIoctlGET_OUT_MASK,&cmbf) < 0) return TimLibErrorIO;
      if (ccvm & TimLibCcvMaskPOLARITY) cmbf.Polarity = ccv->Polarity;
      if (ccvm & TimLibCcvMaskOMASK)    cmbf.Mask     = ccv->OutputMask;
      if (ioctl(ctr,CtrIoctlSET_OUT_MASK,&cmbf) < 0) return TimLibErrorIO;
   }

   if (rcmd & TimLibRemoteBITS) {
      crmb.Remote = rcmd;
      if (ioctl(ctr,CtrIoctlREMOTE,&crmb) < 0) return TimLibErrorIO;
   }
   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Get a counters remote configuration                                  */

TimLibError CtrLibGetRemote(unsigned long module,
			       unsigned long cntr,
			       unsigned long *remflg,
			       TimLibCcvMask *ccvm,
			       TimLibCcv     *ccv) {

CtrDrvrCounterConfiguration    *cnf;
CtrDrvrCounterConfigurationBuf cnfb;
CtrdrvrRemoteCommandBuf        crmb;
CtrDrvrCounterMaskBuf          cmbf;

   uint32_t m = module;

   if (ctr == 0) return TimLibErrorINIT;

   ioctl(ctr,CtrIoctlSET_MODULE,&m);

   crmb.Counter = cntr;

   if (ioctl(ctr,CtrIoctlGET_REMOTE,&crmb) < 0) return TimLibErrorIO;
   if (remflg) *remflg = crmb.Remote;
   if (crmb.Remote == 0) return TimLibErrorSUCCESS;

   if ((ccvm) && (ccv)) {
      cnfb.Counter = cntr;
      if (ioctl(ctr,CtrIoctlGET_CONFIG,&cnfb) < 0) return TimLibErrorIO;
      cnf = &cnfb.Config;

      *ccvm = 0;
      *ccvm |= TimLibCcvMaskSTART;  ccv->Start     = cnf->Start;
      *ccvm |= TimLibCcvMaskMODE;   ccv->Mode      = cnf->Mode;
      *ccvm |= TimLibCcvMaskCLOCK;  ccv->Clock     = cnf->Clock;
      *ccvm |= TimLibCcvMaskDELAY;  ccv->Delay     = cnf->Delay;
      *ccvm |= TimLibCcvMaskPWIDTH; ccv->PulsWidth = cnf->PulsWidth;

      *ccvm |= TimLibCcvMaskENABLE; ccv->Enable = TimLibEnableNOOUT;
      if (cnf->OnZero & CtrDrvrCounterOnZeroOUT) ccv->Enable |= TimLibEnableOUT;
      if (cnf->OnZero & CtrDrvrCounterOnZeroBUS) ccv->Enable |= TimLibEnableBUS;

      cmbf.Counter = cntr;
      if (ioctl(ctr,CtrIoctlGET_OUT_MASK,&cmbf) < 0) return TimLibErrorIO;
      *ccvm |= TimLibCcvMaskPOLARITY;
      ccv->Polarity = cmbf.Polarity;

      *ccvm |= TimLibCcvMaskOMASK;
      ccv->OutputMask = cmbf.Mask;

   }
   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Read the instantaneous value of the time in UTC. The module parameter*/
/* can be set to zero in which case the system decideds which module to */
/* read the time from, otherwise it can be set to a value between 1 and */
/* the number of installed modules.                                     */

TimLibError CtrLibGetTime(unsigned long module, /* Module number to read from */
			   TimLibTime    *utc) { /* Returned time value */


uint32_t cbl;
CtrDrvrCTime  ct;
CtrDrvrTime   *t;

   uint32_t m = module;

   if (ctr == 0) return TimLibErrorINIT;

   t = &ct.Time;

   if (module) {
      if (ioctl(ctr,CtrIoctlSET_MODULE,&m)) return TimLibErrorMODULE;
   }
   if (ioctl(ctr,CtrIoctlGET_UTC,&ct) < 0) return TimLibErrorIO;

   ioctl(ctr,CtrIoctlGET_CABLE_ID,&cbl);

   utc->Machine = TgvTgvToTgmMachine(TgvFirstMachineForCableId(cbl));
   utc->CTrain  = ct.CTrain;
   utc->Second  = t->Second;
   utc->Nano    = CtrHptdcToNano(t->TicksHPTDC);

   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Read a machines telegram from a timing receiver. The module can be   */
/* either zero, in which case the system decides which device to use,   */
/* or it can be explicitly set between 1 and the number of installed    */
/* modules. The telegram object returned has an opaque structure and    */
/* can only be decoded through the Tgm library routine .....            */

/* unsigned long grval = TgmGetGroupValueFromTelegram(unsigned long grnum,     */
/*                                                    TgmTelegram   *telegram) */

/* WARNING: The only task that should call this routine will be, get_tgm_lib,  */
/* all other, LOWER PRIORITY tasks must NEVER call this routine, instead they  */
/* should call the telegram library directly like this ...                     */

/* TgmTelegram telegram;                                                       */
/*                                                                             */
/* if (TgmGetTelegram(machine, index, offset, &telegram) == TgmSUCCESS) { ...  */
/*                                                                             */
/* For more information on this function see the Tgm library man pages.        */

TimLibError CtrLibGetTelegram(unsigned long module,
			      TgmMachine    machine,
			      TgmPTelegram   *telegram) {

int            i;
CtrDrvrTgmBuf  tgmb;

   if (ctr == 0) return TimLibErrorINIT;

   if (module) {
      uint32_t m = module;
      if (ioctl(ctr,CtrIoctlSET_MODULE,&m)) return TimLibErrorMODULE;
   }
   tgmb.Machine = TgvTgmToTgvMachine(machine);
   if (ioctl(ctr,CtrIoctlREAD_TELEGRAM,&tgmb) < 0) return TimLibErrorIO;
   telegram->Size = TgmLastGroupNumber(machine);
   telegram->Machine = machine;
   for (i=0; i<telegram->Size; i++) {
      TgmSetGroupValueInTelegram(i+1,(long) tgmb.Telegram[i], (TgmTelegram *) telegram);
   }
   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Get the description of a given PTIM equipment. The dimension returns */
/* the PPM dimension, counter and module are obvious.                   */

TimLibError CtrLibGetPtimObject(unsigned long ptim, /* PTIM equipment number */
				 unsigned long *module,
				 unsigned long *counter,
				 unsigned long *dimension) {

CtrDrvrPtimBinding ob;

   if (ctr == 0) return TimLibErrorINIT;

   ob.EqpNum = ptim;
   if (ioctl(ctr,CtrIoctlGET_PTIM_BINDING,&ob) < 0) return TimLibErrorPTIM;

   if (module) *module = ob.ModuleIndex +1;
   if (counter) *counter = ob.Counter;
   if (dimension) *dimension = ob.Size;

   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Get the event code corresponding to a given CTIM equipment number.   */

TimLibError CtrLibGetCtimObject(unsigned long ctim, /* CTIM equipment number */
				 unsigned long *eventcode) {

int i;
CtrDrvrCtimObjects ctimo;

   if (ctr == 0) return TimLibErrorINIT;

   if (ioctl(ctr,CtrIoctlLIST_CTIM_OBJECTS,&ctimo) < 0) return TimLibErrorCTIM;
   for (i=0; i<ctimo.Size; i++) {
      if (ctimo.Objects[i].EqpNum == ctim) {
	 *eventcode = ctimo.Objects[i].Frame.Long;
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

TimLibError CtrLibGetHandle(int *fd) {

uint32_t qflag;

   if (ctr == 0) return TimLibErrorINIT;
   if (ioctl(ctr,CtrIoctlGET_QUEUE_FLAG,&qflag) < 0) return TimLibErrorIO;
   if (qflag == 1) return TimLibErrorQFLAG;
   *fd = ctr;
   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Create a new PTIM object, the CCV settings will be defaulted.        */


TimLibError CtrLibCreatePtimObject(unsigned long ptimeqp, /* PTIM equipment number */
				   unsigned long module,
				   unsigned long counter,
				   unsigned long dimension) {
int cc;
CtrDrvrPtimBinding ptim;

   if (ctr == 0) return TimLibErrorINIT;

   ptim.EqpNum = ptimeqp;
   ptim.ModuleIndex = module -1;
   ptim.Counter = counter;
   ptim.Size = dimension;
   ptim.StartIndex = 0;

   cc = ioctl(ctr,CtrIoctlCREATE_PTIM_OBJECT,&ptim);
   if (errno == EBUSY)  return TimLibErrorEXISTS;
   if (errno == ENOMEM) return TimLibErrorNOMEM;
   if (cc < 0) return TimLibErrorIO;

   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Create a new CTIM object. If a payload is to be used for this event  */
/* be sure to set the low 16-Bits to 0xFFFF                             */

TimLibError CtrLibCreateCtimObject(unsigned long ctimeqp, /* CTIM equipment number */
				   unsigned long eventcode) {

int cc;
CtrDrvrCtimBinding ctim;

   if (ctr == 0) return TimLibErrorINIT;

   ctim.EqpNum = ctimeqp;
   ctim.Frame.Long = eventcode;

   cc = ioctl(ctr,CtrIoctlCREATE_CTIM_OBJECT,&ctim);
   if (cc == EBUSY)  return TimLibErrorEXISTS;
   if (cc == ENOMEM) return TimLibErrorNOMEM;
   if (cc < 0) return TimLibErrorIO;

   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Get the cable identifier to which a given module is attached so that */
/* the correct module can be used to read telegrams. This function will */
/* be used by the program get_tgm_tim only; it is of no interest to the */
/* majority of clients because calls to ReadTelegram are diverted.      */

TimLibError CtrLibGetCableId(unsigned long module,   /* The given module */
			     unsigned long *cable) { /* The cable ID */

   uint32_t cbl;
   uint32_t m = module;

   if (ctr == 0) return TimLibErrorINIT;
   if (ioctl(ctr,CtrIoctlSET_MODULE,&m)) return TimLibErrorMODULE;
   if (ioctl(ctr,CtrIoctlGET_CABLE_ID,&cbl)) return TimLibErrorIO;
   *cable = cbl;
   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Set cable identifier. This is needed to handle CTR boards receiving  */
/* events from other event generators such as the SMP module that sends */
/* beam energy, intensity etc as an event stream. The SMP dose not send */
/* Cable ID events, so we set the module cable with this function.      */
/* This routine is hidden and can not be called accross TimLib, it must */
/* be explicitly declared. Setting the ID to zero causes the module to  */
/* return its true cable ID register, any other value over rides it.    */

TimLibError CtrLibSetCableId(unsigned long module,  /* The given module */
			     unsigned long cable) { /* The given CablID */

   uint32_t m = module;
   uint32_t cbl = cable;

   if (ctr == 0) return TimLibErrorINIT;
   if (ioctl(ctr,CtrIoctlSET_MODULE,&m))  return TimLibErrorMODULE;
   if (ioctl(ctr,CtrIoctlSET_CABLE_ID,&cbl)) return TimLibErrorIO;
   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Get the status of a module and its device type.                      */

TimLibStatus CtrLibGetStatus(unsigned long module, TimLibDevice *dev) {

TimLibStatus  tstat;
CtrDrvrStatus cstat;

   uint32_t m = module;

   if (dev) *dev = TimLibDevice_CTR;

   if (ioctl(ctr,CtrIoctlSET_MODULE,&m)) return TimLibErrorIO;
   if (ioctl(ctr,CtrIoctlGET_STATUS,&cstat))  return TimLibErrorIO;

   tstat = 0;
   if (cstat & CtrDrvrStatusGMT_OK)       tstat |= TimLibStatusGMT_OK;
   if (cstat & CtrDrvrStatusPLL_OK)       tstat |= TimLibStatusPLL_OK;
   if (cstat & CtrDrvrStatusSELF_TEST_OK) tstat |= TimLibStatusSELF_OK;
   if (cstat & CtrDrvrStatusENABLED)      tstat |= TimLibStatusENABLED;

// if ((cstat & CtrDrvrStatusNO_BUS_ERROR)
// &&  (cstat & CtrDrvrStatusNO_LOST_INTERRUPTS)) tstat |= TimLibStatusBUS_OK;

   if (cstat & CtrDrvrStatusNO_BUS_ERROR) tstat |= TimLibStatusBUS_OK;

   return tstat;
}

/* ==================================================================== */
/* Get the list of all defined PTIM objects                             */

TimLibError CtrLibGetAllPtimObjects(unsigned long *ptimlist,  /* List of ptim equipments */
				    unsigned long *psize,     /* Number of ptims in list */
				    unsigned long size) {     /* Max size of list */

int i;
CtrDrvrPtimObjects ptimo;

   bzero((void *) &ptimo, sizeof(CtrDrvrPtimObjects));
   if (psize) *psize = 0;

   if (ioctl(ctr,CtrIoctlLIST_PTIM_OBJECTS,&ptimo) < 0) return TimLibErrorIO;

   for (i=0; i<ptimo.Size; i++) {

      if (i>=size) break;

      if (ptimlist) ptimlist[i] = ptimo.Objects[i].EqpNum;
      if (psize)    *psize = i +1;
   }

   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Get the list of all defined CTIM objects                             */

TimLibError CtrLibGetAllCtimObjects(unsigned long *ctimlist,  /* List of ctim equipments */
				    unsigned long *csize,     /* Number of ctims in list */
				    unsigned long size) {     /* Max size of list */

int i;
CtrDrvrCtimObjects ctimo;

   bzero((void *) &ctimo, sizeof(CtrDrvrCtimObjects));
   if (csize) *csize = 0;

   if (ioctl(ctr,CtrIoctlLIST_CTIM_OBJECTS,&ctimo) < 0) return TimLibErrorIO;

   for (i=0; i<ctimo.Size; i++) {

      if (i>=size) break;

      if (ctimlist) ctimlist[i] = ctimo.Objects[i].EqpNum;
      if (csize)    *csize = i +1;
   }

   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Get the CTR input status                                             */

TimLibError CtrLibGetIoStatus(unsigned long module,
			      TimLibLemo *input) {

uint32_t m = module;
uint32_t iostat;

   if (ctr == 0) return TimLibErrorINIT;
   if (input == NULL) return TimLibErrorNOMEM;

   if (ioctl(ctr,CtrIoctlSET_MODULE,&m)) return TimLibErrorIO;

   if (ioctl(ctr,CtrIoctlGET_IO_STATUS,&iostat) < 0) return TimLibErrorIO;

   *input = 0;

   if (iostat & CtrDrvrIoStatusCTRXI) {
      if (iostat & CtrDrvrIoStatusO1) *input |= TimLibLemoOUT_1;
      if (iostat & CtrDrvrIoStatusO2) *input |= TimLibLemoOUT_2;
      if (iostat & CtrDrvrIoStatusO3) *input |= TimLibLemoOUT_3;
      if (iostat & CtrDrvrIoStatusO4) *input |= TimLibLemoOUT_4;
      if (iostat & CtrDrvrIoStatusO5) *input |= TimLibLemoOUT_5;
      if (iostat & CtrDrvrIoStatusO6) *input |= TimLibLemoOUT_6;
      if (iostat & CtrDrvrIoStatusO7) *input |= TimLibLemoOUT_7;
      if (iostat & CtrDrvrIoStatusO8) *input |= TimLibLemoOUT_8;
      if (iostat & CtrDrvrIoStatusS1) *input |= TimLibLemoXST_1;
      if (iostat & CtrDrvrIoStatusS2) *input |= TimLibLemoXST_2;
      if (iostat & CtrDrvrIoStatusX1) *input |= TimLibLemoXCL_1;
      if (iostat & CtrDrvrIoStatusX2) *input |= TimLibLemoXCL_2;
      return TimLibErrorSUCCESS;
   }
   return TimLibErrorNOT_IMP;
}

/* ==================================================================== */
/* Set the outputs by changing the polarity                             */

TimLibError CtrLibSetOutputs(unsigned long module,
			     TimLibLemo output,
			     TimLibLemo mask) {

TimLibLemo input;
TimLibError err;
CtrDrvrCounterMaskBuf cmsb;
unsigned long msk, cntr;

   uint32_t m = module;

   if (ctr == 0) return TimLibErrorINIT;

   if (ioctl(ctr,CtrIoctlSET_MODULE,&m)) return TimLibErrorIO;

   err = CtrLibGetIoStatus(module,&input);
   if (err != TimLibErrorSUCCESS) return err;

   cntr = 0;

   for (msk = TimLibLemoOUT_1; msk <= TimLibLemoOUT_8; msk <<= 1) {
      cntr ++;
      if (mask & msk) {

	 cmsb.Counter = cntr;
	 if (ioctl(ctr,CtrIoctlGET_OUT_MASK,&cmsb) < 0) return TimLibErrorIO;

	 if (msk & output) cmsb.Polarity = CtrDrvrPolarityTTL_BAR;
	 else              cmsb.Polarity = CtrDrvrPolarityTTL;

	 if (ioctl(ctr,CtrIoctlSET_OUT_MASK,&cmsb) < 0) return TimLibErrorIO;
      }
   }

   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Get VHDL/Firmware version of all modules, and the correct version.   */

TimLibError CtrLibGetModuleVersion(TimLibModuleVersion *tver) {

CtrDrvrVersion cver;
CtrDrvrHardwareType ht;
char *cp, *ep, txt[128];
uint32_t cnt, m;
FILE *fver;

   if (ctr == 0)     return TimLibErrorINIT;
   if (tver == NULL) return TimLibErrorNOMEM;

   bzero((void *) tver, sizeof(TimLibModuleVersion));

   m = 1;
   if (ioctl(ctr,CtrIoctlSET_MODULE, &m)    < 0) return TimLibErrorIO;
   if (ioctl(ctr,CtrIoctlGET_VERSION,&cver) < 0) return TimLibErrorIO;

   tver->DrvVer    = cver.DrvrVersion;
   tver->ModVer[0] = cver.VhdlVersion;

   if      (cver.HardwareType == CtrDrvrHardwareTypeCTRP) tver->ModTyp = TimLibModuleTypeCTRP;
   else if (cver.HardwareType == CtrDrvrHardwareTypeCTRI) tver->ModTyp = TimLibModuleTypeCTRI;
   else if (cver.HardwareType == CtrDrvrHardwareTypeCTRV) tver->ModTyp = TimLibModuleTypeCTRV;
   else                                                   tver->ModTyp = TimLibModuleTypeNONE;

   fver = fopen("/usr/local/drivers/ctr/Vhdl.versions","r");
   if (fver) {
      for (ht=CtrDrvrHardwareTypeNONE; ht<CtrDrvrHardwareTYPES; ht++) {
	 if (fgets(txt,128,fver) != NULL) {
	    if (ht == cver.HardwareType) {
	       cp = ep = txt;
	       tver->CorVer = strtoul(cp,&ep,10);
	       break;
	    }
	 }
      }
      fclose(fver);
   }

   if (ioctl(ctr,CtrIoctlGET_MODULE_COUNT,&cnt) < 0) return TimLibErrorIO;
   for (m=2; m<=cnt; m++) {
      if (ioctl(ctr,CtrIoctlSET_MODULE, &m)    < 0) return TimLibErrorIO;
      if (ioctl(ctr,CtrIoctlGET_VERSION,&cver) < 0) return TimLibErrorIO;
      tver->ModVer[m-1] = cver.VhdlVersion;
   }
   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Convert time to string                                               */

static char *TmToSt(CtrDrvrTime *t) {

static char tbuf[32];

char tmp[128];
char *yr, *ti, *md, *mn, *dy;

    bzero((void *) tbuf, 32);
    bzero((void *) tmp,  32);

    if (t->Second) {
	time_t second = t->Second;

	ctime_r(&second, tmp); /* Day Mon DD HH:MM:SS YYYY\n\0 */

        tmp[3] = 0;
        dy = &(tmp[0]);
        tmp[7] = 0;
        mn = &(tmp[4]);
        tmp[10] = 0;
        md = &(tmp[8]);
        if (md[0] == ' ')
            md[0] = '0';
        tmp[19] = 0;
        ti = &(tmp[11]);
        tmp[24] = 0;
        yr = &(tmp[20]);

	sprintf (tbuf, "%s-%s/%s/%s %s"  , dy, md, mn, yr, ti);

    } else {
	sprintf (tbuf, "--- Zero ---");
    }
    return (tbuf);
}

/* ==================================================================== */
/* My strcat with boundary check                                        */

#define RESLN 1024
static char res[RESLN];

int mystrcat(char *cp) {
int sz;

   sz = strlen(res) + strlen(cp);
   if (sz < RESLN) {
      strcat(res,cp);
      return 1;
   }

   sprintf(res,"Result string too big:Required:%d Available:%d\n",sz,RESLN);
   return 0;
}

/* ==================================================================== */
/* Return CTR specific status string                                    */

char *CtrLibGetSpecificInfo(unsigned long module) { /* The given module */

#define TMPLN 512

CtrDrvrBoardId brid;
CtrDrvrReceptionErrors rers;
CtrDrvrTime t;
TimLibModuleVersion tver;
CtrDrvrPll plb;
float aspn, tempr;
double ph, er;
char tmp[TMPLN], txd[32];
uint32_t stat;
CtrDrvrModuleStats mstat;

   uint32_t m = module;

   if (ctr == 0) return NULL;

   if (ioctl(ctr,CtrIoctlSET_MODULE, &m) < 0) return NULL;

   bzero((void *) tmp, TMPLN);
   bzero((void *) res, RESLN);

   if (ioctl(ctr,CtrIoctlGET_PLL,&plb) >= 0) {
      /* FIXME: aspn is float??? Guess its size */
      if (ioctl(ctr,CtrIoctlGET_PLL_ASYNC_PERIOD,&aspn) >= 0) {

	 ph = aspn * ((double) (int) plb.Phase / (double) (int) plb.NumAverage);
	 er = aspn * ((double) (int) plb.Error / (double) (int) plb.NumAverage);

	 sprintf(tmp,"\nPLL Parameters\n"
		    "PllError:0x%08x %d\n"
		    "Integrator:0x%08x %d\n"
		    "Dac:0x%08x %d\n"
		    "LastItLen:0x%08x %d\n"
		    "KI:0x%08x %d\n"
		    "KP:0x%08x %d\n"
		    "NumAverage:0x%08x %d\n"
		    "Phase:0x%08x %d\n"
		    "AsPrdNs:%f (ns)\n"
		    "Error*Period/Naverage:%fns\n"
		    "Phase*Period/Naverage:%fns\n"
		    ,(int) plb.Error     ,(int) plb.Error
		    ,(int) plb.Integrator,(int) plb.Integrator
		    ,(int) plb.Dac       ,(int) plb.Dac
		    ,(int) plb.LastItLen ,(int) plb.LastItLen
		    ,(int) plb.KI        ,(int) plb.KI
		    ,(int) plb.KP        ,(int) plb.KP
		    ,(int) plb.NumAverage,(int) plb.NumAverage
		    ,(int) plb.Phase     ,(int) plb.Phase
		    ,aspn
		    ,er
		    ,ph);
	 mystrcat(tmp);
      }
   }

   if (CtrLibGetModuleVersion(&tver) == TimLibErrorSUCCESS) {
      t.Second = tver.ModVer[module -1];
      t.TicksHPTDC = 0;
      sprintf(tmp,"\nVHDL Version:%s\n",TmToSt(&t));
      mystrcat(tmp);
   }

   if (ioctl(ctr,CtrIoctlGET_RECEPTION_ERRORS,&rers) >= 0) {
      t.Second = rers.LastReset;
      t.TicksHPTDC = 0;
      if (t.Second < tver.CorVer)
	 sprintf(txd,"Reset:Has never been reset");
      else
	 sprintf(txd,"Reset:UTC[%d]/%s", (int) rers.LastReset, TmToSt(&t));

      sprintf(tmp,"\nReception ErrLog\n"
		  "%s\n"
		  "Parity:%d "
		  "Sync:%d "
		  "CodViol:%d "
		  "Queue:%d "
		  "Total:%d\n"
		  ,txd
		  ,(int) rers.PartityErrs
		  ,(int) rers.SyncErrs
		  ,(int) rers.CodeViolErrs
		  ,(int) rers.QueueErrs
		  ,(int) rers.TotalErrs);
      mystrcat(tmp);
   }

   if (ioctl(ctr,CtrIoctlGET_IO_STATUS,&stat) >= 0) {
      sprintf(tmp,"\nIoStat:Available\n");
      mystrcat(tmp);

      if (stat & CtrDrvrIoStatusCTRXE) {
	 sprintf(tmp,"EnergyExt:Present\n");
	 mystrcat(tmp);
	 bzero((void *) tmp, TMPLN);
      }

      if (stat & CtrDrvrIoStatusCTRXI) {
	 sprintf(tmp,"IoExt:Present\n");
	 mystrcat(tmp);
	 bzero((void *) tmp, TMPLN);
      }

      if (stat & CtrDrvrIoStatusV1_PCB) {
	 sprintf(tmp,"PcbVer:ONE\n");
	 mystrcat(tmp);
	 bzero((void *) tmp, TMPLN);
      }

      if (stat & CtrDrvrIoStatusV2_PCB) {
	 sprintf(tmp,"PcbVer:TWO\n");
	 mystrcat(tmp);
	 bzero((void *) tmp, TMPLN);
      }

      if (stat & CtrDrvrIOStatusIDOkP) {
	 sprintf(tmp,"BoardID:Present\n");
	 mystrcat(tmp);
	 if (ioctl(ctr,CtrIoctlGET_IDENTITY,&brid) >= 0) {
	    if (brid.IdMSL != 0) {
	       if (brid.IdMSL == 0xFFFFFFFF)
		  sprintf(tmp,"BoardID:0xFFFFFFFF:Old PCB\n");
	       else
		  sprintf(tmp,"BoardID:0x%08x%08X\n",(int) brid.IdMSL,(int) brid.IdLSL);
	    }
	    mystrcat(tmp);
	 }
      }

      if (stat & CtrDrvrIOStatusDebugHistory) {
	 sprintf(tmp,"DebugHis:ON\n");
	 mystrcat(tmp);
      }

      if (stat & CtrDrvrIOStatusUtcPllEnabled) {
	 sprintf(tmp,"PllUtc:ON\n");
	 mystrcat(tmp);
      }

      if (stat & CtrDrvrIOStatusTemperatureOk) {
	 tempr = (float) mstat.Temperature / 2.0;
	 sprintf(tmp,"Temperature:%2.1f C\n",tempr);
	 mystrcat(tmp);
      }

      if (stat & CtrDrvrIOStatusExtendedMemory) {
	 sprintf(tmp,"ExtMem:Present\n");
	 if (ioctl(ctr,CtrIoctlGET_MODULE_STATS,&mstat) >= 0) {
	    sprintf(tmp,"PllErrThresh:%d\n",(int) mstat.PllErrorThreshold);
	    mystrcat(tmp);
	    sprintf(tmp,"PllDacLowPass:%d\n",(int) mstat.PllDacLowPassValue);
	    mystrcat(tmp);
	    sprintf(tmp,"PllDacCICConst:%d\n",(int) mstat.PllDacCICConstant);
	    mystrcat(tmp);
	    sprintf(tmp,"PllMonCICConst:%d\n",(int) mstat.PllMonitorCICConstant);
	    mystrcat(tmp);
	    sprintf(tmp,"PhaseDCM:%d\n",(int) mstat.PhaseDCM);
	    mystrcat(tmp);
	    sprintf(tmp,"UtcPllPhaseErr:%d\n",(int) mstat.UtcPllPhaseError);
	    mystrcat(tmp);
	    sprintf(tmp,"MsMissedErrs:%d\n",(int) mstat.MsMissedErrs);
	    mystrcat(tmp);
	    sprintf(tmp,"LastMsMissed:%s\n",TmToSt(&(mstat.LastMsMissed)));
	    mystrcat(tmp);
	    sprintf(tmp,"PllErrs:%d\n",(int) mstat.PllErrors);
	    mystrcat(tmp);
	    sprintf(tmp,"LastPllErr:%s\n",TmToSt(&(mstat.LastPllError)));
	    mystrcat(tmp);
	    sprintf(tmp,"MissedFrms:%d\n",(int) mstat.MissedFrames);
	    mystrcat(tmp);
	    sprintf(tmp,"LastFrmMissed:%s\n",TmToSt(&(mstat.LastFrameMissed)));
	    mystrcat(tmp);
	    sprintf(tmp,"BadRecCycles:%d\n",(int) mstat.BadReceptionCycles);
	    mystrcat(tmp);
	    sprintf(tmp,"RecvFrms:%d\n",(int) mstat.ReceivedFrames);
	    mystrcat(tmp);
	    sprintf(tmp,"SentFrms:%d\n",(int) mstat.SentFramesEvent);
	    mystrcat(tmp);
	    sprintf(tmp,"UtcPllErrs:%d\n",(int) mstat.UtcPllErrs);
	    mystrcat(tmp);
	    sprintf(tmp,"LastExt1Str:%s\n",TmToSt(&(mstat.LastExt1Start)));
	    mystrcat(tmp);
	 }
      }
   }
   return res;
}

/* ==================================================================== */
/* Get Module statistics                                                */

TimLibError CtrLibGetModuleStats(unsigned long module,
				 TimLibModuleStats *stats) {

CtrDrvrBoardId brid;
CtrDrvrReceptionErrors rers;
CtrDrvrPll plb;
float aspn;
unsigned long stat;
CtrDrvrModuleStats mstat;
TimLibTime tlt;

   uint32_t m = module;

   if (ctr == 0)      return TimLibErrorINIT;
   if (stats == NULL) return TimLibErrorNOMEM;
   if (ioctl(ctr,CtrIoctlSET_MODULE, &m) < 0) return TimLibErrorIO;

   bzero((void *) stats, sizeof(TimLibModuleStats));
   CtrLibGetTime(module, &tlt);

   stats->Module = module;

   if (ioctl(ctr,CtrIoctlGET_PLL,&plb) >= 0) {
      stats->Pll.Valid      = 1;
      stats->Pll.Error      = plb.Error;
      stats->Pll.Integrator = plb.Integrator;
      stats->Pll.Dac        = plb.Dac;
      stats->Pll.LastItLen  = plb.LastItLen;
      stats->Pll.KI         = plb.KI;
      stats->Pll.KP         = plb.KP;
      stats->Pll.NumAverage = plb.NumAverage;
      stats->Pll.Phase      = plb.Phase;

      /* FIXME: aspn is float??? */
      if (ioctl(ctr,CtrIoctlGET_PLL_ASYNC_PERIOD,&aspn) >= 0) {
	 stats->Pll.AsPrdNs    = aspn;
      }
   }

   if (ioctl(ctr,CtrIoctlGET_RECEPTION_ERRORS,&rers) >= 0) {
      stats->Rec.Valid    = 1;
      stats->Rec.PrtyErrs = rers.PartityErrs;
      stats->Rec.SyncErrs = rers.SyncErrs;
      stats->Rec.CodeErrs = rers.CodeViolErrs;
      stats->Rec.QueuErrs = rers.QueueErrs;
      stats->Rec.TotlErrs = rers.TotalErrs;

      stats->Rec.LastRset.Second  = rers.LastReset;
      stats->Rec.LastRset.Machine = tlt.Machine;
   }

   if (ioctl(ctr,CtrIoctlGET_IO_STATUS,&stat) >= 0) {
      stats->Cst.Valid = 1;
      stats->Cst.Stat  = stat;
      if (stat & CtrDrvrIOStatusIDOkP) {
	 if (ioctl(ctr,CtrIoctlGET_IDENTITY,&brid) >= 0) {
	    stats->Cst.IdMSL = brid.IdMSL;
	    stats->Cst.IdLSL = brid.IdLSL;
	 }
      }

      if (stat & CtrDrvrIOStatusExtendedMemory) {
	 if (ioctl(ctr,CtrIoctlGET_MODULE_STATS,&mstat) >= 0) {
	    stats->Ext.Valid         = 1;
	    stats->Ext.PllErrThresh  = mstat.PllErrorThreshold;
	    stats->Ext.PllDacLowPass = mstat.PllDacLowPassValue;
	    stats->Ext.PllDacCIConst = mstat.PllDacCICConstant;
	    stats->Ext.PllMonCIConst = mstat.PllMonitorCICConstant;
	    stats->Ext.PllPhaseDCM   = mstat.PhaseDCM;
	    stats->Ext.PllUtcPhasErr = mstat.UtcPllPhaseError;
	    stats->Ext.Temperature   = mstat.Temperature;
	    stats->Ext.MsMissed      = mstat.MsMissedErrs;
	    stats->Ext.PllErrCount   = mstat.PllErrors;
	    stats->Ext.FrmMissed     = mstat.MissedFrames;
	    stats->Ext.RecBadCycles  = mstat.BadReceptionCycles;
	    stats->Ext.RecRcvdFrms   = mstat.ReceivedFrames;
	    stats->Ext.RecSentFrms   = mstat.SentFramesEvent;
	    stats->Ext.PllUtcErrs    = mstat.UtcPllErrs;

	    stats->Ext.FrmMissedLast.Second  = mstat.LastFrameMissed.Second;
	    stats->Ext.FrmMissedLast.Nano    = CtrHptdcToNano(mstat.LastFrameMissed.TicksHPTDC);
	    stats->Ext.FrmMissedLast.Machine = tlt.Machine;

	    stats->Ext.PllLastErr.Second     = mstat.LastPllError.Second;
	    stats->Ext.PllLastErr.Nano       = CtrHptdcToNano(mstat.LastPllError.TicksHPTDC);
	    stats->Ext.PllLastErr.Machine    = tlt.Machine;

	    stats->Ext.MsLastErr.Second      = mstat.LastMsMissed.Second;
	    stats->Ext.MsLastErr.Nano        = CtrHptdcToNano(mstat.LastMsMissed.TicksHPTDC);
	    stats->Ext.MsLastErr.Machine     = tlt.Machine;

	    stats->Ext.StartOne.Second       = mstat.LastExt1Start.Second;
	    stats->Ext.StartOne.Nano         = CtrHptdcToNano(mstat.LastExt1Start.TicksHPTDC);
	    stats->Ext.StartOne.Machine      = tlt.Machine;
	 }
      }
   }
   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Control how the PLL locks after synchronization loss                 */

TimLibError CtrLibSetPllLocking(unsigned long module,
				unsigned long lockflag) { /* 1=> Brutal, else Slow */

   uint32_t m = module;
   uint32_t lf = lockflag;

   if (ctr == 0) return TimLibErrorINIT;
   if (ioctl(ctr,CtrIoctlSET_MODULE, &m) < 0) return TimLibErrorIO;

   if (ioctl(ctr,CtrIoctlSET_BRUTAL_PLL,&lf) < 0) return TimLibErrorIO;

   return TimLibErrorSUCCESS;
}
