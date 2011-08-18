/* ****************************************************************************** */
/* TimLib test program                                                            */
/* Defining PS_VER (PS Version) causes the program to use the Tgm and Tgv libs    */
/* for proper telegram decoding and name space handling.                          */
/* Julian Lewis Mon 25th April 2005                                               */
/* ****************************************************************************** */

#include <tgm/tgm.h>
#include <tgv/tgv.h>
#include <ctrdrvr.h>

#include <time.h>   /* ctime */

/**************************************************************************/

static TgmMachine tmch = TgmCPS;

static int counter = 1;
static int module = 1;
static char *editor = "e";

typedef struct {
   unsigned long Eqp;
   char Name[32];
   char Comment[64];
   unsigned long Flg;
 } PtmNames;

static PtmNames ptm_names[128];
static char ptm_name_txt[32];
static char ptm_comment_txt[64];
static int ptm_names_size = 0;


/**************************************************************************/

char *defaultconfigpath = "/dsc/bin/tim/timtest.config";

char *configpath = NULL;
char localconfigpath[128];  /* After a CD */

/**************************************************************************/

static char path[128];

char *GetFile(char *name) {
FILE *gpath = NULL;
char txt[128];
int i, j;

   if (configpath) {
      gpath = fopen(configpath,"r");
      if (gpath == NULL) {
	 configpath = NULL;
      }
   }

   if (configpath == NULL) {
      configpath = "./timtest.config";
      gpath = fopen(configpath,"r");
      if (gpath == NULL) {
	 configpath = "/dsc/local/data/timtest.config";
	 gpath = fopen(configpath,"r");
	 if (gpath == NULL) {
	    configpath = defaultconfigpath;
	    gpath = fopen(configpath,"r");
	    if (gpath == NULL) {
	       configpath = NULL;
	       sprintf(path,"./%s",name);
	       return path;
	    }
	 }
      }
   }

   bzero((void *) path,128);

   while (1) {
      if (fgets(txt,128,gpath) == NULL) break;
      if (strncmp(name,txt,strlen(name)) == 0) {
	 for (i=strlen(name); i<strlen(txt); i++) {
	    if (txt[i] != ' ') break;
	 }
	 j= 0;
	 while ((txt[i] != ' ') && (txt[i] != 0)) {
	    path[j] = txt[i];
	    j++; i++;
	 }
	 strcat(path,name);
	 fclose(gpath);
	 return path;
      }
   }
   fclose(gpath);
   return NULL;
}

/*****************************************************************/
/* News                                                          */

int News(int arg) {

char sys[128], npt[128];

   arg++;

   if (GetFile("tim_news")) {
      strcpy(npt,path);
      sprintf(sys,"%s %s",GetFile(editor),npt);
      printf("\n%s\n",sys);
      system(sys);
      printf("\n");
   }
   return(arg);
}

/**************************************************************************/

static int yesno=1;

static int YesNo(char *question, char *name) {
int yn, c;

   if (yesno == 0) return 1;

   printf("%s: %s\n",question,name);
   printf("Continue (Y/N):"); yn = getchar(); c = getchar();
   if ((yn != (int) 'y') && (yn != 'Y')) return 0;
   return 1;
}

/**************************************************************************/
/* Launch a task                                                          */

static void Launch(char *txt) {
pid_t child;

   if ((child = fork()) == 0) {
      if ((child = fork()) == 0) {
	 system(txt);
	 exit (127);
      }
      exit (0);
   }
}

/**************************************************************************/
/* Check error codes                                                      */

int CheckErr(TimLibError err) {
   if (err == TimLibErrorSUCCESS) return 1;
   if (err < TimLibERRORS) {
      fprintf(stderr,"timtest:Error:%s\n",TimLibErrorToString(err));
      if (err == TimLibErrorIO) perror("TimLib:errno");
      return 0;
   }
   fprintf(stderr,"timtest:Error:Illegal error number:%d\n",(int) err);
   return 0;
}

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
      ctime_r((time_t *) &t->Second, tmp);
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
/* If its a CPS TG8 we can get the names from the tg8infofile             */

char *GetTg8PtmName(unsigned long eqp) {

char *cp, *ep;
int i, j, nf, gf;
FILE *inp;
char txt[128];

   if (ptm_names_size == 0) {
      inp = fopen("/tmp/tg8infofile","r");
      if (inp) {
	 gf = 0;
	 while (fgets(txt,128,inp) != NULL) {
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
	    if (ptm_names_size >= 128) break;
	 }
	 fclose(inp);
      }
   }

   for (i=0; i<ptm_names_size; i++) {
      if (ptm_names[i].Eqp == eqp) {
	 sprintf(ptm_name_txt,"%04d:%s",(int) eqp,ptm_names[i].Name);
	 return ptm_name_txt;
      }
   }

   sprintf(ptm_name_txt,"%04d",(int) eqp);
   return ptm_name_txt;
}

/**************************************************************************/
/* Convert a ptim to its name                                             */

char *GetPtmName(unsigned long eqp, int pfl) {

char *cp, *ep;
int i;
FILE *inp;

   if (ptm_names_size == 0) {

      bzero((void *) &ptm_names, sizeof(ptm_names));

      GetFile("ltim.obnames");
      inp = fopen(path,"r");
      if (inp) {
	 while (fgets(ptm_name_txt,128,inp) != NULL) {
	    cp = ep = ptm_name_txt;
	    ptm_names[ptm_names_size].Eqp = strtoul(cp,&ep,0);
	    if (cp == ep) continue;
	    if ((*ep != 0) && (*ep != '\n')) cp = ep +1;
	    ptm_names[ptm_names_size].Flg = strtoul(cp,&ep,0);
	    if (cp != ep) ep++;

	    cp = index(ep,':');
	    if (cp) { *cp = '\0'; cp++; }

	    if (cp) {
	       for (i=strlen(cp); i>=0; i--) {
		  if (cp[i] == '\n') {
		     cp[i] = 0;
		     break;
		  }
	       }
	       if (strcmp(cp,"It's a SPS device") == 0) strcpy(ptm_names[ptm_names_size].Comment,"-");
	       else                                     strcpy(ptm_names[ptm_names_size].Comment,cp);
	    }
	    if (ep) {
	       for (i=strlen(ep); i>=0; i--) {
		  if (ep[i] == '\n') {
		     ep[i] = 0;
		     break;
		  }
	       }
	       strcpy(ptm_names[ptm_names_size].Name,ep);
	    }

	    if (++ptm_names_size >= 128) break;
	 }
	 fclose(inp);
      } else if (Dev == TimLibDevice_TG8_CPS) return GetTg8PtmName(eqp);
   }

   bzero((void *) ptm_name_txt, 32);
   bzero((void *) ptm_comment_txt, 64);

   for (i=0; i<ptm_names_size; i++) {
      if (ptm_names[i].Eqp == eqp) {
	 sprintf(ptm_comment_txt,"%s",ptm_names[i].Comment);
	 if (pfl) {
	    sprintf(ptm_name_txt,"%s",ptm_names[i].Name);
	 } else {
	    if (ptm_names[i].Flg) {
	       sprintf(ptm_name_txt,"%04d:RtAqn:%s",
				    (int) eqp,
					  ptm_names[i].Name);
	    } else {
	       sprintf(ptm_name_txt,"%04d:%s",
				    (int) eqp,
					  ptm_names[i].Name);
	    }
	 }
	 return ptm_name_txt;
      }
   }

   sprintf(ptm_name_txt,"%04d",(int) eqp);
   return ptm_name_txt;
}

/**************************************************************************/
/* Status To String                                                       */

#define STATAE 5

static char *StatusOn[STATAE] = { "GmtOk", "PllOk", "SlfOk", "EnbOk", "BusOk"  };
static char *StatusOf[STATAE] = { "***GmtErr***","***PllErr***","***SlfErr***","***EnbErr***","***BusErr***" };

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

static char *MStatOn[TimLibCstSTATAE] = {
   "CTRXE", "CTRXI", "V1_PCB", "V2_PCB ",
   "S1", "S2", "X1", "X2",
   "O1", "O2", "O3", "O4",
   "O5", "O6", "O7", "O8",
   "IDOK", "DEBHIS", "SOFTPLL", "XMEM", "TEMPOK" };

#define MS_STR_SZ 80
static char MStatStr[MS_STR_SZ];

char * MStatToStr(TimLibCstStat mstat) {
int i;
unsigned long msk;

   bzero((void *) MStatStr,MS_STR_SZ);
   for (i=0; i<TimLibCstSTATAE; i++) {
      msk = 1 << i;
      if (msk & mstat) {
	 strcat(MStatStr,MStatOn[i]);
	 strcat(MStatStr,":");
      }
   }
   return MStatStr;
}

/**************************************************************************/
/* Hardware Interrupt to String                                           */

#define SOURCES 14
static char *HardNames[SOURCES] =
   {"Cntr0","Cntr1","Cntr2","Cntr3","Cntr4","Cntr5","Cntr6","Cntr7","Cntr8",
    "PllIt","GmtEv","OneHz","OneKHz","Match" };

#define HD_STR_SZ 80
static char HrdStr[HD_STR_SZ];

char *HardToStr(TimLibHardware hrd) {
int i;
unsigned long msk;

   bzero((void *) HrdStr,HD_STR_SZ);
   for (i=0; i<SOURCES; i++) {
      msk = 1 << i;
      if (msk & TimLibHardwareBITS) {
	 if (msk & hrd) {
	    strcat(HrdStr,HardNames[i]);
	    strcat(HrdStr,":");
	 }
      } else break;
   }
   return HrdStr;
}

/*****************************************************************/
/* Output Mask to String                                         */

