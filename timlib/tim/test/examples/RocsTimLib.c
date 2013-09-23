/* ========================================== */
/* Replace Tg8 functionality on ROCS by a CTR */
/* Julian Thu 21/Feb/07                       */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <errno.h>        /* Error numbers */
#include <sys/file.h>
#include <a.out.h>

#include <err/err.h>      /* Error handling */
#include <tgm/tgm.h>      /* Telegram definitions for application programs */
#include <tgv/tgv.h>
#include <TimLib.h>
#include "/dsrc/drivers/ctr/src/driver/ctrdrvr.h"

#include <errno.h>

/* ----------------------------------------------- */
/* This crap is to avoid type definition conflicts */
/* between the TG8 firmware and the TGM libraries  */

#define TG8_TYPES
typedef unsigned char  Uchar;
typedef unsigned short Ushort;
typedef unsigned int   Uint;
typedef unsigned int   DWord;
#include <tg8Pub.h>

/* ----------------------------------------------- */

#include <RocsTimLib.h>

extern int errno;
extern CtrDrvrDevice ctr_device;

/* Make TimLib allow the use of old headers */

extern int timlib_allow_old;

/* For each Tg8 action I create one of these to emulate */
/* the action as a PTIM on the CTR. */

typedef struct {
   unsigned long  Frame;    /* Raw event frame, like 0x211C0102 */
   unsigned short Control;  /* Tg8 control word */
   unsigned short Delay;    /* Counter delay */
   unsigned short Data;     /* Action data word */
   unsigned short Ctim;     /* Ctim corresponding to the Frame */
   unsigned long  Ptim;     /* The Ptim on which setting implemented */
 } RocsTimLibSetting;

#define RocsTimLibSETTINGS 256

typedef struct {
   unsigned int      Size;
   RocsTimLibSetting Setting[RocsTimLibSETTINGS];
 } RocsTimLibSettings;

static RocsTimLibSettings settings;
static unsigned int initialized = 0;

#define OX_SSC_CT 386
#define OX_SSC_FR 0x2000FFFF

static TimLibTime ssctm;  /* Start of SSC time */

static unsigned int gn_scnum = 0;

/* ----------------------------------------------------- */
/* Local routine to subtract one time stamp from another */
/* Result is in milli-seconds                            */

static unsigned long SubtractStamps(TimLibTime *left, TimLibTime *right) {

long      secs, mscs;
long long l, r, t;

   l = left->Second; r = right->Second; t = (l - r);
   secs = t;
   l = left->Nano;   r = right->Nano;   t = (l/1000000) - (r/1000000);
   mscs = t;

   while (mscs < 0) {
      mscs += 1000;
      secs -= 1;
   }
   if (secs < 0) return 0;
   return (unsigned long) (secs * 1000) + mscs;
}

/* ==================================================== */
/* Explicit initialize of the library.                  */
/* Attaches to the SPS telegram and initializes         */
/* settings and TimLib (with old style events allowed!) */
/* We need a standard AB/CO Front end and get_tgm_tim   */
/* must be running.                                     */

RocsTimLibError RocsTimLibInit() {
TimLibError ter;
TgmCompletion tcc;

   if (initialized) return RocsTimLibErrorOK;

   bzero((void *) &settings, sizeof(RocsTimLibSettings));
   bzero((void *) &ssctm   , sizeof(TimLibTime));

   tcc = TgmAttach(TgmSPS,TgmTELEGRAM);
   if (tcc != TgmSUCCESS) {
      fprintf(stderr,"RocsTimLib: Can't attach to the SPS telegram\n");
      return RocsTimLibErrorFAILED;
   }

   timlib_allow_old = 1;
   ctr_device = TimLibDevice_ANY;

   gn_scnum = TgmGetGroupNumber(TgmSPS,"SCNUM");

   ter = TimLibInitialize(TimLibDevice_CTR);
   if (ter != TimLibErrorSUCCESS) {
      fprintf(stderr,"RocsTimLib: Can't initialize TimLib:%s\n",TimLibErrorToString(ter));
      return RocsTimLibErrorFAILED;
   }

   ter = TimLibQueue(0,0); /* Queueing ON timeouts OFF */

   initialized = 1;
   return RocsTimLibErrorOK;
}

