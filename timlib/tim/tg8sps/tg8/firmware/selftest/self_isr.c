/**********************************************************************/
/* Macro to handle a Tpu interrupt.                                   */
/* It simply increments the interrupts number                         */
/**********************************************************************/

#define TpuIsr(c) asm("
       .text
       .globl _Tpu"#c"Isr
_Tpu"#c"Isr:
      addil  #1,_tpu_int+(4*"#c")
      andw   #ClrINT_CHANNEL_"#c",TpuIs
      rte
")

/********************************************************************/
/* Macro to provide a general purpose interrupt handler.            */
/* Doing nothing.                                                   */
/********************************************************************/

#define GeneralIsr(c) asm("
	.text
	.globl  _"#c"Isr
_"#c"Isr:
      rte
")

/********************************************************************/
/* Default Interrupt service routine                                */
/********************************************************************/

asm("
 .text
 .globl _default_isr
 _default_isr:
  movew #Tg8DISABLE_INTERRUPTS,SR
  movew sp@(6),Tg8DPM_BASE+Head+VectorOffset
  movel sp@(2),Tg8DPM_BASE+Head+ExceptionPC
  stop  #Tg8DISABLE_INTERRUPTS
  rte" );

/**********************************************************************/
/* Declare TPU Interrupt Service routines                             */
/**********************************************************************/

void dummyTpuIsr() {
   TpuIsr(0);      /* Milli Second clock interrupt */
   TpuIsr(1);      /* Counter 1 output */
   TpuIsr(2);
   TpuIsr(3);
   TpuIsr(4);
   TpuIsr(5);
   TpuIsr(6);
   TpuIsr(7);
   TpuIsr(8);      /* Counter 8 output */
}

/********************************************************************/
/* Handle bus/address/PrivViolation errors                          */
/********************************************************************/

void dummyBusError() {

   asm(" .text
	 .globl _BusErrorIsr
   _BusErrorIsr:
	 .globl _AddressErrorIsr
   _AddressErrorIsr:
	 .globl _PrivViolationIsr
   _PrivViolationIsr:
         addil #1,_bus_int
	 rte ");
}

/********************************************************************/
/* Handle spurious interrupts                                       */
/********************************************************************/

void dummySpurious() {

   asm volatile("
	 .text
	 .globl _SpuriousIsr
   _SpuriousIsr:
	 movel d0,sp@-
	 movel Tg8DSC_INTERRUPT,d0   /* Clear the DSC Interrupt */
	 movel sp@+,d0
	 rte
   ");
};

/********************************************************************/
/* Handle the Abort button interrupt: start the 332 BUG             */
/********************************************************************/

void dummyAbort() {

   asm volatile("
	 .text
	 .globl _AbortIsr
   _AbortIsr:
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
 };

/**********************************************************************/
/* Declare DSC and XR Interrupt Service routines                      */
/**********************************************************************/

void dummyGeneralISR() {

   GeneralIsr(Dsc); /* Do nothing here */

   /* A special ISR for incomming frames */

   asm(" .text
	 .globl  _XrIsr
_XrIsr:  movew   #Tg8DISABLE_INTERRUPTS,SR
	 moveml  d0-d7/a0-a6,_xl_context
	 movel  (Tg8XLX_BASE + XWsscRframe1),d0         /* Read frame */
	 rorw   #8,d0                                   /* Swap bytes */
	 swap   d0
	 rorw   #8,d0
	 swap   d0
	 movel  d0,_event_frame
	 movew  #0,(Tg8XLX_BASE + WClrRerr)  /* Clear an error */
      ");
#if 0
   frame_int++;

   /* Make a special processing of the timing frame */

   switch(event_frame.Any.Header) {

   case Tg8MILLISECOND_HEADER:
      if (++(dpm->Info.Date.Ms) >= 1000) dpm->Info.Date.Ms = 0;
      if (event_frame.Long == 0x010000FF) { /* Start the next CPS cycle */
	dpm->Info.CycleDur = dpm->Info.CTrain+1;
	dpm->Info.CycleNum++;
	dpm->Info.CTrain = 0;
      } else
        dpm->Info.CTrain++;
      break;

   case Tg8TIME_HEADER:
      dpm->Info.Date.Hour   = BCD(event_frame.Time.Tg8_hour);
      dpm->Info.Date.Minute = BCD(event_frame.Time.Tg8_minute);
      dpm->Info.Date.Second = BCD(event_frame.Time.Tg8_second);
      break;

   case Tg8DATE_HEADER:
      dpm->Info.Date.Year  = BCD(event_frame.Date.Tg8_year);
      dpm->Info.Date.Month = BCD(event_frame.Date.Tg8_month);
      dpm->Info.Date.Day   = BCD(event_frame.Date.Tg8_day);
      break;
   };
#endif
ret:
   /* Restore registers and return from an interrupt */
   asm volatile (" moveml  _xl_context,d0-d7/a0-a6
		   rte ");
}

/* eof */
