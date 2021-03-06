/**********************************************************************/
/* Immediate actions Service Routines for the Tg8's firmware program. */
/* Vladimir Kovaltsov for the SL Version, February, 1997	      */
/**********************************************************************/

void dummyImmEtc() {

/***********************************************************************/
/* Trap-routine which is called on completion of IMM-process.          */
/* It removes the entry from the corresponding IMM-queue and           */
/* deallocates memory associated with the process. This is done in     */
/* non-interruptable state because the IMM-queue is a source which is  */
/* shared with XrIsr routine.					       */
/***********************************************************************/

   asm(" .globl _ImmCompletion
_ImmCompletion:
	 movew  #Tg8DISABLE_INTERRUPTS,SR  ");  /* Mask all interrupts  */

   if (--immQueue.Size == 0) {  /* No more events. Go back to the MbxProcess */
      immQueue.Top = immQueue.Bottom = immQueue.Buffer;
      eprog.ImmRun = 0;
      asm volatile (" movew  _mbx_ccr,sp@
		      movel  _mbx_pc,sp@(2)
		      moveml _mbx_context,d0-d7/a0-a6
		      rte");
   };

   /* Serve the next entry in the IMM queue */

   immQueue.Bottom++; /* Move the IMM-queue */
   asm volatile (" movel  #_ImmProcess,sp@(2)
		   rte");

/**********************************************************************/
/* Processing of the Action Table for IMMEDIATE actions in the        */
/* interruptable state.                                               */
/**********************************************************************/

   asm volatile (".globl _ImmProcess
_ImmProcess: ");

imm_proc:

   eprog.ImmEvent.Long = *immQueue.Bottom;

   /*********************************************************/
   /* Copy the block of immediate interrupts into the DPRAM */
   /*********************************************************/

   if (doImm /***??? && eprog.ImmEvent.Any.Header == Tg8MILLISECOND_HEADER***/) {
     dpm->At.aTrace = eprog.ImmSize;
     if (eprog.IntSrc & Tg8ISM_IMM) {
       dpm->At.aAlarms |= Tg8ALARM_MOVED_IMM;
       /* Overwrite the oldest events ??? */
     } else {
       /* Copy the newest interrupts */
       memcpy16((short*)dpm->It.ImmInter,(short*)eprog.ImmInt,
		eprog.ImmSize * sizeof(Tg8Interrupt));
       SignalToHost(Tg8IS_IMM);
       if (!dpm->At.aIntSrc) { /* DSC ready, generate VME bus interrupt immediately */
	 /* Issue the real VME BUS interrupt */
	 asm volatile (ISM_SET_TRAP);
       };
       eprog.ImmSize = eprog.ImmNxt = 0;
     };
     doImm = 0;
   };

   /* Get the wildcard used for the given event code */

   if (eprog.ImmEvent.Any.Header != 0x01) { /* SSC/Simple/SC events */
     eprog.ImmWildCard = in_use.Simple_wc_imm[eprog.ImmEvent.Simple.Event_code] &
                         in_use.SimpleH_wc[eprog.Event.Any.Header];
   } else {
     eprog.ImmWildCard = in_use.Time_wc_imm[eprog.ImmEvent.Simple.Event_code] &
                         in_use.TimeH_wc[eprog.Event.Any.Header];
   };

   asm ("
   /* For each wild card combination present scan the CAM */

   moveal  #_match+ImmMatches,a2 /* a2 points to the resulting list      */
   movew   _eprog+ImmWildCard,d4 /* d4 is the wildcards combination mask */
   beq     5f                    /* An event is not in use - escape */

   movel   _eprog+ImmEvent,d1	 /* d1 is the queued timing frame */
   lea     _wild_c,a0            /* a0 points to the wildcards */
   moveal  _cam,a1               /* a1 points to the CAM register */

0: btst    #0,d4                 /* Is a wildcard present? */
   beq     3f                    /* No, try next */

   /* Look for an event */

   movel   d1,d2                  /* We will OR the timing frame with    */
   orl     a0@,d2                 /* the wildcard bit mask               */
   movel   d2,_eprog+ImmEvent     /* to the comparand register in 3 steps*/

   /* Load the template for the CAM. Interrupts OFF */

   movew   #1,_camBusy              /* Protect the CAM access for the AT-process */

   oriw    #CamDATA_MODE,SimDataF1  /* Set Data mode for CAM             */
   movew   _eprog+ImmEvent,a1@      /* Load the first 16 bits            */
   movew   _eprog+ImmEvent+2,a1@    /* Load the second 16 bits           */
   movew   #0x00,a1@		    /* Load 3rd word (ZERO for imm.action) */

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

   /* No more matches for that wildcard - stop the CAM work. Interrupts ON */

2: movew   #CamCLEAR_SKIP_BITS,a1@  /* Clear all skip bits */

   /* Start the AT-process if the frames queue is not empty now */

   movew   #Tg8DISABLE_INTERRUPTS,SR 
   tstl    _atQueue+Size
   beq     5f                   /* No pending frames, continue */

   movew   #0,_camBusy          /* Allow the CAM access for the AT-process */
   trap    #4                   /* and start the AT-process. */

5: movew   #0,_camBusy          /* Allow the CAM access for the AT-process */
   movew  #Tg8ENABLE_INTERRUPTS,SR 

  /* Try with the next wildcard type */

3: lsrw    #1,d4                /* Try the next one */
   tstw    d4
   beq     4f                   /* No more wildcards at all, escape */

   addql   #4,a0                /* The next wildcard type */
   bra     0b                   /* Loop ... */
4:
   cmpal   #_match+ImmMatches,a2 /* a2 points to the resulting list */
   beq     5f

   movel   a2,_match+ImmEndMa    /* Set the End of list (match.End) */
   bsr     _StartImmActions	 /* Post-processing of the list of started actions */

5: ");

   /****************************************************/
   /* Move the IMM queue. Select the next entry if any */
   /****************************************************/

   if (immQueue.Size > 1) {
     immQueue.Size--;
     immQueue.Bottom++;
     goto imm_proc;   /* Serve the next event */
   };

  /* Mark the DPRAM was changed (for the driver only) */

   dpm->At.aSem++;

   asm(" trap #5 "); /* End of the queue */
}

/***************************************************************/
/* Start all the actions we have prepared in the matching list */
/* for the given trigger-frame.                                */
/***************************************************************/

  static Tg8Action    *a,*aa;
  static Tg8Interrupt *ip;
  static Tg8IntAction  u,uu; static Word actn, dim,mach,gt,gv;
  static Byte *alist;

void StartImmActions() {

   for (alist = match.ImmMatches; alist < match.ImmEndMa; alist++) {

     actn = *alist;
     a = act.Pointers[actn];             /* Take the action's descriptor */

#if 0
     if (mach=a->Gate.Machine) { /* use the extra PLS condition ! */
       gt = mach & 0xF;     /* Take the group type */
       mach >>= 4;          /* and the Tg8 machine number */
       dim = a->Enabled>>1; /* Get the timing unit dimension */
                            /* The current value of that group: */
       gv = tel.aTeleg[mach][a->Gate.GroupNum-1];
       if (gt == Tg8GT_EXC) { /* That is the special case !! */
	 /* Here 'dim' is the upper limit for GV (32 is maximum) */
	 if (gv==0 || gv > dim)
	   continue; /* The GV is outside of the range used by the action */
	 gv--;
	 a += gv; /* Take the PPM line corresponding to the current group value */
	 actn += gv; /* and its number in the user table */
	 if (!(a->Enabled &Tg8ENABLED)) continue; /* That line is disabled */
	 /* The PPM line is declared and enabled, hence start it */
       } else {
	for (;dim>0; dim--,actn++,a++) {
	  if (!(a->Enabled &Tg8ENABLED)) continue; /* That line is disabled */
	  if (gt == Tg8GT_BIT) {
	    if (gv & a->Gate.GroupVal) break; /* The PLS checking successed */
	  } else
	    if (gv == a->Gate.GroupVal) break; /* The PLS checking successed */
	};
	if (!dim) continue; /* Nothing to start */
       };
     } else
#endif
      if (!(a->Enabled &Tg8ENABLED)) continue; /* That action is disabled */

     ip = &eprog.ImmInt[eprog.ImmNxt++]; /* Take the next free interr. descriptor */
     if (eprog.ImmSize < Tg8IMM_INTS) {
       eprog.ImmSize++;
       ip->iExt.iSem = 0;    /* New free item */
     } else {
       /* Error - the Queue is full */
       SignalAlarm(Tg8ALARM_LOST_IMM);
       if (eprog.ImmNxt > Tg8IMM_INTS) {
	 eprog.ImmNxt = 0;   /* Overwrite the oldest item */
	 ip = eprog.ImmInt;
       };
     };

     /* Update the records table */

     a->Rec.rSc = dpm->At.aSc;      /* The Super Cycle number */
     a->Rec.rOcc= dpm->At.aScTime;  /* Occurence time from the start of the last S-Cycle */
     a->Rec.rOut= a->Rec.rOcc;      /* Output time is equal to the current time */
     a->Rec.rNumOfTrig++;           /* How many times an action was triggered */

     LW(u,ip->iExt);               /* Read the present interr. data into the RAM */
     uu.iRcvErr = rcv_error.Err;   /* Place the receiver error code here */
     if (u.iSem) {
       /* Interrupt will be lose */
       a->Rec.rOvwrAct = u.iAct;  /* Source of an overwitting counting */
       a->Rec.rOvwrCnt++;         /* Number of overwritten counting */
       SignalAlarm(Tg8ALARM_INT_LOST);
       continue;
     };
#if 0
     if (u.iSem) {
       /* The current action will overwrite the old one ! */
       uu.iOvwrAct = u.iAct;        /* Which action was overwritten */
       aa = act.Pointers[u.iAct-1]; /* Get its descriptor */
       aa->Rec.rOvwrAct = actn+1;   /* Source of an overwitting counting */
       aa->Rec.rOvwrCnt++;          /* Number of overwritten counting */
     } else
#endif
       uu.iOvwrAct = 0;           /* No problems about of action overwritting */

     ip->iEvent.Long = *immQueue.Bottom; /* The trigger event frame */
     ip->iSc = dpm->At.aSc;       /* The Super Cycle number */
     ip->iOcc= dpm->At.aScTime;   /* Triggered time */
     ip->iOut= ip->iOcc;          /* Output time is equals to the current time */
     uu.iAct = actn+1;            /* Place the number of the current action */
     uu.iSem = 1;                 /* Block the data */
     LW(ip->iExt,uu);             /* Insert the new data into the interr. table */

   };/*actions*/
}

/* eof */