/* ================================================================ */
/* Emulate setting up a TG8 action accross TimLib.                  */
/* The "data" word is bound to the "config" and will be returned by */
/* the wait routine should the config produce an interrupt.         */
/* If "config->uControl" is zero, we connect to a cable event, else */
/* I create a PTIM ID uControl to emulate the TG8 action.           */

RocsTimLibError RocsTimLibSet(Tg8User *config, unsigned short data) {
TimLibError ter;
TimLibCcv ccv;
TimLibCcvMask ccm;
unsigned long ctm, frm, ptm, mod, chn, dim, tmp;
int i, connect;

   if (initialized) {

      if (settings.Size >= RocsTimLibSETTINGS) {
	 fprintf(stderr,"RocsTimLib: No memory space for setting\n");
	 return RocsTimLibErrorFAILED;
      }

      for (i=0; i<settings.Size; i++) {
	 if ((settings.Setting[i].Frame   == config->uEvent.Long) &&
	     (settings.Setting[i].Control == config->uControl)) {
	    fprintf(stderr,
		    "RocsTimLib: Can't create the same action twice: Fr:0x%08X Cw:0x%04X\n",
		    (int) config->uEvent.Long,
		    (int) config->uControl);
	    return RocsTimLibErrorFAILED;
	 }
      }

      frm = config->uEvent.Long | 0xFFFF;
      ctm = TgvGetMemberForFrame(frm);
      if (ctm == 0) {
	 fprintf(stderr,
		 "RocsTimLib: Can't get CTIM for frame:0x%08X\n",
		 (int) frm);
	 return RocsTimLibErrorFAILED;
      }

      if (config->uControl == 0) {

	 /* Connect to a CTIM event */

	 ter = TimLibConnect(TimLibClassCTIM,ctm,0);
	 if (ter != TimLibErrorSUCCESS) {
	    fprintf(stderr,"RocsTimLib: Can't connect to CTIM:%d:%s\n",(int) ctm,TimLibErrorToString(ter));
	    return RocsTimLibErrorFAILED;
	 }
	 settings.Setting[settings.Size].Ptim = 0;

      } else {

	 /* Set up a PTIM. First check if its already created (relaunch of callers program) */

	 ptm = config->uControl + config->uEvent.Long;
	 ter = TimLibGetPtimObject(ptm,&mod,&chn,&dim);
	 if (ter == TimLibErrorPTIM) {

	    chn = (config->uControl >> Tg8CW_CNT_BITN) & Tg8CW_CNT_BITM;
	    if (chn == 0) chn = 8;
	    mod = 1; dim = 1;

	    ter = TimLibCreatePtimObject(ptm,mod,chn,dim);
	    if (ter != TimLibErrorSUCCESS) {
	       fprintf(stderr,"RocsTimLib: Can't create PTIM:%d\n",(int) ptm);
	       return RocsTimLibErrorFAILED;
	    }
	 }

	 if (ter != TimLibErrorSUCCESS) {
	    fprintf(stderr,"RocsTimLib: Can't get PTIM:%d:%s\n",(int) ptm, TimLibErrorToString(ter));
	    return RocsTimLibErrorFAILED;
	 }

	 /* Set up the ptim control values */

	 bzero((void *) &ccv, sizeof(TimLibCcv));

	 tmp = (config->uControl >> Tg8CW_INT_BITN) & Tg8CW_INT_BITM;
	 if (tmp == 0) {
	    fprintf(stderr,"RocsTimLib: Bad control word, No interrupt and no Output\n");
	    return RocsTimLibErrorFAILED;
	 }

	 /* Will connect to PTIM later if needed */

	 connect = 0;
	 if (tmp & Tg8DO_OUTPUT)    ccv.Enable = TimLibEnableOUT;
	 if (tmp & Tg8DO_INTERRUPT) connect = 1;

	 tmp = (config->uControl >> Tg8CW_START_BITN) & Tg8CW_START_BITM;
	 if      (tmp == Tg8CM_NORMAL)   ccv.Start = TimLibStartNORMAL;
	 else if (tmp == Tg8CM_CHAINED)  ccv.Start = TimLibStartCHAINED;
	 else if (tmp == Tg8CM_EXTERNAL) ccv.Start = TimLibStartEXT1;
	 else {
	    fprintf(stderr,"RocsTimLib: Bad control word, Illegal Start\n");
	    return RocsTimLibErrorFAILED;
	 }

	 tmp = (config->uControl >> Tg8CW_CLOCK_BITN) & Tg8CW_CLOCK_BITM;
	 if      (tmp == Tg8CLK_MILLISECOND) ccv.Clock = TimLibClock1KHZ;
	 else if (tmp == Tg8CLK_X1)          ccv.Clock = TimLibClockEXT1;
	 else if (tmp == Tg8CLK_X2)          ccv.Clock = TimLibClockEXT2;
	 else {
	    fprintf(stderr,"RocsTimLib: Bad control word, Illegal Clock\n");
	    return RocsTimLibErrorFAILED;
	 }

	 ccm = TimLibCcvMaskENABLE
	     | TimLibCcvMaskSTART
	     | TimLibCcvMaskMODE
	     | TimLibCcvMaskCLOCK
	     | TimLibCcvMaskPWIDTH
	     | TimLibCcvMaskDELAY
	     | TimLibCcvMaskPOLARITY
	     | TimLibCcvMaskCTIM
	     | TimLibCcvMaskPAYLOAD;

	 ccv.Mode = TimLibModeNORMAL;
	 ccv.PulsWidth = 399;
	 ccv.Delay = config->uDelay;
	 ccv.Polarity = TimLibPolarityTTL_BAR; /* Shit ! I hope there are no wired ORs The CTR is totem pole */
	 ccv.Ctim = ctm;
	 ccv.Payload = config->uEvent.Long & 0xFFFF;

	 ter = TimLibSet(ptm,0,0,0,ccm,&ccv);
	 if (ter != TimLibErrorSUCCESS) {
	    fprintf(stderr,
		    "RocsTimLib: Can't set PTIM:%d:%s\n",
		    (int) ptm,
		    TimLibErrorToString(ter));
	    return RocsTimLibErrorFAILED;
	 }

	 if (connect) {
	    ter = TimLibConnect(TimLibClassPTIM,ptm,mod);
	    if (ter != TimLibErrorSUCCESS) {
	       fprintf(stderr,
		       "RocsTimLib: Can't Connect PTIM:%d:%s\n",
		       (int) ptm,
		       TimLibErrorToString(ter));
	       return RocsTimLibErrorFAILED;
	    }
	 }

	 settings.Setting[settings.Size].Ptim = ptm;
      }

      settings.Setting[settings.Size].Frame   = config->uEvent.Long;
      settings.Setting[settings.Size].Control = config->uControl;
      settings.Setting[settings.Size].Delay   = config->uDelay;
      settings.Setting[settings.Size].Data    = data;
      settings.Setting[settings.Size].Ctim    = ctm;
      settings.Size++;

      return RocsTimLibErrorOK;
   }

   fprintf(stderr,"RocsTimLib: Library has not been initialized\n");
   return RocsTimLibErrorFAILED;
}

