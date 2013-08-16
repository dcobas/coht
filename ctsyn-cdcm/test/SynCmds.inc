#include <time.h>   /* ctime */

/* Jtag public code */

static int xsvf_iDebugLevel = 0;

#include <jtag/lenval.h>
#include <jtag/micro.h>
#include <jtag/ports.h>

#include <jtag/lenval.c>
#include <jtag/micro.c>
#include <jtag/ports.c>

#define SECONDNS 1000000000.00
#define TCK40MHZ 25.00

#define uint32_t int

/*****************************************************************/
/* Some useful routines                                          */
/*****************************************************************/

/**************************************************************************/

int IOCTL(int syn, int fun, void *arg) {

   if (syn) return ioctl(syn,fun,arg);
   return 0;
}

/**************************************************************************/

int READ(int bst, void *buf, int size) {

   if (syn) return read(syn,buf,size);
   return 1;
}

/**************************************************************************/

static void IErr(char *name, int *value) {

   if (value != NULL)
      printf("SynDrvr: [%02d] IOCTL=%s arg=%d :Error\n",
	     (int) syn, name, (int) *value);
   else
      printf("SynDrvr: [%02d] IOCTL=%s :Error\n",
	     (int) syn, name);
   perror("SynDrvr");
}

/**************************************************************************/
/* Convert a long long time in milliseconds to a string routine.          */
/* Result is a pointer to a static string representing the time.          */
/*    the format is: Thu-18/Jan/2001 08:25:14.967                         */
/*                   day-dd/mon/yyyy hh:mm:ss                             */

static char *TimeToStr (unsigned long uvt) {

static char buf[32];

char tmp[32];
char *yr, *ti, *md, *mn, *dy;

    if (uvt) {

        ctime_r (&uvt, tmp
#ifdef __Lynx__
#ifdef __68k__
                 , sizeof (tmp) - 1
#endif
#endif
#ifdef __sparc__
                 , sizeof (tmp) - 1
#endif
            );                  /* Day Mon DD HH:MM:SS YYYY\n\0 */

        tmp[3] = 0;
        dy = &(tmp[0]);
        tmp[7] = 0;
        mn = &(tmp[4]);
        tmp[10] = 0;
        md = &(tmp[8]);
        if (md[0] == ' ')
            md[0] = '0';
        tmp[19] = 0;
        ti = &(tmp[11]);
        tmp[24] = 0;
        yr = &(tmp[20]);

	sprintf (buf, "%s-%s/%s/%s %s", dy, md, mn, yr, ti);

    }
    else {
        sprintf (buf, "--- Zero ---");
    }
    return (buf);
}


/*****************************************************************/
/* Status to string                                              */

#define MAX_STAT_LEN 128

static char *StatusToString(int stat) {

static char *onbits[SynDrvrStatusSTATAE] = {"No10MHz", "NoPPS", "UNStablePPS", "PhaseErrPPS", "OutputsEnabled", "BusError", "JtagOpen"};
static char *ofbits[SynDrvrStatusSTATAE] = {"10MHzOK", "PPSOK", "StablePPS",   "PPSPhaseOK",  "OutputDisabled", "BusOK",    "JtagClosed"};

static char rslt[MAX_STAT_LEN];
int i, m;

   bzero((void *) rslt, MAX_STAT_LEN);

   for (i=0; i<SynDrvrStatusSTATAE; i++) {
      m = 1 << i;
      if (m & stat) strcat(rslt,onbits[i]);
      else          strcat(rslt,ofbits[i]);
      strcat(rslt," : ");
   }

   return rslt;
}

/*****************************************************************/
/* Connection mask to string                                     */

#define MAX_CON_LEN 256