#define OUT_MASKS  12
static char *OtmNames[OUT_MASKS] = {"Ctim","Cntr1","Cntr2","Cntr3","Cntr4","Cntr5","Cntr6","Cntr7","Cntr8",
				    "40Mh","ExCk1","ExCk2" };

#define OTM_STR_SZ 80
static char OtmStr[OTM_STR_SZ];

char *OtmToStr(TimLibOutput otm) {
int i;
unsigned long msk;

   bzero((void *) OtmStr,OTM_STR_SZ);
   if (otm == 0) {
      strcat(OtmStr,"NotSet");
      return OtmStr;
   }
   for (i=0; i<OUT_MASKS; i++) {
      msk = 1 << i;
      if (msk & TimLibOutputBITS) {
	 if (msk & otm) {
	    strcat(OtmStr,OtmNames[i]);
	    strcat(OtmStr,":");
	 }
      } else break;
   }
   return OtmStr;
}

/*****************************************************************/

#define CCV_FIELDS 13
#define POLARATIES 3

static char *CounterStart [TimLibSTARTS]  = {"Nor", "Ext1", "Ext2", "Chnd", "Self", "Remt", "Pps", "Chnd+Stop"};
static char *CounterMode  [TimLibMODES]   = {"Once", "Mult", "Brst", "Mult+Brst"};
static char *CounterClock [TimLibCLOCKS]  = {"1KHz", "10MHz", "40MHz", "Ext1", "Ext2", "Chnd" };
static char *Polarity     [POLARATIES]    = {"TTL","BAR","TTL"};
static char *Enable       [TimLibENABLES] = {"NoOut","Out","Bus","OutBus"};

#define CCV_STR_SZ 128
static char CcvStr[CCV_STR_SZ];

char * CcvToStr(TimLibCcvMask ccm, TimLibCcv *ccv, int pln) {

int i, w;
unsigned long msk;
TgmGroupDescriptor desc;
char tmp[CCV_STR_SZ];
TgvName tname;
char *cp;

   bzero((void *) CcvStr,CCV_STR_SZ);
   bzero((void *) tmp   ,CCV_STR_SZ);

   if (pln) sprintf(CcvStr,"%02d:",pln);

   for (i=0; i<CCV_FIELDS; i++) {
      msk = 1 << i;
      if (msk & TimLibCcvMaskBITS) {
	 if (msk & ccm) {
	    switch (msk) {
	       case TimLibCcvMaskENABLE:
		  if (ccv->Enable < TimLibENABLES)
		     sprintf(tmp,"%s ",Enable[ccv->Enable]);
		  else
		     sprintf(tmp,"En:?[%d] ",(int) ccv->Enable);
	       break;

	       case TimLibCcvMaskSTART:
		  if (ccv->Start < TimLibSTARTS)
		     sprintf(tmp,"St%s:",CounterStart[ccv->Start]);
		  else
		     sprintf(tmp,"St?[%d]:",(int) ccv->Start);
	       break;

	       case TimLibCcvMaskMODE:
		  if (ccv->Mode < TimLibMODES)
		     sprintf(tmp,"%s:",CounterMode[ccv->Mode]);
		  else
		     sprintf(tmp,"Md?[%d]:",(int) ccv->Mode);
	       break;

	       case TimLibCcvMaskCLOCK:
		  if (ccv->Clock < TimLibCLOCKS)
		     sprintf(tmp,"Ck%s ",CounterClock[ccv->Clock]);
		  else
		     sprintf(tmp,"Ck?[%d] ",ccv->Clock);
	       break;

	       case TimLibCcvMaskPWIDTH:
		  w = ccv->PulsWidth * 25;
		  if      (w >= 1000000) { w = w/1000000; cp = "ms"; }
		  else if (w >= 1000   ) { w = w/1000;    cp = "us"; }
		  else                   {                cp = "ns"; }
		  sprintf(tmp,"%d[%d%s]",(int) ccv->PulsWidth, w, cp);
	       break;

	       case TimLibCcvMaskDELAY:
		  sprintf(tmp,"%d>",(int) ccv->Delay);
	       break;

	       case TimLibCcvMaskOMASK:
		  sprintf(tmp,"%s",OtmToStr(ccv->OutputMask));
	       break;

	       case TimLibCcvMaskPOLARITY:
		  if ((ccv->Polarity == TimLibPolarityTTL_BAR)
		  ||  (ccv->Polarity == TimLibPolarityTTL)
		  ||  (ccv->Polarity == 0))
		     sprintf(tmp,"%s ",Polarity[ccv->Polarity]);
		  else
		     sprintf(tmp,"Vp:?[%d] ",(int) ccv->Polarity);
	       break;

	       case TimLibCcvMaskCTIM:
		  if (TgvGetNameForMember(ccv->Ctim,&tname) == NULL) sprintf(tname,"BadCtim");
		  sprintf(tmp,"%d-%s ",(int) ccv->Ctim,(char *) tname);
	       break;

	       case TimLibCcvMaskPAYLOAD:
		  if (ccv->Payload == 0) break;
		  sprintf(tmp,"Py:0x%04X ",(int) ccv->Payload);
	       break;

	       case TimLibCcvMaskMACHINE:
		  if (ccv->GrNum == 0) break;
		  cp = TgmGetMachineName(ccv->Machine);
		  if (cp) sprintf(tmp,"%s.",TgmGetMachineName(ccv->Machine));
		  else    sprintf(tmp,"???[%d].",(int) ccv->Machine);
	       break;

	       case TimLibCcvMaskGRNUM:
		  if (ccv->GrNum == 0) break;
		  if (TgmGetGroupDescriptor(ccv->Machine,ccv->GrNum,&desc) == TgmSUCCESS) {
		     sprintf(tmp,"%s",desc.Name);
		     if (desc.Type == TgmBIT_PATTERN) strcat(tmp,"&");
		     else                             strcat(tmp,"=");
		  } else sprintf(tmp,"???[%d]*",(int) ccv->GrNum);
	       break;

	       case TimLibCcvMaskGRVAL:
		  if (ccv->GrNum == 0) break;
		  if (TgmGetGroupDescriptor(ccv->Machine,ccv->GrNum,&desc) == TgmSUCCESS) {
		     if (desc.Type == TgmEXCLUSIVE) sprintf(tmp,"%s",(char *) TgmGetLineName(ccv->Machine,desc.Name,ccv->GrVal));
		     else                           sprintf(tmp,"%d",(int) ccv->GrVal);
		  } else sprintf(tmp,"%d",(int) ccv->GrVal);
	       break;

	       default:
	       break;
	    }
	    strcat(CcvStr,tmp);
	    bzero((void *) tmp,CCV_STR_SZ);
	 }
      }
   }
   return CcvStr;
}

/*****************************************************************/
/* Commands used in the test program.                            */
/*****************************************************************/

int ChangeEditor(int arg) {
static int eflg = 0;

   arg++;
   if (eflg++ > 4) eflg = 1;

   if      (eflg == 1) editor = "e";
   else if (eflg == 2) editor = "emacs";
   else if (eflg == 3) editor = "nedit";
   else if (eflg == 4) editor = "vi";

   printf("Editor: %s\n",GetFile(editor));
   return arg;
}

/*****************************************************************/

int ChangeDirectory(int arg) {
ArgVal   *v;
AtomType  at;
char txt[128], fname[128], *cp;
int n, earg;

   arg++;
   for (earg=arg; earg<pcnt; earg++) {
      v = &(vals[earg]);
      if ((v->Type == Close)
      ||  (v->Type == Terminator)) break;
   }

   v = &(vals[arg]);
   at = v->Type;
   if ((v->Type != Close)
   &&  (v->Type != Terminator)) {

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

      strcpy(localconfigpath,fname);
      strcat(localconfigpath,"/timtest.config");
      if (YesNo("Change timtest config to:",localconfigpath))
	 configpath = localconfigpath;
      else
	 configpath = NULL;
   }

   sprintf(txt,"%s %s",GetFile(editor),configpath);
   printf("\n%s\n",txt);
   system(txt);
   printf("\n");
   return(arg);
}

/*****************************************************************/

int NextCounter(int arg) {

   arg++;
   counter++;
   if (counter > 8) counter = 1;
   return arg;
}

/*****************************************************************/

int NextModule(int arg) {
int mcnt;

   arg++;
   mcnt = TimLibGetInstalledModuleCount();
   module++;
   if (module > mcnt) module = 1;
   return arg;
}

/*****************************************************************/

int GetStatus(int arg) {

TimLibDevice dev;
TimLibStatus sts;
char *cp;

   arg++;
   sts = TimLibGetStatus(module,&dev);
   printf("Mod:%d Dev:%d[%s] Status:0x%04X[%s]\n",
	  module,
	  dev,
	  DevNames[(int) dev],
	  sts,
	  StatusToStr(sts));

   cp = TimLibGetSpecificInfo(module);
   if (cp) printf("%s\n",cp);
   return arg;
}

/*****************************************************************/

int GetSetCounter(int arg) {
ArgVal   *v;
AtomType  at;

unsigned long cnt;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      cnt = v->Number;
      if ((cnt>0) && (cnt<=8)) counter = cnt;
   }
   printf("Cntr:%d Selected\n",counter);
   return arg;
}

/*****************************************************************/