/* ===================================================== */
/* Wait for a PTIM or a CTIM arriving from settings made */

RocsTimLibError RocsTimLibWait(unsigned long      *frame,      /* Full 32-bit event frame and payload */
			       unsigned short     *scnum,      /* Super-cycle number */
			       unsigned long      *msinsc,     /* Current milli-second in Super-cycle */
			       unsigned long      *sclenms,    /* Super-cycle length in milli-seconds */
			       unsigned short     *data,       /* Setting data */
			       unsigned long long *cystamp) {  /* Cycle stamp in nano-seconds */

TimLibClass    iclss;
unsigned long  equip;
TimLibTime     onzero;
TimLibTime     trigger;
unsigned long  ctim;
unsigned long  payload;

TimLibError ter;
int i, gv_scnum;


   if (initialized && settings.Size) {

      ter = TimLibWait(&iclss,   /* Class of interrupt */
		       &equip,   /* PTIM CTIM or hardware mask */
		       NULL,     /* Ptim line number 1..n or 0 */
		       NULL,     /* Hardware source of interrupt */
		       &onzero,  /* Time of interrupt/output */
		       &trigger, /* Time of counters load */
		       NULL,     /* Time of counters start */
		       &ctim,    /* CTIM trigger equipment ID */
		       &payload, /* Payload of trigger event */
		       NULL,     /* Module that interrupted */
		       NULL,     /* Number of missed interrupts */
		       NULL,     /* Remaining interrupts on queue */
		       NULL);    /* Corresponding TgmMachine */

      if (ter != TimLibErrorSUCCESS) {
	 fprintf(stderr,"RocsTimLib: Error in Wait:%s\n",TimLibErrorToString(ter));
	 return RocsTimLibErrorFAILED;
      }

      *cystamp = TgmGetLastTelegramTimeStamp(TgmSPS) * 1000000;

      if (TgmGetGroupValue(TgmSPS,TgmCURRENT,0,gn_scnum,&gv_scnum) != TgmSUCCESS) {
	 fprintf(stderr,"RocsTimLib: Error: Cant get SCNUM value\n");
	 return RocsTimLibErrorFAILED;
      }
      *scnum = (unsigned short) gv_scnum;

      if (ctim == OX_SSC_CT) {
	 bcopy((void *) &trigger,(void *) &ssctm,sizeof(TimLibTime));
	 ssctm.Nano += 1000000;
      }

      if (iclss == TimLibClassCTIM) {
	 for (i=0; i<settings.Size; i++) {
	    if ((settings.Setting[i].Ctim == equip)
	    &&  (settings.Setting[i].Ptim == 0    )) {

	       if (((settings.Setting[i].Frame & 0xFFFF) == 0xFFFF)
	       ||  ((settings.Setting[i].Frame & 0xFFFF) == payload)) {

		  *frame = (settings.Setting[i].Frame & 0xFFFF0000) | (payload & 0x0000FFFF);
		  *data  = settings.Setting[i].Data;
		  if (ssctm.Second == 0) *msinsc = 0;
		  else                   *msinsc = SubtractStamps(&onzero,&ssctm);
		  *sclenms = TgmGetSCLengthBPs(TgmSPS) * TgmGetBPWidth();
		  return RocsTimLibErrorOK;
	       }
	    }
	 }
      }

      if (iclss == TimLibClassPTIM) {
	 for (i=0; i<settings.Size; i++) {
	    if (settings.Setting[i].Ptim == equip) {
	       *frame = (settings.Setting[i].Frame & 0xFFFF0000) | (payload & 0x0000FFFF);
	       *data  = settings.Setting[i].Data;
	       if (ssctm.Second == 0) *msinsc = 0;
	       else                   *msinsc = SubtractStamps(&onzero,&ssctm);
	       *sclenms = TgmGetSCLengthBPs(TgmSPS) * TgmGetBPWidth();
	       return RocsTimLibErrorOK;
	    }
	 }
      }

#if DONT_ALLOW_UNEXPECTED_INTERRUPTS

      fprintf(stderr,"RocsTimLib: Unexpected interrupt:%d (0x%X) Class:%d Settings:%d\n",
	      (int) equip,
	      (int) equip,
	      (int) iclss,
	      (int) settings.Size);
      for (i=0; i<settings.Size; i++) {
	 fprintf(stderr,"RocsTimLib:Setting:%d Equip:%d Frame:0x%X Cntrl:0x%X\n",
		 i,
		 (int) settings.Setting[i].Ptim,
		 (int) settings.Setting[i].Frame,
		 (int) settings.Setting[i].Control);
      }
      return RocsTimLibErrorFAILED;

#else

      /* Return 0 in data and the equipment number */

      *frame = equip; *data = 0;
      return RocsTimLibErrorOK;

#endif
   }

   fprintf(stderr,"RocsTimLib: Library has not been initialized\n");
   return RocsTimLibErrorFAILED;
}
