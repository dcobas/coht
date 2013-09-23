/**********************************************************************/
/* Macro definitions for the Tg8's firmware program.		      */
/* Vladimir Kovaltsov for ROCS Project, February, 1997		      */
/**********************************************************************/

#define STRUCPY(d,s) memcpy16((short*)(d),(short*)(s),sizeof(*(s)))
#define LW32(d,s)    memcpy16((short*)&(d),(short*)&(s),sizeof(int))
#define LW(d,s)  {((short*)&(d))[0]=((short*)&(s))[0];((short*)&(d))[1]=((short*)&(s))[1];}
#define LWi(d,v) {((short*)&(d))[0]=v>>16;((short*)&(d))[1]=v;}

/*****************/
/* Trap routines */
/*****************/

#define ATC_Trap "trap #0"
#define ATC_TRAP_VECTOR  0	  /* Finish the execution of At-process */

#define Insert_CAM_Trap "trap #1"
#define INSERT_CAM_TRAP_VECTOR  1 /* Insert an event header to CAM */

#define Clear_CAM_Trap "trap #2"
#define CLEAR_CAM_TRAP_VECTOR  2  /* Clear the CAM */

#define ISM_SET_TRAP "trap  #3"   /* Set the interrupt source mask */
#define ISM_TRAP_VECTOR  3

#define ATR_Trap "trap #4"
#define ATR_TRAP_VECTOR 4 /* Start the execution of At-process from the mbx process */

#define IMMC_Trap "trap #5"
#define IMMC_TRAP_VECTOR  5	  /* Finish the execution of IMM-process */

/**********************************************************************/
/* Macro to get link trigger                                          */
/**********************************************************************/

#define MILLSC_MASK (Tg8MILLISECOND_HEADER<<24)
#define LINK_EVENT(trig) (long)(MILLSC_MASK | (trig))

/**********************************************************************/
/* Macros  (General Purpose)                                          */
/**********************************************************************/

/* Interrupts ON or OFF */

#define RESET  asm(" reset ")
#define INTOFF asm(" movew #Tg8DISABLE_INTERRUPTS,SR ")
#define INTON  asm(" movew #Tg8ENABLE_INTERRUPTS,SR ")
#define WAIT   asm volatile (" movel d0,d0 ")

/**********************************************************************/
/* Some macros for handling the Content Addressed Memory CAM          */
/**********************************************************************/

#define CamSetCommandMode asm(" andiw #CamCOMMAND_MODE,SimDataF1 ")
#define CamSetDataMode    asm(" oriw  #CamDATA_MODE,SimDataF1 ")

#define CamClearSkipBits  *cam= CamCLEAR_SKIP_BITS
#define CamClear asm(" movel _cam,a0
		       movew #0,a0@")

#define CamMatch(wb) \
   CamSetDataMode; \
   *cam = wb[0];   \
   *cam = wb[1];   \
   *cam = wb[2];   \
   CamSetCommandMode;

#define CamInsertToCAM() \
   CamSetDataMode; \
   *cam = ins.Event.Short.Short_1;\
   *cam = ins.Event.Short.Short_2;\
   *cam = ins.Last_word.Short;\
   CamSetCommandMode; \
   *cam = ins.Addr | CamWRITE;

#define CamWriteArray(adr,wb) \
   CamSetDataMode; \
   *cam = wb[0];   \
   *cam = wb[1];   \
   *cam = wb[2];   \
   CamSetCommandMode; \
   *cam = adr | CamWRITE;

#define CamReadArray(adr,wb) \
   CamSetCommandMode;   \
   *cam = adr | 0xF000; \
   *cam = adr | 0x7000; \
   CamSetDataMode;      \
   wb[0] = *cam;        \
   CamSetCommandMode;   \
   *cam = adr | 0xF400; \
   *cam = adr | 0x7000; \
   CamSetDataMode;      \
   wb[1] = *cam;        \
   CamSetCommandMode;   \
   *cam = adr | 0xF800; \
   *cam = adr | 0x7000; \
   CamSetDataMode;      \
   wb[2] = *cam;

/******************************/
/* Generate VME Bus interrupt */
/******************************/

#define MakeDscInterrupt (*((short *) (Tg8VME_INTERRUPT)) = 1)

/**********************************************************************/
/* Prepare events for the host computer                               */
/**********************************************************************/

#define SignalToHost(intrN)  eprog.IntSrc |= 1<<(intrN);

/**********************************************************************/
/* Alarms generation macros                                           */
/**********************************************************************/

#define SignalAlarm(acode) { \
  eprog.Alarms |= (acode);   \
  dpm->At.aAlarms |= eprog.Alarms; \
  dpm->At.aFwStat |= Tg8FS_ALARMS; \
}

#define ShowAlarms() { \
  eprog.Alarms = 0;    \
  /*debug SignalToHost(Tg8IS_ERR);*/\
}

#define ResetAlarms() {\
  dpm->At.aAlarms = 0; \
  dpm->At.aFwStat &= ~Tg8FS_ALARMS;\
}

/**********************************************************************/
/* Macro to check simple events					      */
/**********************************************************************/

#define ServeEventTel(m) { \
     if (timing_frame.Simple.Event_code == Tg8READY_TELEGRAM) { \
	       asm(" moveal #_tel+next"#m",a0
		     moveal #At+aTeleg"#m",a1
		     movel  #Tg8GROUPS/2-1,d0 /* telegram size in longwords */
		  0: movel  a0@+,a1@+
		     dbf    d0,0b"); }; \
   }

#define ServeSimpleEventTel(m) { \
     if (timing_frame.Simple.Event_code == Tg8READY_TELEGRAM) { \
	       asm(" moveal #_tel+next"#m",a0
		     moveal #At+aTeleg"#m",a1
		     movel  #Tg8GROUPS/2-1,d0 /* telegram size in longwords */
		  0: movel  a0@+,a1@+
		     dbf    d0,0b"); }; \
     if (!(in_use.Simple_event[timing_frame.Simple.Event_code] &\
        (1<<Tg8##m##))) goto ret_from_int;\
   }

#define ServeSimpleEvent(m) \
     if (!(in_use.Simple_event[timing_frame.Simple.Event_code] &\
        (1<<Tg8##m##))) goto ret_from_int;

/**********************************************************************/
/* Macro to store incomming group values                              */
/**********************************************************************/

#define ServeTelegramEvent(m) \
  if (timing_frame.Telegram.Group_number <= Tg8GROUPS)  \
     tel.next##m##[timing_frame.Telegram.Group_number-1] \
     = timing_frame.Telegram.Group_value;

/* eof macros.h */
