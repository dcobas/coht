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

#include <TimLib.h>

#define MAX_MODULE_COUNT 16
#define SLEEP_TIME 10
#define SUPPRESS_MES_COUNT 10

/**************************************************************************/
/* Convert a TimLib time in milliseconds to a string routine.             */
/* Result is a pointer to a static string representing the time.          */
/*    the format is: Thu-18/Jan/2001 08:25:14.967                         */
/*                   day-dd/mon/yyyy hh:mm:ss.ddd                         */

volatile char *TimeToStr(TimLibTime *t) {

static char tbuf[128];

char tmp[128];
char *yr, *ti, *md, *mn, *dy;

   bzero((void *) tbuf, 128);
   bzero((void *) tmp, 128);

   if (t->Second) {
#ifdef __68k__
      ctime_r(&t->Second, tmp, 128);
#else
      ctime_r(&t->Second, tmp);
#endif
      tmp[3] = 0;
      dy = &(tmp[0]);
      tmp[7] = 0;
      mn = &(tmp[4]);
      tmp[10] = 0;
      md = &(tmp[8]);
      if (md[0] == ' ') md[0] = '0';
      tmp[19] = 0;
      ti = &(tmp[11]);
      tmp[24] = 0;
      yr = &(tmp[20]);
      sprintf (tbuf, "%s-%s/%s/%s %s"  , dy, md, mn, yr, ti);
      if (t->Nano) {
      	  sprintf(&tbuf[strlen(tbuf)],".%09lu",t->Nano);
      }

   } else sprintf (tbuf, "--- Zero ---");

   
   if ((t->Machine != TgmMACHINE_NONE) && (t->CTrain > 0)) {
      strcat(tbuf," ");
      strcat(tbuf,TgmGetMachineName(t->Machine));
      strcat(tbuf," C:");
      sprintf(tmp,"%d",(int) t->CTrain);
      strcat(tbuf,tmp);
   }
   return (tbuf);
}

/**************************************************************************/
/* Status To String                                                       */

#define STATAE 5

static char *StatusOn[STATAE] = {    "GmtOk",
				     "PllOk",
				     "SlfOk",
				     "EnbOk",
				     "BusOk"  };

static char *StatusOf[STATAE] = { "***GmtErr***",
				  "***PllErr***",
				  "***SlfErr***",
				  "***EnbErr***",
				  "***BusErr***" };

#define ST_STR_SZ 48
static char StsStr[ST_STR_SZ];

char *StatusToStr(TimLibStatus sts) {
int i;
unsigned long msk;

   bzero((void *) StsStr,ST_STR_SZ);
   for (i=0; i<STATAE; i++) {
      msk = 1 << i;
      if (msk & TimLibStatusBITS) {
	 if (msk & sts) strcat(StsStr,StatusOn[i]);
	 else           strcat(StsStr,StatusOf[i]);
	 strcat(StsStr,":");
      } else break;
   }
   return StsStr;
}

/**************************************************************************/
/* Monitor errors for clic                                                */

static char *DevNames[TimLibDEVICES] = { "...",
					 "CTR",
					 "CPS_TG8",
					 "SPS_TG8",
					 "NETWORK" };
/* ============================= */

int main(int argc,char *argv[]) {

TimLibDevice dev;
TimLibStatus sts;
TimLibError  err;
TimLibTime   utc;

int cnt, mod;        /* Module count and module */
int ecount[MAX_MODULE_COUNT];  /* Error counts */

   for (mod=0; mod<MAX_MODULE_COUNT; mod++) ecount[mod] = 0;

   err = TimLibInitialize(TimLibDevice_ANY); /*initialize the hardware*/
   if (err) {
      fprintf(stderr,"TimSurvey:Fatal Error:%s\n",TimLibErrorToString(err));
      exit(err);
   }

   cnt = TimLibGetInstalledModuleCount();
   if (cnt) {
      fprintf(stderr,"TimSurvey:Running:Found:%d timing modules\n",cnt);
      for (mod=1; mod<=cnt; mod++) {
	 sts = TimLibGetStatus(mod,&dev);
	 fprintf(stderr,"TimSurvey:Module:%d %s\n",mod,DevNames[(int) dev]);
      }
   } else {
      fprintf(stderr,"TimSurvey:Fatal Error:No modules installed\n");
      exit(TimLibErrorMODULE);
   }

   while(1) {
      sleep(SLEEP_TIME);
      for (mod=1; mod<=cnt; mod++) {
	 sts = TimLibGetStatus(mod,&dev);
	 if (((sts & TimLibStatusGMT_OK)  == 0)    /* Timing cable error, not connected */
	 ||  ((sts & TimLibStatusSELF_OK) == 0)    /* Self test error */
	 ||  ((sts & TimLibStatusBUS_OK)  == 0)    /* Bus interrupt fault */
	 ||  ((sts & TimLibStatusENABLED) == 0)) { /* Timing reception off */

	    ecount[mod]++;

	    if (ecount[mod] >  SUPPRESS_MES_COUNT) continue;
	    if (ecount[mod] == SUPPRESS_MES_COUNT) {
	       fprintf(stderr,
		       "TimSurvey:Module:%d AlarmPrintingSuppressed:%d ConsecutiveAlarms\n",
		       mod,
		       ecount[mod]);
	       continue;
	    }
	    fprintf(stderr,
		    "TimSurvey:AlarmCount:%d Module:%d Device:%s Alarm:%s\n",
		    ecount[mod],
		    (int) mod,
		    DevNames[(int) dev],
		    StatusToStr(sts));
	 } else {
	    if (ecount[mod]) {
	       TimLibGetTime(mod,&utc);
	       fprintf(stderr,"TimSurvey:Module:%d OK again %s\n",mod,TimeToStr(&utc));
	    }
	    ecount[mod] = 0;
	 }
      }
   }
   exit(0);
}