int GetSetModule(int arg) {
ArgVal   *v;
AtomType  at;

unsigned long mod, mcnt, cbl;
TgvName cblnam;
TgmMachine mch;
TgmNetworkId nid;

   arg++;
   mcnt = TimLibGetInstalledModuleCount();

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      mod = v->Number;
      if (mod <= mcnt) module = mod;
   }

   for (mod=1; mod<=mcnt; mod++) {
      if (CheckErr(TimLibGetCableId(mod,&cbl))) {
	 printf("Mod:%d ",(int) mod);
	 if (TgvGetCableName(cbl,&cblnam)) {
	    mch = TgvTgvToTgmMachine(TgvFirstMachineForCableId(cbl));
	    printf("Cable:%s Tgm:%s",cblnam,TgmGetMachineName(mch));
	 } else {
	    printf("WARNING: Incorrect cable:%d connected to module:%d\n",(int) cbl, (int) module);
	    printf("WARNING: This DSC is NOT configured to handle cable:%d ",(int) cbl);
	 }
	 if (mod == module) printf(" <<==");
	 printf("\n");
      }
   }
   if (mod == 0) printf("Mod:0 No module selected: Auto\n");
   else          printf("Mod:%d Selected\n",module);
   nid = TgmGetDefaultNetworkId();
   printf("DSC Configuration: TGM_NETWORK@%s:%s\n",
	  (char *) TgmGetNetworkPath(),
	  (char *) TgmNetworkIdToName(nid));
   return arg;
}

/*****************************************************************/

static unsigned long TimDebug = 0;

int SwDeb(int arg) {
ArgVal   *v;
AtomType  at;

   arg++;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      if (v->Number) TimDebug = v->Number;
      else           TimDebug = 0;
      TimLibSetDebug(TimDebug);
   }
   if (TimDebug != 0)
      printf("TimLibDebug: Level:[0x%02X] Enabled\n",(int) TimDebug);
   else
      printf("TimLibDebug: Level:[0] Disabled\n");

   return arg;
}

/*****************************************************************/

static unsigned long QFlag = 0;
static unsigned long TmOut = 2000;

int GetSetTmo(int arg) { /* Arg=0 => Timeouts disabled, else Arg = Timeout */
ArgVal   *v;
AtomType  at;

   arg++;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      TmOut = v->Number;
      CheckErr(TimLibQueue(QFlag,TmOut));
   }
   if (TmOut > 0) printf("Timeout: [%d] Enabled\n",(int) TmOut);
   else           printf("Timeout: [Zero] Disabled\n");

   return arg;
}

/*****************************************************************/

int GetSetQue(int arg) { /* Arg=Flag */
ArgVal   *v;
AtomType  at;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      QFlag = v->Number;
      CheckErr(TimLibQueue(QFlag,TmOut));
   }
   if (QFlag) printf("QueueFlag: Set,   Queuing is Off\n");
   else       printf("QueueFlag: ReSet, Queuing is On\n");
   return arg;
}

/*****************************************************************/

static char *mtyp[TimLibModuleTYPES] = {"???","CTRI","CTRP","CTRV",
					"CPS_TG8", "SPS_TG8" };

int GetVersion(int arg) {

TimLibModuleVersion tver;
TimLibTime t;
int i;

   arg++;

   printf("TimLib:  Compiled: %s\n",TimLibGetVersion());
   printf("timtest: Compiled: %s %s %s\n", "timtest", __DATE__, __TIME__);

   if (TimLibGetModuleVersion(&tver) == TimLibErrorSUCCESS) {

      bzero((void *) &t, sizeof(TimLibTime));
      t.Machine = TgmMACHINE_NONE;
      t.Second = tver.DrvVer;
      printf("Drv: Ver:[%d] %s\n",(int) t.Second, TimeToStr(&t));
      t.Second = tver.CorVer;
      printf("Cor:%s Ver:[%d] %s\n",
	      mtyp[(int) tver.ModTyp],
	      (int) t.Second,
	      TimeToStr(&t));

      for (i=0; i<TimLibMODULES; i++) {
	 if (tver.ModVer[i]) {
	    t.Second  = tver.ModVer[i];
	    printf("Mod:%d Ver:[%d] %s\n",
		   i+1,
		   (int) t.Second,
		   TimeToStr(&t));
	 }
      }
   }
   return arg;
}

/*****************************************************************/

extern int timlib_real_utc;

int GetUtc(int arg) {
ArgVal   *v;
AtomType  at;

TimLibTime ct;

   arg++;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Alpha) {
      if ((v->Text[0] == 'A') || (v->Text[0] == 'a')) {
	 timlib_real_utc = 0;
	 arg++;
	 if (Dev == TimLibDevice_NETWORK) {
	    printf("Adjust UTC Time: Not implemented for NETWORK devices\n");
	    return arg;
	 }

	 printf("Adjusted/Legacy UTC ON (Real +2ms)\n");
	 return arg;
      }
      if ((v->Text[0] == 'R') || (v->Text[0] == 'r')) {
	 timlib_real_utc = 1;
	 arg++;
	 if (Dev == TimLibDevice_TG8_CPS) {
	    printf("Real UTC Time: Not implemented for TG8-CPS devices, (-2ms)\n");
	    return arg;
	 }
	 if (Dev == TimLibDevice_TG8_SPS) {
	    printf("Real UTC Time: SPS-TG8 modules are erratic, usually (-1ms)\n");
	    return arg;
	 }
	 printf("Real UTC ON\n");
	 return arg;
      }
   }
   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 printf("A: Adjust UTC time by +2ms to legacy TG8 module time\n");
	 printf("R: Use real non adjusted UTC time\n");
	 return arg;
      }
   }

   if (CheckErr(TimLibGetTime(module,&ct))) {
      printf("Mod:%d UTC:%s\n",
	     module,
	     TimeToStr(&ct));
   }
   return arg;
}

/*****************************************************************/

static int connected = 0;
static char *class_names[3] = {"Hard","Ctim","Ptim"};

int WaitInterrupt(int arg) { /* msk */
ArgVal   *v;
AtomType  at;

int i, msk;

TimLibClass    iclss, xclss;
unsigned long  equip, xquip, usr, gn;
unsigned long  plnum;
TimLibHardware source;
TimLibTime     onzero;
TimLibTime     trigger;
TimLibTime     start;
TimLibTime     cstamp;
unsigned long  ctim;
unsigned long  payload;
unsigned long  mod;
unsigned long  missed;
unsigned long  qsize;
unsigned long  cytag;
unsigned long  ncytag;
unsigned long  tin;
TgmMachine     mch;
TgmGroupDescriptor gdesc;
TgvName tname;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 for (i=0; i<16; i++) {
	    msk = 1 << i;
	    if (msk & TimLibHardwareBITS) {
	       printf("%s 0x%04X ",HardToStr(msk),msk);
	       if ((i%4) == 0)  printf("\n");
	    } else break;
	 }
	 printf("\nP<ptim object> C<ctim object> CM<ctim object on current module>\n");
	 return arg;
      }
   }

   xquip = 0;
   xclss = TimLibClassHARDWARE;

   if (at == Numeric) {                 /* Hardware mask ? */
      arg++;

      if (v->Number) {
	 equip = v->Number;
	 if (CheckErr(TimLibConnect(TimLibClassHARDWARE,equip,module))) connected++;
      } else {
	 if (CheckErr(TimLibConnect(TimLibClassHARDWARE,0,0))) {
	    connected = 0;
	    printf("Disconnected from all interrupts\n");
	 }
	 return arg;
      }
   }

   if (at == Alpha) {
      if ((v->Text[0] == 'P') || (v->Text[0] == 'p')) { /* Ptim equipment ? */
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;
	 if (at == Numeric) {
	    equip = v->Number;
	    arg++;
	    v = &(vals[arg]);
	    at = v->Type;
	    if (CheckErr(TimLibConnect(TimLibClassPTIM,equip,0))) {
	       xclss = TimLibClassPTIM;
	       xquip = equip;
	       connected++;
	    }
	 }
      } else if ((v->Text[0] == 'C') || (v->Text[0] == 'c')) { /* Ctim equipment ? */

	 if ((v->Text[1] == 'M') || (v->Text[1] == 'm')) mod = module;
	 else                                            mod = 0;

	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;
	 if (at == Numeric) {
	    equip = v->Number;
	    arg++;
	    v = &(vals[arg]);
	    at = v->Type;
	    if (CheckErr(TimLibConnect(TimLibClassCTIM,equip,mod))) {
	       xclss = TimLibClassCTIM;
	       xquip = equip;
	       connected++;
	    }
	 }
      }
   }

   tin = 0;
   if (at == Alpha) {
      if ((v->Text[0] == 'T') || (v->Text[0] == 't')) { /* Tgm Info ? */
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;
	 tin = 1;
      }
   }

   if (connected == 0) {
      fprintf(stderr,"timtest:Error:WaitInterrupt:No connections to wait for\n");
      return arg;
   }

   while(1) {
      CheckErr(TimLibWait(&iclss,    /* Class of interrupt */
			  &equip,    /* PTIM CTIM or hardware mask */
			  &plnum,    /* Ptim line number 1..n or 0 */
			  &source,   /* Hardware source of interrupt */
			  &onzero,   /* Time of interrupt/output */
			  &trigger,  /* Time of counters load */
			  &start,    /* Time of counters start */
			  &ctim,     /* CTIM trigger equipment ID */
			  &payload,  /* Payload of trigger event */
			  &mod,      /* Module that interrupted */
			  &missed,   /* Number of missed interrupts */
			  &qsize,    /* Remaining interrupts on queue */
			  &mch));    /* Corresponding TgmMachine */

      if (xquip == 0) break;         /* Any event will do */

      if ((iclss == xclss) && (xquip == equip)) break;
   }

   if (module)          printf("Mod:%d ",(int) mod);
			printf("Cls:%s ",class_names[iclss]);
   if (equip) {
      if (iclss == TimLibClassPTIM)
	 printf("Eqp:%s ",GetPtmName(equip,0));
      else {
	 if (TgvGetNameForMember(equip,&tname) == NULL)
	    printf("Eqp:0x%04X[%d] ",(int) equip,(int) equip);
	 else
	    printf("Eqp:0x%04X:%s ",(int) equip,tname);
      }
   }
   if (plnum)           printf("Lnm:%d ",(int) plnum);
   if (source)          printf("Src:%d[%s] ",(int) source,HardToStr(1 << source));
   if (onzero.Second)   printf("\nOzr:%s ",TimeToStr(&onzero));
   if (trigger.Second)  printf("\nTrg:%s ",TimeToStr(&trigger));
   if (start.Second)    printf("\nStm:%s ",TimeToStr(&start));

   if (ctim) {
      if (TgvGetNameForMember(ctim,&tname) == NULL) sprintf(tname,"BadCtim");
      printf("\nCtm:%d[%s] ",(int) ctim,(char *) tname);
   }

   if (mch != -1)       printf("Mch:%d[%s] ",mch,TgmGetMachineName(mch));
   if (payload)         printf("Pay:0x%4X ",(int) payload);
   if (qsize)           printf("\nQsz:%d ",(int) qsize);
   if (missed)          printf("Qms:%d ",(int) missed);

   if (tin) {
      onzero.Machine = mch;

      if (CheckErr(TimLibGetTgmInfo(onzero,&cstamp,&cytag,&ncytag))) {
	 printf("\nTgm:%s Tag:0x%X:0x%X",
		TimeToStr(&cstamp),
		(int) cytag,
		(int) ncytag);
      }
      gn = TgmGetGroupNumber(mch,"USER");
      if (TgmGetGroupDescriptor(mch,gn,&gdesc) == TgmSUCCESS) {
	 if (CheckErr(TimLibGetGroupValueFromStamp(cstamp,gn,0,&usr))) {
	    printf(" %s:%s",
		   gdesc.Name,
		   (char *) TgmGetLineName(mch,gdesc.Name,usr));
	 }
      }
      gn = TgmGetNextGroupNumber(mch,gn);
      if (TgmGetGroupDescriptor(mch,gn,&gdesc) == TgmSUCCESS) {
	 if (CheckErr(TimLibGetGroupValueFromStamp(cstamp,1,1,&usr))) {
	    printf(":%s",
		   (char *) TgmGetLineName(mch,gdesc.Name,usr));
	 }
      }
      printf("\n");
   }
   printf("\n------------\n");

   return arg;
}

