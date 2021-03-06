/**********************************************************************/
/* This is a "firmware" program for TG8.                              */
/* It is designed to run on a built-in MC68332 controller.            */
/*             1992-1994                                              */
/* Sikolenko V., Lewis J., Kovaltsov V.                               */
/* Last edition: 20 Sep 1994                                          */
/* Vladimir Kovaltsov for the SL Timing, February-June, 1997          */
/* e-mail: kovaltsov@tx.oea.ihep.su, kovalcov@oea.ihep.su             */

/* Fri 10th June Julian Lewis: Modifications as follows...            */

/*    Removed statically initialized memory so that program can be    */
/*    downloaded accross the VME bus.                                 */

/*    Added static memory initialization to the SoftInit routine for  */
/*    download capability                                             */

/*    Implemented SPS telegram support                                */

/*    Removed all logic to do with the old date/time formats for the  */
/*    legacy events for tg3, and PS tg8                               */

/*    Implemented full UTC time support based on the Tgv events       */

/*    The handling of the SUPER-CYCLE number in the old 0x20 header   */
/*    now returns the 32 bit UTC time instead of the 24 bit payload   */

/*    Some general tidy up of the code, in particular so that the O3  */
/*    optimization can be turned on hardware addresses are declared   */
/*    as volatile. This makes the code much faster.                   */

/**********************************************************************/

#define CONVERTOR 0     /* 1 = use to obtain symbols table for 'asm' */

/* Data/Definitions import */

#include "tg8.h"	 /* DPRAM layout, the protocol definition */
#include "tg8P.h"        /* Prived definitions  */
#include "/tmp/tg8Sym.c" /* Symbols table for 'asm' part of code */

/**********************************************************************/
/* The main program.                                                  */
/*    Initialize hardware and software                                */
/*    Poll the mail box forever                                       */
/*                                                                    */
/* The mail box process is interrupted by incomming events which will */
/* cause action table processes to be queued and dispatched at higher */
/* priority. TPU interrupts occur at 1KHz, and once for each counter  */
/* output.							      */
/**********************************************************************/

