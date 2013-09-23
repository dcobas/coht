/* ================================================================================= */
/*                                                                                   */
/* Survey daemon to monitor all hardware and timings on a DSC                        */
/* This program monitors all timings over a super-cycle and stores the results       */
/* It answers calls  to an input socket and sends 3 types of data back to the caller */
/*    1) GetStatus:<msk>  Returns the status of the modules described by <msk>       */
/*    2) GetStatic:<msk>  Returns the settings of all ptims implemented on <msk>     */
/*    3) GetDynamic:<msk> Returns one complete super-cycle of acquisitions for <msk> */
/*                                                                                   */
/* There are some additional commands this task responds to                          */
/*                                                                                   */
/*   4) Strings:<flg>    Sets the strings option on or off according to <flg>        */
/*   5) Help             Sends back some basic help text, good for testing the link  */
/*   6) List             Sends back list on connected and disconnected ptims         */
/*   And more see Help text below for a complete list of commands                    */
/*                                                                                   */
/* A stream of UDP packets not exceeding 1024 bytes each is sent back to the caller, */
/* The last packet returned contains the text "EOF\n"                                */
/*                                                                                   */
/* The main purpose of this program is to provide timing diagnostics to TimDiag, the */
/* Java program presenting the timing map, and to provide a WEB page to observe all  */
/* information about the timing on the DSCs independently of FESA, GM etc, and for   */
/* any timing reciever module type, CTR TG8 etc                                      */
/*                                                                                   */
/* Julian Lewis AB/CO/HT Wed 30th May 2007                                           */
/*                                                                                   */
/* ================================================================================= */

#include <pthread.h>
#include <sched.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/time.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/sem.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <signal.h>
#include <wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/file.h>
#include <a.out.h>
#include <ctype.h>

#include <tgv/tgv.h>
#include <tgm/tgm.h>
#include <TimLib.h>

#define SEND_PORT 2002
#define RECV_PORT 2001

#define SinSIZE (sizeof(struct sockaddr_in))

#define MAX_MODULE_COUNT 16
#define LN 128

extern pid_t getpgid(pid_t pid);

static int domain = AF_INET;

#define SSC_SAMPLES 4                   /* Maximum number of SSCs to disconnect  */
#define TIME_OUT 1200                   /* 10 Basic periods time out for SCY     */

static unsigned int ssc_samples = 0;    /* Number of SSCs since sampling started */
static unsigned int sampling    = 0;    /* Zero when disconnected, else PTIMs    */
static unsigned int use_strings = 1;
static unsigned int debug       = 0;

static char ScTimeStamp[2][LN];
static TimLibTime ScTimeOnzt[2];

#define PTIMS 256
static unsigned long psize;
static unsigned long ptimlist[PTIMS];
static unsigned long ptimodes[PTIMS];
static unsigned long pconnect[PTIMS];

#define OUT_BUF_SZ (100*1024)
static char *out_buf = NULL;
					 /* CPS, PSB, LEI, ADE, SPS, LHC, SCT,  FCT            */
static unsigned long sscs[TgmMACHINES] = {  104, 207, 1,   402, 301, 522, 1104, 0 }; /* SSC-CT */
static unsigned long scys[TgmMACHINES] = {  105, 205, 2,   404, 308, 0,   1105, 0 }; /* SCY-CT */

typedef enum {
   SPORT,
   RPORT,
   DEBUG,
   OPTIONS
 } Options;

static char *options[OPTIONS] = { "-sport",
				  "-rport",
				  "-debug" };
static char *help =
   "   -sport <PORT>       ;Sets the SEND port number\n"
   "   -rport <PORT>       ;Sets the RECEIVE port number\n"
   "   -debug              ;Set Debug printing on\n";

static char *cmd_hlp =
   "Commands:\n"
   "   Help                ;Send this help text\n"
   "   Kill                ;Kills survey task\n"
   "   List                ;List all PTIMs surveyed and not\n"
   "   Strings:<Flag>      ;Strings format on if <Flag> is not zero\n"
   "   Sampling:<Mask>     ;Sample AQNs 16 times on modules by <Mask>\n"
   "   GetStatus           ;Get status\n"
   "   GetStatic:[<Mask>]  ;Get settings by <Mask>\n"
   "   GetDynamic          ;Get dynamic data for Sampling mask\n"
   "   Debug:<Flag>        ;Switch debug printout on if <Flag> is not zero\n"
   "   Version             ;Get TimSurvey version\n"
   "   Ping                ;Used to see if this daemon is alive\n"
   "   Reset               ;Reset status history\n"
   "\n"
   "   If <Module> has value zero, this means all modules\n"
   "\n"
   "Keywords: Lists enclosed in [] are exclusive and start from zero\n"
   "          Lists not enclosed in [] are binary masks\n"
   "   C:  <Millisecond modulo>        0..n\n"
   "   ENB:<Enable flag>               [Dis,Enb]\n"
   "   STR:<Counter start>             [Nor,Ext1/2,Chnd,Self,Remt,Pps,C+S]\n"
   "   MDE:<Counter mode>              [Once,Mult,Brst,M+B]\n"
   "   CLK:<Counter clock>             [1KHz,10MHz,40MHz,Ext1/2,Chnd]\n"
   "   PWD:<Pulse width in 25ns ticks> 1..n\n"
   "   DLY:<Delay value in CLK units>  0..n\n"
   "   OMS:<List:Counter outputs>      [Ctim,Cntr1/2/3/4/5/6/7/8,40Mh,ExCk1,ExCk2]\n"
   "   POL:<Polarity>                  [TTL,BAR]\n"
   "   CTM:<Ctim event trigger>        1..n/Name\n"
   "   PLD:<Payload trigger>           0..n\n"
   "   MCH:<Telegram machine>          [CPS,PSB,LEI,ADE,SPS,LHC,SCT,FCT]\n"
   "   GRN:<Telegram group number>     1..n/Name\n"
   "   GRV:<Telegram group value>      0..n/Name\n"
   "   SSC:<Super cycle time>          Day-dd/Mon/Year hh:mm:ss.ddd <MCH> C:dddd\n"
   "   ERR:<Error string>              Error message text\n"
   "   INF:<Information>               Information string\n"
   "   DEB:<Debug Flag>                [Off,On]\n"
   "   SMP:<Sampling>                  The number of PTIMs currently sampled\n"
   "   CNT:<Remaining SSCs>            The number of SSCs left to sample\n"
   "   MOD:<Module number>             1..16\n"
   "   DEV:<Device type>               [ANY,CTR,CPS,SPS,NET]\n"
   "   TYP:<Module type>               [NONE,CTRI,CTRP,CTRV,CPS_TG8,SPS_TG8]\n"
   "   STS:<List:Status>               [GmtOk/Bad,PllOk/Bad,SlfOK/Bad,BusOk/Bad]\n"
   "   BAD:<Count>                     Number times module status surveyed bad\n"
   "   PTM:<Ptim identifier>           1..n\n"
   "   CHN:<Channel number>            1..8\n"
   "   DIM:<Dimension of PTM>          1..1024\n"
   "   PLN:<Program line number>       1..32\n"
   "   AQN:<Acquisition time value>    n/Date_string\n"
   "   SVY:<PTIM survey status>        NotSurveyed/Surveyed\n"
   "   PKT:<Number:hostname>           1..n Packet number in stream, hostname\n"
   "   EXI:<Host>                      Exit as a result of kill on host\n"
   "   PNG:<Host>                      Ping reply from host, daemon is alive\n"
   "   VER:<Date string>               Version string as a date\n"
   "   VHD:<UTC time>                  VHDL version number\n"
   "   FMW:<UTC time>                  What VHDL version should be installed\n"
   "   LIB:<UTC time>                  TimLib version number\n"
   "   DRV:<UTC time>                  Driver version number\n"
   "   EOF:<Count:hostname>            1..n Packet count in stream, hostname\n"
   "\n"
   "Syntax:\n"
   "   <keyword>:<value>\n"
   "   <keyword>::<list>\n";

#define SCY_MARKER 0xffffffff
#define MAX_EVENTS 1024

static unsigned long scptm[2][MAX_EVENTS]; /* PTIM ID */
static unsigned long scpld[2][MAX_EVENTS]; /* Payload */
static unsigned long scpln[2][MAX_EVENTS]; /* User PLS Line number */
static TimLibTime    sconz[2][MAX_EVENTS]; /* Output time */
static float         sctim[2][MAX_EVENTS]; /* SC Time diff */

static int sci = 0; /* Super-cycle index 0..1 */
static int evi = 0; /* Event index 0..MAX_EVENTS */
static int pec = 0; /* Previous Event index 0..MAX_EVENTS */

#define MAX_ERRORS 50

static int ecnt = 0; /* Error count */

#define HOST_NAME_SZ 32
char host[HOST_NAME_SZ];

/* ====================================== */
/* See if an event should be disconnected */

#define MAX_EVENTS_SC 100

int TooFast(unsigned long eqp) {

int i, cnt, evt;

   if (evi < MAX_EVENTS_SC) return 0;
   for (i=0, cnt=0; i<evi; i++) {
      evt = scptm[sci][i];
      if (eqp == evt) cnt++;
      if (cnt >= MAX_EVENTS_SC) return 1; /* Too fast */
   }
   return 0;
}

/* ============================================ */
/* Subtract times and return a float in seconds */

float TimeDiff(TimLibTime *l, TimLibTime *r) {

float s, n;
unsigned long nl, nr;

   s = (float) (l->Second - r->Second);

   nl = l->Nano;
   nr = r->Nano;

   if (nr > nl) {
      nl += 1000000000;
      s -= 1;
   }
   n = (unsigned long) ((nl - nr)/1000)*1000;

   return s + (n / 1000000000.0);
}

