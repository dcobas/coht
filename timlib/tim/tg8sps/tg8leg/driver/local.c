/***************************************************************************/
/* Local procedures and variables used by the Tg8 driver program.	   */
/* November 1993. Julian Lewis.						   */
/* Vladimir Kovaltsov for the SL Version, February, 1997		   */
/***************************************************************************/

#include <tg8drvrP.h>

/* References */

extern char *sysbrk();		/* Driver's memory allocate function */
extern void sysfree();		/* memory free function */
extern int timeout();
extern void cprintf();
void memcpy16();
void ReadRawStatus();
void DisableModule();
void EnableModule();
void ResetModule();
void ReadInterruptVector();
void IntrHandler();

/*********************************/
/* Set the driver's version date */
/*********************************/

char version[] = __DATE__;

/***************************************************************************/
/* Drivers working area is private. But a pointer to it is passed back     */
/* in the module context to the ISR.                                       */
/***************************************************************************/

Tg8DrvrWorkingArea *wa = 0;

/***************************************************************************/
/* Release all memory used in a working area.                              */
/***************************************************************************/

static void FreeWorkingArea()
{

   Tg8DrvrModuleContext *mcon;	/* Module CONtext */
   int i;

   if (wa) {

      /* Free each module contexts used. */

      for (i = 0; i < Tg8DrvrMODULES; i++) {
	 mcon = wa->ModuleContexts[i];
	 if (mcon)
	    sysfree((char *) mcon, sizeof(Tg8DrvrModuleContext));
      };

      /* Release the drivers working area memory. */

      sysfree((char *) wa, sizeof(Tg8DrvrWorkingArea));
      wa = NULL;
   };
}

/***************************************************************************/
/* Print the meaning of low level Tg8 error code.			   */
/***************************************************************************/

static int PrError(mcon, n)
Tg8DrvrModuleContext *mcon;
int n;
{
   char *cp;

   switch (n) {
   case Tg8ERR_ILLEGAL_OPERATION:
      cp = "BAD MAILBOX OP-CODE";
      break;
   case Tg8ERR_ILLEGAL_ARG:
      cp = "Illegal argument supplied";
      break;
   case Tg8ERR_TIMEOUT:
      cp = "MAILBOX HAS TIMED OUT";
      break;
   case Tg8ERR_WRONG_DIM:
      cp = "BAD ROW COUNT";
      break;
   case Tg8ERR_WRONG_DELAY:
      cp = "BAD DELAY VALUE";
      break;
   case Tg8ERR_WRONG_CLOCK:
      cp = "BAD CLOCK TYPE";
      break;
   case Tg8ERR_WRONG_MODE:
      cp = "BAD COUNTER MODE";
      break;
   case Tg8ERR_REJECTED:
      cp = "Operation was rejected by the driver";
      break;
   case Tg8ERR_BAD_OBJECT:
      cp = "Bad firmware object format";
      break;
   case Tg8ERR_NO_ACK:
      cp = "There is no acknowledge from the boot program";
      break;
   case Tg8ERR_NOT_RUN:
      cp = "The firmware is not running";
      break;
   case Tg8ERR_NO_FILE:
      cp = "No such the firmware executable file";
      break;
   case Tg8ERR_PENDING:
      cp = "The previous request was not served";
      break;
   case Tg8ERR_BUS_ERR:
      cp = "The VME BUS Error detected";
      break;
   case Tg8ERR_FIRMWARE:
      cp = "The Tg8 Firmware Exception detected";
      break;
   case Tg8ERR_BAD_VECT:
      cp = "Bad Interrupt vector position";
      break;
   case Tg8ERR_BAD_SWITCH:
      cp = "Bad Switch position";
      break;
   case Tg8ERR_BAD_TRIGGER:
      cp = "Bad Trigger code (its header is 0xFF)";
      break;
   default:
      cp = "??? UNKNOWN ERROR!";
   };
   strcpy(mcon->ErrMsg, cp);
   mcon->ErrCode = n;
   mcon->ErrCnt++;
   if (n == Tg8ERR_BUS_ERR)
      cprintf("MODULE:%d  VME Addr=%X Vect=%X Err:%d %s\n", mcon->ModuleIndex + 1,
	      mcon->ModuleAddress.VMEAddress, mcon->ModuleAddress.InterruptVector, n, mcon->ErrMsg);
   return n;
}