static char *ConMaskToStr(int cnmsk, int cr) {

static char *onbits[SynDrvrConMaskSOURCES] = {"0x01 [No10MHz]",
					      "0x02 [NoPPS]",
					      "0x04 [UNStablePPS]",
					      "0x08 [PhaseErrPPS]",
					      "0x10 [InternalPPS]",
					      "0x20 [IteratePLL]",
					      "0x40 [GpsPPS]",
					      "0x80 [Sync]" };

static char tmp[32], rslt[MAX_CON_LEN];
int i, m;

   bzero((void *) rslt, MAX_CON_LEN);
   if (cnmsk == 0) return rslt;

   for (i=0; i<16; i++) {
      m = 1 << i;
      if ((m & SynDrvrConMaskMASK) == 0) break;
      if (m & cnmsk) {
	 if (cr) sprintf(tmp,"Cnx: %s\n",onbits[i]);
	 else    sprintf(tmp,"%s ",onbits[i]);
	 strcat(rslt,tmp);
      }
   }

   return rslt;
}

/*****************************************************************/
/* Commands used in the test program.                            */
/*****************************************************************/

int GetVersion(int arg) { /* No Arg */
SynDrvrVersion version;

   arg++;
   bzero((void *) &version, sizeof(SynDrvrVersion));
   if (IOCTL(syn,SynDrvrGET_VERSION,&version) < 0) {
      IErr("GET_VERSION",NULL);
      return arg;
   }

   printf("VHDL Compiled: [%u] %s\n",
    (int) version.VhdlVersion,
	  TimeToStr(version.VhdlVersion));

   printf("Drvr Compiled: [%u] %s\n",
    (int) version.DrvrVersion,
	  TimeToStr(version.DrvrVersion));

   printf("%s Compiled: %s %s\n",
	  pname,
	  __DATE__,
	  __TIME__);

   return arg;
}

/*****************************************************************/

int GetSetTmo(int arg) { /* Arg=0 => Timeouts disabled, else Arg = Timeout */
ArgVal   *v;
AtomType  at;
int timeout;

   arg++;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      timeout = v->Number;
      if (IOCTL(syn,SynDrvrSET_TIMEOUT,&timeout) < 0) {
	 IErr("SET_TIMEOUT",&timeout);
	 return arg;
      }
   }
   timeout = -1;
   if (IOCTL(syn,SynDrvrGET_TIMEOUT,&timeout) < 0) {
      IErr("GET_TIMEOUT",NULL);
      return arg;
   }
   if (timeout != 0)
      printf("Timeout: [%dus] Enabled\n",timeout);
   else
      printf("Timeout: [zero] Disabled\n");

   return arg;
}

/*****************************************************************/

int GetSetModule(int arg) {  /* Arg = Module */
ArgVal   *v;
AtomType  at;

int i, modules, module;
SynDrvrModuleAddress moad;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   modules = -1;
   if (IOCTL(syn,SynDrvrGET_MODULE_COUNT,&modules) < 0) {
      IErr("GET_MODULE_COUNT",NULL);
      return arg;
   }

   if (at == Numeric) {
      arg++;
      module = v->Number;
      if ((module > 0) && (module <= modules)) {
	 if (IOCTL(syn,SynDrvrSET_MODULE,&module) < 0) {
	    IErr("SET_MODULE",&module);
	    return arg;
	 }
      }
   }

   module = -1;
   if (IOCTL(syn,SynDrvrGET_MODULE,&module) < 0) {
      IErr("GET_MODULE",NULL);
      return arg;
   }

   for (i=1; i<=modules; i++) {
      if (IOCTL(syn,SynDrvrGET_MODULE_ADDRESS,&moad) < 0) {
	 IErr("GET_MODULE_ADDRESS",NULL);
	 return arg;
      }
      printf("Module: %d VMEAddress: %x JTGAddress: %x Vector: %x Level: %x CopyAddress: %x",
	     i,
	     (int) moad.VMEAddress,
	     (int) moad.JTGAddress,
	     (int) moad.InterruptVector,
	     (int) moad.InterruptLevel,
	     (int) moad.CopyAddress );

      if (i == module) printf("<==");
      printf("\n");
   }

   printf("\nFound: %d Modules installed\n",modules);
   printf("Current module: %d\n",module);

   return arg;
}

/*****************************************************************/