/* ============================================================= */
/* Convert a TimLib time in milliseconds to a string routine.    */
/* Result is a pointer to a static string representing the time. */
/*    the format is: Thu-18/Jan/2001 08:25:14.967                */
/*                   day-dd/mon/yyyy hh:mm:ss.ddd                */

#define NANO_TO_MS 1000000

char *TmToStr(TimLibTime *t, int flg) {

static char tbuf[LN];

char tmp[LN];
char *yr, *ti, *md, *mn, *dy;
int prec;

   bzero((void *) tbuf, LN);
   bzero((void *) tmp,  LN);

   if (((t->Nano/NANO_TO_MS)*NANO_TO_MS) != t->Nano) prec = 1;
   else                                              prec = NANO_TO_MS;

   if (use_strings) {
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
      if (flg == 0) sprintf(tbuf, "%s",ti);
      if (flg == 1) sprintf(tbuf, "%s-%s/%s/%s %s", dy, md, mn, yr, ti);
      if (flg != 2) sprintf(&tbuf[strlen(tbuf)],".%03d ",(int) t->Nano/prec);
   
      if (t->Machine != TgmMACHINE_NONE) {
	 strcat(tbuf,TgmGetMachineName(t->Machine));
	 strcat(tbuf," C:");
	 sprintf(tmp,"%d",(int) t->CTrain);
	 strcat(tbuf,tmp);
      }
   } else {
      sprintf(tbuf,"%u.%03d %d C:%u",
	      (int) t->Second,
	      (int) t->Nano/prec,
	      (int) t->Machine,
	      (int) t->CTrain);
   }
   return (tbuf);
}

/* ================= */
/* Status To String. */

#define STATAE 5

static char *StatusOn[STATAE] = { "GmtOk",
				  "PllOk",
				  "SlfOk",
				  "EnbOk",
				  "BusOk"  };

static char *StatusOf[STATAE] = { "GmtBad",
				  "PllBad",
				  "SlfBad",
				  "EnbBad",
				  "BusBad"};

#define ST_STR_SZ 48
static char StsStr[ST_STR_SZ];

char *StatusToStr(TimLibStatus sts) {
int i;
unsigned long msk;

   bzero((void *) StsStr,ST_STR_SZ);
   if (use_strings) {
      for (i=0; i<STATAE; i++) {
	 msk = 1 << i;
	 if (msk & TimLibStatusBITS) {
	    strcat(StsStr,":");
	    if (msk & sts) strcat(StsStr,StatusOn[i]);
	    else           strcat(StsStr,StatusOf[i]);
	 } else break;
      }
   } else
      sprintf(StsStr,"0x%02X",(int) sts);
   return StsStr;
}

/* ================= */
/* Device to string. */

#define DV_STR_SZ 8
static char DevStr[DV_STR_SZ];

static char *DevNames[TimLibDEVICES] = { "ANY", "CTR", "CPS", "SPS", "NET" };

char *DeviceToStr(TimLibDevice dev) {

   bzero((void *) DevStr,DV_STR_SZ);
   if (use_strings) {
      if ((dev <1) || (dev >= TimLibDEVICES)) dev = 0;
      strcpy(DevStr,DevNames[(int) dev]);
   } else sprintf(DevStr,"%d",(int) dev);
   return DevStr;
}

/* ============================================== */
/* Get PTIM names. They depend on the device type */

#define PTM_NAME_SZ 32

typedef struct {
   unsigned long Eqp;
   char Name[PTM_NAME_SZ];
   unsigned long Flg;
 } PtmNames;

static int ptm_names_size = 0;
static PtmNames ptm_names[PTIMS];

static char ptm_name_txt[PTM_NAME_SZ];

/* ===================================================== */
/* CPS_TG8 procedure to get names. Look at the info file */

char *GetCpsTg8PtmName(unsigned long eqp) {

char *cp, *ep;
int i, j, nf, gf;
FILE *inp;
char txt[LN];   /* One line of text for temp storage */

   if (ptm_names_size == 0) {
      inp = fopen("/tmp/tg8infofile","r");
      if (inp) {
	 gf = 0;
	 while (fgets(txt,LN,inp) != NULL) {
	    nf = 0;
	    for (i=0; i<strlen(txt); i++) {
	       if (txt[i] == '=') {
		  gf = nf = 1;
	       }
	       if ((nf) && (txt[i] != '=') && (txt[i] != ' ')) {
		  cp = &(txt[i]);
		  for (j=0; j<strlen(cp); j++) {
		     if (cp[j] == ' ') {
			cp[j] = 0;
			break;
		     }
		  }
		  strcpy(ptm_names[ptm_names_size].Name,cp);
		  break;
	       }
	    }
	    if (gf) {
	       cp = txt;
	       if (strncmp("   MEMBER{",cp,10) == 0) {
		  cp += 10;
		  ptm_names[ptm_names_size].Eqp = strtoul(cp,&ep,0);
		  ptm_names_size++;
	       }
	    }
	    if (ptm_names_size >= PTIMS) break;
	 }
	 fclose(inp);
      }
   }

   bzero((void *) ptm_name_txt, PTM_NAME_SZ);

   if (eqp) {
      for (i=0; i<ptm_names_size; i++) {
	 if (ptm_names[i].Eqp == eqp) {
	    sprintf(ptm_name_txt,"%04d:%s",(int) eqp,ptm_names[i].Name);
	    return ptm_name_txt;
	 }
      }
   }
   return ptm_name_txt;
}

/* ======================================= */
/* If its a CTR read the ltim.obnames file */

char *GetCtrPtmName(unsigned long eqp) {

char *cp, *ep;
int i;
FILE *inp;

   if (ptm_names_size == 0) {

      bzero((void *) &ptm_names, sizeof(ptm_names));

      inp = fopen("/tmp/ltim.obnames","r");
      if (inp) {
	 while (fgets(ptm_name_txt,LN,inp) != NULL) {
	    cp = ep = ptm_name_txt;
	    ptm_names[ptm_names_size].Eqp = strtoul(cp,&ep,0);
	    if (cp == ep) continue;
	    if ((*ep != 0) && (*ep != '\n')) cp = ep +1;
	    ptm_names[ptm_names_size].Flg = strtoul(cp,&ep,0);
	    if (cp != ep) ep++;

	    cp = index(ep,':');
	    if (cp) { *cp = '\0'; cp++; }

	    if (ep) {
	       for (i=strlen(ep); i>=0; i--) {
		  if (ep[i] == '\n') {
		     ep[i] = 0;
		     break;
		  }
	       }
	       strcpy(ptm_names[ptm_names_size].Name,ep);
	    }

	    if (++ptm_names_size >= PTIMS) break;
	 }
	 fclose(inp);
      }
   }

   bzero((void *) ptm_name_txt, PTM_NAME_SZ);

   if (eqp) {
      for (i=0; i<ptm_names_size; i++) {
	 if (ptm_names[i].Eqp == eqp) {
	    sprintf(ptm_name_txt,"%s",ptm_names[i].Name);
	    return ptm_name_txt;
	 }
      }
   }
   sprintf(ptm_name_txt,"EQP%d",(int) eqp);
   return ptm_name_txt;
}

/* ==================================== */
/* Get Ptm name for a given device type */

char *GetPtmName(TimLibDevice dev, unsigned long eqp) {

   bzero((void *) ptm_name_txt, PTM_NAME_SZ);

   if (use_strings) {
      switch (dev) {
	 case TimLibDevice_CTR:     GetCtrPtmName(eqp);
	 break;
	 case TimLibDevice_TG8_CPS: GetCpsTg8PtmName(eqp);
	 break;
	 default:                   sprintf(ptm_name_txt,"EQP%d",(int) eqp);
      }
   } else sprintf(ptm_name_txt,"%d",(int) eqp);

   return ptm_name_txt;
}

/* ===================== */

#define USR_NAME_SZ 10
static char usr_name_txt[USR_NAME_SZ];

char *GetUsrName(int usr, TgmMachine mch) {

unsigned long gn;
TgmGroupDescriptor desc;
TgmLineNameTable ltab;

   bzero((void *) usr_name_txt, USR_NAME_SZ);
   if (use_strings) {
      if (usr > 0) {
	 gn = TgmGetGroupNumber(mch,"USER");
	 if (gn) {
	    if (TgmGetGroupDescriptor(mch,gn,&desc) == TgmSUCCESS) {
	       if (TgmGetLineNameTable(mch,"USER",&ltab) == TgmSUCCESS) {
		  if (usr <= ltab.Size) {
		     sprintf(usr_name_txt,"%s=0x%x",ltab.Table[usr -1].Name,usr);
		     return usr_name_txt;
		  }
	       }
	    }
	 }
      }
      sprintf(usr_name_txt,"USR=0x%X",usr);
      return usr_name_txt;
   }
   sprintf(usr_name_txt,"%d",usr);
   return usr_name_txt;
}

/* ===================== */
/* Get Program line name */

#define PLN_NAME_SZ 32
static char pln_name_text[PLN_NAME_SZ];