/*****************************************************************/

int SimulateInterrupt(int arg) { /* msk */
ArgVal   *v;
AtomType  at;

int i;
unsigned long msk, eqp, pld;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 for (i=0; i<SOURCES; i++) {
	    msk = 1 << i;
	    if (msk & TimLibHardwareBITS) {
	       printf("0x%04X %s\t",(int) msk,HardToStr(msk));
	       if (((i+ 1) % 4) == 0) printf("\n");
	    }
	 }
	 printf("P<ptim object> C<ctim object>\n");
	 return arg;
      }
   }

   if (at == Numeric) {                 /* Hardware mask ? */
      arg++;
      CheckErr(TimLibSimulate(TimLibClassHARDWARE,v->Number,module,0,0,0));
      return arg;

   }

   if (at == Alpha) {
      if ((v->Text[0] == 'P') || (v->Text[0] == 'p')) { /* Ptim equipment ? */
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;
	 if (at == Numeric) {
	    CheckErr(TimLibSimulate(TimLibClassPTIM,v->Number,0,0,0,0));
	    return arg;
	 }
      }

      if ((v->Text[0] == 'C') || (v->Text[0] == 'c')) { /* Ctim equipment ? */
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;
	 if (at == Numeric) {
	    eqp = v->Number;
	    pld = 0;

	    arg++;
	    v = &(vals[arg]);
	    at = v->Type;
	    if (at == Numeric) pld = v->Number; /* Payload */

	    CheckErr(TimLibSimulate(TimLibClassCTIM,eqp,module,0,0,pld));
	    return arg;
	 }
      }
   }
   fprintf(stderr,"timtest:Error:Bad interrupt spec\n");
   return arg;
}

/*****************************************************************/

static char *eccv_help =

"<CrLf>                 Next action           \n"
"/<Action Number>       Open action for edit  \n"
"?                      Print this help text  \n"
".                      Exit from the editor  \n"
"i<CTIM>                Change CTIM trigger number  CTIM                                           \n"
"f<Frame>               Change frame                Frame                                          \n"
"m<Trigger Machine>     Change trigger machine      0/CPS,1/PSB,2/LEI,3/ADE,4/SPS,5/LHC,6/SCT,7/FCT\n"
"n<Trigger Group>       Change trigger group number Group Number                                   \n"
"g<Trigger group value> Change trigger group value  Group Value                                    \n"
"s<Start>               Change Start                1KHz/Ext1/Ext2/Chnd/Self/Remt/Pps/Chnd+Stop    \n"
"k<Clock>               Change Clock                1KHz/10MHz/40MHz/Ext1/Ext2/Chnd                \n"
"o<Mode>                Change Mode                 Once/Mult/Brst/Mult+Brst                       \n"
"w<Pulse Width>         Change Pulse Width          Pulse Width                                    \n"
"v<Delay>               Change Delay                Delay                                          \n"
"e<enable>              Change OnZero behaviour     NoOut/Bus/Out/OutBus                           \n"
"p<polarity>            Change polarity             TTL/TTL_BAR/TTL                                \n"
"q<outmask>             Change output mask          1<<cntr 0x200/40MHz 0x400/ExCk1 0x800/ExCk2    \n";

#define TXT_SZ 128

void EditCcvs(unsigned long ptm) {

unsigned long mod, cnt, dim;
unsigned long pln, grn, grv;
TimLibCcvMask ccm;
TimLibCcv ccv;

int i, n;
char c, *cp, *ep, *cnm;
TgvName tname;
char txt[TXT_SZ];
unsigned long eqp, frm, mch, str, clk, mde, plw, dly, enb, pol, oms;

   if (CheckErr(TimLibGetPtimObject(ptm,&mod,&cnt,&dim))) {

      pln = 1;

      while (1) {
	 if (CheckErr(TimLibGet(ptm,pln,grn,grv,&ccm,&ccv))) {
	    grn = 0; grv = 0;
	    if (ccm & TimLibCcvMaskGRNUM) {
	       grn = ccv.GrNum;
	       grv = ccv.GrVal;
	    }
	    if (CcvToStr(ccm,&ccv,pln)) {
	       printf("%s : ",CcvStr);
	       fflush(stdout);
	    } else return;

	    bzero((void *) txt, TXT_SZ); n = 0; c = 0;
	    cp = ep = txt;
	    while ((c != '\n') && (n < TXT_SZ -1)) c = txt[n++] = (char) getchar();

	    ccm = 0;
	    while (*cp != 0) {
	       switch (*cp++) {

		  case '\n':
		     if (ccm) if (!CheckErr(TimLibSet(ptm,pln,grn,grv,ccm,&ccv))) return;
		     if (n==1) {
			pln++;
			if (pln > dim) {
			   pln = 1;
			   printf("\n");
			}
		     }
		  break;

		  case '/':
		     i = strtoul(cp,&ep,0);
		     if (cp != ep) {
			cp = ep;
			if (i<=dim) pln = i;
		     }
		  break;

		  case '?':
		     printf("%s\n",eccv_help);
		  break;

		  case '.': return;

		  case 'i':
		     eqp = strtoul(cp,&ep,0); cp = ep;
		     if (eqp) {
			cnm = (char *) TgvGetNameForMember(eqp,&tname);
			if (cnm) {
			   ccv.Ctim = eqp;
			   ccm |= TimLibCcvMaskCTIM;
			   break;
			}
		     }
		     fprintf(stderr,"timtest:Error:No such CTIM equipment: %d\n",(int) eqp);
		  break;

		  case 'f':
		     frm = strtoul(cp,&ep,0); cp = ep;
		     if (frm) {
			eqp = TgvGetMemberForFrame(frm);
			if (eqp) {
			   ccv.Ctim = eqp;
			   ccv.Payload = frm & 0xFFFF;
			   ccm |= TimLibCcvMaskCTIM | TimLibCcvMaskPAYLOAD;
			   break;
			}
		     }
		     fprintf(stderr,"timtest:Error:No such CTIM frame: 0x%X\n",(int) frm);
		  break;

		  case 'm':
		     mch = strtoul(cp,&ep,0); cp = ep;
		     if (mch < TgmMACHINES) {
			ccv.Machine = mch;
			ccm |= TimLibCcvMaskMACHINE;
			break;
		     }
		     fprintf(stderr,"timtest:Error:No such CTIM machine: %d\n",(int) mch);
		  break;

		  case 'n':
		     grn = strtoul(cp,&ep,0); cp = ep;
		     ccv.GrNum = grn;
		     ccm |= TimLibCcvMaskGRNUM;
		  break;

		  case 'g':
		     grv = strtoul(cp,&ep,0); cp = ep;
		     ccv.GrVal = grv;
		     ccm |= TimLibCcvMaskGRVAL;
		  break;

		  case 's':
		     str = strtoul(cp,&ep,0); cp = ep;
		     if (str < TimLibSTARTS) {
			ccv.Start = str;
			ccm |= TimLibCcvMaskSTART;
			break;
		     }
		     fprintf(stderr,"timtest:Error:No such start: %d\n",(int) str);
		  break;

		  case 'k':
		     clk = strtoul(cp,&ep,0); cp = ep;
		     if (clk < TimLibCLOCKS) {
			ccv.Clock = clk;
			ccm |= TimLibCcvMaskCLOCK;
			break;
		     }
		     fprintf(stderr,"timtest:Error:No such counter Clock: %d\n",(int) clk);
		  break;

		  case 'o':
		     mde = strtoul(cp,&ep,0); cp = ep;
		     if (mde < TimLibMODES) {
			ccv.Mode = mde;
			ccm |= TimLibCcvMaskMODE;
			break;
		     }
		     fprintf(stderr,"timtest:Error:No such counter Mode: %d\n",(int) mde);
		  break;

		  case 'w':
		     plw = strtoul(cp,&ep,0); cp = ep;
		     ccv.PulsWidth = plw;
		     ccm |= TimLibCcvMaskPWIDTH;
		  break;

		  case 'v':
		     dly = strtoul(cp,&ep,0); cp = ep;
		     ccv.Delay = dly;
		     ccm |= TimLibCcvMaskDELAY;
		  break;

		  case 'e':
		     enb = strtoul(cp,&ep,0); cp = ep;
		     ccv.Enable = enb;
		     ccm |= TimLibCcvMaskENABLE;
		  break;

		  case 'p':
		     pol = strtoul(cp,&ep,0); cp = ep;
		     if (pol == 1) ccv.Polarity = TimLibPolarityTTL_BAR;
		     else          ccv.Polarity = TimLibPolarityTTL;
		     ccm |= TimLibCcvMaskPOLARITY;
		  break;

		  case 'q':
		     oms = strtoul(cp,&ep,0);
		     if (cp == ep) oms = 1 << cnt;
		     cp = ep;
		     ccv.OutputMask = oms & 0xFFF;
		     ccm |= TimLibCcvMaskOMASK;
		  break;

		  default: break;
	       }
	    }
	 }
      }
   }
}

