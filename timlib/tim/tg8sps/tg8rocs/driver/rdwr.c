/***************************************************************************/
/* Tg8 Device driver - select, read, write functions.                      */
/* November 1993. CERN/PS Version. Julian Lewis.			   */
/* Vladimir Kovaltsov for ROCS Project, February, 1997			   */
/***************************************************************************/

/***************************************************************************/
/* SELECT                                                                  */
/***************************************************************************/
#define MJJ20031016 /* activate correction from M.Jonker on 2003 10 16 */

int Tg8DrvrSelect(wa, flp, wch, ffs)
Tg8DrvrWorkingArea * wa;        /* Working area */
struct file * flp;              /* File pointer */
int wch;                        /* Read/Write direction */
struct sel * ffs; {             /* Selection structurs */

int mnum,dnum;                  /* Module/Devive number */
Tg8DrvrModuleContext * mcon;    /* Module context */
Tg8DrvrDeviceContext * dcon;    /* Device context */

   dnum = minor(flp->dev) -1;
   mnum = dnum / Tg8DrvrDEVICES;
   if (dnum < 0 || mnum >= Tg8DrvrMODULES) {
      pseterr(EBADF);           /* Bad file number */
      return(SYSERR);
   };

   /* We cant read a file that is not open. */

   mcon = wa->ModuleContexts[mnum];
   if (!mcon) {
      pseterr(ENODEV);          /* No such device */
      return(SYSERR);
   };

   dnum %= Tg8DrvrDEVICES;
   dcon = &mcon->Device[dnum];

   /* Hand the system back the semaphore if reading. */

   if (wch == SREAD) {
      ffs->iosem = (int *) &dcon->AppSemaphore;
      return(OK);
   };

   pseterr(EACCES);         /* Permission denied */
   return(SYSERR);

} /* <<Tg8DrvrSelect>> */

/***************************************************************************/
/* READ                                                                    */
/* Read the queue of the events (1 entry only)                             */
/***************************************************************************/