char *GetPlnName(unsigned long pln, TimLibCcv *ccv) {

int i, j;
unsigned long msk;
TgmGroupDescriptor desc;
TgmLineNameTable ltab;

   bzero((void *) pln_name_text, PLN_NAME_SZ);

   if (use_strings) {
      if (ccv->GrNum != 0) {
	 if ((ccv->Machine < 0) || (ccv->Machine >= TgmMACHINES)) ccv->Machine = 0;
	 if (TgmGetGroupDescriptor(ccv->Machine,ccv->GrNum,&desc) == TgmSUCCESS) {
	    if (desc.Type != TgmNUMERIC_VALUE) {
	       if (TgmGetLineNameTable(ccv->Machine,desc.Name,&ltab) == TgmSUCCESS) {
		  if (desc.Type == TgmEXCLUSIVE) {
		     if ((ccv->GrVal > 0) && (ccv->GrVal <= ltab.Size)) j = ccv->GrVal -1;
		     else                                               j = 0;
		     sprintf(pln_name_text,"%s.%s",desc.Name,ltab.Table[j].Name);
		     return pln_name_text;
		  }
		  sprintf(pln_name_text,"%s",desc.Name);
		  for (i=0; i<ltab.Size; i++) {
		     msk = 1 << i;
		     if (msk & ccv->GrVal) {
			strcat(pln_name_text,".");
			strcat(pln_name_text,ltab.Table[i].Name);
		     }
		  }
		  return pln_name_text;
	       }
	    }
	 }
      }
      sprintf(pln_name_text,"GRV%d",(int) pln);
      return pln_name_text;
   }
   sprintf(pln_name_text,"%d",(int) pln);
   return pln_name_text;
}

/* =============================== */
/* Counters output mask to string. */

#define OUT_MASKS  12
static char *OtmNames[OUT_MASKS] = {"Ctim","Cntr1","Cntr2","Cntr3","Cntr4","Cntr5","Cntr6","Cntr7","Cntr8",
				    "40Mh","ExCk1","ExCk2" };

#define OTM_STR_SZ 80
static char OtmStr[OTM_STR_SZ];

char *OtmToStr(TimLibOutput otm) {
int i;
unsigned long msk;

   bzero((void *) OtmStr,OTM_STR_SZ);
   if (use_strings) {
      if (otm == 0) {
	 strcat(OtmStr,"NotSet");
	 return OtmStr;
      }
      for (i=0; i<OUT_MASKS; i++) {
	 msk = 1 << i;
	 if (msk & TimLibOutputBITS) {
	    if (msk & otm) {
	       strcat(OtmStr,":");
	       strcat(OtmStr,OtmNames[i]);
	    }
	 } else break;
      }
   } else sprintf(OtmStr,"0x%X",(int) otm);

   return OtmStr;
}

/* ======================= */
/* Counters CCV to string. */

#define CCV_FIELDS 13
#define POLARATIES 3

static char *CounterStart [TimLibSTARTS]  = {"Nor", "Ext1", "Ext2", "Chnd", "Self", "Remt", "Pps", "C+S"};
static char *CounterMode  [TimLibMODES]   = {"Once", "Mult", "Brst", "M+B"};
static char *CounterClock [TimLibCLOCKS]  = {"1KHz", "10MHz", "40MHz", "Ext1", "Ext2", "Chnd" };
static char *Polarity     [POLARATIES]    = {"TTL","BAR","TTL"};
static char *Enable       [TimLibENABLES] = {"Dis","Enb","Dis","Enb"};

#define CCV_STR_SZ 132
static char CcvStr[CCV_STR_SZ];

char * CcvToStr(TimLibCcvMask ccm, TimLibCcv *ccv, int pln) {

int i, j;
unsigned long msk;
TgmGroupDescriptor desc;
TgmLineNameTable ltab;
char tmp[CCV_STR_SZ];
TgvName tname;

   bzero((void *) CcvStr,CCV_STR_SZ);
   bzero((void *) tmp   ,CCV_STR_SZ);

   sprintf(CcvStr,"PLN:%d ",pln);

   if (use_strings) {
      for (i=0; i<CCV_FIELDS; i++) {
	 msk = 1 << i;
	 if (msk & TimLibCcvMaskBITS) {
	    if (msk & ccm) {
	       switch (msk) {
		  case TimLibCcvMaskENABLE:
		     if (ccv->Enable >= TimLibENABLES) ccv->Enable = 0;
		     sprintf(tmp,"ENB:%s ",Enable[(int) ccv->Enable]);
		  break;

		  case TimLibCcvMaskSTART:
		     if (ccv->Start >= TimLibSTARTS) ccv->Start = 0;
		     sprintf(tmp,"STR:%s ",CounterStart[(int) ccv->Start]);
		  break;

		  case TimLibCcvMaskMODE:
		     if (ccv->Mode >= TimLibMODES) ccv->Mode = 0;
		     sprintf(tmp,"MDE:%s ",CounterMode[(int) ccv->Mode]);
		  break;

		  case TimLibCcvMaskCLOCK:
		     if (ccv->Clock >= TimLibCLOCKS) ccv->Clock = 0;
		     sprintf(tmp,"CLK:%s ",CounterClock[(int) ccv->Clock]);
		  break;

		  case TimLibCcvMaskPWIDTH:
		     sprintf(tmp,"PWD:%d ",(int) ccv->PulsWidth);
		  break;

		  case TimLibCcvMaskDELAY:
		     sprintf(tmp,"DLY:%d ",(int) ccv->Delay);
		  break;

		  case TimLibCcvMaskOMASK:
		     sprintf(tmp,"OMS:%s ",OtmToStr(ccv->OutputMask));
		  break;

		  case TimLibCcvMaskPOLARITY:
		     if (ccv->Polarity >= POLARATIES) ccv->Polarity = 0;
		     sprintf(tmp,"POL:%s ",Polarity[(int) ccv->Polarity]);
		  break;

		  case TimLibCcvMaskCTIM:
		     if (TgvGetNameForMember(ccv->Ctim,&tname)) {
			sprintf(tmp,"CTM:%s ",(char *) tname);
		     } else {
			sprintf(tmp,"CTM:%d ",(int) ccv->Ctim);
		     }
		  break;

		  case TimLibCcvMaskPAYLOAD:
		     if (ccv->Payload) sprintf(tmp,"PLD:0x%X ",(int) ccv->Payload);
		  break;

		  case TimLibCcvMaskMACHINE:
		     if (ccv->GrNum == 0) break;
		     if ((ccv->Machine < 0) || (ccv->Machine >= TgmMACHINES)) ccv->Machine = 0;
		     sprintf(tmp,"MCH:%s ",TgmGetMachineName(ccv->Machine));
		  break;

		  case TimLibCcvMaskGRNUM:
		     if (ccv->GrNum == 0) break;
		     if (TgmGetGroupDescriptor(ccv->Machine,ccv->GrNum,&desc) == TgmSUCCESS) {
			sprintf(tmp,"GRN:%s ",desc.Name);
		     } else {
			sprintf(tmp,"GRN:%d ",(int) ccv->GrNum);
		     }
		  break;

		  case TimLibCcvMaskGRVAL:
		     if (ccv->GrNum == 0) break;
		     if (TgmGetGroupDescriptor(ccv->Machine,ccv->GrNum,&desc) == TgmSUCCESS) {
			if (desc.Type != TgmNUMERIC_VALUE) {
			   if (TgmGetLineNameTable(ccv->Machine,desc.Name,&ltab) == TgmSUCCESS) {
			      if (desc.Type == TgmEXCLUSIVE) {
				 if ((ccv->GrVal > 0) && (ccv->GrVal <= ltab.Size)) {
				    j = ccv->GrVal -1;
				 } else {
				    j = 0;
				 }
				 sprintf(tmp,"GRV=%s ",ltab.Table[j].Name);
				 break;
			      }
			      sprintf(tmp,"GRV:");
			      for (i=0; i<ltab.Size; i++) {
				 msk = 1 << i;
				 if (msk & ccv->GrVal) {
				    strcat(tmp,ltab.Table[i].Name);
				    strcat(tmp,":");
				 }
			      }
			      tmp[strlen(tmp) -1] = ' ';
			      break;
			   }
			}
		     }
		     sprintf(tmp,"GRV:%d ",(int) ccv->GrVal);
		  break;

		  default:
		  break;
	       }
	       if ((strlen(tmp) && (strlen(CcvStr) + strlen(tmp)) < CCV_STR_SZ))
		  strcat(CcvStr,tmp);

	       bzero((void *) tmp,CCV_STR_SZ);
	    }
	 }
      }
      if (strlen(CcvStr)) CcvStr[strlen(CcvStr) -1] = 0;
   } else {
      for (i=0; i<CCV_FIELDS; i++) {
	 msk = 1 << i;
	 if (msk & TimLibCcvMaskBITS) {
	    if (msk & ccm) {
	       switch (msk) {
		  case TimLibCcvMaskENABLE:
		     sprintf(tmp,"ENB:%d ",(int) ccv->Enable);
		  break;

		  case TimLibCcvMaskSTART:
		     sprintf(tmp,"STR:%d ",(int) ccv->Start);
		  break;

		  case TimLibCcvMaskMODE:
		     sprintf(tmp,"MDE:%d ",(int) ccv->Mode);
		  break;

		  case TimLibCcvMaskCLOCK:
		     sprintf(tmp,"CLK:%d ",(int) ccv->Clock);
		  break;

		  case TimLibCcvMaskPWIDTH:
		     sprintf(tmp,"PWD:%d ",(int) ccv->PulsWidth);
		  break;

		  case TimLibCcvMaskDELAY:
		     sprintf(tmp,"DLY:%d ",(int) ccv->Delay);
		  break;

		  case TimLibCcvMaskOMASK:
		     sprintf(tmp,"OMS:%d ",(int) ccv->OutputMask);
		  break;

		  case TimLibCcvMaskPOLARITY:
		     sprintf(tmp,"POL:%d ",(int) ccv->Polarity);
		  break;

		  case TimLibCcvMaskCTIM:
		     sprintf(tmp,"CTM:%d ",(int) ccv->Ctim);
		  break;

		  case TimLibCcvMaskPAYLOAD:
		     if (ccv->Payload) sprintf(tmp,"PLD:0x%X ",(int) ccv->Payload);
		  break;

		  case TimLibCcvMaskMACHINE:
		     if (ccv->GrNum == 0) break;
		     sprintf(tmp,"MCH:%d ",(int) ccv->Machine);
		  break;

		  case TimLibCcvMaskGRNUM:
		     if (ccv->GrNum == 0) break;
		     sprintf(tmp,"GRN:%d ",(int) ccv->GrNum);
		  break;

		  case TimLibCcvMaskGRVAL:
		     if (ccv->GrNum == 0) break;
		     sprintf(tmp,"GRV:%d ",(int) ccv->GrVal);
		  break;

		  default:
		  break;
	       }
	       if ((strlen(tmp) && (strlen(CcvStr) + strlen(tmp)) < CCV_STR_SZ))
		  strcat(CcvStr,tmp);
	       bzero((void *) tmp,CCV_STR_SZ);
	    }
	 }
      }
      if (strlen(CcvStr)) CcvStr[strlen(CcvStr) -1] = 0;
   }
   return CcvStr;
}