/*****************************************************************/

static char *eptim_help =

"<CrLf>                   Next PTIM equipment   \n"
"/<Id>                    Go to entry PTIM=Id   \n"
"?                        Print this help text  \n"
".                        Exit from the editor  \n"
"a                        Edit actions          \n"
"y<Id>,<Mod><Cntr>,<Size> Create PTIM equipment \n";

#define PSIZE 256

void EditPtim(int id) {

unsigned long ptimlist[PSIZE];
int i, n, ix;
unsigned long psize, mod, cnt, dim, nid;
char str[128], c, *cp, *ep;
TimLibCcv ccv;
TimLibCcvMask ccm;

   bzero((void *) &ccv, sizeof(TimLibCcv));
   ccm = 0;
   psize = 0;

   if (CheckErr(TimLibGetAllPtimObjects(ptimlist,&psize,PSIZE))) {

      if (id == 0) {
	 ix = 0;
	 id = ptimlist[ix];
      } else {
	 for (ix=0; ix<psize; ix++) {
	    if (id == ptimlist[ix]) break;
	 }
	 if (ix >= psize) {
	    printf("No such PTIM: %d\n",id);
	    return;
	 }
      }
      while (1) {

	 if (psize == 0) printf(">>>Ptm:None defined : ");
	 else {
	    if (CheckErr(TimLibGetPtimObject(id,&mod,&cnt,&dim)) == 0) {
	       printf("Error:%d:Ptm:%d Mo:%d Ch:%d Sz:%d : ",
		      ix+1,id,(int) mod,(int) cnt,(int) dim);
	       printf("\n");
	       return;
	    }
	    printf("%02d:Ptm:%d:%s [%s] Mo:%d Ch:%d Sz:%d : ",
		   ix+1,
		   id,
		   GetPtmName(id,1),
		   ptm_comment_txt,
		   (int) mod,
		   (int) cnt,
		   (int) dim);
	    fflush(stdout);
	 }

	 bzero((void *) str, 128); n = 0; c = 0;
	 cp = ep = str;
	 while ((c != '\n') && (n < 128)) c = str[n++] = (char) getchar();

	 while (*cp != 0) {

	    switch (*cp++) {

	       case '\n':
		  ix++;
		  if (ix>=psize) {
		     ix = 0;
		     printf("\n");
		  }
		  id = ptimlist[ix];
	       break;

	       case '/':
		  nid = strtoul(cp,&ep,0);
		  for (i=0; i<psize; i++) {
		     if (ptimlist[i] == nid) {
			ix = i;
			id = nid;
			break;
		     }
		  }
		  if (cp != ep) {
		     cp = ep; *cp = 0;
		  }
	       break;

	       case '.': return;

	       case '?':
		  printf("%s\n",eptim_help);
	       break;

	       case 'a':
		  EditCcvs(id);
	       break;


	       case 'y':
		  nid = strtoul(cp,&ep,0);
		  if (cp == ep) {
		     fprintf(stderr,"timtest:Error:No PTIM ID defined\n");
		     break;
		  }
		  cp = ep;

		  mod = strtoul(cp,&ep,0);
		  if (cp == ep) {
		     fprintf(stderr,"timtest:Error:No Mod defined\n");
		     break;
		  }
		  cp = ep;
		  if (mod == 0) mod = module;

		  cnt = strtoul(cp,&ep,0);
		  if (cp == ep) {
		     fprintf(stderr,"timtest:Error:No Cntr defined\n");
		     break;
		  }
		  cp = ep;

		  dim = strtoul(cp,&ep,0);
		  if (cp == ep) dim = 1;
		  cp = ep;

		  for (i=0; i<psize; i++) {
		     if (nid == ptimlist[i]) {
			fprintf(stderr,"timtest:Error:That PTIM:%d already exists\n",(int) nid);
			return;
		     }
		  }

		  if (CheckErr(TimLibCreatePtimObject(nid,mod,cnt,dim))) {
		     printf(">>>Ptm:%d Mo:%d Ch:%d Sz:%d Created\n",
			    (int) nid,(int) mod,(int) cnt,(int) dim);
		     ptimlist[psize] = nid;
		     psize++;
		     ix++;
		     id = ptimlist[ix];

		     CheckErr(TimLibGet(nid,1,0,0,&ccm,&ccv));
		     ccv.Enable     = TimLibCcvDEFAULT_ENABLE;
		     ccv.Start      = TimLibCcvDEFAULT_START;
		     ccv.Mode       = TimLibCcvDEFAULT_MODE;
		     ccv.Clock      = TimLibCcvDEFAULT_CLOCK;
		     ccv.PulsWidth  = TimLibCcvDEFAULT_PULSE_WIDTH;
		     ccv.Delay      = TimLibCcvDEFAULT_DELAY;
		     ccv.OutputMask = 1<<cnt;
		     ccv.Polarity   = TimLibCcvDEFAULT_POLARITY;
		     ccv.Ctim       = 0;
		     ccv.Payload    = TimLibCcvDEFAULT_PAYLOAD;

		     ccv.Machine    = TimLibCcvDEFAULT_MACHINE;
		     ccv.GrNum      = 0;
		     ccv.GrVal      = TimLibCcvDEFAULT_GRVAL;

		     for (i=0; i<dim; i++) {
			if (CheckErr(TimLibSet(nid,i+1,0,0,ccm,&ccv))) {
			   ccv.GrVal++;
			} else {
			   fprintf(stderr,"timtest:Error:Can't set ccv for PTIM:%d Line%d\n",(int) nid,i+1);
			   break;
			}
		     }
		  } else {
		     fprintf(stderr,"timtest:Error:Cant create PTIM:%d\n",(int) nid);
		     return;
		  }
	       break;

	       default: ;
	    }
	 }
      }
   }
}

/*****************************************************************/

static char *erem_help =

"<CrLf>                 Next remote counter   \n"
"/<Counter Number>      Open counter for edit  \n"
"?                      Print this help text  \n"
".                      Exit from the editor  \n"
"s<Start>               Change Start                1KHz/Ext1/Ext2/Chnd/Self/Remt/Pps/Chnd+Stop \n"
"k<Clock>               Change Clock                1KHz/10MHz/40MHz/Ext1/Ext2/Chnd             \n"
"o<Mode>                Change Mode                 Once/Mult/Brst/Mult+Brst                    \n"
"w<Pulse Width>         Change Pulse Width          Pulse Width                                 \n"
"v<Delay>               Change Delay                Delay                                       \n"
"e<enable>              Change OnZero behaviour     NoOut/Bus/Out/OutBus                        \n"
"p<polarity>            Change polarity             TTL/TTL_BAR/TTL                             \n"
"q<outmask>             Change output mask          1<<cntr 0x200/40MHz 0x400/ExCk1 0x800/ExCk2 \n";

