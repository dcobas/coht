/***************************************************************************/
/* Tg8 Device driver - open, close functions.                              */
/* November 1993. CERN/PS Version. Julian Lewis.			   */
/* Vladimir Kovaltsov for ROCS Project, February, 1997			   */
/***************************************************************************/

/***************************************************************************/
/* OPEN                                                                    */
/***************************************************************************/

int Tg8DrvrOpen(wa, dnm, flp)
Tg8DrvrWorkingArea * wa;    /* Working area */
int		     dnm;   /* Device number */
struct file	   * flp; { /* File pointer */

int dnum,mnum,dm,f_vect;        /* Device & Module number & Vector */
Tg8DrvrModuleContext * mcon;    /* Module context */
Tg8DrvrDeviceContext * dcon;    /* Device context */

   dnum = minor(flp->dev) - 1;
   mnum = dnum / Tg8DrvrDEVICES;
   if (dnum < 0 || mnum >= Tg8DrvrMODULES) {
      pseterr(EBADF);           /* Bad file number */
      return(SYSERR);
   };

   mcon = wa->ModuleContexts[mnum];
   if (!mcon) {
nodev: pseterr(ENODEV);          /* No such device */
       return(SYSERR);
   };

   /* Check the module presence. */
   /* Read the module's vector number ignoring bus interrupts */
   if (recoset()) {
     noreco();
     goto nodev; /* No such device */
   };
   ReadInterruptVector(mcon, &f_vect);
   noreco(); /* Remove bus interrupt trap */

   /* If already open by someone else, give a permission denied error */

   dnum %= Tg8DrvrDEVICES;
   dm = 1<<dnum;
   if (mcon->DevMask & dm) {
      pseterr(EBUSY); /* Device busy */
      return(SYSERR);
   };

   dcon = &mcon->Device[dnum];
   bzero((void *) dcon,sizeof(Tg8DrvrDeviceContext));
   dcon->ModuleIndex = mnum;
   dcon->DeviceIndex = dnum;
   dcon->Mode = flp->access_mode;
   mcon->DevMask |= dm;

   /* Purge the queue */
   dcon->Tail = mcon->Queue.Head; /* End of the queue */
   sreset(&dcon->AppSemaphore);

   return(OK);

} /* <<Tg8DrvrOpen>> */

/***************************************************************************/
/* CLOSE                                                                   */
/***************************************************************************/

int Tg8DrvrClose(wa, flp)
Tg8DrvrWorkingArea * wa;           /* Working area */
struct file * flp; {               /* File pointer */

int dnum,mnum,dm;                  /* Device & Module number */
int an,cw,ot,*mt;       	   /* Action number, control word, output type */

Tg8DrvrModuleContext * mcon;   /* Module context */
Tg8DrvrDeviceContext * dcon;   /* Device context */
Tg8User              * acon;   /* Action description */
Tg8DrvrUserTable     * tab;    /* User table */

   dnum = minor(flp->dev) -1;
   mnum = dnum / Tg8DrvrDEVICES;
   if (dnum < 0 || mnum >= Tg8DrvrMODULES) {
      pseterr(EBADF);           /* Bad file number */
      return(SYSERR);
   };

   /* We cant close a file that is not open. */

   mcon = wa->ModuleContexts[mnum];
   if (!mcon) {
      pseterr(ENODEV);          /* No such device */
      return(SYSERR);
   };

   dnum %= Tg8DrvrDEVICES;
   dm = 1<<dnum;
   dcon = &mcon->Device[dnum];
   if (!(mcon->DevMask & dm)) {
     pseterr(EBADF);           /* Bad file number */
     return(SYSERR);
   };
   mcon->DevMask &= ~dm;

   /*******************************/
   /* Now really close the device */
   /* Cancel any pending timeouts */
   /*******************************/

   CancelTimeout(&dcon->Timer);
   tab = &mcon->UserTable; /* User table address */

   switch (dcon->CloseMode.Mode) {
     case Tg8ONCL_LEAVE:    /* Leave the card in working state */
       break;
     case Tg8ONCL_DISABLE:      /* Disable the card asynchronously */
       DisableModule(mcon);
simu:  if (an = dcon->CloseMode.SimPulseMask) {
	 /* Simulate the card's ouput pulses */
	 if (SimulatePulse(mcon,an) != Tg8ERR_OK) {
	   pseterr(EIO);       /* IO error of some description */
	   return(SYSERR);
	 };
       };
       break;
     case Tg8ONCL_CLEAR:        /* Disable the card asynchronously AND clear the user table */
       DisableModule(mcon);
       ClearUserTable(mcon,0,tab->Size);
       goto simu;
     case Tg8ONCL_DISABLE_SYNC: /* Disable the card synchronously */
       DisableSync(mcon);
       break;
   };

   /***********************************/
   /* Release the user table          */
   /***********************************/

   for (an=0,acon=tab->Table,mt=tab->DevMask;
	an<Tg8ACTIONS; an++,acon++,mt++) {

     if (!acon->uEvent.Long) continue; /* Free slot */
     if (!(*mt & dm)) continue;        /* Not used by the device */

     *mt &= ~dm; /* Clean the device usage bit for given action */
     if (*mt) continue; /* There are other clients */

     /* Clear the bus interrupt. */
     /* Suppress the action altogether if it does not make outputs. */
     if (ClearBusInterrupt(mcon,acon,an) != Tg8ERR_OK) break;
   };

   bzero((void *) dcon,sizeof(Tg8DrvrDeviceContext));
   return(OK);

} /* <<Tg8DrvrClose>> */

/*eof opencl.c */