/* ==================================== */
/* Open a UDP port for SendSocket data  */

static unsigned short send_port = 0;
static int send_sock = 0;
#define SEND_IP_SZ 16
static char send_ip[SEND_IP_SZ];

int OpenSendPort(unsigned short port) {

struct sockaddr_in sin;
int    s;

   if (!send_sock) {
      s = socket(domain, SOCK_DGRAM, 0);
      if (s < 0) {
	 fprintf(stderr,"TimSurvey:OpenSendPort:Error:Cant open sockets\n");
	 perror("TimSurvey:OpenSendPort:errno");
	 return 0;
      }
      send_sock = s;

      bzero((void *) &sin, SinSIZE);
      sin.sin_family = domain;
      sin.sin_port = htons(port);
      sin.sin_addr.s_addr = htonl(INADDR_ANY);
      if (bind(s,(struct sockaddr *) &sin, SinSIZE) < 0) {
	 fprintf(stderr,"TimSurvey:OpenSendPort:Error:Cant bind port:%d\n",port);
	 perror("TimSurvey:OpenSendPort:errno");
	 close(s);
	 send_sock = 0;
	 return 0;
      }
   }
   return 1;
}

/* ============================= */
/* Send packet to a port via UDP */

int SendToPort(char *ip, unsigned short port, char *pkt, int sze) {

int cc;
struct sockaddr_in sin;

   if (send_sock) {
      bzero((void *) &sin, SinSIZE);
      sin.sin_family = domain;
      sin.sin_port = htons(port);
      sin.sin_addr.s_addr = inet_addr(ip);
      cc = sendto(send_sock,
		  pkt,
		  sze,
		  0,
		  (struct sockaddr *) &sin,
		  SinSIZE);

      if (cc < 0) {
	 fprintf(stderr,
		"TimSurvey:SendToPort:Error:Cant sendto port:%d at:%s\n",
		 port,
		 ip);
	 perror("TimSurvey:SendToPort:errno");
	 return 0;
      }
      usleep(10000); /* Avoid saturating network and client */
   }
   return 1;
}

/* ==================================== */
/* Open a UDP port for RecvSocket data  */

static unsigned short recv_port = 0;
static int recv_sock = 0;

int OpenRecvPort(unsigned short port) {

struct sockaddr_in sin;

int s;

   if (!recv_sock) {
      s = socket(domain, SOCK_DGRAM, 0);
      if (s < 0) {
	 fprintf(stderr,"TimSurvey:OpenRecvPort:Error:Cant open sockets\n");
	 perror("TimSurvey:OpenRecvPort:errno");
	 return 0;
      }
      recv_sock = s;

      bzero((void *) &sin, SinSIZE);
      sin.sin_family = domain;
      sin.sin_port = htons(port);
      sin.sin_addr.s_addr = htonl(INADDR_ANY);
      if (bind(s,(struct sockaddr *) &sin, SinSIZE) < 0) {
	 fprintf(stderr,"TimSurvey:OpenRecvPort:Error:Cant bind port:%d\n",port);
	 perror("TimSurvey:OpenRecvPort:errno");
	 close(recv_sock);
	 recv_sock = 0;
	 return 0;
      }
   }
   return 1;
}

/* ============================ */
/* Receive packets from socket  */

#define PKB_SZ 132
static char rbuf[PKB_SZ];

int RecvFromPort(unsigned short source_port) {

int                cc;
socklen_t          from;
struct sockaddr_in sin;

   if (recv_sock) {
      bzero((void *) &sin, SinSIZE);
      from = sizeof(sin);
      cc = recvfrom(recv_sock,
		    (char *) rbuf,
		    PKB_SZ,
		    0,
		    (struct sockaddr *) &sin,
		    &from);
      if (cc < 0) {
	 fprintf(stderr,
		"TimSurvey:RecvFromPort:Error:Cant recvfrom port:%d\n",
		 source_port);
	 perror("TimSurvey:RecvFromPort");
	 return 0;
      }

      strcpy(send_ip,inet_ntop(domain,&(sin.sin_addr),send_ip,SEND_IP_SZ)); /* Reply address */

      return 1;
   }
   return 0;
}

/* =============================================== */
/* Break up packets into sub packets and send them */

#define PKT_SZ 1024
#define EN_PKT_SZ 16
#define IO_PKT_SZ (PKT_SZ + 48)
static char EnPkt[EN_PKT_SZ];
static char IoPkt[IO_PKT_SZ];

int BreakAndSend(char *pkt, char *cmd) {

int i, from, to, chrs, pkln, cnt;

   pkln = strlen(pkt);

   for (i=0, from=0, chrs=0, cnt=0; i<pkln; i++) {

      if (pkt[i] == '\n') to = i;

      if (chrs >= PKT_SZ) {
	 chrs = to - from + 1; cnt++;

	 bzero((void *) IoPkt, IO_PKT_SZ);
	 sprintf(IoPkt,"PKT:%d:%s:%s\n",cnt,host,cmd);
	 strncat(IoPkt,&(pkt[from]),chrs);
	 SendToPort(send_ip, send_port, IoPkt, strlen(IoPkt));

	 from = to + 1; i = from; chrs = 0;
      } else chrs++;
   }

   if (from < pkln) {
      chrs = pkln - from; cnt++;

      bzero((void *) IoPkt, IO_PKT_SZ);
      sprintf(IoPkt,"PKT:%d:%s:%s\n",cnt,host,cmd);
      strncat(IoPkt,&(pkt[from]),chrs);
      SendToPort(send_ip, send_port, IoPkt, strlen(IoPkt));
   }

   sprintf(EnPkt,"EOF:%d:%s:%s\n",cnt,host,cmd);
   chrs = strlen(EnPkt);
   SendToPort(send_ip, send_port, EnPkt, chrs);

   return 1;
}

/* ========================================== */
/* Catch broken pipe signals when client dies */

static int pok = 1;

void sigpipehand(int signo) {
   pok = 0;
}

/* =================================== */
/* Catch Sig Child to clean up zombies */

void sigchildhand(int signo) {
   while (waitpid(-1,NULL,WNOHANG) >0);
}

/* ========================== */
/* Catch Sig HUP from wreboot */

void sighuphand(int signo) {
   exit(kill(0,SIGHUP));
}

/* ============= */
/* Other signals */

void sigother(int signo) {
   printf("TimSurvey:Caught signal: %d: Shutting down\n",signo);
   sighuphand(SIGHUP);
}

/* ==================================== */
/* Convert module mask to module number */

unsigned long MaskToMod(unsigned long *mmask, int flag) {

unsigned long i, msk;

   for (i=16; i>0; i--) {
      msk = 1 << (i-1);
      if (msk & *mmask) {
	 if (flag) *mmask &= ~msk;
	 return i;
      }
   }
   return 0;
}

/* ==================================== */
/* Convert module number to module mask */

unsigned long ModToMask(unsigned long mod) {

   if ((mod > 0) && (mod <= 16)) {
      return (1 << (mod-1));
   }
   return 0;
}

/* ========================================= */
/* Convert a module count into a module mask */

unsigned long CntToMask(unsigned long cnt) {

static unsigned long mmask;

int i;

   mmask = 0;
   if ((cnt > 0) && (cnt <= 16)) {
      for (i=0; i<cnt; i++) mmask |= 1<<i;
   }
   return mmask;
}

/* ========================================= */
/* Convert a module mask into a module count */

unsigned long MaskToCnt(unsigned long mmask) {

static unsigned long cnt;

int i;

   cnt = 0;
   for (i=0; i<16; i++) {
      if ((1 << i) & mmask) cnt++;
   }
   return cnt;
}

/* ====================================================================== */
/* ComsThread waits for start of the super-cycle signaled by main thread. */
/* There needs to be at least 2 SSCs since sampling was switched on.      */

void WaitSsc() {

int si;

   if (sampling) {
      if (ssc_samples < 2) {
	 while (ssc_samples < 2) usleep(10000);
      } else {
	 si = sci;
	 while (si == sci)       usleep(10000);
      }
   }
   return;
}

/* ============================== */
/* Paranoid buffer append routine */

int AppendBuf(char *data) {

int i;

   for (i=OUT_BUF_SZ; i <OUT_BUF_SZ + 16; i++) {
      if (out_buf[i] != (char) (0xFF & i)) {
	 fprintf(stderr,"TimSurvey:AppendBuf:Fatal:Buffer Over-run error\n");
	 exit(1);
      }
   }

   if (strlen(out_buf) >= OUT_BUF_SZ) {
      fprintf(stderr,"TimSurvey:AppendBuf:Fatal:Buffer corrupted error\n");
      exit(1);
   }

   if ((strlen(data) + strlen(out_buf)) >= OUT_BUF_SZ) return 0;

   strcat(out_buf,data);
   return strlen(out_buf);
}