asm(" .text
      .globl __main
      __main: bra Main ");

main_prog () {
asm ("
 .text
 .globl Main
Main: ");

  Init();       /* Initialize the hardware and the firmware */
  MbxProcess();	/* Run the Mailbox process permanently      */
}

/**********************************************************************/
/* Initialization routine                                             */
/**********************************************************************/

static void Init() {

void         ( * p)();
long           * v;
register int     j, k;
register short * f;

   INTOFF;	/* Interrupts OFF */

   /* Set up the default ISR for each vector */

   v = (long *) 4;
   for (k=255; k>=0; k--)  *v++ = (long) Default_Isr;

   /*************************************************/
   /* Set up the interrupt service routine vectors. */

   v = (long*) (Tg8TPU_BASE_VECTOR*4);
   p = Tpu0_Isr; *v++ = (long) p;	/* Millisecond clock */
   p = Tpu1_Isr; *v++ = (long) p;	/* Counter 1 output */
   p = Tpu2_Isr; *v++ = (long) p;
   p = Tpu3_Isr; *v++ = (long) p;
   p = Tpu4_Isr; *v++ = (long) p;
   p = Tpu5_Isr; *v++ = (long) p;
   p = Tpu6_Isr; *v++ = (long) p;
   p = Tpu7_Isr; *v++ = (long) p;
   p = Tpu8_Isr; *v   = (long) p;	/* Counter 8 output */

   v = (long*) Tg8ABT_VECTOR; p = Abort_Isr; *v = (long) p;
   v = (long*) Tg8XLX_VECTOR; p = Xr_Isr;    *v = (long) p;
   v = (long*) Tg8DSC_VECTOR; p = Dsc_Isr;   *v = (long) p;

   v = (long*) McpVECTOR_BUS_ERROR;          p = BusError_Isr; *v = (long) p;
   v = (long*) McpVECTOR_ADDRESS_ERROR;      p = AddressError_Isr; *v = (long) p;
   v = (long*) McpVECTOR_PRIVILEGE_VIOLATION;p = PrivViolation_Isr; *v = (long) p;
   v = (long*) McpVECTOR_SPURIOUS_INTERRUPT; p = Spurious_Isr; *v = (long) p;

   /* Set Trap ISRs: */

   v = (long *) ((ATC_TRAP_VECTOR + McpTRAP_VECTOR_BASE)*4);
   p = AtCompletion; *v = (long) p;
   v = (long*) ((INSERT_CAM_TRAP_VECTOR + McpTRAP_VECTOR_BASE)*4);
   p = InsertToCam; *v = (long) p;
   v = (long*) ((CLEAR_CAM_TRAP_VECTOR + McpTRAP_VECTOR_BASE)*4);
   p = ClearCam; *v = (long) p;
   v = (long *) ((ISM_TRAP_VECTOR + McpTRAP_VECTOR_BASE)*4);
   p = SetIntSourceMask; *v = (long) p;
   v = (long *) ((ATR_TRAP_VECTOR + McpTRAP_VECTOR_BASE)*4);
   p = AtStartProcess; *v = (long) p;
   v = (long *) ((IMMC_TRAP_VECTOR + McpTRAP_VECTOR_BASE)*4);
   p = ImmCompletion; *v = (long) p;

   /********************************************************/
   /* Define base address pointers to Tg8 hardware modules */

   sim = (McpSim *) McpSIM_BASE;        /* System Integration Module */
   tpu = (McpTpu *) McpTPU_BASE;        /* Time Processor Unit */
   xlx = (Tg8Xlx *) Tg8XLX_BASE;        /* XiLinX gate arrays */
   cam = (short  *) Tg8CAM_BASE;        /* Content Addressed Memory */
   dpm = (Tg8Dpm *) Tg8DPM_BASE;        /* Dual Port RAM */

   /***************************************/
   /* Set up the chip selects for the SIM */

   sim->SimCS3 = Tg8CS3; /* Selects the Xilinx memory map for X1 X2 and XR. */
   sim->SimCS4 = Tg8CS4; /* Is the Abort button interrupt autovector. */
   sim->SimCS5 = Tg8CS5; /* Is the XILINX frame interrupt autovector. */
   sim->SimCS6 = Tg8CS6; /* Is Write the CAM with an option control bit. */
   sim->SimCS7 = Tg8CS7; /* Is Read the CAM */
   sim->SimCS8 = Tg8CS8; /* Selects the DPRAM */

   /********************************************/
   /* Initialize the Time Processor Unit (TPU) */

   tpu->TpuConf = 7; /* IMB interrupt arbitration ID number */

   /* Set up the interrupt configuration register */

   tpu->TpuIc = Tg8TPU_BASE_VECTOR | (Tg8TPU_INT_LEVEL<<8);

   /* Set 9 TPU channels up to Input capture/ Input transition counter */
   /* by loading the predefined time function labled "$A" into the */
   /* Channel Function Selection registers. */

   tpu->TpuCfs1 = 0xa;     /* TPU channel 8 function select "A" = ITC */
   tpu->TpuCfs2 = 0xaaaa;  /* Channels 4 to 7 */
   tpu->TpuCfs3 = 0xaaaa;  /* Channels 0 to 3 */

   /* Initialize the ITC functions for channels 0 to 8 by setting */
   /* each of the two bit Host Sequence Registers to the value 1. */
   /* The value 1 is defined as "Initialize" for function "$A".   */

   tpu->TpuHs0 = 1;
   tpu->TpuHs1 = 0x5555;
   tpu->TpuIe  = 0; /* Disable any TPU interrupts */

   /* We want to initialize the Interrupt status Register to zero.  */
   /* But tpu->TpuIs = 0; will not work because the gcc compiler    */
   /* generates a CLR instruction in this case, which would provoke */
   /* a bus error because CLR contains a READ cycle !! */

   asm(" movew #0,TpuIs");

   /* Set TPU ITC function params for channels 0 - 8 */
   /* First set up the addresses of the 16 tpu parameter blocks. */

   k = McpTPU_ACT;    /* TPU Parameter RAM map */
   for(j=0; j<16; j++) {
      var.tpu_parameters[j] = (short*) k;
      k += 16;
   };

   for(j=0; j<9; j++) {
      f = var.tpu_parameters[j];
      *f++ = 0x7; /* Channel control: PSC=DontCare PAC=RisingEdge TBS=Input */
      *f++ = 0xe; /* Start link:      LastChannel IE No link blocks */
      *f++ = 0x1; /* Link count:      1 Must be greater than zero !! */
      *f++ = 0x0; /* Bank address:    I will do my own incrementing */
   }

   /* Initialize each channel again by setting 0x01 in each in the */
   /* Host Sequence Registers. */

   tpu->TpuHsr0 = 1;
   tpu->TpuHsr1 = 0x5555;

   /* Channel priority registers */

   tpu->TpuCp0 = 2;         /* Set the high priority for channel 0 (ms clock) and */
   tpu->TpuCp1 = 0xaaab;    /* middle priority for channels 1-8 */
   tpu->TpuIe  = 0x1fe;     /* Enable TPU interrupts 1-8 */

   /********************************************/
   /* Set up Port F IO pins, and set OK status */

   sim->SimPinF   = 0xfff1;
   sim->SimDirF   = 0xe;
   sim->SimDataF1 |= (short)  SelfTestOk | OkLed;
   sim->SimDataF1  = (short)  0;
   sim->SimDataF1 |= (short)  SelfTestOk | OkLed;

   /***********************/
   /* XILINX initializing */

   xlx->XWsscRframe1 = Tg8SPS_SSC_HEADER; /* Set the default SPS SSC header code */

   /***************** Software settings *********/
   /* Interrupts and watch dog shall be enabled */
   /*********************************************/
   
   SoftInit();

} /* <<Init>> */

/********************************************************************/
/* Initialize the CAM                                               */
/********************************************************************/

static void CamInitialize() {

   CamSetCommandMode;
   CamClear;                            /* Clear all */

   /* Write zeros to CAM */

   asm(" moveal _cam,a0
	 movew  #CamWRITE,d0
	 movel  #255,d1
      0: oriw   #CamDATA_MODE,SimDataF1  /* Set data mode */
	 movew  #0,a0@                   /* Clear the comparand register */
	 movew  #0,a0@                   /* words 0 1 2. */
	 movew  #0,a0@
	 andiw  #CamCOMMAND_MODE,SimDataF1 /* Set command mode */
	 movew  d0,a0@                     /* Do the COPY command */
	 addiw  #1,d0                      /* Next CAM array address */
	 dbf    d1,0b
	 movew  #0,a0@ ");               /* Clear ALL: skip and mark empty */
}

/**********************************************/
/* Software initialization.		      */
/* Interrupts and watch dog shall be enabled. */
/**********************************************/

static void SoftInit() {
Tg8Action *a; int j; short err;
StDpm *std;

   INTOFF;		/* Interrupts OFF */
   sim->SimProtect = 0; /* Disable the Watch Dog */

   /* Erase the CAM content */

   CamInitialize();

   memset((volatile short *) &info,0,sizeof(info));

   /* Erase the DPRAM data */

   dpm->ExceptionVector = 0;
   dpm->ExceptionPC = 0;
   memset((volatile short *) &dpm->It,0,sizeof(Tg8InterruptTable));
   memset((volatile short *) &dpm->At,0,sizeof(Tg8Aux));
   memset((volatile short *) &dpm->BlockData,0,sizeof(Tg8BlockData));

   /* Erase the RAM data */

   camBusy = 0;
   memset((volatile short *) &eprog,0,sizeof(EProg));
   memset((volatile short *) &act,0,sizeof(InterActTable));
   memset((volatile short *) &clk,0,sizeof(InterClockTable));
   memset((volatile short *) &hist,0,sizeof(hist));
   memset((volatile short *) &match,0,sizeof(StartedAct));
   memset((volatile short *) &tel,0,sizeof(InterTelegram));

   memset((volatile short *) &atQueue,0,sizeof(Queue));
   memset((volatile short *) &immQueue,0,sizeof(Queue));

   memset((volatile short *) &ins,0,sizeof(ins));
   memset((volatile short *) &var,0,sizeof(var));
   memset((volatile short *) &in_use,0,sizeof(in_use));

   /* Set up the wildcard data. Enable ORing with 0 mask. */

   wild_c[0] = 0x000000L;
   wild_c[1] = 0x0000ffL;
   wild_c[2] = 0x00ff00L;
   wild_c[3] = 0x00ffffL;
   wild_c[4] = 0xff0000L;
   wild_c[5] = 0xff00ffL;
   wild_c[6] = 0xffff00L;
   wild_c[7] = 0xffffffL;

   /* Events decoder table */

   memset((volatile short *) time_event_index,0,sizeof(time_event_index));
   time_event_index[Tg8MILLISECOND_HEADER]= MILLISECOND_EVENT_INDEX;
   time_event_index[Tg8psTIME_HEADER]=
   time_event_index[Tg8SECOND_HEADER]=
   time_event_index[Tg8MINUTE_HEADER]=
   time_event_index[Tg8HOUR_HEADER]  = TIME_EVENT_INDEX;
   time_event_index[Tg8psDATE_HEADER]=
   time_event_index[Tg8DAY_HEADER]   =
   time_event_index[Tg8MONTH_HEADER] =
   time_event_index[Tg8YEAR_HEADER]  = DATE_EVENT_INDEX;

   /* Set up the telegram pointers */

   tel.aTeleg[Tg8LHC] = (Word *) dpm->At.aTelegLHC;
   tel.aTeleg[Tg8SPS] = (Word *) dpm->At.aTelegSPS;
   tel.aTeleg[Tg8CPS] = (Word *) dpm->At.aTelegCPS;
   tel.aTeleg[Tg8PSB] = (Word *) dpm->At.aTelegPSB;
   tel.aTeleg[Tg8LEA] = (Word *) dpm->At.aTelegLEA;

   /* Set up the action pointers */
   for (j=0,a=act.Table; j<Tg8ACTIONS; j++,a++) act.Pointers[j] = a;

   /* Set up the clock pointer */
   clk.Clock_p = clk.Clocks;

   /* Set up the history pointer */
   hist.History_p = hist.Histories;

   /* Set up the at&imm queues */

   atQueue.Top  = atQueue.Bottom = atQueue.Buffer;
   atQueue.End  = atQueue.Buffer + QUEUE_SIZE; /* That's the constant */
   immQueue.Top = immQueue.Bottom = immQueue.Buffer;
   immQueue.End = immQueue.Buffer + QUEUE_SIZE; /* That's the constant */

   /* Copy the firmware program compilation date */
   memcpy16((short*)&dpm->At.aFwVer,(short*)&(__DATE__),12);

   /* Set the receiver error */

   err = xlx->WClrRerr; /* Read the current receiver error */
   xlx->WClrRerr = 0;   /* and clear it. */
   rcv_error.Err = err;
   rcv_error.Hour= 0;
   *(Word*)&dpm->At.aDt.aRcvErr = *(Word*)&rcv_error; /* RcvErr & Hour bytes */

   /* Set up the mailbox protocol */
   dpm->At.aMbox = Tg8OP_NO_COMMAND;
   dpm->At.aCoco = Tg8ERR_OK;

   /* Set up the default machine */
   dpm->At.aSscHeader = Tg8SPS_SSC_HEADER; /* Use the SSC header for the SPS */

   /* Module is RUNNING */
   dpm->At.aFwStat = Tg8FS_RUNNING;

   sim->SimProtect = Tg8ENABLE_WATCHDOG; /* Enable the watch dog */

   dm[0]=dm[2]=dm[4]=dm[6]=dm[7]=dm[9]=dm[11]=31;
   dm[1]=28;
   dm[3]=dm[5]=dm[8]=dm[10]=30;

   INTON;       /* Interrupts on */

} /* <<SoftInit>> */

/**********************************************************************/
/* Insert the new action in to the Action Table.		      */
/* It returns an error code or Tg8ERR_OK.			      */
/* The variable ins.Addr gives the action address.		      */
/**********************************************************************/

static int InsertAction(action,ppmfl)
Tg8User *action; /* The action to be inserted */
int ppmfl;       /* 1=PPM timing; 0=non PPM action */ {

Tg8PpmUser *ppm;
Tg8Action  *a,*aa;
Tg8Event trigger;
int st; short j,k,m,wm, config, cw,lcw, ln,dd;

   a = act.Pointers[ins.Addr];
   memset((volatile short *) a,0,sizeof(*a));  /* Initially clear the new action */

   if (ppmfl) { /* PPM timing unit inserted */
     ppm = (Tg8PpmUser*) action;
     ln = ppm->Dim; /* Number of lines per timing unit */
     if (ins.Addr+ln > Tg8ACTIONS) return Tg8ERR_WRONG_DIM;
     a->User.uEvent   = ppm->Trigger;
     a->User.uControl = ppm->LineCw[0];
     a->User.uDelay   = ppm->LineDelay[0];
     a->Gate.Machine  = (ppm->Machine<<4) | ppm->GroupType;
     a->Gate.GroupNum = ppm->GroupNum;
     a->Gate.GroupVal = ppm->LineGv[0];
   } else {
     a->User = *action;
     ln = 1; /* Lines per timing unit */
   };

   a->Enabled = ln<<1; /* Keep the timing unit's dimension */
   trigger = a->User.uEvent;

   /* Precalculate the XILINX counter, delay and configuration registers. */

   cw = a->User.uControl;   /* Take action's control word value */
   k = Tg8CW_CNT_Get(cw);   /* Its counter number */
   k = (k==0? 7: k-1);      /* Use the channel number 0-7 */
   a->CntNum = k;           /* Set up the counter number */

   /* Calculate the "Counter Configuration" bit mask to be loaded into  */
   /* the XILINX configuration register. */

   config = GetXConfiguration(cw);
   if (config<0) return config; /* On errors - exit. */
   a->CntControl = config;      /* Set up the configuration part */

   /* For non Immediate action use the delay+1 ! */

   if (config &ConfOUTPUT) {
     if (a->User.uDelay == 0xFFFF) return Tg8ERR_WRONG_DELAY;
     a->User.uDelay++;
     dd = 1;
   } else {
     /* For BUS INTERRUPT only - use delay=0 (let be Immediate action) ! */
     a->User.uDelay = 0;
     dd = 0;
   };

   if (!Tg8CW_INT_Get(cw)) { /* Disabled action */
     Tg8CW_STATE_Set(cw,1);
     a->User.uControl = cw;
   };

   if (Tg8CW_PPML_Get(cw) != Tg8TM_LINE) { /* Except of lines of PPM action's */

   /********************************************/
   /* Correct the table of wildcards if needed */
   /********************************************/

   j = 0;
   if (trigger.Any.Byte_2 == Tg8DONT_CARE) j |= 4;
   if (trigger.Any.Byte_3 == Tg8DONT_CARE) j |= 2;
   if (trigger.Any.Byte_4 == Tg8DONT_CARE) j |= 1;
   wm = 1<<j;

   /* Dont create AT processes for simple and ms, date/time events unless */
   /* such actions actually present in the table. */

   if (trigger.Any.Header != 0x01) { /* SPS Simple and SC events */

      in_use.SimpleH_wc[trigger.Any.Header] |= wm;
      m = trigger.Any.Header>>4; /* Get the machine code */
      m = 1<<m;                  /* Make the machines bitmap */
      if ((j&4) == 0) { /* None of the wildcard for event code (byte_2) */
	                /* We shall serve this Event */
	in_use.Simple_event[trigger.Simple.Event_code] |= m;
	if (config &ConfOUTPUT)
	  in_use.Simple_wc[trigger.Simple.Event_code] |= wm;
	else
	  in_use.Simple_wc_imm[trigger.Simple.Event_code] |= wm;
      } else            /* All event codes shall be served */
	for (k=255; k>=0; k--) {
	  in_use.Simple_event[k] |= m;
	  if (config &ConfOUTPUT)
	    in_use.Simple_wc[k] |= wm;
	  else
	    in_use.Simple_wc_imm[k] |= wm;
	};

   } else { /* 1KHz or Time/Date (e.g. calendar) event (up to 15 event headers) */

     in_use.TimeH_wc[trigger.Any.Header] |= wm;
     m = time_event_index[trigger.Any.Header];
     m = 1<<m;
     if ((j&4) == 0) { /* None of wildcard for event code (byte_2) */
       in_use.Time_event[trigger.Simple.Event_code] |= m;
       if (config &ConfOUTPUT)
	 in_use.Time_wc[trigger.Simple.Event_code] |= wm;
       else
	 in_use.Time_wc_imm[trigger.Simple.Event_code] |= wm;
     } else            /* All event codes shall be served */
       for (k=255; k>=0; k--) {
	 in_use.Time_event[k] |= m;
	 if (config &ConfOUTPUT)
	   in_use.Time_wc[k] |= wm;
	 else
	   in_use.Time_wc_imm[k] |= wm;
       };
   };

   /********************************************************/
   /* The CAM trap routine uses the the structure 'ins' to */
   /* know which event to insert and where.                */
   /********************************************************/

   if (Tg8CW_STATE_Get(cw) == 0) { /* Action enabled */
     ins.Event.Long = trigger.Long;
     /* Now calculate the last 16 bits to be inserted to CAM */
     ins.Last_word.Short = (config &ConfOUTPUT)? 0xFF/*counting*/: 0x00/*immediate*/;
   } else {
     ins.Event.Long = 0;
     ins.Last_word.Short = 0;
   };

   /* In the case of the PPM timing unit - insert its lines into the user table */

   if (ppmfl) {
     Tg8CW_PPML_Set(a->User.uControl,Tg8TM_PPM);
     for (k=1,aa=a; k<ln; k++) {
       *++aa = *a;
       aa->Gate.GroupVal = ppm->LineGv[k];
       aa->User.uControl = lcw = ppm->LineCw[k];
       aa->User.uDelay   = dd+ppm->LineDelay[k];

       /* Precalculate the XILINX counter, delay and configuration registers. */

       j = Tg8CW_CNT_Get(lcw);  /* Its counter number */
       j = (j==0? 7: j-1);      /* Use the channel number 0-7 */
       if (j != a->CntNum) return Tg8ERR_ILLEGAL_ARG;

       /* Calculate the "Counter Configuration" bit mask to be loaded into  */
       /* the XILINX configuration register. */

       j = GetXConfiguration(lcw);
       if (j<0) return j;  /* On errors - exit. */
       if ((j&ConfOUTPUT) != (config&ConfOUTPUT)) return Tg8ERR_ILLEGAL_ARG;

       aa->CntControl = j; /* Set up the configuration part */

       /* For non Immediate action use delay+1 ! */

       if (j &ConfOUTPUT) {
	 if (!aa->User.uDelay) return Tg8ERR_WRONG_DELAY;
       } else
	 /* For BUS INTERRUPT only - use delay=0 (let be Immediate action) ! */
	 aa->User.uDelay = 0;

       if (!Tg8CW_INT_Get(lcw)) /* Disabled action */
	 Tg8CW_STATE_Set(lcw,1);

       Tg8CW_PPML_Set(lcw,Tg8TM_LINE); /* That is the PPM Line */
       aa->User.uControl = lcw;
       if (Tg8CW_STATE_Get(lcw) == 0) aa->Enabled |= Tg8ENABLED;
     };
   };

   /* Insert the "frame" part of the table item to the CAM. Do it in a Trap-  */
   /* routine in non-interruptable state to prevent mixture of CAM operations */
   /* by the AT-process.                                                      */

   asm(Insert_CAM_Trap);

   };/* Except of lines of PPM action's */

   /* Now we may enable the action if required by an user */
   if (Tg8CW_STATE_Get(cw) == 0) a->Enabled |= Tg8ENABLED;

   /* Move the action's number */
   ins.Addr += ln;
   if (dpm->At.aNumOfAct < ins.Addr) dpm->At.aNumOfAct = ins.Addr;

   return(Tg8ERR_OK);

}/* <<InsertAction>> */

/**********************************************************************/
/* Precalculate the XILINX configuration.                             */
/**********************************************************************/

static int GetXConfiguration(cw)
int cw; {
int config = 0;  /* initial setting */

   switch(Tg8CW_INT_Get(cw)) {
   case Tg8DO_OUTPUT:
     config |= ConfOUTPUT;
   break;
   case Tg8DO_OUTPUT_AND_INTERRUPT:
     config |= ConfOUTPUT;
     /* +++ */
   case Tg8DO_INTERRUPT:
     config |= ConfINTERRUPT;
   break;
   };

   switch(Tg8CW_CLOCK_Get(cw)) {
   case Tg8CLK_MILLISECOND:
   break;
   case Tg8CLK_X1:
     config |= ConfEXT_CLOCK_1;
   break;
   case Tg8CLK_X2:
     config |= ConfEXT_CLOCK_2;
   break;
   default:
     return Tg8ERR_WRONG_CLOCK;
   };

   switch(Tg8CW_START_Get(cw)) {
   case Tg8CM_NORMAL:
     config |= ConfNORMAL;
   break;
   case Tg8CM_CHAINED:
     config |= ConfCHAINE;
   break;
   case Tg8CM_EXTERNAL:
     config |= ConfEXT_START;
   break;
   default:
      return Tg8ERR_WRONG_MODE;
   };
   return config;
}

/*******************************************/
/* Fill the memory block using word access */
/*******************************************/

static void memset(volatile short *m,short v,unsigned int bc) {
volatile short *p = (volatile short *) m;
  bc >>= 1;	/* Words counter */
  while (bc--) *p++ = v;
}

/***************************************************************/
/* Copy structures as the 16 bits arrays (important for DPRAM) */
/***************************************************************/

static void memcpy16(short *d,short *s,int bc) {
register int n;
  for (n=(bc>>1); n>0; n--) *d++ = *s++;
  if (bc&1) *(char*)d = *(char*)s;
}

static void bcopy(short *s,short *d,int bc) {
register int n;
  for (n=(bc>>1); n>0; n--) *d++ = *s++;
  if (bc&1) *(char*)d = *(char*)s;
}

/**************************/
/* Tracing                */
/**************************/

static Debug(n) short n; { while (dpm->At.aTrace); dpm->At.aTrace = n; }

/***************************/
/* The rest program's code */
/***************************/

#include "UtcToTime.c"
#include "mbx.c"	 /* Mail box protocol handler */
#include "isr.c"	 /* ISR and TRAP handlers */
#include "immproc.c"     /* Immediate actions processing */


/* EOF tg8.c */