int NextModule(int arg) { /* No Arg */
int modules, module;

   arg++;
   modules = -1;
   if (IOCTL(syn,SynDrvrGET_MODULE_COUNT,&modules) < 0) {
      IErr("GET_MODULE_COUNT",NULL);
      return arg;
   }

   module = -1;
   if (IOCTL(syn,SynDrvrGET_MODULE,&module) < 0) {
      IErr("GET_MODULE",NULL);
      return arg;
   }

   module++;
   if (module > modules) module = 1;

   if (IOCTL(syn,SynDrvrSET_MODULE,&module) < 0) {
      IErr("SET_MODULE",&module);
      return arg;
   }

   printf("Current module: %d\n",module);

   return arg;
}

/*****************************************************************/

int PrevModule(int arg) { /* No Arg */
int modules, module;

   arg++;
   modules = -1;
   if (IOCTL(syn,SynDrvrGET_MODULE_COUNT,&modules) < 0) {
      IErr("GET_MODULE_COUNT",NULL);
      return arg;
   }

   module = -1;
   if (IOCTL(syn,SynDrvrGET_MODULE,&module) < 0) {
      IErr("GET_MODULE",NULL);
      return arg;
   }

   module--;
   if (module < 1) module = modules;

   if (IOCTL(syn,SynDrvrSET_MODULE,&module) < 0) {
      IErr("SET_MODULE",&module);
      return arg;
   }

   printf("Current module: %d\n",module);

   return arg;
}

/*****************************************************************/

int ResetMod(int arg) { /* No Arg */
int module;

   arg++;

   if (IOCTL(syn,SynDrvrRESET,NULL) < 0) {
      IErr("RESET",NULL);
      return arg;
   }

   module = -1;
   if (IOCTL(syn,SynDrvrGET_MODULE,&module) < 0) {
      IErr("GET_MODULE",NULL);
      return arg;
   }

   printf("Reset Module: %d\n",module);

   return arg;
}

/*****************************************************************/

int GetStatusMod(int arg) { /* No Arg */

unsigned long stat;

   arg++;

   stat = -1;
   if (IOCTL(syn,SynDrvrGET_STATUS,&stat) < 0) {
      IErr("GET_STATUS",NULL);
      return arg;
   }

   printf("Status: [0x%04X] %s\n",(int) stat, StatusToString(stat));

   return arg;
}

/*****************************************************************/

int GetClientConnections(int arg) { /* No Arg */

SynDrvrClientConnections cons;
SynDrvrConnection *con;
SynDrvrClientList clist;
int i, j, pid;

   arg++;

   if (IOCTL(syn,SynDrvrGET_CLIENT_LIST,&clist) < 0) {
      IErr("GET_CLIENT_LIST",NULL);
      return arg;
   }

   for (i=0; i<clist.Size; i++) {
      pid = clist.Pid[i];

      bzero((void *) &cons, sizeof(SynDrvrClientConnections));
      cons.Pid = pid;

      if (IOCTL(syn,SynDrvrGET_CLIENT_CONNECTIONS,&cons) < 0) {
	 IErr("GET_CLIENT_CONNECTIONS",NULL);
	 return arg;
      }

      printf("Pid: %d ",pid);

      if (cons.Size) {
	 printf("Mod: ");
	 for (j=0; j<cons.Size; j++) {
	    con = &(cons.Connections[j]);
	    printf("%d\n%s\n\n",(int) con->Module,ConMaskToStr(con->ConMask,0));
	 }
      } else printf("No connections");
      printf("\n");
   }
   return arg;
}

/*****************************************************************/