/* ================================ */
/* Check status and kepp in history */

static char ErStsLog[MAX_MODULE_COUNT][LN];
static long ErStsCnt[MAX_MODULE_COUNT];
static long OkStsFlg[MAX_MODULE_COUNT];

void CheckStatus() {

unsigned long sts, mcn, mod;
TimLibDevice dev;
int i;

   mcn = TimLibGetInstalledModuleCount();
   for (i=0; i<mcn; i++) {
      mod = i+1;
      sts = TimLibGetStatus(mod,&dev);
      if (sts == TimLibStatusBITS) {
	 OkStsFlg[i]++;
	 continue;
      }

      if (OkStsFlg[i] && (strlen(ScTimeStamp[sci]))) {
	 OkStsFlg[i] = 0;
	 ErStsCnt[i]++;
	 sprintf(ErStsLog[i],
		 "BAD:%d MOD:%d STS:%s SSC:%s\n",
		 (int) ErStsCnt[i],
		 (int) mod,
		 StatusToStr(sts),
		 ScTimeStamp[sci]);
      }
   }
}

/* ================================================ */
/* Get the versions of VHDL/Firmware on all modules */

static char *mtyp[TimLibModuleTYPES] = {"NONE","CTRI","CTRP","CTRV",
					"CPS_TG8", "SPS_TG8" };

char *BuildVersion() {

TimLibModuleVersion tver;
TimLibTime t;
char tmp[LN];
int i;

#ifndef COMPILE_TIME
#define COMPILE_TIME 0
#endif

   bzero((void *) out_buf, OUT_BUF_SZ);

   bzero((void *) &t, sizeof(TimLibTime));
   t.Machine = TgmMACHINE_NONE;
   t.Second = COMPILE_TIME;

   sprintf(out_buf,"LIB:%d VER:%s\n",(int) t.Second, TmToStr(&t,1));

   if (TimLibGetModuleVersion(&tver) == TimLibErrorSUCCESS) {

      t.Second = tver.DrvVer;
      sprintf(tmp,"DRV:%d VER:%s\n",(int) t.Second, TmToStr(&t,1));
      strcat(out_buf,tmp);
      t.Second = tver.CorVer;
      sprintf(tmp,"TYP:%s FMW:%d VER:%s\n",
	      mtyp[(int) tver.ModTyp],
	      (int) t.Second,
	      TmToStr(&t,1));
      strcat(out_buf,tmp);

      for (i=0; i<TimLibMODULES; i++) {
	 if (tver.ModVer[i]) {
	    t.Second  = tver.ModVer[i];
	    if (tver.CorVer == t.Second) {
	       sprintf(tmp,"MOD:%d VHD:%d\n",
		       i+1,
		       (int) t.Second);
	    } else {
	       sprintf(tmp,"MOD:%d VHD:%d VER:%s FMW:%d ERR:Bad VHD not equal FMW\n",
		       i+1,
		       (int) t.Second,
		       TmToStr(&t,1),
		       (int) tver.CorVer);
	    }
	    strcat(out_buf,tmp);
	 }
      }
   }
   return out_buf;
}

/* ============================= */
/* Build the status data strings */

char *BuildStatus(unsigned long mmask) {

char tmp[LN];
int i, si;
unsigned long sts, mcn, mod;
TimLibDevice dev;

   bzero((void *) out_buf, OUT_BUF_SZ);
   if (sci) si = 0; else si = 1;
   sprintf(out_buf,"SSC:%s\n",ScTimeStamp[si]);

   mcn = TimLibGetInstalledModuleCount();
   mmask &= CntToMask(mcn);

   while ((mod = MaskToMod(&mmask,1))) {
      sts = TimLibGetStatus(mod,&dev);
      sprintf(tmp,
	      "MOD:%d DEV:%s STS:%s\n",
	      (int) mod,
	      DeviceToStr(dev),
	      StatusToStr(sts));
      if (!AppendBuf(tmp)) {
	 sprintf(out_buf,"ERR:(out_buf string too small)\n");
	 fprintf(stderr,"TimSurvey:BuildStatus:Error:(out_buf string too small)");
	 return out_buf;
      }

      if (mod) {
	 i = mod -1;
	 if (ErStsCnt[i]) {
	    if (!AppendBuf(ErStsLog[i])) {
	       sprintf(out_buf,"ERR:(out_buf string too small)\n");
	       fprintf(stderr,"TimSurvey:BuildStatus:Error:(out_buf string too small)");
	       return out_buf;
	    }
	 }
      }
   }
   sprintf(tmp,
	   "SMP:%d CNT:%d DEB:%d\n",
	   sampling,
	   (SSC_SAMPLES - ssc_samples),
	   debug);
   if (!AppendBuf(tmp)) {
      sprintf(out_buf,"ERR:(out_buf string too small)\n");
      fprintf(stderr,"TimSurvey:BuildStatus:Error:(out_buf string too small)");
   }

   return out_buf;
}

/* ============================================ */
/* Build the list of all ptims surveyed and not */

#define NAMES_PER_LINE 6

char *BuildList() {

char tmp[LN];
int i, si;
TimLibDevice dev;

   bzero((void *) out_buf, OUT_BUF_SZ);
   if (sci) si = 0; else si = 1;
   sprintf(out_buf,"SSC:%s\n",ScTimeStamp[si]);

   TimLibGetStatus(1,&dev);

   if (use_strings) sprintf(tmp,"SVY:Surveyed\n");
   else             sprintf(tmp,"SVY:1\n");
   strcat(out_buf,tmp);
   for (i=0; i<psize; i++) {
      if ((sampling) && (ptimodes[i] == 0) && pconnect[i]) {
	 sprintf(tmp,":%s",GetPtmName(dev,ptimlist[i]));
	 if (((i+1)%NAMES_PER_LINE)==0) strcat(tmp,"\n");
	 if (!AppendBuf(tmp)) {
	    sprintf(out_buf,"ERR:(out_buf string too small)\n");
	    fprintf(stderr,"TimSurvey:BuildList:Error:(out_buf string too small)");
	    return out_buf;
	 }
      }
   }
   if (use_strings) sprintf(tmp,"\nSVY:NotSurveyed\n");
   else             sprintf(tmp,"\nSVY:0\n");
   strcat(out_buf,tmp);
   for (i=0; i<psize; i++) {
      if ((sampling == 0) || (ptimodes[i]) || (pconnect[i] == 0)) {
	 sprintf(tmp,":%s",GetPtmName(dev,ptimlist[i]));
	 if (((i+1)%NAMES_PER_LINE)==0) strcat(tmp,"\n");
	 if (!AppendBuf(tmp)) {
	    sprintf(out_buf,"ERR:(out_buf string too small)\n");
	    fprintf(stderr,"TimSurvey:BuildList:Error:(out_buf string too small)");
	    return out_buf;
	 }
      }
   }
   AppendBuf("\n");
   return out_buf;
}

/* =================== */
/* Short CCV to String */

char *ShCcvToStr(TimLibCcv *ccv) {

static char res[LN];
TgvName tname;

   if (ccv->Start >= TimLibSTARTS) ccv->Start = 0;
   if (ccv->Clock >= TimLibCLOCKS) ccv->Clock = 0;

   if (TgvGetNameForMember(ccv->Ctim,&tname) == NULL)
      sprintf((char *) tname,"%d",(int) ccv->Ctim);

   if (use_strings) {
      sprintf(res,
	      "CLK:%s STR:%s DLY:%d CTM:%s",
	      CounterClock[(int) ccv->Clock],
	      CounterStart[(int) ccv->Start],
	      (int) ccv->Delay,
	      (char *) tname);
   } else {
      sprintf(res,
	      "CLK:%d STR:%d DLY:%d CTM:%d",
	      (int) ccv->Clock,
	      (int) ccv->Start,
	      (int) ccv->Delay,
	      (int) ccv->Ctim);
   }
   return res;
}

/* ============================= */
/* Build the static data strings */

#define MAX_MODULES_IN_ONE_GO 3

