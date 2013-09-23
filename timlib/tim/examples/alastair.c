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
      ctime_r(&t->Second, tmp);
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

unsigned long cnt, mod; /* Module count and module */

char clic_message[128];

   err = TimLibInitialize(TimLibDevice_ANY); /*initialize the hardware*/
   if (err) {
      printf("%s: Fatal Error: %s\n", argv[0], TimLibErrorToString(err));
      exit(err);
   }

   cnt = TimLibGetInstalledModuleCount();
   while(1) {
      for (mod=1; mod<=cnt; mod++) {
	 sts = TimLibGetStatus(mod,&dev);
	 if ((sts & TimLibStatusBITS) != TimLibStatusBITS) { /* They should all be one/OK */
	    TimLibGetTime(mod,&utc);
	    sprintf(clic_message,
		    "%s: module:%d %s:clic-timing-alarm:%s at:%s",
		    argv[0],
		    (int) mod,
		    DevNames[(int) dev],
		    StatusToStr(sts),
		    TimeToStr(&utc));

	    fprintf(stderr,"%s\n",clic_message);

	    /* LogErrorInClick(clic_message) ==> Alastair fill in this bit */

	 }
      }
   }
   exit(0);
}
