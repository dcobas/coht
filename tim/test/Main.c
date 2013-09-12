/**************************************************************************/
/* TimLib test program                                                    */
/* Julian Lewis 25th April 2005                                           */
/**************************************************************************/

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

#include <err/err.h>      /* Error handling */

#include <TimLib.h>

extern int timlib_debug;  /* 1/Print stuff, 2/Wipe memory */
extern int timlib_delay;  /* Input delay when not zero */
extern int timlib_enable; /* Enable modules */
extern int timlib_jtag;   /* Reload outdated VHDL accross jtag */
extern int timlib_ctims;  /* Size of CTIM table in CPS TG8 driver */
extern int timlib_oby1_8; /* CTRV P2 Output bytes for modules 8..1 */

#include <ctrdrvr.h>

extern CtrDrvrDevice ctr_device; /* To handle specific kinds of CTR devices */
extern char *devnames[];         /* by name ctr ctri ctrp ctrv ctre etc */

/**************************************************************************/
/* Code section from here on                                              */
/**************************************************************************/

/* Print news on startup if not zero */

typedef enum {
   HELP,
   ENABLE,
   DELAY,
   DEBUG,
   JTAG,
   P2BYTES,
   DEVICE,  /* This must be last as it causes the call to TimLibInit !! */
   OPTIONS
 } Options;

char *options[OPTIONS] = {"help","enable","delay","debug","jtag","p2bytes","device"};

#define NEWS 1

#define HISTORIES 24
#define CMD_BUF_SIZE 128

static char history[HISTORIES][CMD_BUF_SIZE];
static char *cmdbuf = &(history[0][0]);
static unsigned int  cmdindx = 0;
static char prompt[32];
static char *pname = NULL;

static char *DevNames[TimLibDEVICES] = { "...", "CTR", "CPS_TG8", "SPS_TG8", "NETWORK" };
static TimLibDevice Dev = TimLibDevice_ANY;

#include "Cmds.h"
#include "GetAtoms.c"
#include "PrintAtoms.c"
#include "DoCmd.c"
#include "Cmds.c"
#include "TimCmds.c"

/******************************************************************/
/* Get size of CTIM table needed on CPS TG8. The driver allocates */
/* memory dynamically via sysbrk. Once the memory is allocated it */
/* can not change size. The "extra" parameter is used to allocate */
/* a few more CTIM for dynamic use through the CreateCtim routine */

unsigned int GetCtimSize(unsigned int extra) {
unsigned int cnt;
unsigned long equip;

   cnt = extra; equip = TgvFirstGMember();

   while (equip) {
      cnt++;
      equip = TgvNextGMember();
   }
   return cnt;
}

/******************************************************************/

static Boolean error_handler(class,text)
ErrClass class;
char *text; {

   fprintf(stderr,"%s\n",text);
   return(False);
}

/**************************************************************************/
/* Prompt and do commands in a loop                                       */
/**************************************************************************/

#define SPARE_CTIMS 10    /* Space for dynamic CTIMs in TG8 CPS */

