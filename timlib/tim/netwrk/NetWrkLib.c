#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <tgv/tgv.h>

/*
 * ==================================================================== 
 * Do it for the DTM library 
 * Julian Lewis May 2004 
 * ==================================================================== 
 */

#include <tgm/tgm.h>

typedef struct {
   unsigned long mtype;
   unsigned char mtext[16];
} TimLibSimMesBuf;

extern TgmCompletion _TgmHisBuffAddElement(TgmHistoryBuffer * hm);

#define CONNECTIONS (TgmMACHINES * 2)

static int netwrk_timeout = 12;
static int netwrk_initialized = 0;
static int netwrk_connected = 0;
static int netwrk_connections[CONNECTIONS];

static int dtmskt = -1;		/* Incomming event DTM input socket */

/*
 * ==================================================================== 
 * Initialize DTM history mechanism 
 */

TimLibError NetWrkLibInitialize(TimLibDevice device)
{				/* Initialize hardware/software */

   int i;

   dtmskt = TgmDtmHisGetSocket();
   if (dtmskt == -1)
      return TimLibErrorOPEN;

   if (netwrk_initialized == 0) {
      netwrk_initialized = 1;
      netwrk_connected = 0;
      for (i = 0; i < CONNECTIONS; i++) {
	 netwrk_connections[i] = 0;
      }
   }
   return TimLibErrorSUCCESS;
}

/* ====== */

int NetWrkLibFdInitialize(TimLibDevice device) {
   return 0;
}

/*
 * ==================================================================== 
 * Connect to an interrupt.  
 * Only CTIMS in the list above are allowed 
 */


TimLibError NetWrkLibConnect(TimLibClass iclss,	/* Class of interrupt */
			     unsigned long equip,	/* Equipment or hardware mask */
			     unsigned long module)
{				/* For HARD or CTIM classes */

   int i;

   if (netwrk_initialized) {

      if (iclss == TimLibClassPTIM)
	 return TimLibErrorPTIM;
      if (iclss == TimLibClassHARDWARE)
	 return TimLibErrorHARDWARE;

      if (iclss == TimLibClassCTIM) {
	 for (i = 0; i < netwrk_connected; i++) {
	    if (netwrk_connections[i] == equip) {
	       return TimLibErrorSUCCESS;
	    }
	 }
	 i = netwrk_connected;
	 if (i < CONNECTIONS) {
	    netwrk_connections[i] = equip;
	    netwrk_connected++;
	    if (TgmAttach(TgvTgvToTgmMachine(TgvGetMachineForMember(equip)), TgmTELEGRAM) != TgmSUCCESS)
	       return TimLibErrorCTIM;
	    return TimLibErrorSUCCESS;
	 }
	 return TimLibErrorNOMEM;
      }
   }
   return TimLibErrorINIT;
}

/* ====== */

TimLibError NetWrkLibFdConnect(int fd,                 /* File descriptor */
			       TimLibClass iclss,      /* Class of interrupt */
			       unsigned long equip,    /* Equipment or hardware mask */
			       unsigned long module) { /* For HARD or CTIM classes */
   return TimLibErrorNOT_IMP;
}

/* ==================================================================== */
/* Disconnect from an interrupt                                         */

TimLibError NetWrkLibDisConnect(TimLibClass iclss,         /* Class of interrupt */
				unsigned long equip,       /* Equipment or hardware mask */
				unsigned long module) {
   return TimLibErrorINIT;
}

/*
 * ==================================================================== 
 * Set queueing On or Off, and the time out value in micro seconds.  
 * A timeout value of zero means no time out, you wait for ever.  
 */

TimLibError NetWrkLibQueue(unsigned long qflag,   /* 0=>Queue, 1=>NoQueue */
			   unsigned long tmout) { /* 0=>No time outs */
netwrk_timeout = tmout;

   return TimLibErrorSUCCESS;
}

/* ====== */

TimLibError NetWrkLibFdQueue(int           fd,      /* File descriptor */
			     unsigned long qflag,   /* 0=>Queue, 1=>NoQueue */
			     unsigned long tmout) { /* 0=>No time outs */
   return TimLibErrorNOT_IMP;
}

/* ==================================================================== */
/* To know if a call to wait will block, this call returns the Queue    */
/* size. If the size iz greater than zero a call to wait will not block */
/* and return without waiting. If the qflag is set to NoQueue, zero is  */
/* allways returned and all calls to wait will block.                   */