int WaitInterrupt(int arg) {
ArgVal   *v;
AtomType  at;

SynDrvrConnection con;
SynDrvrReadBuf rbf;
int cc, qflag, qsize, interrupt, cnt;

static int connected = 0;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   interrupt = 0;
   if (at == Numeric) {
      arg++;

      if (v->Number == 0) {
	  printf("%s\n",ConMaskToStr(SynDrvrConMaskMASK,1));
	  return arg;
      }
      interrupt = v->Number;
      con.Module  = 0;
      con.ConMask = interrupt;
      if (IOCTL(syn,SynDrvrCONNECT,&con) < 0) {
	 IErr("CONNECT",&interrupt);
	 return arg;
      }
      connected = 1;
   }

   if (connected == 0) {
      printf("WaitInterrupt: Error: No connections to wait for\n");
      return arg;
   }

   cnt = 0;
   do {
      cc = READ(syn,&rbf,sizeof(SynDrvrReadBuf));
      if (cc <= 0) {
	 printf("Time out or Interrupted call\n");
	 return arg;
      }
      if (cnt++ > 64) {
	 printf("Aborted wait loop after 64 iterations\n");
	 return arg;
      }
   } while ((interrupt != 0) && ((interrupt & rbf.Connection.ConMask) == 0));

   if (IOCTL(syn,SynDrvrGET_QUEUE_FLAG,&qflag) < 0) {
      IErr("GET_QUEUE_FLAG",NULL);
      return arg;
   }
   if (IOCTL(syn,SynDrvrGET_QUEUE_SIZE,&qsize) < 0) {
      IErr("GET_QUEUE_SIZE",NULL);
      return arg;
   }

   printf("Interrupt: Mod[%d] Int[0x%08x = %s] Stat[0x%08x]",
	 (int) rbf.Connection.Module,
	 (int) rbf.Connection.ConMask,
	       ConMaskToStr(rbf.Connection.ConMask,0),
	 (int) rbf.Status);

   if (qflag == 0) {
      if (qsize) printf(" Queued: %d",(int) qsize);
   }
   printf("\n");

   return arg;
}

/*****************************************************************/

int GetSetQue(int arg) { /* Arg=Flag */
ArgVal   *v;
AtomType  at;
long qflag, qsize, qover;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      qflag = v->Number;

      if (IOCTL(syn,SynDrvrSET_QUEUE_FLAG,&qflag) < 0) {
	 IErr("SET_QUEUE_FLAG",(int *) &qflag);
	 return arg;
      }
   }
   qflag = -1;
   if (IOCTL(syn,SynDrvrGET_QUEUE_FLAG,&qflag) < 0) {
      IErr("GET_QUEUE_FLAG",NULL);
      return arg;
   }
   if (IOCTL(syn,SynDrvrGET_QUEUE_SIZE,&qsize) < 0) {
      IErr("GET_QUEUE_SIZE",NULL);
      return arg;
   }
   if (IOCTL(syn,SynDrvrGET_QUEUE_OVERFLOW,&qover) < 0) {
      IErr("GET_QUEUE_FLAG",NULL);
      return arg;
   }

   if (qflag==1) printf("NoQueueFlag: Set,   Queuing is Off\n");
   else        { printf("NoQueueFlag: ReSet, Queuing is On\n");
		 printf("QueueSize: %d\n",    (int) qsize);
		 printf("QueueOverflow: %d\n",(int) qover);
	       }
   return arg;
}

/*****************************************************************/

int ReloadJtag(int arg) {
ArgVal   *v;
AtomType  at;
char fname[128], *cp, c;
int n, earg, cc, yn;
SynDrvrVersion version;

   arg++;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      xsvf_iDebugLevel = v->Number;
   } else xsvf_iDebugLevel = 1;

   for (earg=arg; earg<pcnt; earg++) {
      v = &(vals[earg]);
      if ((v->Type == Close)
      ||  (v->Type == Terminator)) break;
   }

   v = &(vals[arg]);
   at = v->Type;
   if ((v->Type == Close)
   ||  (v->Type == Terminator)) {

      strcpy(fname,"/usr/local/drivers/ctg/Syn.xsvf");

   } else {

      bzero((void *) fname, 128);

      n = 0;
      cp = &(cmdbuf[v->Pos]);
      do {
	 at = atomhash[(int) (*cp)];
	 if ((at != Seperator)
	 &&  (at != Close)
	 &&  (at != Terminator))
	    fname[n++] = *cp;
	 fname[n] = 0;
	 cp++;
      } while ((at != Close) && (at != Terminator));
   }

   printf("VHDL-Compiled BitStream DebugLevel: %d Filename: %s\n",xsvf_iDebugLevel,fname);
   printf("Continue (Y/N):"); yn = getchar(); c = getchar();
   if ((yn != (int) 'y') && (yn != 'Y')) return earg;

   inp = fopen(fname,"r");
   if (inp) {
      if (IOCTL(syn,SynDrvrJTAG_OPEN,NULL)) {
	 IErr("JTAG_OPEN",NULL);
	 printf("ABORTED: Write VHDL bit-stream\n");
	 return earg;
      }

      cc = xsvfExecute(); /* Play the xsvf file */
      if (cc) printf("Jtag: xsvfExecute: ReturnCode: %d Error\n",cc);
      else    printf("Jtag: xsvfExecute: ReturnCode: %d All OK\n",cc);
      fclose(inp);

      sleep(5); /* Wait for hardware to reconfigure */

      if (IOCTL(syn,SynDrvrJTAG_CLOSE,NULL) < 0) {
	 IErr("JTAG_CLOSE",NULL);
	 printf("WARNING: Write VHDL bit-stream contained garbage\n");
      }

      bzero((void *) &version, sizeof(SynDrvrVersion));
      if (IOCTL(syn,SynDrvrGET_VERSION,&version) < 0) {
	 IErr("GET_VERSION",NULL);
      }
      printf("New VHDL bit-stream loaded, Version: [%u] %s\n",
       (int) version.VhdlVersion,
	     TimeToStr(version.VhdlVersion));
   } else {
      perror("fopen");
      printf("Could not open the file: %s for reading\n",fname);
   }
   return earg;
}