static void EditRemote(unsigned long mod, unsigned long cnt) {

TimLibCcvMask ccm;
TimLibCcv ccv;

int i, n;
char c, *cp, *ep;
char txt[TXT_SZ];
unsigned long str, clk, mde, plw, dly, enb, pol, oms, rfl;

   while (1) {
      if (CheckErr(TimLibGetRemote(mod,cnt,&rfl,&ccm,&ccv))) {
	 if (rfl) {
	    if (CcvToStr(ccm,&ccv,0)) {
	       printf("Cntr:%d %s : ",(int) cnt, CcvStr);
	       fflush(stdout);
	    } else return;
	 } else {
	    printf("Cntr:%d Mod:%d Under CTR-Trigger control\n",(int) cnt,(int) module);
	    cnt++;
	    if (cnt>8) return;
	    continue;
	 }

	 bzero((void *) txt, TXT_SZ); n = 0; c = 0;
	 cp = ep = txt;
	 while ((c != '\n') && (n < TXT_SZ -1)) c = txt[n++] = (char) getchar();

	 ccm = 0;
	 while (*cp != 0) {
	    switch (*cp++) {

	       case '\n':
		  if (ccm) if (!CheckErr(TimLibRemoteControl(rfl,mod,cnt,0,ccm,&ccv))) return;
		  if (n==1) {
		     cnt++;
		     if (cnt > 8) {
			cnt = 1;
			printf("\n");
		     }
		  }
	       break;

	       case '/':
		  i = strtoul(cp,&ep,0);
		  if (cp != ep) {
		     cp = ep;
		     if ((i>0) && (i<=8)) cnt = i;
		  }
	       break;

	       case '?':
		  printf("%s\n",erem_help);
	       break;

	       case '.': return;

	       case 's':
		  str = strtoul(cp,&ep,0); cp = ep;
		  if (str < TimLibSTARTS) {
		     ccv.Start = str;
		     ccm |= TimLibCcvMaskSTART;
		     break;
		  }
	       break;

	       case 'k':
		  clk = strtoul(cp,&ep,0); cp = ep;
		  if (clk < TimLibCLOCKS) {
		     ccv.Clock = clk;
		     ccm |= TimLibCcvMaskCLOCK;
		     break;
		  }
	       break;

	       case 'o':
		  mde = strtoul(cp,&ep,0); cp = ep;
		  if (mde < TimLibMODES) {
		     ccv.Mode = mde;
		     ccm |= TimLibCcvMaskMODE;
		     break;
		  }
	       break;

	       case 'w':
		  plw = strtoul(cp,&ep,0); cp = ep;
		  ccv.PulsWidth = plw;
		  ccm |= TimLibCcvMaskPWIDTH;
	       break;

	       case 'v':
		  dly = strtoul(cp,&ep,0); cp = ep;
		  ccv.Delay = dly;
		  ccm |= TimLibCcvMaskDELAY;
	       break;

	       case 'e':
		  enb = strtoul(cp,&ep,0); cp = ep;
		  ccv.Enable = enb;
		  ccm |= TimLibCcvMaskENABLE;
	       break;

	       case 'p':
		  pol = strtoul(cp,&ep,0); cp = ep;
		  if (pol == 1) ccv.Polarity = TimLibPolarityTTL_BAR;
		  else          ccv.Polarity = TimLibPolarityTTL;
		  ccm |= TimLibCcvMaskPOLARITY;
	       break;

	       case 'q':
		  oms = strtoul(cp,&ep,0);
		  if (cp == ep) oms = 1 << cnt;
		  cp = ep;
		  ccv.OutputMask = oms & 0xFFF;
		  ccm |= TimLibCcvMaskOMASK;
	       break;

	       default: break;
	    }
	 }
      } else break;
   }
}

/*****************************************************************/

int Config(int arg) { /* Remote flag */

   arg++;

   EditRemote(module,counter);
   return arg;
}

/*****************************************************************/

int GetSetRemote(int arg) { /* Remote flag */
ArgVal   *v;
AtomType  at;

unsigned long rfl;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;
	 printf("[<Flag:0/CTR-Trigger 1/Remote>]\n");
      }
      return arg;
   }

   if (at == Numeric) {
      arg++;
      rfl = v->Number;
      CheckErr(TimLibRemoteControl(rfl,module,counter,0,0,NULL));
   }

   if (CheckErr(TimLibGetRemote(module,counter,&rfl,NULL,NULL))) {
      if (rfl == 0) printf("Cntr:%d Mod:%d Under CTR-Trigger control\n",(int) counter,(int) module);
      else          printf("Cntr:%d Mod:%d Under Remote control\n",(int) counter,(int) module);
      return arg;
   }
   return arg;
}

/*****************************************************************/

#define REM_CMDS 5
static char *RemNames[REM_CMDS] = {"Load","Stop","Start","OutNow","BusNow"};

int SetRemoteCmd(int arg) { /* Remote flag */
ArgVal   *v;
AtomType  at;

int i;
unsigned long msk, rcm, rfl;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Cmd:\n");
	 for (i=0; i<REM_CMDS; i++) {
	    msk = 1 << i;
	    if (msk & TimLibRemoteBITS) {
	       printf("0x%02X %s\n",(int) msk,RemNames[i]);
	    } else break;
	 }
      }
      return arg;
   }

   rcm = 0;
   if (at == Numeric) {
      rcm = v->Number & TimLibRemoteBITS;
      arg++;
      v = &(vals[arg]);
      at = v->Type;
   }

   if (CheckErr(TimLibGetRemote(module,counter,&rfl,NULL,NULL))) {
      if (rfl) {
	 if (CheckErr(TimLibRemoteControl(rfl,module,counter,rcm,0,NULL))) {
	    printf("Send:Cntr:%d Mod:%d Cmd:0x%02X OK\n",(int) counter,(int) module,(int) rcm);
	    return arg;
	 }
      } else fprintf(stderr,"timtest:Error:Cntr:%d Mod:%d Not in REMOTE\n",(int) counter,(int) module);
   }
   return arg;
}

/*****************************************************************/

#define C_EVENT 911

int ConnectCTime(int arg) { /* C-Time */
ArgVal   *v;
AtomType  at;

int ctime;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   ctime = 0;
   if (at == Numeric) {
      arg++;
      ctime = v->Number;
   }
   if (CheckErr(TimLibConnectPayload(C_EVENT,ctime,module))) {
      connected++;
      printf("Connected to C-%d\n",ctime);
   }

   return arg;
}

/*****************************************************************/

int GetSetPtim(int arg) { /* PTIM ID */
ArgVal   *v;
AtomType  at;

int ptim;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   ptim = 0;
   if (at == Numeric) {
      arg++;
      ptim = v->Number;
   }
   EditPtim(ptim);
   return arg;
}

/*****************************************************************/

#define CSIZE 1024
unsigned long ctimlist[CSIZE], csize;

static char *ectim_help =

"<CrLf>        Next CTIM equipment    \n"
"/<Index>      Go to entry Index      \n"
"?             Print this help text   \n"
".             Exit from the editor   \n"
"f<Frame>      Change  CTIM Frame     \n"
"x             Disable CTIM equipment \n"
"y<Id>,<Frame> Create  CTIM equipment \n";

void EditCtim(int id) {

char c, *cp, *ep, str[128];
int n, i, ix, nadr;
unsigned long eqp, frm, xfrm;
char comment[128];

   if (!CheckErr(TimLibGetAllCtimObjects(ctimlist,&csize,CSIZE))) {
       printf("Error: Can't get CTIM list\n");
       return;
   }

   if (id) {
      ix = -1;
      for (i=0; i<csize; i++) {
	 if (id == ctimlist[i]) {
	    ix = i;
	    break;
	 }
      }
      if (ix < 0) {
	 printf("Error: No such CTIM equipment: %d\n",id);
	 return;
      }
   } else ix = 0;

   i = ix;

   while (1) {

      if (!CheckErr(TimLibGetAllCtimObjects(ctimlist,&csize,CSIZE))) {
	  printf("Error: Can't get CTIM list\n");
	  return;
      }

      if (csize) {

	 eqp = ctimlist[i];
	 if (!CheckErr(TimLibGetCtimObject(eqp,&frm))) {
	    printf("Error: Cant get CTIM Object: %d\n",(int) eqp);
	    return;
	 }

	 bzero((void *) comment,128);
	 if ((frm & 0xFFFF0000) == 0x01000000) {
	    strcpy(comment,"Millisecond C-Event");
	 } else {
	    if (frm == 0) strcpy(comment,"Disabled");
	    else if ((xfrm = TgvGetFrameForMember(eqp))) {
	       if (frm == xfrm) TgvFrameExplanation(frm,comment);
	       else {
		  sprintf(comment,
			  "%s: Modified",
			  TgvFrameExplanation(xfrm,str));
	       }
	    } else {
	       strcpy(comment,"Locally Defined");
	    }
	 }
	 printf("[%d]Ctm:%d Fr:0x%08X ;%s\t: ",
		      i,
		(int) eqp,
		(int) frm,
		      comment);

      } else
	 printf(">>>Ctm:None defined : ");

      fflush(stdout);

      bzero((void *) str, 128); n = 0; c = 0;
      cp = ep = str;
      while ((c != '\n') && (n < 128)) c = str[n++] = (char) getchar();

      while (*cp != 0) {

	 switch (*cp++) {

	    case '\n':
	       if (n<=1) {
		  i++;
		  if (i>=csize) {
		     i = ix;
		     printf("\n------------\n");
		  }
	       }
	    break;

	    case '/':
	       i = ix;
	       nadr = strtoul(cp,&ep,0);
	       if (cp != ep) {
		  if (nadr < csize) i = nadr;
		  cp = ep;
	       }
	    break;

	    case '.': return;

	    case '?':
	       printf("%s\n",ectim_help);
	    break;

	    case 'f':
	       frm = strtoul(cp,&ep,0);
	       if (cp == ep) break;
	       cp = ep;
	       if (!CheckErr(TimLibCreateCtimObject(eqp,frm))) {
		  printf("Error: Cant create/modify CTIM object: %d\n",(int) eqp);
		  return;
	       }
	       printf(">>>Ctm:%d Fr:0x%08X : Frame changed\n",
		      (int) eqp,
		      (int) frm);
	    break;

	    case 'x':
	       if (!CheckErr(TimLibCreateCtimObject(eqp,0))) {
		  printf("Error: Cant disable CTIM object: %d\n",(int) eqp);
		  return;
	       }
	       printf(">>>Ctm:%d Fr:0x%08X : Disabled\n",
		      (int) eqp,
		      (int) frm);
	       if (ix>=(csize-1)) ix=0;
	       i = ix;
	    break;

	    case 'y':
	       eqp = strtoul(cp,&ep,0);
	       if (cp == ep) break;
	       cp = ep;
	       frm = strtoul(cp,&ep,0);
	       if (cp == ep) break;
	       cp = ep;
	       if (!CheckErr(TimLibCreateCtimObject(eqp,frm))) {
		  printf("Error: Cant create CTIM object: %d\n",(int) eqp);
		  return;
	       }
	       printf(">>>Ctm:%d Fr:0x%08X : Created\n",
		      (int) eqp,
		      (int) frm);
	       i = csize;
	    break;

	    default: ;
	 }
      }
   }
}