unsigned long NetWrkLibGetQueueSize() {

   return 0;
}

/*
 * ==================================================================== 
 * Wait for an interrupt. The parameters are all returned from the call 
 * so you can know which interrupt it was that came back. Note, when 
 * waiting for a hardware interrupt from either CTIM or from a counter, 
 * it is the CTIM or PTIM object that caused the interrupt returned.  
 * The telegram will have been read already by the high prioity task 
 * get_tgm_ctr/tg8, be aware of the race condition here, hence payload. 
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
 * mch: The Tgm machine of the trigger event 
 */

TimLibError NetWrkLibWait(TimLibClass * iclss,	/* Class of interrupt */
			  unsigned long *equip,	/* PTIM CTIM or hardware mask */
			  unsigned long *plnum,	/* Ptim line number 1..n or 0 */
			  TimLibHardware * source,	/* Hardware source of interrupt */
			  TimLibTime * onzero,	/* Time of interrupt/output */
			  TimLibTime * trigger,	/* Time of counters load */
			  TimLibTime * start,	/* Time of counters start */
			  unsigned long *ctim,	/* CTIM trigger equipment ID */
			  unsigned long *payload,	/* Payload of trigger event */
			  unsigned long *module,	/* Module that interrupted */
			  unsigned long *missed,	/* Number of missed interrupts */
			  unsigned long *qsize,	/* Remaining interrupts on queue */
			  unsigned long *mch)
{				/* Corresponding TgmMachine */

TgmHistoryBuffer *h;
unsigned long e, f, sime;
int tries, i, simkey, simskt;
TimLibTime t;
TimLibSimMesBuf mbf;
long *mlong;

   tries = TgmMACHINES << 1;	/* Number of packets to timeout */

   if (netwrk_initialized) {
      while (1) {
	 mbf.mtype = 1;
	 mlong = (long *) mbf.mtext;
	 sime = *mlong = 0;
	 simkey = TgmGetKey("TimLibSIM");
	 simskt = msgget(simkey, 0666 | IPC_CREAT);
	 if (msgrcv(simskt, &mbf, sizeof(long) +1, 1, IPC_NOWAIT) != -1) {
	    sime = *mlong;
	 }

	 if (TgmDtmHisWait()) {
	    h = TgmDtmHisRead();
	    if (h) {
	       if (sime)
		  TgmHisSetEventId(h, sime);
	       e = TgmHisGetEventId(h);
	       for (i = 0; i < netwrk_connected; i++) {
		  if (netwrk_connections[i] == e) {

		     if (iclss) *iclss = TimLibClassCTIM;
		     if (mch) *mch = TgmHisGetMachine(h);
		     if (ctim) *ctim = e;
		     if (equip) *equip = e;

		     if (plnum) *plnum = 0;
		     if (source) *source = 0;
		     if (module) *module = 0;
		     if (missed) *missed = 0;
		     if (qsize) *qsize = 0;

		     if (onzero) {
			t.Second = TgmHisGetEvtTimeSec(h);
			t.Nano = TgmHisGetEvtTimeMSc(h) * 1000000;
			t.Machine = TgmHisGetMachine(h);
			t.CTrain = TgmHisGetEvtTimeMSc(h) - TgmHisGetCycleTimeMSc(h);
			*onzero = t;
		     }
		     if (trigger) {
			t.Second = TgmHisGetAqnTimeSec(h);
			t.Nano = TgmHisGetAqnTimeMSc(h) * 1000000;
			t.Machine = TgmHisGetMachine(h);
			t.CTrain = TgmHisGetAqnTimeMSc(h) - TgmHisGetCycleTimeMSc(h);
			*trigger = t;
		     }
		     if (start) {
			t.Second = TgmHisGetCycleTimeSec(h);
			t.Nano = TgmHisGetCycleTimeMSc(h) * 1000000;
			t.Machine = TgmHisGetMachine(h);
			t.CTrain = 0;
			*start = t;
		     }
		     if (payload) {
			f = TgvGetFrameForMember(e);
			if (f & 0xFFFF)
			   *payload = TgmHisGetTelegram(h, 1);
			else
			   *payload = 0;
		     }

		     return TimLibErrorSUCCESS;

		  }		/* End_if (netwrk_connections[i] == e) */
	       }		/* End_for (i = 0; i < netwrk_connected; i++) */
	    } else {
	       return TimLibErrorIO;	/* End_if (h) */
	    }
	    if (netwrk_timeout) {
	       if (--tries <= 0)
		  return TimLibErrorTIMEOUT;
	    }
	 }			/* End_if TgmDtmHisWait() */
      }				/* End_while */
   }                            /* End_if (netwrk_initialized) */
   return TimLibErrorINIT;
}

