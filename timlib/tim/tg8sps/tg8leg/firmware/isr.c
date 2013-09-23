/**********************************************************************/
/* Interrupt Service Routines for the Tg8's firmware program.         */
/*             1992-1994                                              */
/* Sikolenko V., Lewis J., Kovaltsov V.                               */
/* Last edition: 20 Sep 1994                                          */
/* Vladimir Kovaltsov for the SL Version, February, 1997	      */

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

/*    SCTime is now the real c-time from the ms modulo                */

/**********************************************************************/

#include <macros.h>

/**********************************************************************/
/* Macro to handle a Tpu interrupt for a counter channel.             */
/* An argument (c) gives the TPU channel 1-8                          */
/**********************************************************************/

#define TpuIsr(c) asm("
       .globl _Tpu"#c"_Isr
_Tpu"#c"_Isr:
       movew   #Tg8DISABLE_INTERRUPTS,SR
       moveml  a0,_context_c              /* Save registers */
       moveal _eprog+iFired+(("#c"-1)*4),a0  /* Points to the interr. table */
       movew   At+aScTime,a0@(iOut)
       movew   At+aScTime+2,a0@(iOut+2)
       moveal _eprog+rFired+(("#c"-1)*4),a0  /* Points to the recording table */
       movew   At+aScTime,a0@(rOut)
       movew   At+aScTime+2,a0@(rOut+2)
       moveml _context_c,a0	             /* Restore registers */
       andw   #ClrINT_CHANNEL_"#c",TpuIs     /* Clear the TPU channel 'c' interrupt */
       rte
")

/********************************************************************/
/* Macro to provide a general purpose interrupt handler.            */
/********************************************************************/

#define GeneralIsr(c) asm("
	.text
	.globl  _"#c"_Isr
_"#c"_Isr:  rte")

/**********************************************************************/
/* Block of all ISR and Trap routines				      */
/**********************************************************************/