int Tg8DrvrRead(wa, flp, u_buf, cnt)
Tg8DrvrWorkingArea * wa;        /* Working area */
struct file * flp;              /* File pointer */
char * u_buf;                   /* Users buffer */
int cnt; {                      /* Byte count in buffer */

Tg8DrvrModuleContext * mcon;  /* Module context */
Tg8DrvrDeviceContext * dcon;  /* Device context */
Tg8DrvrEvent         * conn;  /* Queued event */
Tg8DrvrQueue         * queue; /* Driver's queue */

int mnum,dnum,dm; /* Module/Device/DevMask numbers */
int wcnt; /* Writable byte count at u_buf address */
int *mt;
unsigned long scnum, b1, b2;

   dnum = minor(flp->dev) -1;
   mnum = dnum / Tg8DrvrDEVICES;
   if (dnum < 0 || mnum >= Tg8DrvrMODULES) {
      pseterr(EBADF);           /* Bad file number */
      return(SYSERR);
   };

   /* We cant read a file that is not open. */

   mcon = wa->ModuleContexts[mnum];
   if (!mcon) {
      pseterr(ENODEV);          /* No such device */
      return(SYSERR);
   };

   dnum %= Tg8DrvrDEVICES;
   dm = 1<<dnum;
   dcon = &mcon->Device[dnum];

   /* Check the user has specified a real address at which "cnt" bytes */
   /* can be written without provoking a segmentation fault in the driver. */

   wcnt = wbounds((unsigned long) u_buf); /* Number of user writable bytes without error */
   if (wcnt < cnt || cnt!=sizeof(Tg8DrvrEvent)) {
      /* Specified user byte count would provoke error */
      pseterr(EINVAL);    /* Invalid argument */
      return(SYSERR);
   };

   if (!mcon->UserTable.Size) return 0; /* No actions being declared */

   if (!dcon->AppSemaphore) { /* Has to wait */
     if (dcon->Mode &FNDELAY) {
       pseterr(EWOULDBLOCK);
       return(SYSERR);
     };
   };

   if (dcon->AppSemaphore>Tg8DrvrQUEUE_SIZE)
     dcon->AppSemaphore = Tg8DrvrQUEUE_SIZE;

#if 0
   CancelTimeout(&dcon->Timer);
   dcon->Timer = timeout(HandleTimeout, dcon, mcon->TimeOut);
   if (dcon->Timer <= 0) {
     dcon->Timer = 0;
     pseterr(EAGAIN);
     return(SYSERR);
   };
#endif
   queue = &mcon->Queue;
   mt = mcon->UserTable.DevMask;

wait_ev:
   if (swait(&dcon->AppSemaphore, SEM_SIGABORT)) {
     /*** CancelTimeout(&dcon->Timer); ***/
     pseterr(EINTR); /* Interrupted */
     return (SYSERR);
   };
#if 0
   if (!dcon->Timer) { /* Timed out */
     ((Tg8DrvrEvent*)u_buf)->Id = 0;
     return 0;
   };
#endif
   /* Take the oldest event from the queue and move the pointer */

   for (;;) {
     if (dcon->Tail == queue->Head) { /* The queue was overwritten fast */
       /*** sreset(&dcon->AppSemaphore); ***/
       goto wait_ev;
     };
     conn = &queue->Queue[dcon->Tail++];
     if (dcon->Tail >= Tg8DrvrQUEUE_SIZE) dcon->Tail = 0;
     if (conn->DevMask &dm) { /* Found */
       /*** CancelTimeout(&dcon->Timer); ***/

       /* Julian 26/07/2005 Super-cycle number mod */

       scnum = conn->Inter.iSc;
       scnum = (0x00FFFF00 & scnum) >> 8; /* Throw away msb */
       b1 = scnum & 0xFF;
       b2 = (scnum >> 8) & 0xFF;
       scnum = (b1 << 8) | b2;            /* Swap bytes */
       conn->Inter.iSc = scnum;

       *(Tg8DrvrEvent*) u_buf = *conn;
       return(cnt);
     };
   };

} /*<<Tg8DrvrRead>>*/


/***************************************************************************/
/* WRITE                                                                   */
/* Write a part of the user table.                                         */
/***************************************************************************/