/* ====== */

TimLibError NetWrkLibFdWait(int fd,                 /* File descriptor */
			    TimLibClass * iclss,    /* Class of interrupt */
			    unsigned long *equip,   /* PTIM CTIM or hardware mask */
			    unsigned long *plnum,   /* Ptim line number 1..n or 0 */
			    TimLibHardware * source,/* Hardware source of interrupt */
			    TimLibTime * onzero,    /* Time of interrupt/output */
			    TimLibTime * trigger,   /* Time of counters load */
			    TimLibTime * start,     /* Time of counters start */
			    unsigned long *ctim,    /* CTIM trigger equipment ID */
			    unsigned long *payload, /* Payload of trigger event */
			    unsigned long *module,  /* Module that interrupted */
			    unsigned long *missed,  /* Number of missed interrupts */
			    unsigned long *qsize,   /* Remaining interrupts on queue */
			    unsigned long *mch) {   /* Corresponding TgmMachine */
   return TimLibErrorNOT_IMP;
}

/*
 * ==================================================================== 
 * Set the Ccv of a PTIM equipment. Note neither the counter number nor 
 * the trigger condition can be changed.  
 */

TimLibError NetWrkLibSet(unsigned long ptim,	/* PTIM to write to */
			 unsigned long plnum,	/* Ptim line number 1..n or 0 */
			 unsigned long grnum,	/* Tgm group number or Zero */
			 unsigned long grval,	/* Group value if num not zero */
			 TimLibCcvMask ccvm,	/* Which values to write */
			 TimLibCcv * ccv)
{				/* Current control value */

   return TimLibErrorNOT_IMP;
}

/*
 * ==================================================================== 
 * Get the Ccv of a PTIM equipment.  
 */

TimLibError
NetWrkLibGet(unsigned long ptim,
	     unsigned long plnum, unsigned long grnum, unsigned long grval, TimLibCcvMask * ccvm, TimLibCcv * ccv)
{

   return TimLibErrorNOT_IMP;
}

/*
 * ==================================================================== 
 * By writing to the driver this call simulates an interrupt for the 
 * connected clients. Also it can be used as a way of synchronizing 
 * processes, this is especially important in Linux systems where the 
 * schedular is not preemptive.  
 *
 * Arguments: 
 * iclss: Class of interrupt to simulate, PTIM, CTIM or Hardware 
 * equip: Equipment number for PTIM or CTIM, hardware mask for Hardware 
 * module: When class is CTIM or Hardware, the module number is used 
 * machine: Telegram ID is used for PTIM interrupts if grnum is not zero 
 * grnum: If zero, no telegram checking, else the PTIM triggers group number 
 * grval: The telegram group value for the PTIM trigger 
 */

TimLibError
NetWrkLibSimulate(TimLibClass iclss,
		  unsigned long equip, unsigned long module, TgmMachine machine, unsigned long grnum, unsigned long grval)
{

TimLibSimMesBuf mbf;
int simkey, simskt;
long *mlong;

   switch (iclss) {
   case TimLibClassHARDWARE:	/* Class is direct hardware connection */
      return TimLibErrorNOT_IMP;

   case TimLibClassCTIM:	/* A Ctim timing object carried by an event on the cable */
      mlong = (long *) mbf.mtext;
      mbf.mtype = 1;
      *mlong = equip;
      simkey = TgmGetKey("TimLibSIM");
      simskt = msgget(simkey, 0666 | IPC_CREAT);
      if (msgsnd(simskt, &mbf, sizeof(long) +1, IPC_NOWAIT) != -1)
	 return TimLibErrorSUCCESS;
      perror("msgsnd in NetWrkLibSimulate:");
      break;

   case TimLibClassPTIM:	/* A PTIM timing object implemented on a counter */
      return TimLibErrorNOT_IMP;
   }

   return TimLibErrorCTIM;
}

