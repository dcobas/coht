/* ================================================ */
/* Simply library interface to the AC Dipole system */
/* Julian Lewis AB/CO/HT Fri 19th Sept 2008         */
/* ================================================ */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <errno.h>        /* Error numbers */
#include <sys/file.h>
#include <sys/stat.h>
#include <a.out.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>

#include <drm.h>
#include <acdxdrvr.h>
#include <acdxdrvrP.h>

#include <libacdx.h>

#include "/acc/src/dsc/drivers/coht/acdx/test/AcdxCmds.h"

#ifndef PI
#define PI M_PI
#endif

double round(double x);

static short    func[TOTAL_POINTS];
static unsigned long srise, sftop, sfall;

static unsigned long freq = 3000;
static unsigned long ampl = 500;
static unsigned long rise = 200000;
static unsigned long ftop = 200000;
static unsigned long fall = 200000;

static unsigned long spts = SIN_POINTS_PER_SECOND;

static int acdx = 0;

/* ================================================ */

int AcdxOpen() {

char fnm[32];
int  i, fn;

   for (i = 1; i <= AcdxDrvrCLIENT_CONTEXTS; i++) {
      sprintf(fnm,"/dev/acdx.%1d",i);
      if ((fn = open(fnm,O_RDWR,0)) > 0) return(fn);
   };
   return(0);
}

/* ================================================ */

static double GetAmplitude(unsigned long t) {
unsigned long ft;

   if (rise > 0) {

      srise = 0; sftop = rise + srise; sfall = rise + ftop + srise;

      if (t <= sftop) return (double) (t * ampl) / rise;

      if (t <= sfall) return (double) ampl;

      if (t <= sfall + fall) {
	 ft = t - sfall;
	 return (double) ampl * ( (double) (fall - ft) / (double) fall );
      }
   }
   return 0.0;
}

/* ================================================ */

static short FuncToShort(double val) {

double dac;
short dval;

   dac = val / 10000.0;
   dac = dac * (double) 0x7FFF;
   dval = (short) round(dac);
   return dval;
}

/* ================================================ */

unsigned long GetSampClock() {

unsigned long i;
unsigned long array[2];
AcdxDrvrRawIoBlock iob;

   iob.Size = 1;
   iob.UserArray = array;
   iob.Offset = AcdxSmpClkHz;

   if (ioctl(acdx,AcdxDrvrRAW_READ,&iob) < 0)
      return SIN_POINTS_PER_SECOND;

   i = array[0];
   if ((i < SIN_POINTS_PER_SECOND - 512)
   ||  (i > SIN_POINTS_PER_SECOND + 512)) {
      return SIN_POINTS_PER_SECOND;
   }
   return i;
}

/* ================================================ */

static int SinWave() {

double omega, tee;

unsigned long i;

   bzero((void *) func, sizeof(short)*TOTAL_POINTS);

   spts = GetSampClock();

   omega = 2.0 * PI * (double) freq;
   for (i=0; i<TOTAL_POINTS; i++) {
      tee = (double) i / spts;
      func[i] = FuncToShort(sin(omega * tee) * GetAmplitude(i));
   }
   return 1;
}

/* ================================================ */

static int PutFunc() {

AcdxMap *map = NULL;
AcdxDrvrRawIoBlock iob;
unsigned long array[2];
unsigned long zbtaddr, zbtdata;
unsigned long i;

   zbtaddr = (unsigned long) ((unsigned long) &(map->ZBTSRAMAddr)) / sizeof(long);
   zbtdata = (unsigned long) ((unsigned long) &(map->ZBTSRAMData)) / sizeof(long);

   for (i=0; i<TOTAL_POINTS; i+=2) {

      iob.Size = 1;
      iob.UserArray = array;
      iob.Offset = zbtaddr;
      array[0] = i>>1;
      if (ioctl(acdx,AcdxDrvrRAW_WRITE,&iob) < 0) return 0;

      iob.Size = 1;
      iob.UserArray = array;
      iob.Offset = zbtdata;
      array[0] = ( (0x0000FFFF & (func[i  ]      ))
	       |   (0xFFFF0000 & (func[i+1] << 16)) );
      if (ioctl(acdx,AcdxDrvrRAW_WRITE,&iob) < 0) return 0;
   }

   return 1;
}