char *BuildStatic(unsigned long mmask) {

char tmp[CCV_STR_SZ];
int i, j, si;
unsigned long mcn, ptm, pmd, chn, dim, pln, grn, grv;
TimLibCcvMask ccm;
TimLibCcv ccv;
TimLibError err;
TimLibDevice dev;

   bzero((void *) out_buf, OUT_BUF_SZ);
   if (sci) si = 0; else si = 1;
   sprintf(out_buf,"SSC:%s\n",ScTimeStamp[si]);

   mcn = TimLibGetInstalledModuleCount();
   mmask &= CntToMask(mcn);

   if (MaskToCnt(mmask) > MAX_MODULES_IN_ONE_GO) {
      sprintf(out_buf,"MOD:%d ERR:(Too many modules installed)\n",(int) mcn);
      return out_buf;
   }

   err = TimLibGetAllPtimObjects(ptimlist, &psize, PTIMS);
   if (err) {
      fprintf(stderr,"TimSurvey:BuildStatic:Error:(Can't get PTIM list)\n");
      fprintf(stderr,"TimSurvey:BuildStatic:Error:%s\n",TimLibErrorToString(err));

      sprintf(tmp,"ERR:(Can't get PTIM object list)\n");
      if (!AppendBuf(tmp)) strcpy(out_buf,tmp);
      return out_buf;
   }

   for (i=0; i<psize; i++) {

      ptm = ptimlist[i];
      if (ptm) {

	 err = TimLibGetPtimObject(ptm,&pmd,&chn,&dim);
	 if (err) {
	    fprintf(stderr,"TimSurvey:BuildStatic:Error:(Can't get PTIM object:%d)\n",(int) ptm);
	    fprintf(stderr,"TimSurvey:BuildStatic:Error:%s\n",TimLibErrorToString(err));

	    sprintf(tmp,"ERR:(Can't get PTIM object:%d)\n",(int) ptm);
	    if (!AppendBuf(tmp)) strcpy(out_buf,tmp);
	    return out_buf;
	 }
	 if (ModToMask(pmd) & mmask) {
	    err = TimLibGetStatus(pmd,&dev);
	    sprintf(tmp,
		    "\nPTM:%s MOD:%d CHN:%d DIM:%d\n",
		    GetPtmName(dev,ptm),
		    (int) pmd,
		    (int) chn,
		    (int) dim);
	    if (!AppendBuf(tmp)) {
	       strcpy(out_buf,tmp);
	       return out_buf;
	    }

	    for (j=0; j<dim; j++) {
	       pln = j+1; grn = 0; grv = 0;
	       err = TimLibGet(ptm,pln,grn,grv,&ccm,&ccv);
	       if (err) {
		  fprintf(stderr,"TimSurvey:BuildStatic:Error:(Can't get PTIM CCV:%d.%d)\n",
			  (int) ptm, (int) pln);
		  fprintf(stderr,"TimSurvey:Main:Static:Error:%s\n",TimLibErrorToString(err));

		  sprintf(tmp,"ERR:(Can't get PTIM CCV:%d.%d)\n",(int) ptm, (int) pln);
		  if (!AppendBuf(tmp)) strcpy(out_buf,tmp);
		  return out_buf;
	       }
	       sprintf(tmp,"%s\n",CcvToStr(ccm,&ccv,pln));
	       if (!AppendBuf(tmp)) {
		  fprintf(stderr,"TimSurvey:BuildStatic:Error:(out_buf string too small)");
		  sprintf(out_buf,"ERR:(out_buf string too small)\n");
		  return out_buf;
	       }
	    }
	 }
      }
   }
   return out_buf;
}

/* ============================== */
/* Build the dynamic data strings */

char *BuildDynamic(unsigned long mmask) {

char tmp[LN];
int i, si;
unsigned long mcn, ptm, sts, pmd, chn, dim, pln, grn, grv;
TimLibError err;
TimLibDevice dev;
TimLibCcvMask ccm;
TimLibCcv ccv;

   WaitSsc();

   bzero((void *) out_buf, OUT_BUF_SZ);
   if (sci) si = 0; else si = 1;
   sprintf(out_buf,"0.000 SSC:%s:%s\n",host,ScTimeStamp[si]);

   mcn = TimLibGetInstalledModuleCount();
   mmask &= CntToMask(mcn);

   for (i=0; i<pec; i++) {
      ptm = scptm[si][i];
      if (ptm) {
	 if (ptm == SCY_MARKER) {
	    sprintf(tmp,
		    "%2.3f USR:%s SCY:%s:%s\n",
		    sctim[si][i],
		    GetUsrName(scpld[si][i],
			       sconz[si][i].Machine),
		    host,
		    TmToStr(&(sconz[si][i]),0));
	    if (!AppendBuf(tmp)) {
	       fprintf(stderr,"TimSurvey:BuildDynamic:Error:(out_buf string too small)");
	       sprintf(out_buf,"ERR:(out_buf string too small)\n");
	       return out_buf;
	    }
	    continue;
	 }
	 err = TimLibGetPtimObject(ptm,&pmd,&chn,&dim);
	 if (err) {
	    fprintf(stderr,"TimSurvey:BuildDynamic:Error:(Can't get PTIM object:%d)\n",(int) ptm);
	    fprintf(stderr,"TimSurvey:BuildDynamic:Error:%s\n",TimLibErrorToString(err));

	    sprintf(tmp,"ERR:(Can't get PTIM object:%d)\n",(int) ptm);
	    if (!AppendBuf(tmp)) strcpy(out_buf,tmp);
	    return out_buf;
	 }
	 if (ModToMask(pmd) & mmask) {
	    sts = TimLibGetStatus(pmd,&dev);
	    pln = scpln[si][i]; grn = 0; grv = 0;
	    err = TimLibGet(ptm,pln,grn,grv,&ccm,&ccv);
	    if (dim>1) {
	       if (scpld[si][i]) {
		  sprintf(tmp,
			  "%2.3f PTM:%s PLN:%s PLD:0x%X AQN:%s MOD:%d CHN:%d %s\n",
			  sctim[si][i],
			  GetPtmName(dev,ptm),
			  GetPlnName(pln,&ccv),
			  (int) scpld[si][i],
			  TmToStr(&(sconz[si][i]),2),
			  (int) pmd,
			  (int) chn,
			  ShCcvToStr(&ccv));
	       } else {
		  sprintf(tmp,
			  "%2.3f PTM:%s PLN:%s AQN:%s MOD:%d CHN:%d %s\n",
			  sctim[si][i],
			  GetPtmName(dev,ptm),
			  GetPlnName(pln,&ccv),
			  TmToStr(&(sconz[si][i]),2),
			  (int) pmd,
			  (int) chn,
			  ShCcvToStr(&ccv));
	       }
	    } else {
	       if (scpld[si][i]) {
		  sprintf(tmp,
			  "%2.3f PTM:%s PLD:0x%X AQN:%s MOD:%d CHN:%d %s\n",
			  sctim[si][i],
			  GetPtmName(dev,ptm),
			  (int) scpld[si][i],
			  TmToStr(&(sconz[si][i]),2),
			  (int) pmd,
			  (int) chn,
			  ShCcvToStr(&ccv));
	       } else {
		  sprintf(tmp,
			  "%2.3f PTM:%s AQN:%s MOD:%d CHN:%d %s\n",
			  sctim[si][i],
			  GetPtmName(dev,ptm),
			  TmToStr(&(sconz[si][i]),2),
			  (int) pmd,
			  (int) chn,
			  ShCcvToStr(&ccv));
	       }
	    }
	    if (!AppendBuf(tmp)) {
	       fprintf(stderr,"TimSurvey:BuildDynamic:Error:(out_buf string too small)");
	       sprintf(out_buf,"ERR:(out_buf string too small)\n");
	       return out_buf;
	    }
	 }
      } else break;
   }
   if (sampling == 0) AppendBuf("SMP:0 (Sampling is OFF, no PTIM results)\n");

   return out_buf;
}

/* ===================== */
/* DisConnect from PTIMS */

int DisconnectPtims() {

int i, cnt;

unsigned long ptm;
unsigned long dim;
unsigned long mod;
unsigned long chn;
TimLibError   err;

   cnt = 0;
   if (sampling != 0) {
      for (i=0; i<psize; i++) {
	 ptm = ptimlist[i];
	 if (ptm && (ptimodes[i] == 0) && pconnect[i]) {
	    pconnect[i] = 0;
	    err = TimLibGetPtimObject(ptm,&mod,&chn,&dim);
	    if (err) continue;
	    err = TimLibDisConnect(TimLibClassPTIM,ptm,mod);
	    if (err) continue;
	    cnt++;
	 }
      }

      bzero((void *) scptm[0], MAX_EVENTS * sizeof(unsigned long));
      bzero((void *) scptm[1], MAX_EVENTS * sizeof(unsigned long));
      sampling = 0;
   }
   return cnt;
}

/* ========================================== */
/* Connect to the PTIM timings for monitoring */

int ConnectPtims(unsigned long mmask) {

int i, j, cnt;
TimLibDevice  dev;
TimLibStatus  sts;
unsigned long mcn;
unsigned long ptm;
unsigned long dim;
unsigned long pmd;
unsigned long chn;
unsigned long pln;
unsigned long grn;
unsigned long grv;
TimLibCcvMask ccm;
TimLibCcv     ccv;
TimLibError   err;

   mcn = TimLibGetInstalledModuleCount();
   mmask &= CntToMask(mcn);

   cnt = 0;
   if (sampling == 0) {
      for (i=0; i<psize; i++) {

	 ptm = ptimlist[i];
	 if (ptm) {

	    err = TimLibGetPtimObject(ptm,&pmd,&chn,&dim);
	    if (err) {
	       fprintf(stderr,"TimSurvey:ConnectPtims:(Can't get PTIM object:%d)\n",(int) ptm);
	       fprintf(stderr,"TimSurvey:ConnectPtims:(%s)\n",TimLibErrorToString(err));
	       return 0;
	    }
	    sts = TimLibGetStatus(pmd,&dev);
	    if (debug) {
	       printf("TimSurvey:ConnectPtims:Debug:PTM:%s MOD:%02d CHN:%d DEV:%s DIM:%d\n",
		      GetPtmName(dev,ptm),(int) pmd,(int) chn,DeviceToStr(dev),(int) dim);
	    }

	    for (j=0; j<dim; j++) {
	       pln = j+1; grn = 0; grv = 0;
	       err = TimLibGet(ptm,pln,grn,grv,&ccm,&ccv);
	       if (err) {
		  fprintf(stderr,"TimSurvey:ConnectPtims:(Can't get PTIM CCV:%d.%d)\n",
			  (int) ptm, (int) pln);
		  fprintf(stderr,"TimSurvey:ConnectPtims:(%s)\n",TimLibErrorToString(err));
		  return 0;
	       }
	       if (ccv.Mode != TimLibModeNORMAL) ptimodes[i]++;
	    }

	    if (ptimodes[i]) {
	       if (debug) {
		  printf("TimSurvey:ConnectPtims:Debug:PTM:%s (Skipped mode not NORMAL)\n",
			 GetPtmName(dev,ptm));
	       }
	       continue;
	    }

	    if (mmask & ModToMask(pmd)) {
	       err = TimLibConnect(TimLibClassPTIM,ptm,pmd);
	       if (err) {
		  fprintf(stderr,"TimSurvey:ConnectPtims:(Can't connect PTIM:%d)\n",(int) ptm);
		  fprintf(stderr,"TimSurvey:ConnectPtims:(%s)\n",TimLibErrorToString(err));
		  return 0;
	       }
	       cnt++;
	       pconnect[i] = 1;
	    }
	 }
      }
      sampling = cnt;
      ssc_samples = 0;
   }
   return cnt;
}