/*
 * ==================================================================== 
 * Set a counter under full remote control (IE under DSC tasks control) 
 * This feature permits you to do what you like with counters even if 
 * there is no timing cable attached. With this you can drive stepper 
 * motors, wire scanners or whatever. No PTIM or CTIM is involved, the 
 * configuration is loaded directly by the application. Note that when 
 * the argument remflg is set to 1, the counter can not be written to 
 * by incomming triggers so all PTIM objects using the counter stop 
 * overwriting the counter configuration and are effectivley disabled.  
 * Setting the remflg 0 permits PTIM triggers to write to the counter 
 * configuration, the write block is removed. Also note that in some 
 * cases it is useful to perform remote actions, such as remoteSTOP, 
 * even if the remflg is set to zero. The remflg simply blocks PTIM 
 * overwrites, the counter configuration can still be accessed ! 
 */

TimLibError NetWrkLibRemoteControl(unsigned long remflg,	/* 0 = Normal, 1 = Remote */
				   unsigned long module,	/* Module or zero */
				   unsigned long cntr,	/* 1..8 counter number */
				   TimLibRemote rcmd,	/* Command */
				   TimLibCcvMask ccvm,	/* Fields to be set */
				   TimLibCcv * ccv)
{				/* Value to load in counter */

   return TimLibErrorNOT_IMP;
}

/* ==================================================================== */
/* Get a counters remote configuration                                  */

TimLibError NetWrkLibGetRemote(unsigned long module,
			       unsigned long cntr, unsigned long *remflg, TimLibCcvMask * ccvm, TimLibCcv * ccv)
{

   return TimLibErrorNOT_IMP;
}

/*
 * ==================================================================== 
 * Read the instantaneous value of the time in UTC. The module parameter
 * can be set to zero in which case the system decideds which module to 
 * read the time from, otherwise it can be set to a value between 1 and 
 * the number of installed modules.  
 */

TimLibError NetWrkLibGetTime(unsigned long module,	/* Module number to read from */
			     TimLibTime * utc)
{				/* Returned time value */


   TgmHistoryBuffer *h;
   TimLibTime t;

   if (netwrk_initialized) {
      while (TgmDtmHisWait()) {
	 h = TgmDtmHisRead();
	 if (h) {
	    t.Second = TgmHisGetEvtTimeSec(h);
	    t.Nano = TgmHisGetEvtTimeMSc(h) * 1000000;
	    t.Machine = TgmHisGetMachine(h);
	    t.CTrain = TgmHisGetEvtTimeMSc(h) - TgmHisGetCycleTimeMSc(h);
	    *utc = t;

	    return TimLibErrorSUCCESS;
	 } else
	    return TimLibErrorIO;
      }
   }
   return TimLibErrorINIT;
}

/*
 * ==================================================================== 
 * Read a machines telegram from a timing receiver. The module can be 
 * either zero, in which case the system decides which device to use, 
 * or it can be explicitly set between 1 and the number of installed 
 * modules. The telegram object returned has an opaque structure and 
 * can only be decoded through the Tgm library routine .....  
 *
 * unsigned long grval = TgmGetGroupValueFromTelegram(unsigned long grnum, 
 * TgmTelegram *telegram) 
 *
 * WARNING: The only task that should call this routine will be, get_tgm_lib, 
 * all other, LOWER PRIORITY tasks must NEVER call this routine, instead they 
 * should call the telegram library directly like this ...  
 *
 * TgmTelegram telegram; 
 * if (TgmGetTelegram(machine, index, offset, &telegram) == TgmSUCCESS) { ...  
 * For more information on this function see the Tgm library man pages.  
 */

TimLibError NetWrkLibGetTelegram(unsigned long module, TgmMachine machine, TgmPTelegram * telegram)
{

   TgmHistoryBuffer *h;
   long gv;
   unsigned long m, gn;

   if (netwrk_initialized) {
      while (TgmDtmHisWait()) {
	 h = TgmDtmHisRead();
	 if (h) {
	    m = TgmHisGetMachine(h);
	    if (m == machine) {
	       telegram->Size = TgmLastGroupNumber(m);
	       telegram->Machine = machine;
	       for (gn = 1; gn <= telegram->Size; gn++) {
		  gv = (long) TgmHisGetTelegram(h, gn);
		  TgmSetGroupValueInTelegram(gn, gv, (TgmTelegram *) telegram);
	       }
	       return TimLibErrorSUCCESS;
	    }
	 } else
	    return TimLibErrorIO;
      }
   }
   return TimLibErrorINIT;
}

/*
 * ==================================================================== 
 * Lets you know how many installed modules there are on this host.  
 */

unsigned long NetWrkLibGetInstalledModuleCount()
{

   return 0;
}