void dummyIsrEtc() {
  register int err;

   asm(" .text");

/**********************************************************************/
/* Trap-routine to Set the interrupt source mask                      */
/**********************************************************************/

   asm(" .globl _SetIntSourceMask
_SetIntSourceMask:
	 movew  #Tg8DISABLE_INTERRUPTS,SR  /* Mask all interrupts */
	 moveml a2/a3,_regic
       ");
   if (eprog.IntSrc && !dpm->At.aIntSrc) {
     /* DSC ready, generate VME bus interrupt */
     dpm->At.aIntSrc = eprog.IntSrc; eprog.IntSrc = 0;
     /* Issue the real VME BUS interrupt */
     (*(short*)(Tg8HR_INT_VME+(char*)dpm)) = 1;
   };
   asm ("  moveml  _regic,a2/a3
	   rte ");

/**********************************************************************/
/* Trap-routine to insert the item to CAM.			      */
/**********************************************************************/

   asm(" .globl _InsertToCam
_InsertToCam:
	 movew  #Tg8DISABLE_INTERRUPTS,SR
	 moveml d4/a2/a3,_regic ");
   CamInsertToCAM();
   asm(" moveml _regic,d4/a2/a3
	 rte ");

/**********************************************************************/
/* Trap-routine to clear the whole CAM.				      */
/**********************************************************************/

   asm(" .globl _ClearCam
_ClearCam:
	 movew  #Tg8DISABLE_INTERRUPTS,SR
	 movel a0,_regic ");
   CamSetCommandMode;
   CamClear;
   asm(" movel _regic,a0
         rte ");

/**********************************************************************/
/* Macro to handle a Tpu interrupt 0 (1MS clock) - not used.          */
/**********************************************************************/

asm(" .globl _Tpu0_Isr
_Tpu0_Isr:
       movew  #Tg8DISABLE_INTERRUPTS,SR
       andw   #-1-Tg8HS_DPRAM_INTERRUPT,TpuIs /* Clear the TPU ch.'0' inter.*/
       rte
");

/**********************************************************************/
/* ISR for handling interrupts from the TPU channels 1-8              */
/* See the macro TpuIsr(c) above                                      */
/**********************************************************************/

   TpuIsr(1);
   TpuIsr(2);
   TpuIsr(3);
   TpuIsr(4);
   TpuIsr(5);
   TpuIsr(6);
   TpuIsr(7);
   TpuIsr(8);

/**********************************************************************/
/* Abort button ISR                                                   */
/**********************************************************************/

   asm(" .globl _Abort_Isr
_Abort_Isr:
	 movew #Tg8DISABLE_INTERRUPTS,SR
         movel #JmpToMonitor,sp@(2)
         rte
         .globl JmpToMonitor
JmpToMonitor: ");
         INTOFF;            /* Disable interrupts */
         tpu->TpuIe = 0x0;  /* Disable TPU interrupts 0-8 */
	 sim->SimProtect = 0; /* Disable the watchdog */
    asm volatile ("
         movel #0x6e12a,a0  /* and start the 332 BUG */
         jmp   a0@");

/********************************************************************/
/* Spurious interrupts ISR                                          */
/********************************************************************/

   asm volatile("
	 .globl _Spurious_Isr
_Spurious_Isr:
	 movel d0,sp@-
	 movel Tg8DSC_INTERRUPT,d0              /* Clear DSC Interrupt */
	 movel sp@+,d0
	 addql #1,At+aNumOfSpur
	 rte
   ");

/********************************************************************/
/* Bus/Address/PrivViolation errors ISR                             */
/********************************************************************/

   asm(" .globl _BusError_Isr
_BusError_Isr:
	 .globl _AddressError_Isr
_AddressError_Isr:
	 .globl _PrivViolation_Isr
_PrivViolation_Isr:
	 addql #1,At+aNumOfBus
	 rte ");

/********************************************************************/
/* ISR for interrupts provoked by the DSC			    */
/********************************************************************/

   GeneralIsr(Dsc);

/********************************************************************/
/* Default Interrupt service routine				    */
/********************************************************************/

asm("
 .globl _Default_Isr
 _Default_Isr:
  movew #Tg8DISABLE_INTERRUPTS,SR
  movew sp@(6),ExceptionVector
  movel sp@(2),ExceptionPC
  stop  #Tg8DISABLE_INTERRUPTS
  rte" );

}

void dummyIsr() {
  register Word err;

   asm(" .text");

/***********************************************************************/
/* Interrupt service routine for interrupts of incoming timing frames. */
/* It reads the timing frame, makes the simplest checks, performs      */
/* special processing of some types of events and creates an entry for */
/* new AT-process if the processing of Action Table for the event is   */
/* required.                                                           */
/* ***** WARNING ***** BE SURE (A6) IS NOT USED INSIDE THE CODE        */
/***********************************************************************/

   asm (" .globl _Xr_Isr
_Xr_Isr:
	  movew   #Tg8DISABLE_INTERRUPTS,SR
	  moveml  d0-d7/a0-a6,_xr_context" );

   if (!camBusy && !atQueue.Size) {

     /* It gives the possibility to launch the AT-process from the ISR. */

     if (eprog.ImmRun) { /* IMM proc. was interrupted */
	 asm (" movew   sp@,_imm_ccr    /* Save CCR */
                movel   sp@(2),_imm_pc  /* Save PC */
	        moveml  d0-d7/a0-a6,_imm_context /* Save registers */
	      ");
       } else { /* MBX proc. was interrupted */
	 asm (" movew   sp@,_mbx_ccr    /* Save CCR */
                movel   sp@(2),_mbx_pc  /* Save PC */
	        moveml  d0-d7/a0-a6,_mbx_context /* Save registers */
	      ");
       };
   };

   /*  Read the timing frame */

   asm(" movel  XWsscRframe1,d0       /* Read frame's word1 and word2 */
	 rorw   #8,d0                 /* Unscramble incomming frame byte order */
	 movew  d0,_timing_frame+2
	 swap   d0
	 rorw   #8,d0
	 movew  d0,_timing_frame
       ");

   curhd = timing_frame.Any.Header;     /* Current header */

   /* ======================================================== */
   /* Deal with the millisecond as soon as possible            */

   if (curhd == 0x01) {

     /* Normal ms event processing. Keep a scew corrected ms counter */
     /* running in DPRAM. The ms modulo represents accelerator time. */

     curms = timing_frame.Short.Event_half2;
     incms = curms - lstms;
     if (incms > 4) {           /* Happens at cycle boundaries */
	if (curms < 4) {
	   incms = curms + 1;   /* If we missed one this is the inc */
	   lstms = curms;
	} else {
	   incms = 1;           /* Garbage in curms */
	   lstms++;
	   curms = lstms + 1;   /* A resonable value */
	}
     } else
	lstms = curms;

     if (curms > sscms) sscms  = curms;
     else               sscms += incms;

     dpm->At.aScTime  = sscms;

     dpm->At.aMsEvent          = timing_frame;
     dpm->At.aDt.aMilliSecond += incms;

     err = xlx->WClrRerr;  /* gcc will use a register here. Donnt move this command! */
     rcv_error.Err = err;
     err = *(Word*)&rcv_error;
     *(Word*)&(dpm->At.aDt.aRcvErr) = err; /* RcvErr & Hour bytes */

     /* Be sure the AT queue is empty. That's FATAL! */

     if (atQueue.Size) {
       dpm->At.aAlarms |= Tg8ALARM_MOVED_PROC;
       dpm->At.aMovedFrames = atQueue.Size;
       dpm->At.aMovedScTime = dpm->At.aScTime;
       dpm->At.aMovedSc = dpm->At.aSc;
     } else
       dpm->At.aProcFrames = 0;

     if (rcv_error.Err) {

       /* Analyze an error ... */

       xlx->WClrRerr = 0; /* Reset an error */
       clk.Clock_p->cMsEvent = dpm->At.aMsEvent;   /* The last valid ms frame */
       clk.Clock_p->cSc = dpm->At.aSc;             /* The Super Cycle number */
       clk.Clock_p->cOcc= dpm->At.aScTime;         /* Occurence time from the SC start */

       /* Copy rcverr,h,m,s */

       *(int*)&clk.Clock_p->cRcvErr = *(int*)&dpm->At.aDt.aRcvErr;

       /* Select the next record */

       if (++clk.Clock_i >= Tg8CLOCKS) {
	 clk.Clock_i = 0;
	 clk.Clock_p = clk.Clocks;
       } else
	 clk.Clock_p++; /* Take the next entry */
       clk.Clock_p->cNumOfMs = 0;
     };

     if (eprog.ImmSize) { doImm = 1; goto imediate; }

     /* Avoid starting of the AT process every ms if not required */

     if (in_use.Time_event[timing_frame.Simple.Event_code] &
	 (1<<MILLISECOND_EVENT_INDEX)) goto imediate;
     goto ret_from_int;
   }

   /* ======================================================== */
   /* Next Throw away telegrams and new format events          */

   if   (((curhd & 0x0F) == 0x04)
   ||    ((curhd & 0x0F) == 0x03))
      goto ret_from_int;

   /* ======================================================== */
   /* Now its time to process the SSC event                    */

   if (curhd == 0x20) {

      /* The next S-Cycle starts */

      remms = sscms % 1200;
      if (remms) remms = 1200 - remms;
      sscms += remms;

      dpm->At.aScLen     = sscms;
      dpm->At.aScTime    = curms;
      sscms              = curms;
      dpm->At.aPrNumOfEv = dpm->At.aNumOfEv;
      dpm->At.aNumOfEv   = 0;

      dpm->At.aSc = (timing_frame.Ssc.Scn_low)      +
		    (timing_frame.Ssc.Scn_mid << 8) +
		    (timing_frame.Ssc.Scn_msb << 16);

      dpm->At.aNumOfSc++;
    }

   /* ======================================================== */
   /* Event history, store what ever can trigger an action     */
   /* after processing.                                        */

   if ((curhd & 0xF0) == 0x20) {

      dpm->At.aEvent = timing_frame;
      dpm->At.aNumOfEv++;

      /* Insert the received event into the event history */

      hist.History_p->hEvent = timing_frame;   /* The received frame */
      hist.History_p->hSc = dpm->At.aSc;       /* The Super Cycle number */
      hist.History_p->hOcc= dpm->At.aScTime;   /* Occurence time from the SC start */

      /* Copy rcverr,h,m,s */

      *(int*)&hist.History_p->hRcvErr = *(int*)&dpm->At.aDt.aRcvErr;

      if (++hist.History_i >= Tg8HISTORIES) {
	hist.History_i = 0;
	hist.History_p = hist.Histories;
      } else
	hist.History_p++; /* Take the next entry */
   }

   /* ======================================================== */

   /*****************************************************************/
   /* Make a special processing of the timing frame                 */

   switch(curhd) {

   /**************************/
   /* Tg8's Date/Time events */
   /**************************/

   case TgvUTC_LOW_HEADER:
     utc = timing_frame.Utc.UtcWord;
     break;

   case TgvUTC_HIGH_HEADER:
     utc |= ((unsigned long) timing_frame.Utc.UtcWord) << 16;
     utcl = utc;
     UtcToTime(); utc = 0;
     rcv_error.Hour = dpm->At.aDt.aHour;
     dpm->At.aDt.aMsDrift = dpm->At.aDt.aMilliSecond;
     dpm->At.aDt.aMilliSecond = 0;
     break;

   default: /* IGNORE any unknown events */

     switch (curhd >> 4) { /* Take machine code */

     case Tg8SPS:
       ServeSimpleEvent(SPS);
       break;

     case Tg8NO_MACHINE:
       if (in_use.Time_event[timing_frame.Simple.Event_code] &
	   (1<<ANY_OTHER_EVENT_INDEX)) break;

     default:
       break; /*goto ret_from_int;*/
     };
     break; /* Serve an event */

   }  /* end of switch(hdr) statement */

   /*******************************************************************/
   /* Create an AT-process to process the Action Table with incoming  */
   /* timing frame. Put the AT process on the queue.                  */
   /*******************************************************************/

imediate:

   if (atQueue.Size>=4) {
     dpm->At.aAlarms |= Tg8ALARM_QUEUED_PROC;
     if (atQueue.Size >= QUEUE_SIZE) {
       dpm->At.aAlarms |= Tg8ALARM_MANY_PROC;
       goto ret_from_int;
     };
   };

   dpm->At.aQueue[atQueue.Size] = timing_frame;

   atQueue.Size++;
   *atQueue.Top++ = timing_frame.Long; /* Put the frame in the AT-queue */

   if (immQueue.Size >= QUEUE_SIZE)
     dpm->At.aAlarms |= Tg8ALARM_IMMQ_OVF;
   else {
     immQueue.Size++;
     *immQueue.Top++ = timing_frame.Long; /* Put the frame in the IMM-queue */
   };

   dpm->At.aQueueSize = atQueue.Size;
   if (atQueue.Size > dpm->At.aMaxQueueSize) dpm->At.aMaxQueueSize = atQueue.Size;

   if (atQueue.Size == 1 && !camBusy) {
      /* Start the AT process */
      asm volatile ("
      movel  #_AtProcess,sp@(2)
      rte " );
   };

ret_from_int:

   /* Restore registers and return from the interrupt */

   asm volatile ("
   moveml  _xr_context,d0-d7/a0-a6
   rte ");

/*******************************************************************/
/* Trap-routine which is called from lower priority Imm process.   */
/* It starts the execution of the AT-process.                      */
/*******************************************************************/

   asm(" .globl _AtStartProcess
_AtStartProcess:
	 movew  #Tg8DISABLE_INTERRUPTS,SR    /* Mask all interrupts  */
         movew   sp@,_imm_ccr   /* Save CCR */
         movel   sp@(2),_imm_pc /* Save PC */
	 moveml  d0-d7/a0-a6,_imm_context /* Save registers */
         movel  #_AtProcess,sp@(2) /* Serve the next entry in the AT queue */
	 rte
   ");

/***********************************************************************/
/* Trap-routine which is called on completion of AT-process.           */
/* It removes the entry from the corresponding AT-queue and            */
/* deallocates memory associated with the process. This is done in     */
/* non-interruptable state because the AT-queue is a source which is   */
/* shared with XrIsr routine.					       */
/***********************************************************************/

   asm(" .globl _AtCompletion
_AtCompletion:
	 movew  #Tg8DISABLE_INTERRUPTS,SR  ");  /* Mask all interrupts  */

   if (--atQueue.Size == 0) {  /* No more events. Go to the IMM process */
      atQueue.Top = atQueue.Bottom = atQueue.Buffer;

      if (eprog.ImmRun) { /* IMM proc. was interrupted by the Xr Isr. */
	                  /* Just restore its context. */
	asm volatile (" movew  _imm_ccr,sp@
		        movel  _imm_pc,sp@(2)
		        moveml _imm_context,d0-d7/a0-a6
		        rte");
      };

      /* Start the IMM process */

      asm volatile ("
         movew  #1,_eprog+ImmRun
         movel  #_ImmProcess,sp@(2) /* Serve the next entry in the IMM queue */
	 rte");
   };

   /* Serve the next entry in the AT queue */

   atQueue.Bottom++; /* Move the AT-queue */
   asm volatile (" movel  #_AtProcess,sp@(2)
		   rte");

/**********************************************************************/
/* Processing of Action Table (AT-process).                           */
/**********************************************************************/

   asm volatile (".globl _AtProcess
_AtProcess: ");

at_proc:
   eprog.Event.Long = *atQueue.Bottom;

   /* Get the wildcard used for the given event code */

   if (eprog.Event.Any.Header != 0x01) { /* SSC/Simple/SC events */
     eprog.WildCard = in_use.Simple_wc[eprog.Event.Simple.Event_code] &
                      in_use.SimpleH_wc[eprog.Event.Any.Header];
   } else { /* Calendar, ms, ... */
     eprog.WildCard = in_use.Time_wc[eprog.Event.Simple.Event_code] &
                      in_use.TimeH_wc[eprog.Event.Any.Header];
   };

   asm ("
   /* For each wild card combination present scan the CAM */

   movew   _eprog+WildCard,d4    /* d4 is the wildcards combination mask */
   beq     5f                    /* An event is not in use - escape */

   movel   _eprog+Event,d1	 /* d1 is the queued timing frame */
   lea     _wild_c,a0            /* a0 points to the wildcards */
   moveal  _cam,a1               /* a1 points to the CAM register */
   moveal  #_match+Matches,a2	 /* a2 points to the resulting list      */

0: btst    #0,d4                 /* Is next wildcard presents? */
   beq     3f                    /* No, try next */

   /* Look for an event */

   movel   d1,d2                  /* We will OR the timing frame with    */
   orl     a0@,d2                 /* the wildcard bit mask               */
   movel   d2,_eprog+Event        /* to the comparand register in 3 steps*/

   /* Load the template for the CAM */

   oriw    #CamDATA_MODE,SimDataF1  /* Set Data mode for CAM             */
   movew   _eprog+Event,a1@         /* Load the first 16 bits            */
   movew   _eprog+Event+2,a1@       /* Load the second 16 bits           */
   movew   #0xFF,a1@		    /* Load 3rd word (0xFF for chan. output) */

   /* Start the CAM looking for the template */
   andw    #CamCOMMAND_MODE,SimDataF1 /* Set Command mode for CAM */

   /* Wait for the match to be completed while setting command mode */
1: movel   d0,d0                   /* The matching is in progress: wait   */
   movew   a1@,d0                  /* Read the CAM status                 */
   blt     2f                      /* If no more matches, escape          */

   /* For each match place the action number in resulting buffer (Matches)*/

   moveb   d0,a2@+                 /* Put it to the resulting list        */
   btst    #CamSTATUS_MULTIPLE,d0  /* Multiple matches ?                  */
   bne     2f                      /* No -> break                         */
   andiw   #CamBYTE_MASK,d0        /* Which is the action number          */
   oriw    #CamSET_SKIP_BIT,d0     /* Set the Skip bit for the match      */
   movew   d0,a1@                  /* And look for the next match         */
   bra     1b

   /* No more matches for that wildcard - stop the CAM work */

2: movew   #CamCLEAR_SKIP_BITS,a1@  /* Clear all skip bits */

  /* Try with the next wildcard type */

3: lsrw    #1,d4                /* Try the next one */
   tstw    d4
   beq     4f                   /* No more wildcards at all, escape */

   addql   #4,a0                /* The next wildcard combination */
   bra     0b                   /* Loop ... */

   /* Start all the actions we have prepared in the matching list */
4:
   cmpal   #_match+Matches,a2	/* a2 points to the resulting list */
   beq     5f

   movel   a2,_match+EndMa     	/* Set the End of list (match.End)	*/
   bsr     _StartActions	/* Post-processing of the list of started actions */

5: ");

   /* Move the AT queue. Select the next entry if any */

   dpm->At.aProcFrames++;
   dpm->At.aQueueSize--;

   if (atQueue.Size > 1) {
     atQueue.Size--;
     atQueue.Bottom++;
     goto at_proc;   /* Serve the next event */
   };

asm(" trap #0 "); /* End of the queue */

} /* <<dummyIsr>> */

/************************************************************************/
/* Start actions selected by the AtProcess                      	*/
/************************************************************************/

void StartActions() {
Tg8Action    *a ,*aa;
Tg8Interrupt *ip;
Tg8IntAction  u ,uu;
int j; Byte *alist; short actn, cn, mach,gt,gv;

  for (alist = match.Matches; alist != match.EndMa; alist++) {

    actn = *alist;
    a = act.Pointers[actn];   /* Take the action's descriptor */

    if (!(a->Enabled &Tg8ENABLED)) continue; /* That action is disabled */

    /* Start counting by using of the specified counter. */

    cn = a->CntNum;
    xlx->XlxDelay [cn] = a->User.uDelay;
    xlx->XlxConfig[cn] = a->CntControl;

    /* Fill the counter's interrupt table entry in */

    ip = (Tg8Interrupt *) &dpm->It.CntInter[cn];
    eprog.iFired[cn] = ip;       /* TPU int.handler uses this value */
    eprog.rFired[cn] = &a->Rec;  /* --"-- */

    /* Fill the recording table entry in.                */
    /* This entry is the part of the Action Description. */

    a->Rec.rSc = dpm->At.aSc;     /* The Super Cycle number */
    a->Rec.rOcc= dpm->At.aScTime; /* Occurence time from the start of the last S-Cycle */
    a->Rec.rOut= 0xFFFFFFFF;
    a->Rec.rNumOfTrig++;          /* How many times an action was triggered */

    LW(u,ip->iExt);               /* Read the present interr. data into the RAM */
    if (ip->iOut == 0xFFFFFFFF)   /* The counter is till counting - error */
      SignalAlarm(Tg8ALARM_LOST_OUT);

    if (u.iSem && (a->CntControl & ConfINTERRUPT)) {
      /* Interrupt will be lose */
      eprog.iFired[cn] = &sim_int; /* Safe for the int. table */
      a->Rec.rOvwrAct = u.iAct;    /* Source of an overwitting counting */
      a->Rec.rOvwrCnt++;           /* Number of overwritten counting */
      SignalAlarm(Tg8ALARM_INT_LOST);
      continue;
    };

    /* Fill the interrupt table entry in. Here the 'ip' points to that entry. */
    /* This information is used by the driver only. */

    ip->iEvent.Long = *atQueue.Bottom; /* Take the trigger event frame */
    ip->iSc = dpm->At.aSc;             /* The Super Cycle number */
    ip->iOcc= dpm->At.aScTime;         /* Occurence time from the start of the last S-Cycle */
    ip->iOut= 0xFFFFFFFF;              /* Output time is unknown yet */
    uu.iRcvErr = rcv_error.Err;        /* Place the receiver error code here */
    uu.iOvwrAct = 0;                   /* No problems about of action overwritting */

    uu.iAct = actn+1;                  /* Place the number of the current action */
    uu.iSem = (a->CntControl & ConfINTERRUPT)? 1: 0; /* Protect the data */
    LW(ip->iExt,uu);                   /* Insert the new data into the interr. table */
  };

} /* <<StartActions>> */

/* eof isr.c */