/***************************************************************************/
/* Convert a BinaryCodedDecimal byte into an int                           */
/***************************************************************************/

static int BcdToDec(bcd)
int bcd;
{

   return ((int) (((bcd >> 4) * 10) + (bcd & 0xF)));
}

/***************************************************************/
/* Copy structures as the 16 bits arrays (important for DPRAM) */
/***************************************************************/

void memcpy16(d, s, bc)
short *d, *s;
int bc;
{
   bc >>= 1;
   while (bc-- > 0)
      *d++ = *s++;
}

/*========================================================================*/
/* Cancel a timeout in a safe way                                         */
/*========================================================================*/

static CancelTimeout(t)
int *t;
{
   int ps, v;
   disable(ps);
   if (v = *t) {
      *t = 0;
      cancel_timeout(v);
   };
   restore(ps);
}

/***************************************************************************/
/* Handle timeouts for read/ioctl.                                         */
/* The only way we can get a timeout is if the queue is empty.             */
/***************************************************************************/

static void HandleTimeout(dcon)
Tg8DrvrDeviceContext *dcon;
{

   sreset(&dcon->AppSemaphore);	/* Wake up clients */
   dcon->Timer = 0;		/* Cancel timer */
}

/********************************************************/
/* Compare two events. The events can have wildcards    */
/********************************************************/

int Match(tem, ev)
int tem, ev;
{
   Byte *e, *t;
   if (tem == ev)
      return 1;			/* ok, the same */
   e = (Byte *) & ev;
   t = (Byte *) & tem;
   if (t[0] != 0xFF && /*e[0]!=0xFF && */ t[0] != e[0])
      return 0;			/* Events are different */
   if (t[1] != 0xFF && /*e[1]!=0xFF && */ t[1] != e[1])
      return 0;
   if (t[2] != 0xFF && /*e[2]!=0xFF && */ t[2] != e[2])
      return 0;
   if (t[3] != 0xFF && /*e[3]!=0xFF && */ t[3] != e[3])
      return 0;
   return 1;
}

/*******************************************/
/* Look for object by its ID.              */
/*******************************************/

int ObjById(mcon, id)
Tg8DrvrModuleContext *mcon;
int id;
{
   int i, len = mcon->UserTable.Size, *oi = mcon->UserTable.Id;
   for (i = 0; i < len; i++, oi++)
      if (id == *oi)
	 return i;		/* Found */
   return -1;			/* Not found */
}

/*******************************************/
/* Enable the BUS interrupt request.       */
/*******************************************/

int EnableBusInterrupt(mcon, acon, an)
Tg8DrvrModuleContext *mcon;
Tg8User *acon;
int an;
{
   int cw, ot;			/* Control word, output type */

   cw = acon->uControl;
   ot = Tg8CW_INT_Get(cw);	/* Output mode */
   if (!(ot & Tg8DO_INTERRUPT)) {
      Tg8CW_INT_Set(acon->uControl, Tg8DO_INTERRUPT);
      return SetActionState(mcon, an, 1, Tg8INTERRUPT);
   };
   if (ot == Tg8DO_INTERRUPT && Tg8CW_STATE_Get(cw) /*disabled */ ) {
      /* Enable an action */
      Tg8CW_STATE_Clr(acon->uControl);
      return SetActionState(mcon, an, 1, Tg8ENABLED);
   };
   return Tg8ERR_OK;
}

/*******************************************/
/* Clear the BUS interrupt request.        */
/*******************************************/

int ClearBusInterrupt(mcon, acon, an)
Tg8DrvrModuleContext *mcon;
Tg8User *acon;
int an;
{
   int cw, ot;			/* Control word, output type */

   cw = acon->uControl;
   ot = Tg8CW_INT_Get(cw);	/* Output mode */

   if (!(ot & Tg8DO_INTERRUPT))
      return Tg8ERR_OK;

   if (ot & Tg8DO_OUTPUT) {
      /* Keep an action that produces outputs */
      Tg8CW_INT_Clr(cw);
      Tg8CW_INT_Set(cw, Tg8DO_OUTPUT);
      acon->uControl = cw;
      return SetActionState(mcon, an, 1, Tg8NO_INTERRUPT);
   };
   /* Any other action should be suppressed */
   Tg8CW_STATE_Set(acon->uControl, 1);
   return SetActionState(mcon, an, 1, Tg8DISABLED);
}