/*****************************************************************/

void EditMemory(int address) {

SynDrvrRawIoBlock iob;
char c, *cp, str[128];
int n, radix, nadr, addr;

   printf("EditMemory: [/]: Open, [CR]: Next, [.]: Exit [x]: Hex, [d]: Dec\n\n");

   addr  = address;
   radix = 0;

   while (1) {

      iob.Offset = addr;
      iob.Data   = 0;
      if (IOCTL(syn,SynDrvrRAW_READ,&iob) < 0) {
	 IErr("RAW_READ",NULL);
	 break;
      }

      if (radix)
	 printf("0x%04x: 0x%04x ",(int) addr,(int) iob.Data);
      else
	 printf("%04d: %5d ",(int) addr,(int) iob.Data);

      c = (char) getchar();

      if (c == '/') {
	 bzero((void *) str, 128); n = 0;
	 while ((c != '\n') && (n < 128)) c = str[n++] = (char) getchar();
	 nadr = strtoul(str,&cp,0);
	 if (cp != str) addr = nadr;
      }

      else if (c == 'x')  radix = 1;
      else if (c == 'd')  radix = 0;
      else if (c == '\n') { if (addr++ > 0x1000) addr = 0; }
      else if (c == '.')  { c = getchar(); printf("\n"); break; }

      else {
	 bzero((void *) str, 128); n = 0;
	 str[n++] = c;
	 while ((c != '\n') && (n < 128)) c = str[n++] = (char) getchar();
	 iob.Data = strtoul(str,&cp,0);
	 if (cp != str) {
	    iob.Offset = addr;
	    if (IOCTL(syn,SynDrvrRAW_WRITE,&iob) < 0) {
	       IErr("RAW_WRITE",NULL);
	       break;
	    }
	 }
      }
   }
}

/*****************************************************************/

int MemIo(int arg) { /* Address */
ArgVal   *v;
AtomType  at;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      EditMemory(v->Number);
   } else
      EditMemory(0);

   return arg;
}

/*****************************************************************/

int GetSetEnable(int arg) {         /* Enable/Disable outputs */
ArgVal   *v;
AtomType  at;

unsigned long enable;
unsigned long stat;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;

      enable = v->Number;

      if (IOCTL(syn,SynDrvrENABLE_OUTPUTS,&enable) < 0) {
	 IErr("ENABLE_OUTPUTS",NULL);
	 return arg;
      }
   }


   stat = -1;
   if (IOCTL(syn,SynDrvrGET_STATUS,&stat) < 0) {
      IErr("GET_STATUS",NULL);
      return arg;
   }

   if (stat & SynDrvrStatusOUTPUTS_ENABLED)
      printf("Outputs: Enabled\n");
   else
      printf("Outputs: Disabled\n");

   return arg;
}

/*****************************************************************/