/*****************************************************************/

int GetSetCtim(int arg) { /* CTIM ID */
ArgVal   *v;
AtomType  at;

unsigned long ctim;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   ctim = 0;
   if (at == Numeric) {
      arg++;
      ctim = v->Number;
   }

   EditCtim(ctim);

   return arg;
}

/*****************************************************************/

int LineNames(int arg) {
ArgVal   *v;
AtomType  at;

int gn, m, j, msk;
TgmGroupDescriptor desc;
TgmLineNameTable ltab;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;
	 for (m=0; m<TgmMACHINES; m++) {
	    printf("%d/%s ",(int) m,(char *) TgmGetMachineName(m));
	 }
	 printf("\n");
      }
      return arg;
   }

   m = (int) tmch;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      m = v->Number;
   }
   if (at == Alpha) {
      for (m=0; m<TgmMACHINES; m++) {
	 if (strcmp(TgmGetMachineName(m),v->Text) == 0) {
	    arg++;
	    break;
	 }
      }
   }

   if ((m >=0 ) && (m <= TgmMACHINES)) {
      tmch = (TgmMachine) m;
      TgmAttach(m,TgmTELEGRAM | TgmLINE_NAMES);
      printf("\n%s Telegram Layout:\n",TgmGetMachineName(m));
      for (gn=1; gn<=TgmLastGroupNumber(m); gn++) {
	 TgmGetGroupDescriptor(m,gn,&desc);
	 printf("Group:%02d:%8s:",gn,desc.Name);
	 if (desc.Type == TgmNUMERIC_VALUE) {
	    printf("Numeric:[%d..%05d]Default:%d\t#%s",
		   desc.Minimum,
		   desc.Maximum,
		   desc.Default,
		   desc.Comment);
	    if (desc.Name[0] == 'N') printf(" {NEXT}");
	    printf("\n");
	 } else if (desc.Type == TgmBIT_PATTERN) {
	    TgmGetLineNameTable(m,desc.Name,&ltab);
	    printf("BitPatn:[%d..%05d]Default:%d\t#%s",
		   desc.Minimum,
		   desc.Maximum,
		   desc.Default,
		   desc.Comment);
	    if (desc.Name[0] == 'N') printf(" {NEXT}\n");
	    else {
	       printf("\n");
	       for (j=0; j<ltab.Size; j++) {
		  msk = 1 << j;
		  printf("   0x%04X) AND %8s\t#%s\n",
			 msk,
			 ltab.Table[j].Name,
			 ltab.Table[j].Comment);
	       }
	       printf("\n");
	    }
	 } else {
	    TgmGetLineNameTable(m,desc.Name,&ltab);
	    printf("Exclusv:[%d..%05d]Default:%d\t#%s",
		   desc.Minimum,
		   desc.Maximum,
		   desc.Default,
		   desc.Comment);
	    if (desc.Name[0] == 'N') printf(" {NEXT}\n");
	    else {
	       printf("\n");
	       for (j=0; j<ltab.Size; j++) {
		  printf("   %02d) EQU %8s\t#%s\n",
			 j+1,
			 ltab.Table[j].Name,
			 ltab.Table[j].Comment);
	       }
	       printf("\n");
	    }
	 }
      }
      return arg;
   }

   printf("No such TGM machine\n");
   return arg;
}

/*****************************************************************/

int WaitTelegram(int arg) {
ArgVal   *v;
AtomType  at;

int m;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;
	 for (m=0; m<TgmMACHINES; m++) {
	    printf("%d/%s ",(int) m,(char *) TgmGetMachineName(m));
	 }
	 printf("\n");
      }
      return arg;
   }

   m = (int) tmch;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      m = v->Number;
   }
   if (at == Alpha) {
      for (m=0; m<TgmMACHINES; m++) {
	 if (strcmp(TgmGetMachineName(m),v->Text) == 0) {
	    arg++;
	    break;
	 }
      }
   }

   if ((m >=0 ) && (m <= TgmMACHINES)) {
      tmch = (TgmMachine) m;
      TgmAttach(m,TgmTELEGRAM | TgmLINE_NAMES);
      if (TgmWaitForNextTelegram(m) == TgmSUCCESS) {
	 printf("Wait:%s OK\n",(char *) TgmGetMachineName(m));
	 return arg;
      }
   }

   printf("No such TGM machine\n");
   return arg;
}

/*****************************************************************/

int GetTelegram(int arg) {
ArgVal   *v;
AtomType  at;

int i, j, m, msk;
TgmTelegram tgm;
TgmGroupDescriptor desc;
TgmLineNameTable ltab;
unsigned long gv;
char gvs[256];

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;
	 for (m=0; m<TgmMACHINES; m++) {
	    printf("%d/%s ",(int) m,(char *) TgmGetMachineName(m));
	 }
	 printf("\n");
      }
      return arg;
   }

   m = (int) tmch;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      m = v->Number;
   }
   if (at == Alpha) {
      for (m=0; m<TgmMACHINES; m++) {
	 if (strcmp(TgmGetMachineName(m),v->Text) == 0) {
	    arg++;
	    break;
	 }
      }
   }

   if ((m >=0 ) && (m <= TgmMACHINES)) {
      tmch = (TgmMachine) m;
      printf("%d/%s Telegram\n",(int) m,(char *) TgmGetMachineName(m));
      if (CheckErr(TimLibGetTelegram(0,m,&tgm))) {
	 for (i=0; i<TgmLastGroupNumber(m); i++) {
	    bzero((void *) gvs,256);
	    gv = TgmGetGroupValueFromTelegram(i+1,&tgm);
	    TgmGetGroupDescriptor(m,i+1,&desc);
	    if (desc.Type == TgmNUMERIC_VALUE) {
	       strcat(gvs,":");
	    } else if (desc.Type == TgmBIT_PATTERN) {
	       TgmGetLineNameTable(m,desc.Name,&ltab);
	       for (j=0; j<ltab.Size; j++) {
		  msk = 1 << j;
		  if (msk & gv) {
		     strcat(gvs,":");
		     strcat(gvs,ltab.Table[j].Name);
		  }
	       }
	    } else {
	       TgmGetLineNameTable(m,desc.Name,&ltab);
	       strcat(gvs,":");
	       strcat(gvs,ltab.Table[gv-1].Name);
	    }
	    printf("Gn:%02d[%8s] Gv:%05d:0x%04X[%s]\n",
		   i+1,
		   desc.Name,
		   (int) gv,
		   (int) gv,
		   gvs);
	 }
      }
   }
   return arg;
}

/******************************************************/

int ShowUserMat(int arg) {
ArgVal   *v;
AtomType  at;

int i, j, m, gn;
char *cp, lnam[16];
TgmGroupDescriptor desc;
TgmLineNameTable ltab;
TgmGroupBinding *gbnd;
TgmUserMatrix umat;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;
	 for (m=0; m<TgmMACHINES; m++) {
	    printf("%d/%s ",(int) m,(char *) TgmGetMachineName(m));
	 }
	 printf("\n");
      }
      return arg;
   }

   m = (int) tmch;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      m = v->Number;
   }
   if (at == Alpha) {
      for (m=0; m<TgmMACHINES; m++) {
	 if (strcmp(TgmGetMachineName(m),v->Text) == 0) {
	    arg++;
	    break;
	 }
      }
   }

   if ((m >=0 ) && (m <= TgmMACHINES)) {
      tmch = (TgmMachine) m;
      TgmAttach(m,TgmTELEGRAM | TgmUSER_MATRIX | TgmLINE_NAMES);
      printf("\n%d/%s User Matrix\n",(int) m,(char *) TgmGetMachineName(m));
      printf("-------------\n");

      gn = TgmGetGroupNumber(m,"USER");
      TgmGetGroupDescriptor(m,gn,&desc);
      TgmGetLineNameTable(m,desc.Name,&ltab);

      for (i=0; i<ltab.Size; i++) {
	 TgmGetUserMatrix(m,ltab.Table[i].Name,&umat);
	 if (umat.Width) {
	    printf("\nUser:%d:%s\tWidth:%d (BP)\n",i+1,ltab.Table[i].Name,umat.Width);
	    for (j=0; j<umat.Size; j++) {
	       gbnd = &(umat.Table[j]);
	       gn = TgmGetGroupNumber(m,gbnd->Name);
	       TgmGetGroupDescriptor(m,gn,&desc);
	       strcpy(lnam,"---");
	       if (desc.Type == TgmEXCLUSIVE) {
		  cp = (char *) TgmGetLineName(m,gbnd->Name,gbnd->Value);
		  if (cp) {
		     strcpy(lnam,cp);
		     free(cp);
		  }
	       }
	       printf("Group:%d:%s\tValue:%d:%s\n",gn,gbnd->Name,gbnd->Value,lnam);
	    }
	    printf("-------------\n");
	 }
      }
   }
   return arg;
}

/******************************************************/

int Video(int arg) {
ArgVal   *v;
AtomType  at;

int m;
unsigned long cbl;
char txt[128];
TgvName cblnam;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;
	 for (m=0; m<TgmMACHINES; m++) {
	    printf("%d/%s ",(int) m,(char *) TgmGetMachineName(m));
	 }
	 printf("\n");
      }
      return arg;
   }

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      m = v->Number;
   }

   TimLibGetCableId(module,&cbl);
   if (at == Alpha) {
      for (m=0; m<TgmMACHINES; m++) {
	 if (strcmp(TgmGetMachineName(m),v->Text) == 0) {
	    arg++;
	    break;
	 }
      }
   } else m = (int) TgvTgvToTgmMachine(TgvFirstMachineForCableId(cbl));

   sprintf(txt,"%s %s",GetFile("video"),(char *) TgmGetMachineName(m));
   if (TgvGetCableName(cbl,&cblnam)) {
      printf("Launching:%s\n",txt);
      Launch(txt);
   } else {
      fprintf(stderr,"Can't launch: %s\n",txt);
      printf("This DSC is not correctly configured for machine:%s\n",TgmGetMachineName(m));
   }
   return arg;
}