/* ==================== */
/* Comunications thread */

static void ComsThread(void) {

char *cp, *ep, *dp;
unsigned long mmask, tmask;
char cmd_err[LN];
int i;

   mmask = 0;

   if (OpenSendPort(send_port) == 0) {
      fprintf(stderr,"TimSurvey:ComsThread:Error:(Can't Open Send port:%d)\n",(int) send_port);
      exit(127);
   }

   if (OpenRecvPort(recv_port) == 0) {
      fprintf(stderr,"TimSurvey:ComsThread:Error:(Can't Open receive port:%d)\n",(int) recv_port);
      exit(127);
   }

   while (RecvFromPort(recv_port)) {

      if (debug) printf("TimSurvey:ComsThread:Debug:ReplyIp:%s\n",send_ip);

      dp = "Ping";
      if (strncmp(dp,rbuf,strlen(dp)) == 0) {
	 BreakAndSend("",dp);
	 if (debug) printf("TimSurvey:ComsThread:Debug:Command:(%s)\n",rbuf);
	 bzero((void *) rbuf, PKB_SZ);
	 continue;
      }

      dp = "Kill";
      if (strncmp(dp,rbuf,strlen(dp)) == 0) {
	 sprintf(cmd_err,"EXI:%s\n",host);
	 BreakAndSend(cmd_err,dp);
	 if (debug) printf("TimSurvey:ComsThread:Debug:Command:(%s)\n",rbuf);
	 bzero((void *) rbuf, PKB_SZ);
	 exit(0);
      }

      dp = "Help";
      if (strncmp(dp,rbuf,strlen(dp)) == 0) {
	 BreakAndSend(cmd_hlp,dp);
	 if (debug) printf("TimSurvey:ComsThread:Debug:Command:(%s)\n",rbuf);
	 bzero((void *) rbuf, PKB_SZ);
	 continue;
      }

      dp = "List";
      if (strncmp(dp,rbuf,strlen(dp)) == 0) {
	 BuildList();
	 BreakAndSend(out_buf,dp);
	 if (debug) printf("TimSurvey:ComsThread:Debug:Command:(%s)\n",rbuf);
	 bzero((void *) rbuf, PKB_SZ);
	 continue;
      }

      dp = "Version";
      if (strncmp(dp,rbuf,strlen(dp)) == 0) {
	 BuildVersion();
	 BreakAndSend(out_buf,dp);
	 if (debug) printf("TimSurvey:ComsThread:Debug:Command:(%s)\n",rbuf);
	 bzero((void *) rbuf, PKB_SZ);
	 continue;
      }

      dp = "Debug:";
      if (strncmp(dp,rbuf,strlen(dp)) == 0) {
	 cp = &rbuf[strlen(dp)];
	 debug = strtoul(cp,&ep,0);
	 sprintf(cmd_err,"DEB:%d\n",debug);
	 BreakAndSend(cmd_err,dp);
	 if (debug) printf("TimSurvey:ComsThread:Debug:Command:(%s)\n",rbuf);
	 bzero((void *) rbuf, PKB_SZ);
	 continue;
      }

      dp = "Strings:";
      if (strncmp(dp,rbuf,strlen(dp)) == 0) {
	 cp = &rbuf[strlen(dp)];
	 use_strings = strtoul(cp,&ep,0);
	 sprintf(cmd_err,"PSR:%d\n",use_strings);
	 BreakAndSend(cmd_err,dp);
	 if (debug) printf("TimSurvey:ComsThread:Debug:Command:(%s)\n",rbuf);
	 bzero((void *) rbuf, PKB_SZ);
	 continue;
      }

      dp = "Sampling:";
      if (strncmp(dp,rbuf,strlen(dp)) == 0) {
	 cp = &rbuf[strlen(dp)];
	 DisconnectPtims();
	 sampling = 0;
	 ssc_samples = 0;
	 mmask = strtoul(cp,&ep,0);
	 if (mmask) ConnectPtims(mmask);
	 sprintf(cmd_err,"SMP:%d\n",sampling);
	 BreakAndSend(cmd_err,dp);
	 if (debug) printf("TimSurvey:Main:Debug:Command:(%s)\n",rbuf);
	 bzero((void *) rbuf, PKB_SZ);
	 continue;
      }

      dp = "Reset";
      if (strncmp(dp,rbuf,strlen(dp)) == 0) {
	 for (i=0; i<MAX_MODULE_COUNT; i++) {
	    ErStsCnt[i] = 0;
	    OkStsFlg[i] = 1;
	 }
	 BreakAndSend("",dp);
	 if (debug) printf("TimSurvey:Main:Debug:Command:(%s)\n",rbuf);
	 bzero((void *) rbuf, PKB_SZ);
	 continue;
      }

      dp = "GetStatus";
      if (strncmp(dp,rbuf,strlen(dp)) == 0) {
	 cp = &rbuf[strlen(dp)];
	 tmask = 0xFFFF;
	 BuildStatus(tmask);
	 BreakAndSend(out_buf,dp);
	 if (debug) printf("TimSurvey:Main:Debug:Command:(%s)\n",rbuf);
	 bzero((void *) rbuf, PKB_SZ);
	 continue;
      }

      dp = "GetStatic:";
      if (strncmp(dp,rbuf,strlen(dp)) == 0) {
	 cp = &rbuf[strlen(dp)];
	 tmask = strtoul(cp,&ep,0); if (tmask == 0) tmask = mmask;
	 BuildStatic(tmask);
	 BreakAndSend(out_buf,dp);
	 if (debug) printf("TimSurvey:Main:Debug:Command:(%s)\n",rbuf);
	 bzero((void *) rbuf, PKB_SZ);
	 continue;
      }

      dp = "GetDynamic";
      if (strncmp(dp,rbuf,strlen(dp)) == 0) {
	 BuildDynamic(mmask);
	 BreakAndSend(out_buf,dp);
	 if (debug) printf("TimSurvey:Main:Debug:Command:(%s)\n",rbuf);
	 bzero((void *) rbuf, PKB_SZ);
	 continue;
      }

      sprintf(cmd_err,"ERR:BadString:(%s)",rbuf);
      BreakAndSend(cmd_err,"");
      if (debug) printf("TimSurvey:Main:Debug:%s\n",cmd_err);
      bzero((void *) rbuf, PKB_SZ);
   }
   fprintf(stderr,"TimSurvey:ComsThread:Error:(Error RecvPort port:%d)\n",(int) recv_port);
   perror("TimSurvey:ComsThread");
   exit(127);
}

/* ========================================================== */
/* Ensure TimSurvey runs only once, by taking SysIV semaphore */

int SetRunning(char *name) {

int key = TgmGetKey(name);
int sd;
struct sembuf sop;
union semun {
   int val;
   struct semid_ds *buff;
   unsigned short *array;
} arg;

  sd  = semget(key,1,0666|IPC_CREAT);
  if (sd == -1) {
  fail:
    return -1; /* can not start */
  }

/* Unblock semaphore */

  bzero((char *) &arg, sizeof(arg));
  arg.val = 1;
  if (semctl(sd,0,SETVAL,arg) == -1) goto fail;

/* Block it */

  bzero((char *) &sop, sizeof(sop));
  sop.sem_num = 0;
  sop.sem_op  = -1;
  sop.sem_flg = SEM_UNDO; /* Undo block if task dies */
  if (semop(sd,&sop,1) == -1) goto fail;
  return 0;
}

/* ================================================= */
/* Check semaphore to see if task is running already */

int CheckRunning(char *name) {

int key = TgmGetKey(name);
int sd;
int cnt;
struct semid_ds desc;
union semun {
   int val;
   struct semid_ds *buff;
   unsigned short *array;
} arg;

  sd  = semget(key,1,0);
  if (sd == -1) return 0; /* Task is not running */

  bzero((char *) &desc, sizeof(desc));
  bzero((char *) &arg, sizeof(arg));
  arg.buff = &desc;

  if (semctl(sd,0,IPC_STAT,arg) == -1 || desc.sem_otime == 0) return 0;
  cnt = semctl(sd,0,GETVAL,arg);
  return (cnt == 0); /* cnt is 0 only when the daemon is running */
}

/* ================================================================================ */
/* Main                                                                             */