int ReSync(int arg) {               /* Resync PPS to external PPS */

   arg++;
   if (IOCTL(syn,SynDrvrRESYNC_PPS,NULL) < 0) {
      IErr("RESYNC_PPS",NULL);
      return arg;
   }

   printf("PPS Re-Synchronized\n");
   return arg;
}

/*****************************************************************/

int GetSetDelay(int arg) {          /* Get/Set output delay */
ArgVal   *v;
AtomType  at;

long delay, sign;
double dly;

   arg++;

   v = &(vals[arg]);
   at = v->Type;

   sign = 1;
   if (at == Operator) {
      arg++;
      if (v->OId == OprMI) sign = -1;
   }

   v = &(vals[arg]);
   at = v->Type;

   if (at == Numeric) {
      arg++;

      delay = v->Number * sign;

      if (IOCTL(syn,SynDrvrSET_OUTPUT_DELAY,&delay) < 0) {
	 IErr("SET_OUTPUT_DELAY",NULL);
	 return arg;
      }
   }

   if (IOCTL(syn,SynDrvrGET_OUTPUT_DELAY,&delay) < 0) {
      IErr("GET_OUTPUT_DELAY",NULL);
      return arg;
   }
   dly = (delay*TCK40MHZ);
   printf("Output delay: %d PPS Moved by: %fns\n",(int) delay, dly);
   return arg;
}

/*****************************************************************/

int GetSetSyncPeriod(int arg) {          /* Get/Set sync period */
ArgVal   *v;
AtomType  at;

unsigned long period;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;

      period = v->Number;

      if (IOCTL(syn,SynDrvrSET_SYNC_PERIOD,&period) < 0) {
	 IErr("SET_SYNC_PERIOD",NULL);
	 return arg;
      }
   }

   if (IOCTL(syn,SynDrvrGET_SYNC_PERIOD,&period) < 0) {
      IErr("GET_SYNC_PERIOD",NULL);
      return arg;
   }

   printf("Sync Period: %dms\n",(int) period);
   return arg;
}

/*****************************************************************/

int GetPll(int arg) {               /* Get PLL parameters */

SynDrvrPllBuf plb;
double ph, er;

   arg++;

   if (IOCTL(syn,SynDrvrGET_PLL,&plb) < 0) {
      IErr("GET_PLL",NULL);
      return arg;
   }
   ph = plb.PllAsPrdNs * ((double) plb.PllPhase / (double) plb.PllNumAverage);
   er = plb.PllAsPrdNs * ((double) plb.PllError / (double) plb.PllNumAverage);

   printf("PllError      : [0x%08x] %d\n",(int) plb.PllError     ,(int) plb.PllError);
   printf("PllIntegrator : [0x%08x] %d\n",(int) plb.PllIntegrator,(int) plb.PllIntegrator);
   printf("PllDac        : [0x%08x] %d\n",(int) plb.PllDac       ,(int) plb.PllDac);
   printf("PllLastItLen  : [0x%08x] %d\n",(int) plb.PllLastItLen ,(int) plb.PllLastItLen);
   printf("PllPhase      : [0x%08x] %d\n",(int) plb.PllPhase     ,(int) plb.PllPhase);
   printf("PllNumAverage : [0x%08x] %d\n",(int) plb.PllNumAverage,(int) plb.PllNumAverage);
   printf("PllKP         : [0x%08x] %d\n",(int) plb.PllKP        ,(int) plb.PllKP);
   printf("PllKI         : [0x%08x] %d\n",(int) plb.PllKI        ,(int) plb.PllKI);
   printf("PllAsPrdNs    : %f (ns)\n",plb.PllAsPrdNs);

   printf("Error*Period/Naverage  : %fns\n",er);
   printf("Phase*Period/Naverage  : %fns\n",ph);

   return arg;
}

/*****************************************************************/

int SetPllAsPrdNs(int arg) {          /* Set the PLL async oscillator period */
ArgVal   *v;
AtomType  at;

float apn;
SynDrvrPllBuf plb;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      apn = atof(&(cmdbuf[3]));
      if (IOCTL(syn,SynDrvrSET_PLL_AP,&apn) < 0) {
	 IErr("SET_PLL_AP",NULL);
	 return arg;
      }
   }

   if (IOCTL(syn,SynDrvrGET_PLL,&plb) < 0) {
      IErr("GET_PLL",NULL);
      return arg;
   }

   printf("PllAsPrdNs: %fns\n",plb.PllAsPrdNs);
   return arg;
}

