/**************************************************************************/
/* Tg8 Interrupt handler                                                  */
/* Vladimir Kovaltsov for SL Version, February, 1997			  */
/**************************************************************************/

/* Put the event on the head of the queue and advance */
/* the head pointer with wrap around. */

#define INS_INTO_QUEUE() \
       e = &q->Queue [q->Head];\
       if (++(q->Head) >= Tg8DrvrQUEUE_SIZE) q->Head = 0;\
       dmm = mt[u.iAct-1]; /* Mask of all connected devices */ \
       e->Alarms = dpm->At.aAlarms | module->Alarms;\
       e->FwStat = dpm->At.aFwStat;\
       e->DevMask= dmm;\
       e->Event.Long = at[u.iAct-1].uEvent.Long;\
       e->Id = oi[u.iAct-1];\
       STRUCPY(&e->Inter,ip);\
       for (dev=module->Device,dm=1; dmm; dev++,dm<<=1) {\
	 if (dmm & dm) { /* Wake up the process OR send signal to it */ \
	   dmm -= dm; \
	   ssignal(&dev->AppSemaphore);\
           if (dev->Signal) \
	     if (dev->Pid>0) _kill(dev->Pid,dev->Signal);  /*Signal a process*/ \
             else            _killpg(dev->Pid,dev->Signal);/*Signal a group*/ \
	 };\
       };

void IntrHandler(module)
Tg8DrvrModuleContext *module; {

Tg8DrvrWorkingArea   *w;   /* Driver's working area */
Tg8Dpm               *dpm; /* DPRAM address */
Tg8DrvrDeviceContext *dev; /* Device context */
Tg8DrvrQueue         *q;   /* Incoming events queue address */
Tg8DrvrEvent         *e;   /* Its item */
Tg8Interrupt         *ip;  /* Interrupt table address */
Tg8IntAction          u;   /* Extra info about of action */
Tg8User              *at;  /* Array of defined actions */
int                  *mt;  /* Array of device masks */
int                  *oi;  /* Array of object identifiers */
int     dm,dmm; /* Devices mask */
short   n,msk;  /* variables */
short   cnts;   /* Which counters have fired */
Word    status; /* Interrupt sources */
int ps;

   /* Get and check the VME address of the DPRAM */

   dpm = module->ModuleAddress.VMEAddress;
   at  = module->UserTable.Table;
   mt  = module->UserTable.DevMask;
   oi  = module->UserTable.Id;
   w   = module->WorkingArea;
   q   = &module->Queue;

   for (;;) {

     /* Get and clear interrupt sources */

     cnts = ReadStatusMPC(dpm) & 0x1FF;
     if (!cnts) break;       /* No more interrupt sources, exit */

     if (cnts & Tg8HS_DPRAM_INTERRUPT) {
       ReleaseVME(dpm);	         /* Clear the DPRAM interrupt flip-flop */
       status = dpm->At.aIntSrc; /* Get the interrupt sources */
       dpm->At.aIntSrc = 0;      /* Enable dpram interrupts */
     } else
       status = 0;

     if (cnts &= 0xFF)
       ReleaseCntInt(dpm,cnts);  /* Clear all counter interrupts */

     if (status) {
       if (status & Tg8ISM_MAILBOX) {
	 /* Mailbox -- Signal the appropriate semaphore */
	 ssignal(&module->BusySemaphore);
       };

       if (status & Tg8ISM_IMM) {

	 /************************************/
	 /* Immediate interrupts are present */
	 /************************************/
	 
	 for (n=0,ip=dpm->It.ImmInter; n<Tg8IMM_INTS; n++,ip++) {
	   LW(u,ip->iExt);         /* Read the extra fields */
	   if (!u.iSem) break;     /* No more imm. interrupts */
	   INS_INTO_QUEUE();
	   u.iSem = 0;
	   ((short*)&ip->iExt)[1] = ((short*)&u)[1]; /* Clear the semaphore iSem as well */
	 };
       };

#if 0
       if (status & Tg8ISM_ERR) {
	 /* Alarms are produced -- do nothing */
       };
#endif
     };

     /********************************************************/
     /* Process all the counters which are interrupt sources.*/
     /* Look through the counters interrupt table.           */
     /********************************************************/

     for (msk=1,ip=dpm->It.CntInter; cnts; msk<<=1,ip++) {

       if (!(cnts & msk)) continue; /* No interrupt received from that channel */
       cnts -= msk;

       LW(u,ip->iExt); /* Read the extra field */
       if (!u.iSem) {
	 /* Error - No data was prepared by the firmware program */
	 module->Alarms |= Tg8ALARM_UNCOM;
	 module->Status |= Tg8DrvrDRIVER_ALARMS;
	 continue;
       };
       INS_INTO_QUEUE();
       u.iSem = 0;
       ((short*)&ip->iExt)[1] = ((short*)&u)[1]; /* Clear the semaphore iSem as well */
     };

   }; /* for(;;)*/

#if 0  /*debugg*/
   EnableMtgInt(dpm);
#endif
}

/* eof isr.c */