int main(int argc,char *argv[]) {

int i, j, found_device;
unsigned int cnt, mod, dbg, ind, enb, jtg, oby;
unsigned long cbl;
char *cp, *ep;
char host[49];
char tmpb[CMD_BUF_SIZE];
TimLibError err;
TimLibDevice dev, fdev, ldev;
TgvName cblnam;

   TimLibClient = 1;
   cp = NULL; ep = NULL;

   dev = TimLibDevice_ANY;
   found_device = 0;

   for (i=1; i<argc; i++) {

      if (strcmp(argv[i],options[HELP]) == 0) {
	 printf("\nOptions are:\n\n");
	 for (i=0; i<OPTIONS; i++) {
	    printf("%s ",options[i]);
	    switch ((Options) i) {
	       case HELP:
		  printf("[print this help text]\n");
	       break;

	       case ENABLE:
		  printf("<Enable: 1/Enable>\n");
	       break;

	       case DELAY:
		  printf("<Input delay in 25ns ticks>\n");
	       break;

	       case DEBUG:
		  printf("<Debug level: 0/None 1/Driver 2/ReWriteActions 3/ClearActions>\n");
	       break;

	       case JTAG:
		  printf("<Jtag: 1/Update out of date modules with new VHDL>\n");
	       break;

	       case P2BYTES:
		  printf("<p2bytes: in this order:0x87654321 for VME P2 CTRV>\n");
	       break;

	       case DEVICE:
		  printf("<Device:");
		  for (dev=TimLibDevice_CTR; dev<TimLibDEVICES; dev++) {
		     printf(" %s",DevNames[(int) dev]);
		  }
		  printf(">\n");
	       break;

	       default:
		  printf("For help type: timtest help\n");
	    }
	 }
	 printf("\n\n");
	 exit(0);
      }

      else if (strcmp(argv[i],options[ENABLE]) == 0) {
	 i++;
	 cp = argv[i];
	 enb = 1;
	 if (cp) enb = strtoul(cp,&ep,0);
	 if (enb) printf("Enable All Modules: %d/On\n", (int) enb);
	 timlib_enable = enb;
	 continue;
      }

      else if (strcmp(argv[i],options[DELAY]) == 0) {
	 i++;
	 cp = argv[i];
	 ind = 4;
	 if (cp) ind = strtoul(cp,&ep,0);
	 if (ind) printf("Input Delay: %d = %dns\n", (int) ind, (int) (ind*25));
	 timlib_delay = ind;
	 continue;
      }

      else if (strcmp(argv[i],options[DEBUG]) == 0) {
	 i++;
	 cp = argv[i];
	 dbg = 0;
	 if (cp) dbg = strtoul(cp,&ep,0);
	 TimLibSetDebug(dbg);
	 if (dbg)      printf("Driver debugging ON\n");
	 if (dbg == 2) printf("Re-write all actions\n");
	 if (dbg == 3) printf("Clearing all actions\n");
	 continue;
      }

      else if (strcmp(argv[i],options[JTAG]) == 0) {
	 i++;
	 cp = argv[i];
	 jtg = 1;
	 if (cp) jtg = strtoul(cp,&ep,0);
	 if (jtg) printf("Jtag VHDL Auto update: %d/On\n",(int) jtg);
	 timlib_jtag = jtg;
	 continue;
      }

      else if (strcmp(argv[i],options[P2BYTES]) == 0) {
	 i++;
	 cp = argv[i];
	 oby = 0;
	 if (cp) oby = strtoul(cp,&ep,0);
	 if (oby) printf("P2Bytes for CTRV P2 connector: 0x%X\n",(int) oby);
	 timlib_oby1_8 = oby;
	 continue;
      }

      else if (strcmp(argv[i],options[DEVICE]) == 0) {
	 i++;
	 cp = argv[i];
	 dev = TimLibDevice_ANY;
	 if (cp) {
	    for (fdev=TimLibDevice_CTR; fdev<TimLibDEVICES; fdev++) {
	       if (strcmp(DevNames[(int) fdev],argv[i]) == 0) {
		  dev = ldev = fdev;
		  break;
	       }
	    }
	 }
	 continue;
      }

   }

   if (dev == TimLibDevice_ANY) {
      fdev = TimLibDevice_CTR;
      ldev = TimLibDevice_NETWORK;
   }

   for (j=1; j<argc; j++) {
      for (i=0; i<CtrDrvrDEVICES; i++) {
	 if (strcmp(argv[j],devnames[i]) == 0) {
	    ctr_device = (CtrDrvrDevice) i;
	    printf("Forcing CTR device type:%s\n",devnames[i]);
	    break;
	 }
      }
   }

   for (dev=fdev; dev<=ldev; dev++) {
      err = TimLibInitialize(dev);
      if (err == TimLibErrorSUCCESS) {
	 Dev = dev;
	 fprintf(stderr,"timtest: Initialized library for device: %s OK\n",DevNames[(int) dev]);
	 found_device = 1;

	 /* For the CPS TG8 the table must be allocated once by the driver */
	 /* so I allocate an extra few slots for dynamic CTIM creation. */

	 if (Dev == TimLibDevice_TG8_CPS) {
	    timlib_ctims = GetCtimSize(SPARE_CTIMS);
	    fprintf(stderr,"timtest: CPS TG8 driver Ctim table allocation size:%d\n",
		    (int) timlib_ctims);
	 }
	 break;
      }
   }

   if (found_device == 0) {

      fprintf(stderr,"timtest: Could not initialize TimLib: No device found\n");
      printf("For help type: timtest %s\n",options[HELP]);

      exit(1);
   }

#if NEWS
   printf("timtest: See <news> command\n");
   printf("timtest: Type h for help\n");
#endif

   pname = argv[0];
   printf("%s: Compiled %s %s\n",pname,__DATE__,__TIME__);

   cnt = TimLibGetInstalledModuleCount();
   for (i=0; i<cnt; i++) {
      mod = i+1;
      TimLibGetCableId(mod,&cbl);
      if (cbl) {
	 if (TgvGetCableName(cbl,&cblnam) == 0) {
	    fprintf(stderr,
		    "timtest: WARNING: This DSC is NOT configured for Cable:%d on Module:%d\n",
		    (int) cbl,
		    (int) mod);
	 } else printf("DSC Configuration OK on module:%d for Cable:%s(%d)\n",
		      (int) mod,
			    cblnam,
		      (int) cbl);
      }
   }
   if (cnt) printf("timtest: Found:%d %s Modules\n",(int) cnt, DevNames[(int) Dev]);

   bzero((void *) host,49);
   gethostname(host,48);

   ErrSetHandler((ErrHandler) error_handler);

   while (True) {

      cmdbuf = &(history[cmdindx][0]);
      if (strlen(cmdbuf)) printf("{%s} ",cmdbuf);
      fflush(stdout);

      sprintf(prompt,"%s:Tim:%s[%02d]",host,DevNames[(int) Dev],cmdindx+1);
      printf("%s",prompt);

      bzero((void *) tmpb,CMD_BUF_SIZE);
      if (fgets(tmpb, CMD_BUF_SIZE, stdin)==NULL) exit(1);

      cp = &(tmpb[0]);

      if (*cp == '!') {
	 ++cp;
	 cmdindx = strtoul(cp,&cp,0) -1;
	 if (cmdindx >= HISTORIES) cmdindx = 0;
	 cmdbuf = &(history[cmdindx][0]);
	 continue;
      } else if (*cp == '.') {
	 printf("Execute:%s\n",cmdbuf); fflush(stdout);
      } else if ((*cp == '\n') || (*cp == '\0')) {
	 cmdindx++;
	 if (cmdindx >= HISTORIES) { printf("\n"); cmdindx = 0; }
	 cmdbuf = &(history[cmdindx][0]);
	 continue;
      } else if (*cp == '?') {
	 printf("History:\n");
	 printf("\t!<1..24> Goto command\n");
	 printf("\tCR       Goto next command\n");
	 printf("\t.        Execute current command\n");
	 printf("\this      Show command history\n");
	 continue;
      } else {
	 cmdindx++; if (cmdindx >= HISTORIES) { printf("\n"); cmdindx = 0; }
	 strcpy(cmdbuf,tmpb);
      }
      bzero((void *) val_bufs,sizeof(val_bufs));
      GetAtoms(cmdbuf);
      DoCmd(0);
   }
}