/*****************************************************************/

int SetPllIntegrator(int arg) {          /* Set the PLL integrator */
ArgVal   *v;
AtomType  at;

unsigned long intgr;
SynDrvrPllBuf plb;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      intgr = v->Number;
      if (IOCTL(syn,SynDrvrSET_PLL_INTG,&intgr) < 0) {
	 IErr("SET_PLL_INTG",NULL);
	 return arg;
      }
   }

   if (IOCTL(syn,SynDrvrGET_PLL,&plb) < 0) {
      IErr("GET_PLL",NULL);
      return arg;
   }

   printf("PllIntegrator: [0x%08x] %d\n",(int) plb.PllIntegrator,(int) plb.PllIntegrator);
   return arg;
}

/*****************************************************************/

int SetPllPhase(int arg) {          /* Set the PLL phase */
ArgVal   *v;
AtomType  at;

unsigned long phase;
SynDrvrPllBuf plb;
double ph;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      phase = v->Number;
      if (IOCTL(syn,SynDrvrSET_PLL_PHASE,&phase) < 0) {
	 IErr("SET_PLL_PHASE",NULL);
	 return arg;
      }
   }

   if (IOCTL(syn,SynDrvrGET_PLL,&plb) < 0) {
      IErr("GET_PLL",NULL);
      return arg;
   }

   ph = plb.PllAsPrdNs * ((double) plb.PllPhase / (double) plb.PllNumAverage);

   printf("PllPhase: [0x%08x] %d = %fns\n",(int) plb.PllPhase,(int) plb.PllPhase,ph);
   return arg;
}

/*****************************************************************/

int SetPllNumAverage(int arg) {
ArgVal   *v;
AtomType  at;

unsigned long numav;
SynDrvrPllBuf plb;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      numav = v->Number;
      if (IOCTL(syn,SynDrvrSET_PLL_NUM_AVERAGE,&numav) < 0) {
	 IErr("SET_PLL_NUM_AVERAGE",NULL);
	 return arg;
      }
   }

   if (IOCTL(syn,SynDrvrGET_PLL,&plb) < 0) {
      IErr("GET_PLL",NULL);
      return arg;
   }

   printf("PllNumAverage: [0x%08x] %d\n",(int) plb.PllNumAverage,(int) plb.PllNumAverage);
   return arg;
}

/*****************************************************************/

int SetPllKP(int arg) {
ArgVal   *v;
AtomType  at;

unsigned long kp;
SynDrvrPllBuf plb;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      kp = v->Number;
      if (IOCTL(syn,SynDrvrSET_PLL_KP,&kp) < 0) {
	 IErr("SET_PLL_KP",NULL);
	 return arg;
      }
   }

   if (IOCTL(syn,SynDrvrGET_PLL,&plb) < 0) {
      IErr("GET_PLL",NULL);
      return arg;
   }

   printf("PllKP: [0x%08x] %d\n",(int) plb.PllKP,(int) plb.PllKP);
   return arg;
}

/*****************************************************************/

int SetPllKI(int arg) {
ArgVal   *v;
AtomType  at;

unsigned long ki;
SynDrvrPllBuf plb;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      ki = v->Number;
      if (IOCTL(syn,SynDrvrSET_PLL_KI,&ki) < 0) {
	 IErr("SET_PLL_KI",NULL);
	 return arg;
      }
   }

   if (IOCTL(syn,SynDrvrGET_PLL,&plb) < 0) {
      IErr("GET_PLL",NULL);
      return arg;
   }

   printf("PllKI: [0x%08x] %d\n",(int) plb.PllKI,(int) plb.PllKI);
   return arg;
}

/*****************************************************************/