/*****************************************************************/

int CtimRead(int arg) {

int ccnt, cfal;
unsigned long equip, frame;
TgvName eqname;

   arg++;

   ccnt = 0; cfal = 0; equip = TgvFirstGMember();
   while (equip) {
      TgvGetNameForMember(equip,&eqname);
      frame = TgvGetFrameForMember(equip);
      if (CheckErr(TimLibCreateCtimObject(equip,frame))) {
	 printf("Created: CTIM: %s (%04d) 0x%08X OK\n",eqname,(int) equip, (int) frame);
	 ccnt++;
      } else {
	 printf("Already exists: %s\n",eqname);
	 cfal++;
      }
      equip = TgvNextGMember();
   }
   if (ccnt) printf("Created: %d CTIM equipments OK\n",ccnt);
   if (cfal) printf("Existed: %d CTIM equipments\n"   ,cfal);

   return arg;
}

/*****************************************************************/

#ifndef CERN_VENDOR_ID
#define CERN_VENDOR_ID 0x10DC
#endif

#ifndef CTRP_DEVICE_ID
#define CTRP_DEVICE_ID 0x0300
#endif

int LaunchHwTest(int arg) {

int ctr;
char txt[128];
unsigned long moad[8];

   arg++;

   switch (Dev) {

      case TimLibDevice_CTR:
	 TimLibGetHandle(&ctr);
	 ioctl(ctr,CtrDrvrGET_MODULE_DESCRIPTOR,&moad);
	 if ((moad[3] == CTRP_DEVICE_ID)
	 &&  (moad[4] == CERN_VENDOR_ID))
	    sprintf(txt,"xterm 2>/dev/null -e %s",GetFile("ctrtest"));
	 else
	    sprintf(txt,"xterm 2>/dev/null -e %s",GetFile("ctrvtest"));
	 printf("Launching:%s\n",txt);
	 Launch(txt);
      break;

      case TimLibDevice_TG8_CPS:
	 sprintf(txt,"xterm 2>/dev/null -e %s",GetFile("tg8test"));
	 printf("Launching:%s\n",txt);
	 Launch(txt);
      break;

      case TimLibDevice_TG8_SPS:
	 sprintf(txt,"xterm 2>/dev/null -e %s",GetFile("tg8test-sl"));
	 printf("Launching:%s\n",txt);
	 Launch(txt);
      break;

      default:
	 printf("No network test diagnostic tool available for device: %s\n",DevNames[(int) Dev]);
   }
   return arg;
}

/*****************************************************************/

int LaunchLookat(int arg) {
ArgVal   *v;
AtomType  at;

char txt[128], eqn[16];
int xquip;
TimLibClass xclss;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   xquip = -1;
   xclss = TimLibClassPTIM;

   sprintf(txt,"%s %d ",GetFile("TimLookat"),(int) module);

   if (at == Numeric) {

      xquip = v->Number;
      sprintf(eqn,"P %d",(int) xquip);

      arg++;
      v = &(vals[arg]);
      at = v->Type;
   } else

   if (at == Alpha) {
      if ((v->Text[0] == 'P') || (v->Text[0] == 'p')) { /* Ptim equipment ? */
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 xclss = TimLibClassPTIM;

	 if (at == Numeric) xquip = v->Number;
	 sprintf(eqn,"P %d",(int) xquip);

      } else if ((v->Text[0] == 'C') || (v->Text[0] == 'c')) { /* Ctim equipment ? */
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 xclss = TimLibClassCTIM;

	 if (at == Numeric) xquip= v->Number;
	 sprintf(eqn,"C %d",(int) xquip);

      } else if ((v->Text[0] == 'H') || (v->Text[0] == 'h')) { /* Hardware equipment ? */
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 xclss = TimLibClassHARDWARE;

	 if (at == Numeric) xquip= v->Number;
	 sprintf(eqn,"H 0x%X",(int) xquip);

      }
   }
   if (xquip != -1) {
      strcat(txt,eqn);
      strcat(txt," ");
      strcat(txt,DevNames[Dev]);
      printf("Launching:%s\n",txt);
      Launch(txt);
      return arg;
   }

   printf("Command syntax for LKM: lkm P|C|H <eqn>\n");

   return arg;
}

/*****************************************************************/

int LaunchClock(int arg) {
ArgVal   *v;
AtomType  at;

char txt[128];
int mod;


   arg++;
   v = &(vals[arg]);
   at = v->Type;

   mod = module;
   if (at == Numeric) {
      mod = v->Number;

      arg++;
      v = &(vals[arg]);
      at = v->Type;
   }

   sprintf(txt,"%s %d %s",GetFile("TimClock"), mod, DevNames[Dev]);

   printf("Launching:%s\n",txt);
   Launch(txt);
   return arg;
}

/*****************************************************************/

int ParseCycleString(int arg) {
ArgVal   *v;
AtomType  at;
char cstr[128], *cp;
int n, earg;
unsigned long slix;
TimLibError err;

   arg++;
   for (earg=arg; earg<pcnt; earg++) {
      v = &(vals[earg]);
      if ((v->Type == Close)
      ||  (v->Type == Terminator)) break;
   }

   v = &(vals[arg]);
   at = v->Type;
   if ((v->Type != Close)
   &&  (v->Type != Terminator)) {

      bzero((void *) cstr, 128);

      n = 0;
      cp = &(cmdbuf[v->Pos]);
      do {
	 at = atomhash[(int) (*cp)];
	 if ((at != Seperator)
	 &&  (at != Close)
	 &&  (at != Terminator))
	    cstr[n++] = *cp;
	 cstr[n] = 0;
	 cp++;
      } while ((at != Close) && (at != Terminator));
   }

   err = TimLibStringToSlot(cstr,&slix);
   if (err != TimLibErrorSUCCESS) {
      printf("Error:%s; Can't parse:%s\n",TimLibErrorToString(err),cstr);
      return earg;
   }
   printf("%s Converts to Slot:%d\n",cstr,(int) slix);
   return earg;
}

/*****************************************************************/

static char *LemoName[] = {"CH1","CH2","CH3","CH4","CH5","CH6","CH7","CH8","ST1","ST2","CK1","CK2"};

int DoIo(int arg) {
ArgVal   *v;
AtomType  at;

unsigned long lemos, lmask;
TimLibError err;
int i;
char txt[128];

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   lemos = 0; lmask = 0;

   if (at == Numeric) {
      lemos = (TimLibLemo) v->Number;

      arg++;
      v = &(vals[arg]);
      at = v->Type;

      if (at == Numeric) {
	 lmask = (TimLibLemo) v->Number;

	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 sprintf(txt,"Lemos:0x%X Mask:0x%X",(int) lemos, (int) lmask);
	 if (YesNo("WARNING:Set CTR Output polarities:",txt)) {
	    err = TimLibSetOutputs(module,lemos,lmask);
	    if (err != TimLibErrorSUCCESS) {
	       printf("Error:%s; Can't Set CTR Outputs:%s\n",TimLibErrorToString(err),txt);
	       return arg;
	    }
	 }
      }
   }

   err = TimLibGetIoStatus(module,(TimLibLemo *) &lemos);
   if (err != TimLibErrorSUCCESS) {
      printf("Error:%s; Can't get CTR Inputs\n",TimLibErrorToString(err));
      return arg;
   }

   printf("CtrLemo Logic Levels are:0x%03x\n",(int) lemos);
   for (i=0; i<12; i++) {
      lmask = 1 << i;
      if (lmask & lemos) printf("%s:5V ",LemoName[i]);
      else               printf("%s:0V ",LemoName[i]);
   }
   printf("\n");
   return arg;
}

/*****************************************************************/

int GetSetPll(int arg) {
ArgVal   *v;
AtomType  at;
unsigned long pllflag;
TimLibModuleStats stats;
TimLibError ter;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;
	 printf("PLL Locking: 0=>SLOW, 1=>BRUTAL\n");
      }
      return arg;
   }

   if (at == Numeric) {
      pllflag = v->Number;

      arg++;
      v = &(vals[arg]);
      at = v->Type;

      if ((pllflag != 0) && (pllflag != 1)) {
	 printf("Error:Illegal PLL method[0..1]:%d\n",(int) pllflag);
	 return arg;
      }

      ter = TimLibSetPllLocking(module,pllflag);
      if (ter != TimLibErrorSUCCESS) {
	 printf("Error:%s; Can't set PLL locking\n",TimLibErrorToString(ter));
	 return arg;
      }
   }

   ter = TimLibGetModuleStats(module,&stats);
   if (ter != TimLibErrorSUCCESS) {
      printf("Error:%s; Can't read module ststistice\n",TimLibErrorToString(ter));
      return arg;
   }

   if (stats.Cst.Valid == 0) {
      printf("Error:Pll lock control not available on module\n");
      return arg;
   }

   printf("Cst.Stat:0x%X %s\n",
	  (int) stats.Cst.Stat,
	  MStatToStr(stats.Cst.Stat));

   if (stats.Cst.Stat & TimLibCstStatUtcPllEnabled)
      printf("PLL Slow locking\n");
   else
      printf("PLL Brutal locking\n");

   return arg;
}