int Tg8DrvrWrite(wa, flp, u_buf, cnt)
Tg8DrvrWorkingArea * wa;        /* Working area */
struct file * flp;              /* File pointer */
char * u_buf;                   /* Users buffer */
int cnt; {                      /* Byte count in buffer */

int i, an, mnum,dnum,dm;	/* Action/Module/Device/DevMask number */
int rcnt,cw,mw,st;     		/* Readable byte count at the arg address */
int hole,*mt;

Tg8DrvrModuleContext     * mcon;   /* Module context */
Tg8DrvrDeviceContext     * dcon;   /* Device context */
Tg8DrvrUserTable         * tab;    /* User table */
Tg8User                  * act,*a; /* Users Action */

   dnum = minor(flp->dev) -1;
   mnum = dnum / Tg8DrvrDEVICES;
   if (dnum < 0 || mnum >= Tg8DrvrMODULES) {
      pseterr(EBADF);           /* Bad file number */
      return(SYSERR);
   };

   /* We cant read a file that is not open. */

   mcon = wa->ModuleContexts[mnum];
   if (!mcon) {
      pseterr(ENODEV);          /* No such device */
      return(SYSERR);
   };

   dnum %= Tg8DrvrDEVICES;
   dm = 1<<dnum;
   dcon = &mcon->Device[dnum];

   /* Check the user has specified a real address at which "cnt" bytes */
   /* can be read without provoking a segmentation fault in the driver. */

   rcnt = rbounds((unsigned long) u_buf); /* Number of user readable bytes without error */
   if (rcnt < cnt || cnt<sizeof(Tg8User)) {
      /* Specified user byte count would provoke error */
      pseterr(EINVAL);    /* Invalid argument */
      return(SYSERR);
   };

   /* Write onto the user table */

   tab = &mcon->UserTable;

   /* Some fields can be changed by the driver, so ignore them in comparizon. */
   mw = 0;
#ifdef MJJ20031016
   Tg8CW_INT_Set(mw,Tg8CW_INT_BITM);
#else
   Tg8CW_INT_Set(mw,Tg8DO_INTERRUPT);
#endif
   Tg8CW_STATE_Set(mw,Tg8CW_STATE_BITM);
   Tg8CW_PPML_Set(mw,Tg8CW_PPML_BITM);
   mw = ~mw;

   for (rcnt=0,act=(Tg8User*)u_buf; rcnt<cnt; rcnt+=sizeof(Tg8User),act++) {

     /* Check action's validaty */
     cw = act->uControl & Tg8CW_MASK;
     if (cw != act->uControl || act->uDelay == 0xFFFF || !act->uEvent.Long ||
	 (Tg8CW_INT_Get(cw)==Tg8DO_INTERRUPT && (act->uDelay || (act->uControl&mw)))) {
       pseterr(EINVAL);    /* Invalid argument */
       return(SYSERR);
     };

     /* Look for the same action might be defined already. */

     st = Tg8CW_STATE_Get(act->uControl);
     hole = -1;
     for (i=0,a=tab->Table,mt=tab->DevMask; i<tab->Size; i++,a++,mt++) {
       if (a->uEvent.Long == 0) { /* Unused item */
	 if (hole<0) hole = i;
	 continue;
       };
       if (a->uEvent.Long == act->uEvent.Long &&
	   (mw&a->uControl) == (mw&act->uControl) &&
	   a->uDelay == act->uDelay) {
#ifdef MJJ20031016
         /* set the device usage bit if a VME interrupt requested. */
         if( (Tg8CW_INT_Get(act->uControl)&Tg8DO_INTERRUPT) ) *mt |= dm;

         /* Do we have to activate the output request? */
         if(  (Tg8CW_INT_Get(act->uControl)&Tg8DO_OUTPUT) &&
             !(Tg8CW_INT_Get(  a->uControl)&Tg8DO_OUTPUT)   )
         {
             /* Since there does not seem to be an operation to activate the output,
                we rewrite the whole record */
             int res = Tg8CW_INT_Get(a->uControl) | Tg8DO_OUTPUT;
             if(*mt == dm) res |= Tg8DO_INTERRUPT;
             Tg8CW_INT_Set(a->uControl, res);
             if (LoadAction(mcon,i,1,a) != OK) return(SYSERR);
         }
         /* otherwise, if this is the first device requesting an interupt then enable the busInterrupt */
         else if(*mt == dm) EnableBusInterrupt(mcon, a, i);
#else
         /* Found - set the device usage bit */
	 *mt |= dm;
	 if (st != Tg8CW_STATE_Get(a->uControl))
	   ChangeActionState(mcon,i,1,(st? Tg8DISABLED: Tg8ENABLED));
#endif
	 goto nxt;  /* Take the next user action */
       };
     };

     /* That is the new action. Search the place for it. */

     if (hole<0) {
       if (tab->Size == Tg8ACTIONS) { /* No more free items */
	 PrError(mcon,Tg8ERR_TABLE_FULL);
	 pseterr(ENOSPC);    /* No space */
	 return(SYSERR);
       };
       hole = tab->Size;
     };

     /* Load action into the firmware */
     if (LoadAction(mcon,hole,1,act) != OK) return(SYSERR);

     tab->Table[hole] = *act;
     tab->DevMask[hole] = dm;
     tab->Id[hole] = 0; /* Object's id -- NO OBJECT */
     if (hole == tab->Size) tab->Size++;

     /* Do with the next user's action */
nxt: ;
   };

   return (rcnt);

} /* <<Tg8DrvrWrite>> */

/*eof rdwr.c */