/*******************************************/
/* Load the set of actions.                */
/*******************************************/

int LoadAction(mcon, an, cnt, adesc)
Tg8DrvrModuleContext *mcon;
int an, cnt;
Tg8User *adesc;
{
   int coco;
   if (cnt)
      if ((coco = SetAction(mcon, an, cnt, adesc)) != Tg8ERR_OK) {
	 pseterr(EIO);		/* IO error of some description */
	 return (SYSERR);
      };
   return (OK);
}

/*******************************************/
/* Load gates for the set of actions.      */
/*******************************************/

int LoadGate(mcon, an, cnt, gdesc)
Tg8DrvrModuleContext *mcon;
int an, cnt;
Tg8Gate *gdesc;
{
   int coco;
   if (cnt)
      if ((coco = SetGate(mcon, an, cnt, gdesc)) != Tg8ERR_OK) {
	 pseterr(EIO);		/* IO error of some description */
	 return (SYSERR);
      };
   return (OK);
}

/*******************************************/
/* Load dimension for the set of actions.  */
/*******************************************/

int LoadDim(mcon, an, cnt, desc)
Tg8DrvrModuleContext *mcon;
int an, cnt;
Byte *desc;
{
   int coco;
   if (cnt)
      if ((coco = SetDim(mcon, an, cnt, desc)) != Tg8ERR_OK) {
	 pseterr(EIO);		/* IO error of some description */
	 return (SYSERR);
      };
   return (OK);
}

/*========================================================================*/
/* Change the state (enabled/disabled) for the subset of actions	  */
/*========================================================================*/

int ChangeActionState(mcon, row, rows, state)
Tg8DrvrModuleContext *mcon;
int row, rows, state;
{
   Tg8User *a;
   int i;

   for (i = 0, a = &mcon->UserTable.Table[row]; i < rows; i++, a++) {
      Tg8CW_STATE_Clr(a->uControl);
      if (state == Tg8DISABLED || state == Tg8PPMT_DISABLED)
	 Tg8CW_STATE_Set(a->uControl, 1);
   };
   if ((state = SetActionState(mcon, row, rows, state)) != Tg8ERR_OK) {
      pseterr(EIO);		/* IO error of some description */
      return (SYSERR);
   };
   return (OK);
}

/***************************************************************/
/* Clear the part of the user table.                           */
/***************************************************************/

int ClearUserTable(mcon, row, rows)
Tg8DrvrModuleContext *mcon;
int row, rows;
{

   if (ClearAction(mcon, row, rows) != Tg8ERR_OK) {
      pseterr(EIO);		/* IO error of some description */
      return (SYSERR);
   };
   bzero((void *) &mcon->UserTable.Table[row], rows * sizeof(Tg8User));
   bzero((void *) &mcon->UserTable.Gate[row], rows * sizeof(Tg8Gate));
   bzero((void *) &mcon->UserTable.Id[row], rows * sizeof(int));
   bzero((void *) &mcon->UserTable.DevMask[row], rows * sizeof(int));
   bzero((void *) &mcon->UserTable.Dim[row], rows * sizeof(Byte));
   if (row + rows >= mcon->UserTable.Size)
      mcon->UserTable.Size = row;
   return (OK);
}

/***************************************************************/
/* Reload the whole user table.                                */
/***************************************************************/