int main(int argc,char *argv[]) {

int i;
char *cp, *ep;
pid_t pid;
pthread_t tid;

struct sigaction action, old;

TimLibDevice  dev;
TimLibStatus  sts;
TimLibError   err;
int           mcn;
unsigned long cid;
unsigned long grn;

TimLibClass    icls;
unsigned long  eqp;
unsigned long  pln;
TimLibHardware hsrc;
TimLibTime     onzt;
TimLibTime     trgt;
TimLibTime     strt;
unsigned long  ctm;
unsigned long  ssc;
unsigned long  scy;
unsigned long  pld;
unsigned long  mod;
unsigned long  mis;
unsigned long  qsz;
TgmMachine     mch;

   if (CheckRunning("TimSurvey") == 0) SetRunning("TimSurvey");
   else                                exit(0);

   out_buf = (char *) malloc(OUT_BUF_SZ + 16);
   if (out_buf == NULL) {
      fprintf(stderr,"TimSurvey:Main:(Can't allocate buffers)\n");
      fprintf(stderr,"TimSurvey:Main:Fatal:(Not enough memory)\n");
      exit(1);
   }
   for (i=OUT_BUF_SZ; i <OUT_BUF_SZ + 16; i++) out_buf[i] = (char) (0xFF & i);

   TimLibClient = 1; /* Unblock special TimLib code */

   /* Default arg values */

   debug       = 0;
   use_strings = 1;

   bzero((void *) ScTimeStamp,sizeof(ScTimeStamp));
   bzero((void *) ScTimeOnzt ,sizeof(ScTimeOnzt));

   /* Process command line arguments */

   send_port = SEND_PORT;
   recv_port = RECV_PORT;

   for (i=1; i<argc; i++) {

      cp = NULL;

      if (strncmp(argv[i],options[SPORT],strlen(options[SPORT])) == 0) {
	 i++;
	 cp = argv[i];
	 if (cp) send_port = (unsigned short) strtoul(cp,&ep,0);
	 continue;
      }

      if (strncmp(argv[i],options[RPORT],strlen(options[RPORT])) == 0) {
	 i++;
	 cp = argv[i];
	 if (cp) recv_port = (unsigned short) strtoul(cp,&ep,0);
	 continue;
      }

      if (strncmp(argv[i],options[DEBUG],strlen(options[DEBUG])) == 0) {
	 debug = 1;
	 continue;
      }

      printf("%s\n",help);
      exit(1);
   }

   if ((send_port == 0) || (recv_port == 0)) {
      printf("TimSurvey:Main:Fatal:(Send/Receive ports not specified)\n");
      exit(1);
   }

   setsid();
   chdir("/tmp");
   freopen( "/dev/null"    , "r", stdin);
   freopen( "/dev/console" , "w", stdout);
   freopen( "/dev/console" , "w", stderr);

#ifdef __linux__
   setpgid (pid,getpgid(0));
#else
   setpgid (pid,getpgrp());
#endif

   gethostname(host,HOST_NAME_SZ);

   bzero((void *) &action, sizeof (action));
   action.sa_handler = (void (*)(int)) sigchildhand;
   sigaction(SIGCHLD, &action, &old);

   bzero((void *) &action, sizeof (action));
   action.sa_handler = (void (*)(int)) sighuphand;
   sigaction(SIGHUP, &action, &old);

   err = TimLibInitialize(TimLibDevice_ANY);
   if (err) {
      fprintf(stderr,"TimSurvey:Main:(Can't initialize TimLib)\n");
      fprintf(stderr,"TimSurvey:Main:(%s)\n",TimLibErrorToString(err));
      exit(err);
   }

   mod = 1;
   mcn = TimLibGetInstalledModuleCount();
   if (mcn) {
      sts = TimLibGetStatus(mod,&dev);
      printf("TimSurvey:Main:Modules:%d Device:%s\n",(int) mod,DeviceToStr(dev));
   } else {
      fprintf(stderr,"TimSurvey:Main:Fatal:(No installed modules)\n");
      exit(1);
   }

   err = TimLibGetCableId(mod,&cid);
   if (err) {
      fprintf(stderr,"TimSurvey:Main:(Can't get GMT Cable ID)\n");
      fprintf(stderr,"TimSurvey:Main:Fatal:(%s)\n",TimLibErrorToString(err));
      exit(err);
   }
   mch = TgvTgvToTgmMachine(TgvFirstMachineForCableId(cid));
   if ((mch < 0) || (mch > TgmMACHINES)) {
      fprintf(stderr,"TimSurvey:Main:(Can't get Tgm machine)\n");
      fprintf(stderr,"TimSurvey:Main:Fatal:(Bad GMT cable ID)\n");
      exit(1);
   }
   if (TgmAttach(mch,TgmTELEGRAM | TgmLINE_NAMES) != TgmSUCCESS) {
      fprintf(stderr,"TimSurvey:Main:Fatal:(Can't attach Tgm machine)\n");
      exit(1);
   }

   ssc = sscs[(int) mch];
   if (debug) printf("TimSurvey:Main:Debug:TgmMachine:%s SSC:%d\n",
		     TgmGetMachineName(mch),(int) ssc);
   scy = scys[(int) mch];
   if (debug) printf("TimSurvey:Main:Debug:TgmMachine:%s SCY:%d\n",
		     TgmGetMachineName(mch),(int) scy);

   for (i=0; i<PTIMS; i++) ptimlist[i] = 0;
   for (i=0; i<PTIMS; i++) ptimodes[i] = 0;
   for (i=0; i<PTIMS; i++) pconnect[i] = 0;
   psize = 0;

   for (i=0; i<MAX_MODULE_COUNT; i++) OkStsFlg[i] = 1;
   for (i=0; i<MAX_MODULE_COUNT; i++) ErStsCnt[i] = 0;

   err = TimLibGetAllPtimObjects(ptimlist, &psize, PTIMS);
   if (err) {
      fprintf(stderr,"TimSurvey:Main:(Can't get PTIM list)\n");
      fprintf(stderr,"TimSurvey:Main:Fatal:%s\n",TimLibErrorToString(err));
      exit(err);
   }
   printf("TimSurvey:Main:(Found:%d PTIMS)\n",(int) psize);

   if (ssc) {
      err = TimLibConnect(TimLibClassCTIM,ssc,mod);
      if (err) {
	 fprintf(stderr,"TimSurvey:Main:(Can't connect to SSC-CT:%d)\n",(int) ssc);
	 fprintf(stderr,"TimSurvey:Main:Fatal:(%s)\n",TimLibErrorToString(err));
	 exit(err);
      }
   }
   if (scy) {
      err = TimLibConnect(TimLibClassCTIM,scy,mod);
      if (err) {
	 fprintf(stderr,"TimSurvey:Main:(Can't connect to SCY-CT:%d)\n",(int) scy);
	 fprintf(stderr,"TimSurvey:Main:Fatal:(%s)\n",TimLibErrorToString(err));
	 exit(err);
      }
   }
   err = TimLibQueue(0,TIME_OUT);
   if (err) {
      fprintf(stderr,"TimSurvey:Main:(Can't set timeout)\n");
      fprintf(stderr,"TimSurvey:Main:Warning:(%s)\n",TimLibErrorToString(err));
      exit(err);
   }

   pthread_create(&tid,NULL,(void *(*)()) ComsThread, NULL);

   ecnt = 0;
   while (ecnt < MAX_ERRORS) {

      CheckStatus();

      err = TimLibWait( &icls, &eqp,  &pln, &hsrc,
			&onzt, &trgt, &strt,
			&ctm,  &pld,  &mod,
			&mis,  &qsz,  &mch );
      if (err) {
	 if (err == TimLibErrorTIMEOUT) {
	    if (debug) printf("TimSurvey:Main:Debug:TimeOut:(From TimLibWait)\n");
	    continue;
	 }
	 fprintf(stderr,"TimSurvey:Main:(From TimLibWait)\n");
	 fprintf(stderr,"TimSurvey:Main:(%s)\n",TimLibErrorToString(err));
	 sleep(1);
	 ecnt++;
	 continue;
      }

      if (icls == TimLibClassCTIM) {
	 onzt.Nano = (onzt.Nano/NANO_TO_MS)*NANO_TO_MS;
	 if (eqp == ssc) {
	    pec = evi; evi = 0;
	    if (sci) sci = 0; else sci = 1;
	    bzero((void *) scptm[sci], MAX_EVENTS * sizeof(unsigned long));
	    strcpy(ScTimeStamp[sci],TmToStr(&onzt,1));
	    ScTimeOnzt[sci] = onzt;
	    sched_yield();
	    if (debug) {
	       if (sampling) {
		  printf("TimSurvey:Main:Debug:Event:(SSC:%s CNT:On:%d)\n",
			 ScTimeStamp[sci],
			 (SSC_SAMPLES - ssc_samples));
	       } else {
		  printf("TimSurvey:Main:Debug:Event:(SSC:%s CNT:Off)\n",
			 ScTimeStamp[sci]);
	       }
	    }
	    if (sampling) {
	       if (++ssc_samples >= SSC_SAMPLES) {
		  DisconnectPtims();
		  if (debug) printf("TimSurvey:Main:Debug:Event:(AutoDisconnect PTIMs)\n");
	       }
	    }
	    continue;
	 }
	 if (eqp == scy) {
	    eqp = SCY_MARKER; pld = 0;
	    grn = TgmGetGroupNumber(mch,"USER");
	    if (grn) TgmGetGroupValue(mch,TgmCURRENT,0,grn,(int *) &pld);
	    if (debug) printf("TimSurvey:Main:Debug:Event:(SCY:%s)\n",TmToStr(&onzt,1));
	 }
      }

      if (evi >= MAX_EVENTS) continue;

      scptm[sci][evi] = eqp;
      scpld[sci][evi] = pld;
      scpln[sci][evi] = pln;
      sconz[sci][evi] = onzt;
      sctim[sci][evi] = TimeDiff(&onzt,&(ScTimeOnzt[sci]));

      if (TooFast(eqp)) {
	 TimLibDisConnect(TimLibClassPTIM,eqp,mod);
	 if (debug) printf("TimSurvey:Main:Debug:DisConnect:(PTIM:%d too many interrupts)\n",(int) eqp);
	 for (i=0; i<psize; i++) {
	    if (eqp == ptimlist[i]) ptimodes[i]++;
	 }
      }
      evi++;
   }
   fprintf(stderr,"TimSurvey:Main:Fatal:(Too many errors:%d)\n",(int) ecnt);
   exit(ecnt);
}