int GetPps(int arg) {               /* Get PPS data */

SynDrvrPpsBuf ppb;
double pe, ph;

   arg++;

   if (IOCTL(syn,SynDrvrGET_PPS,&ppb) < 0) {
      IErr("GET_PPS",NULL);
      return arg;
   }
   pe = ((int) (ppb.PpsPeriod+1))*TCK40MHZ/1000000000;
   ph = ((int)  ppb.PpsPhase)    *TCK40MHZ/1000;

   printf("PpsPeriod: [0x%08X] %d %fs\n",(int) ppb.PpsPeriod,(int) ppb.PpsPeriod,pe);
   printf("PpsPhase : [0x%08X] %d Moved by: %fus\n",(int) ppb.PpsPhase ,(int) ppb.PpsPhase ,ph);

   return arg;
}

/*****************************************************************/

int Plot(int arg) {               /* Plot PLL curves */
ArgVal   *v;
AtomType  at;

unsigned long nav, kp, ki, points, qflag, to, tn;
SynDrvrConnection con;
SynDrvrReadBuf rbf;
SynDrvrPllBuf *pll;
FILE *err, *sum, *dac;
int i, cc;

   arg++;

   nav = 0;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      nav = v->Number;
   }

   kp = 0;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      kp = v->Number;
   }

   ki = 0;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      ki = v->Number;
   }

   points = 0;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      points = v->Number;
   }

   if (points < 2) {
      printf("PLOT: Nav(%d) Kp(%d) Ki(%d) Points(%d) :Invalid\n",(int) nav,(int) kp,(int) ki,(int) points);
      return arg;
   }

   pll = (SynDrvrPllBuf *) malloc(sizeof(SynDrvrPllBuf) * points);
   if (pll == NULL) {
      printf("PLOT: Not enough memory to plot %d points\n",(int) points);
      return arg;
   }

   qflag = 1;
   if (IOCTL(syn,SynDrvrSET_QUEUE_FLAG,&qflag) < 0) {
      IErr("SET_QUEUE_FLAG",(int *) &qflag);
      free(pll);
      return arg;
   }

   con.Module  = 0;
   con.ConMask = SynDrvrConMaskPLL_ITERATION;
   if (IOCTL(syn,SynDrvrCONNECT,&con) < 0) {
      IErr("CONNECT",NULL);
      free(pll);
      return arg;
   }

   if (IOCTL(syn,SynDrvrSET_PLL_NUM_AVERAGE,&nav) < 0) {
      IErr("SET_PLL_NUM_AVERAGE",NULL);
      free(pll);
      return arg;
   }

   if (IOCTL(syn,SynDrvrSET_PLL_KP,&kp) < 0) {
      IErr("SET_PLL_KP",NULL);
      free(pll);
      return arg;
   }

   if (IOCTL(syn,SynDrvrSET_PLL_KI,&ki) < 0) {
      IErr("SET_PLL_KI",NULL);
      free(pll);
      return arg;
   }

   i = 0;
   while (i < points) {
      cc = read(syn,&rbf,sizeof(SynDrvrReadBuf));
      if (cc <= 0) {
	 printf("Time out or Interrupted call\n");
	 free(pll);
	 return arg;
      }

      if (SynDrvrConMaskPLL_ITERATION & rbf.Connection.ConMask) {

	 if (IOCTL(syn,SynDrvrGET_PLL,&pll[i++]) < 0) {
	    IErr("GET_PLL",NULL);
	    free(pll);
	    return arg;
	 }
      }
   }

   printf("PLOT: Measurment done: Writing: %d points to output files\n",(int) points);

   err = fopen("/tmp/err","w");
   sum = fopen("/tmp/sum","w");
   dac = fopen("/tmp/dac","w");
   if (err && sum && dac) {
      to = 0;
      for (i=0; i<points; i++) {
	 tn = pll[i].PllLastItLen + to;
	 to = tn;
	 fprintf(err,"%d %d\n",(int) to, (int) pll[i].PllError);
	 fprintf(sum,"%d %d\n",(int) to, (int) pll[i].PllIntegrator);
	 fprintf(dac,"%d %d\n",(int) to, (int) pll[i].PllDac);
      }
   }
   fclose(err);
   fclose(sum);
   fclose(dac);
   free(pll);

   printf("PLOT: Files written: Calling gnuplot\n");

   system("cd /tmp; set term = xterm; gnuplot pllplot");
   return arg;
}