int ReloadUserTable(mcon)
Tg8DrvrModuleContext *mcon;
{

   DisableModule(mcon);

   /* Clear the whole action table for the module */

   if (ClearAction(mcon, 0, Tg8ACTIONS) < 0) {
      pseterr(EIO);		/* IO error of some description */
      return (SYSERR);
   };

   /* Load the module's action table */

   if (LoadAction(mcon, 0, mcon->UserTable.Size, mcon->UserTable.Table) == SYSERR)
      return (SYSERR);
   if (LoadGate(mcon, 0, mcon->UserTable.Size, mcon->UserTable.Gate) == SYSERR)
      return (SYSERR);
   if (LoadDim(mcon, 0, mcon->UserTable.Size, mcon->UserTable.Dim) == SYSERR)
      return (SYSERR);
   if (mcon->Status & Tg8DrvrMODULE_ENABLED)
      EnableModule(mcon);
   return (OK);
}

/*************************************/
/* Check the current driver's status */
/*************************************/

int CheckDriverStatus(mcon)
Tg8DrvrModuleContext *mcon;
{

   Tg8StatusBlock f_hwst;	/* Firmware status */
   int mnum, i;

   mnum = mcon->ModuleIndex;
   if (wa->DebugOn)
      mcon->Status |= Tg8DrvrDRIVER_DEBUG_ON;
   else
      mcon->Status &= ~Tg8DrvrDRIVER_DEBUG_ON;

   /* Read the hardware status to see if the Xilinx PALs are loaded */
   /* and the cable clock setting. */

   if (recoset()) {
      noreco();
      PrError(mcon, Tg8ERR_BUS_ERR);
      mcon->Status = 0;
      pseterr(EINVAL);
      return (SYSERR);
   };

   ReadRawStatus(mcon, &f_hwst);
   noreco();

   if (f_hwst.Hw & Tg8HS_EXTERNAL_CLOCK_JUMPER)
      mcon->Status |= Tg8DrvrMODULE_SWITCH_SET;
   else
      mcon->Status &= ~Tg8DrvrMODULE_SWITCH_SET;

   if (f_hwst.Hw & Tg8HS_RECEIVER_ENABLED)
      mcon->Status |= Tg8DrvrMODULE_ENABLED;
   else
      mcon->Status &= ~Tg8DrvrMODULE_ENABLED;

   if ((f_hwst.Hw & Tg8HW_OK_STATUS) != Tg8HW_OK_STATUS) {
      mcon->Status |= Tg8DrvrMODULE_HARDWARE_ERROR;
      mcon->Status &= ~Tg8DrvrMODULE_RUNNING;
   } else
      mcon->Status &= ~Tg8DrvrMODULE_HARDWARE_ERROR;

   if (f_hwst.Fw & Tg8FS_RUNNING)
      mcon->Status |= Tg8DrvrMODULE_RUNNING;
   else
      mcon->Status &= ~Tg8DrvrMODULE_RUNNING;

   i = f_hwst.Evo;
   if (i != 0) {
      PrError(mcon, Tg8ERR_FIRMWARE);
      cprintf("Tg8Drvr: CheckStatus: FIRMWARE ERROR: MOD: %d VEC: 0x%x PC :0x%x\n", mnum + 1, i, f_hwst.Epc);
      if (i == 0x008)
	 cprintf("Tg8Drvr: BUS-ERROR\n");
      else if (i == 0x00C)
	 cprintf("Tg8Drvr: ADDRESS-ERROR\n");
      else if (i == 0x010)
	 cprintf("Tg8Drvr: ILLEGAL-INSTRUCTION\n");
      else if (i == 0x014)
	 cprintf("Tg8Drvr: DIVISION-BY-ZERO\n");
      else if (i == 0x020)
	 cprintf("Tg8Drvr: PRIVILEGE-VIOLATION\n");
      else if (i == 0x060)
	 cprintf("Tg8Drvr: SPURIOUS-INTERRUPT\n");
      mcon->Status |= Tg8DrvrMODULE_FIRMWARE_ERROR;
   } else
      mcon->Status &= ~Tg8DrvrMODULE_FIRMWARE_ERROR;

   if (mcon->Status & Tg8DrvrMODULE_RUNNING) {

      /* Ping the module to see if it is realy alive, */
      /* or stuck in a loop, or in a hardware block.  */

      if (PingModule(mcon) != Tg8ERR_OK)
	 mcon->Status |= Tg8DrvrMODULE_TIMED_OUT;
      else
	 mcon->Status &= ~Tg8DrvrMODULE_TIMED_OUT;
   };

#if Tg8RESIDENT_V		/* Resident firmware version */
   /* Read the selftest result */
   ReadSelfTestResult(mcon);
#endif
   return (OK);
}

