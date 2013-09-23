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
#include <ctype.h>

#include <tim/TimLib.h>
#include <tgm/tgm.h>

/* ============================================================== */
/* How to get AqnTimeStamps from a telegram.                      */
/* This is a pure Tgm version unless USE_TIM_LIB is defined.      */
/* I have tested both methods on dleipow1.                        */
/* Julian                                                         */
/* ============================================================== */

int main(int argc,char *argv[]) {

#ifdef USE_TIM_LIB
TimLibError err;
#endif

TimLibTime  stamp;
TgmTelegram telegram;
TgmHistoryBuffer hb, *phb = &hb;
unsigned long gn, cytag, sec, msc;

#ifdef USE_TIM_LIB
   err = TimLibInitialize(TimLibDevice_ANY); /*initialize the hardware*/
   if (err) {
   	printf("Error: %s\n", TimLibErrorToString(err));
	exit(err);
   }
#endif

   /* Get the group number of the CYTAG group in the LEIR telegram */

   if (TgmAttach(TgmLEI,TgmTELEGRAM) != TgmSUCCESS) exit(1);
   gn = TgmGetGroupNumber(TgmLEI,"CYTAG");

   while (1) {

      TgmWaitForNextTelegram(TgmLEI);    /* Wait for a LEIR telegram */

      /* You can get the telegram from TimLib, or even better get it */
      /* directly from TgmLib. In the Tgm call you access a history  */
      /* of telegrams using TgmCURRENT => The current telegram, and  */
      /* offset 0 from the current. If offset was -1 you would get   */
      /* the previous telegram, -2 the one before that etc. So take  */
      /* your pick, however to un mix Tgm and Tim lib, I think its   */
      /* better to call the Tgm version. See "man tgm"               */

#ifdef USE_TIM_LIB
      TimLibGetTelegram(0,TgmLEI,&telegram);         /* Get the last LEIR telegram */
#else
      TgmGetTelegram(TgmLEI,TgmCURRENT,0,&telegram); /* Get the last LEIR telegram */
#endif

      cytag = TgmGetGroupValueFromTelegram(gn,&telegram); /* Get the cytag group value */

      /* ============================================================== */
      /* Here is the code to get the AqnTimeStamp                       */
      /* You need a TgmMachine, its telegram, and its CYTAG group numer */
      /* ============================================================== */

      TgmHisClear(phb);                 /* Clear history buffer */
      TgmHisSetMachine(phb,TgmLEI);     /* Set the machine to LEI */
      TgmHisSetCycleTag(phb,cytag);     /* Set the cycle tag to what was in the telegram */
      TgmHisGetHistory(phb,1,&phb);     /* Search history for last entry */
      sec = TgmHisGetAqnTimeSec(phb);   /* Get UTC time in seconds */
      msc = TgmHisGetAqnTimeMSc(phb);   /* Get the millisecond in second */

      /* Make a TimLib Time Stamp from the AqnTimeStamp */

      stamp.Second  = sec;
      stamp.Nano    = msc * 1000000;
      stamp.Machine = TgmLEI;
      stamp.CTrain  = 0;                /* By definition */

      /* Do what you do with the stamp .... */

      /* ============================================================== */
      /* I will just print out the history buffer I got back            */
      /* ============================================================== */

      TgmHisPrint(phb); /* Print out the history buffer */
   }
}