/* ================================================ */

AcdxLibCompletion AcdxLibInit() {

   if (acdx == 0) {
      acdx = AcdxOpen();
      if (acdx == 0) return AcdxLibFAIL;
   }
   return AcdxLibOK;
}

/* ================================================ */

AcdxLibCompletion AcdxLoadFunction(unsigned int f,   /* Frequency in Hertz */
				   unsigned int a) { /* Amplitude in Milli-Volts */

   if (AcdxLibInit() == AcdxLibOK) {

      if (f) freq = f;
      else   freq = 3000;
      if (a) ampl = a;
      else   ampl = 500;

      SinWave();
      if (PutFunc()) return AcdxLibOK;
   }
   return AcdxLibFAIL;
}

/* ================================================ */

#define ARMED 0x518002
#define READY 0x510002
#define AMPON 0x002000

/* ================================================ */

int AcdxAmpIsOn() {

unsigned long stat;

   stat = 0;
   if (AcdxLibInit() == AcdxLibOK) {
      ioctl(acdx,AcdxDrvrGET_STATUS_CONTROL,&stat);
      if (stat & AMPON) return 1;
   }
   return 0;
}

/* ================================================ */

int AcdxIsArmed() {

unsigned long stat;

   stat = 0;
   if (AcdxLibInit() == AcdxLibOK) {
      ioctl(acdx,AcdxDrvrGET_STATUS_CONTROL,&stat);
      if (stat & AcdxDrvrStatusControlARM) return 1;
   }
   return 0;
}

/* ================================================ */

AcdxLibCompletion AcdxArm() {

unsigned long stat;

   if (AcdxLibInit() == AcdxLibOK) {

      stat = 0;
      if (AcdxAmpIsOn()) stat = AMPON;

      stat |= ARMED;
      if (ioctl(acdx,AcdxDrvrSET_STATUS_CONTROL,&stat) >= 0) {
	 return AcdxLibOK;
      }
   }
   return AcdxLibFAIL;
}

/* ================================================ */

AcdxLibCompletion AcdxUnArm() {

unsigned long stat;

   if (AcdxLibInit() == AcdxLibOK) {

      stat = 0;
      if (AcdxAmpIsOn()) stat = AMPON;

      stat |= READY;
      if (ioctl(acdx,AcdxDrvrSET_STATUS_CONTROL,&stat) >= 0) {
	 return AcdxLibOK;
      }
   }
   return AcdxLibFAIL;
}

/* ================================================ */

int AcdxIsBusy() {

unsigned long stat;

   stat = 0;
   if (AcdxLibInit() == AcdxLibOK) {
      ioctl(acdx,AcdxDrvrGET_STATUS_CONTROL,&stat);
      if (stat & AcdxDrvrStatusControlONE_MIN_INHIB) return 1;
   }
   return 0;
}

/* ================================================ */
/* UnArm and Set Amp ON                             */
/* Do not call this routine when amp is armed !!!!  */

AcdxLibCompletion AcdxAmpOn() {

unsigned long stat;

   if (AcdxLibInit() == AcdxLibOK) {

      if (AcdxIsArmed()) return AcdxLibFAIL;

      stat = READY | AMPON;
      if (ioctl(acdx,AcdxDrvrSET_STATUS_CONTROL,&stat) >= 0) {
	 return AcdxLibOK;
      }
   }
   return AcdxLibFAIL;
}

/* ================================================ */
/* UnArm and set Amp OFF                            */
/* Do not call this routine when amp is armed !!!!  */

AcdxLibCompletion AcdxAmpOff() {

unsigned long stat;

   if (AcdxLibInit() == AcdxLibOK) {

      if (AcdxIsArmed()) return AcdxLibFAIL;

      stat = READY;
      if (ioctl(acdx,AcdxDrvrSET_STATUS_CONTROL,&stat) >= 0) {
	 return AcdxLibOK;
      }
   }
   return AcdxLibFAIL;
}