/*************************************/
/* Configurate/Check the Tg8 module  */
/*************************************/

int AddModule(indx, moadr, insfl)
int indx;                       /* Module index (0-based) */
Tg8DrvrModuleAddress *moadr;	/* Module address, vector, level, ssch */
int insfl; /* 1=do the installation; 0=check the installation */
{

Tg8DrvrModuleContext *mcon, *mc;     /* Module context */
Tg8DrvrModuleAddress *moad;  /* Module addresses */
Tg8StatusBlock f_hwst;       /* Hardware status */
int f_vect, swset;           /* Tg8 interrupt vector, switch setting */
int i, coco, goodv = 0;

   mcon = wa->ModuleContexts[indx];

   if (!mcon) {
      insfl = 1;
      mcon = (Tg8DrvrModuleContext *) sysbrk(sizeof(Tg8DrvrModuleContext));
      if (!mcon) {
	 cprintf("Tg8DrvrInstall: NOT ENOUGH MEMORY(ModuleContext)\n");
	 pseterr(ENOMEM);	/* Not enough core */
	 return (SYSERR);
      };
      wa->ModuleContexts[indx] = mcon;
      bzero((void *) mcon, sizeof(Tg8DrvrModuleContext));
      mcon->ModuleIndex = indx;
      mcon->WorkingArea = wa;
      mcon->TransferSemaphore = (-1);	/* Block access until module's reset */
      mcon->TimeOut = 1500;	/* 15 sec. */
   }

   /**************************************************************/
   /* Check configuration INFO and Install the interrupt vector. */
   /**************************************************************/

   mcon->ModuleAddress = *moadr;	/* Keep a module address */
   moad = &mcon->ModuleAddress;

   /* Try to reset the module killing interrupts. If we get a bus */
   /* error then "recoset" traps it via a set jump. */

   if (recoset()) {		/* Catch bus errors */
      noreco();			/* Remove local bus trap */
      if (wa->DebugOn && goodv) {
	 PrError(mcon, Tg8ERR_BUS_ERR);
	 cprintf("Tg8Drvr: Module:%d. VME Addr:%x Vect:%x/%X Level:%x Switch:%d/%d SSC:%X\n",
		 indx + 1,
		 moad->VMEAddress,
		 moad->InterruptVector, f_vect, moad->InterruptLevel, moad->SwitchSettings, swset, moad->SscHeader);
      }
      pseterr(ENXIO);		/* No such device or address */
      sysfree((char *) mcon, sizeof(Tg8DrvrModuleContext));
      wa->ModuleContexts[indx] = NULL;
      return (SYSERR);
   }

   ReleaseVME(moad->VMEAddress);
   asm("sync");
   ResetMPC(moad->VMEAddress);
   asm("sync");

   ReadInterruptVector(mcon, &f_vect);
   asm("sync");

   noreco();			/* Remove local bus trap */

   ReadRawStatus(mcon, &f_hwst);
   swset = (f_hwst.Hw & Tg8HS_EXTERNAL_CLOCK_JUMPER) ? 1 : 0;

   if (f_vect < wa->InfoTable.Vector || f_vect >= wa->InfoTable.Vector + Tg8DrvrCARDS)
      goto badv;
   goodv = 1;

   /* Compare interrupt vectors against the all known cards */

   for (i = 0; i < Tg8DrvrMODULES; i++) {
      if (mc = wa->ModuleContexts[i]) {
	 if (mc != mcon) {
	    if (mc->ModuleAddress.VMEAddress == moad->VMEAddress) {
	       cprintf("Tg8Drvr: MODULES %d and %d USE THE SAME VME BASE ADDRESS: %X\n", indx + 1, i + 1, moad->VMEAddress);
	       goto badvec;
	    };
	    if (mc->ModuleAddress.InterruptVector == f_vect) {
	       cprintf("Tg8Drvr: MODULES %d and %d USE THE SAME INTERRUPT VECTOR: %X\n", indx + 1, i + 1, f_vect);

badvec:        if (i > indx) {
		  /* Clear interrupt handler */
		  iointclr((int) ((mc->ModuleAddress.InterruptVector) << 2));
		  sysfree((char *) mc, sizeof(Tg8DrvrModuleContext));
		  wa->ModuleContexts[i] = NULL;
		  insfl = 1;
	       };
	    };
	 };
      }
   }

   if (!moad->InterruptVector)
      moad->InterruptVector = f_vect;	/* Autodetect mode */
   else if (moad->InterruptVector != f_vect) {
badv:    PrError(mcon, Tg8ERR_BAD_VECT);
      if (wa->DebugOn) {
	 cprintf("Tg8Drvr: BAD INTERRUPT VECTOR ON MODULE %2d (%x)\n", indx + 1, f_vect);
	 if (moad->InterruptVector)
	    cprintf("Tg8Drvr: SHOULD BE %x\n", moad->InterruptVector);
      };
      mcon->Alarms |= Tg8ALARM_BAD_SWITCH;
      /* Uninstall the module */
      insfl = 1;
      pseterr(EFAULT);		/* Bad address */
      return SYSERR;
   };

   /* Check switch settings correspond to expected value. */

   if (moad->SwitchSettings == -1)
      moad->SwitchSettings = swset;	/* Autodetect mode */
   else if (moad->SwitchSettings != swset) {
      PrError(mcon, Tg8ERR_BAD_SWITCH);
      if (wa->DebugOn)
	 cprintf("Tg8Drvr: BAD SWITCH SETTING ON MODULE %2d\n", indx + 1);
      mcon->Alarms |= Tg8ALARM_BAD_SWITCH;
      moad->SwitchSettings = swset;
   };

   /* Display most important info parameters */

   if (wa->DebugOn) {
      cprintf("Tg8Drvr: Module %d. VME Addr:%x Vect:%x Level:%x Switch:%x SSC:%X. *OK*\n", indx + 1,
	      moad->VMEAddress, moad->InterruptVector, moad->InterruptLevel, moad->SwitchSettings, moad->SscHeader);
   };

   if (insfl) {

      switch (moad->InterruptLevel) {
      case 1:
	 iointunmask(VECT_VMEIRQ1);
	 break;
      case 2:
	 iointunmask(VECT_VMEIRQ2);
	 break;
      case 3:
	 iointunmask(VECT_VMEIRQ3);
	 break;
      case 4:
	 iointunmask(VECT_VMEIRQ4);
	 break;
      case 5:
	 iointunmask(VECT_VMEIRQ5);
	 break;
      case 6:
	 iointunmask(VECT_VMEIRQ6);
	 break;
      case 7:
	 iointunmask(VECT_VMEIRQ7);
	 break;
      };

      /* Install the interrupt vector */

      coco = vme_intset((char *) (moad->InterruptVector), IntrHandler,	/* ISR address */
			mcon, 0);	/* ISR gets module context as a parameter */
      if (coco < 0) {
	 cprintf("Tg8Drvr: ERROR %1d ON vme_intset, MODULE %2d\n", coco, indx + 1);
	 pseterr(EFAULT);	/* Bad address */
	 return SYSERR;
      }
   };				/*insfl */
   return (OK);
}

/*************************************/
/* Check the driver's configuration  */
/*************************************/

int CheckConfiguration()
{
   int i, j;
   Tg8DrvrModuleAddress moad;
   Tg8DrvrInfoTable *info;
   info = &wa->InfoTable;
   for (i = j = 0; i < 16 && j < Tg8DrvrMODULES; i++) {
      moad.VMEAddress = (Tg8Dpm *) (info->Address + i * info->Increment);	/* Base address */
      moad.InterruptVector = 0;	/* Interrupt vector number */
      moad.InterruptLevel = info->Level;	/* Interrupt level (2 usually) */
      moad.SwitchSettings = -1;	/* Switch settings */
      moad.SscHeader = info->SscHeader;	/* SSC Header code */
      if (AddModule(j, &moad, 0) == OK)
	 j++;
   };
   return (OK);
}

/* eof */