/*
 * ==================================================================== 
 * Get the description of a given PTIM equipment. The dimension returns 
 * the PPM dimension, counter and module are obvious.  
 */

TimLibError NetWrkLibGetPtimObject(unsigned long ptim,	/* PTIM equipment number */
				   unsigned long *module, unsigned long *counter, unsigned long *dimension)
{

   return TimLibErrorNOT_IMP;
}

/*
 * ==================================================================== 
 * Get the event code corresponding to a given CTIM equipment number.  
 */

TimLibError NetWrkLibGetCtimObject(unsigned long ctim,	/* CTIM equipment number */
				   unsigned long *eventcode)
{

   *eventcode = TgvGetFrameForMember(ctim);
   if (*eventcode)
      return TimLibErrorSUCCESS;
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

TimLibError NetWrkLibGetHandle(int *fd)
{

   if (netwrk_initialized) {
      *fd = TgmDtmHisGetSocket();
      return TimLibErrorSUCCESS;
   }
   return TimLibErrorINIT;
}

/*
 * ==================================================================== 
 * Create a new PTIM object, the CCV settings will be defaulted.  
 */


TimLibError NetWrkLibCreatePtimObject(unsigned long ptim,	/* PTIM equipment number */
				      unsigned long module, unsigned long counter, unsigned long dimension)
{
   return TimLibErrorNOT_IMP;
}

/*
 * ==================================================================== 
 * Create a new CTIM object. If a payload is to be used for this event 
 * be sure to set the low 16-Bits to 0xFFFF 
 */

TimLibError NetWrkLibCreateCtimObject(unsigned long ctim,	/* CTIM equipment number */
				      unsigned long eventcode)
{
   return TimLibErrorNOT_IMP;
}

/*
 * ==================================================================== 
 * Get the cable identifier to which a given module is attached so that
 * the correct module can be used to read telegrams. This function will
 * be used by the program get_tgm_tim only; it is of no interest to the
 * majority of clients because calls to ReadTelegram are diverted.
 */

TimLibError NetWrkLibGetCableId(unsigned long module,	/* The given module */
				unsigned long *cable)
{				/* The cable ID */

   return TimLibErrorNOT_IMP;
}


/* ==================================================================== */
/* Get the status of a module and its device type.                      */

TimLibStatus NetWrkLibGetStatus(unsigned long module, TimLibDevice * dev)
{

   if (dev)
      *dev = TimLibDevice_NETWORK;
   return TimLibStatusBITS;
}

/* ==================================================================== */
/* Get the list of all defined PTIM objects                             */

TimLibError NetWrkLibGetAllPtimObjects(unsigned long *ptimlist,	/* List of ptim equipments */
				       unsigned long *psize,	/* Number of ptims in list */
				       unsigned long size)
{				/* Max size of list */

   *psize = 0;
   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Get the list of all defined CTIM objects                             */

TimLibError NetWrkLibGetAllCtimObjects(unsigned long *ctimlist,	/* List of ctim equicments */
				       unsigned long *csize,	/* Number of ctims in list */
				       unsigned long size)
{				/* Max size of list */

   unsigned long equip, cnt;
   TgmNetworkId nid;

   nid = TgmGetDefaultNetworkId();
   equip = TgvFirstGMember();
   cnt = 0;
   while ((equip) && (cnt < size)) {
      ctimlist[cnt++] = equip;
      equip = TgvNextGMember();
   }
   *csize = cnt;
   return TimLibErrorSUCCESS;
}

/* ==================================================================== */
/* Get VHDL/Firmware version of all modules, and the correct version.   */

TimLibError NetWrkLibGetModuleVersion(TimLibModuleVersion *version) {

   return TimLibErrorNOT_IMP;

}

/* ==================================================================== */
/* Get specific information string                                      */
/* There is some very specific module dependent status information.     */
/* This routine returns a free format human readable string containing  */
/* specific status information that may help diagnosing problems for a  */
/* timing receiver module. A null pointer can be returned if the module */
/* is either dead or not installed.                                     */

char *NetWrkLibGetSpecificInfo(unsigned long module) { /* The given module */

   return NULL;
}

/* ==================================================================== */

TimLibError NetWrkLibConnectPayload(unsigned long ctim,        /* The CTIM ID you want to connect to */
				    unsigned long payload,     /* The 16 bit payload in a long */
				    unsigned long module) {    /* The module, or zero means don't care */
   return TimLibErrorNOT_IMP;
}
