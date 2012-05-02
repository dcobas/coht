/* ****************************************************************************** */
/* libctr test program                                                            *
/* Julian Lewis Mon 30th April 2012                                               */
/* ****************************************************************************** */

#include <libctr.h>

/**************************************************************************/

static int counter = 1;
static int module = 1;
static char *editor = "e";

/**************************************************************************/

char *defaultconfigpath = "/dsc/bin/tim/libctrtest.config";

char *configpath = NULL;
char localconfigpath[128];  /* After a CD */

/**************************************************************************/

static char path[128];

char *get_file(char *name) {
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
      configpath = "./libctrtest.config";
      gpath = fopen(configpath,"r");
      if (gpath == NULL) {
	 configpath = "/dsc/local/data/libctrtest.config";
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

int get_news(int arg) {

char sys[128], npt[128];

   arg++;

   if (get_file("tim_news")) {
      strcpy(npt,path);
      sprintf(sys,"%s %s",get_file(editor),npt);
      printf("\n%s\n",sys);
      system(sys);
      printf("\n");
   }
   return(arg);
}

/**************************************************************************/

static int yesno=1;

static int yes_no(char *question, char *name) {
int yn, c;

   if (yesno == 0) return 1;

   printf("%s: %s\n",question,name);
   printf("Continue (Y/N):"); yn = getchar(); c = getchar();
   if ((yn != (int) 'y') && (yn != 'Y')) return 0;
   return 1;
}

/**************************************************************************/
/* Launch a task                                                          */

static void launch_process(char *txt) {
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
/* Convert a ctr_ time in milliseconds to a string routine.             */
/* Result is a pointer to a static string representing the time.          */
/*    the format is: Thu-18/Jan/2001 08:25:14.967                         */
/*                   day-dd/mon/yyyy hh:mm:ss.ddd                         */

volatile char *time_to_string(CtrDrvrTime *t) {

static char tbuf[128];

char tmp[128];
char *yr, *ti, *md, *mn, *dy;

struct timeval utime;

   bzero((void *) tbuf, 128);
   bzero((void *) tmp, 128);

   ctr_time_to_unix(t,&utime);

   if (utime.tv_sec) {
      ctime_r((time_t *) &utime.tv_sec, tmp);
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

      if (utime.tv_usec) {
	  sprintf(&tbuf[strlen(tbuf)],".%06lu",utime.tv_usec);
      }
   }

   return (tbuf);
}

/*****************************************************************/
/* Convert an int to a name string                               */

static char *int_to_str(int idx, int sze, char **names) {
int i;
static char res[128];

   bzero((void *) res,128);
   if (idx < sze) {
      strcat(res,names[idx]);
      strcat(res,":");
   }
   strcat(res,":");
   return res;
}

/*****************************************************************/
/* Convert an enum to a string                                   */

static char *enum_to_str(int msk, int sze, char **names) {
int i;
static char res[128];

   bzero((void *) res,128);
   for (i=0; i<sze; i++) {
      if (msk & 1<<i) {
	 strcat(res,names[i]);
	 strcat(res,":");
      }
   }
   strcat(res,":");
   return res;
}

/**************************************************************************/

char *status_to_string(CtrDrvrStatus sts) {
char res[256];

static char *stat_names[CtrDrvrSTATAE] = {"Gmt", "Pll", "XCk1","XCk2",
					  "Test","Enb", "Htdc","Ctri",
					  "Ctrp","Ctrv","Intr","Bus"};
   bzero((void *) res,256);
   sprintf(res,
	   "SET=> %s\nCLR=> %s",
	   enum_to_str((int)  sts,CtrDrvrSTATAE,stat_names)
	   enum_to_str((int) ~sts,CtrDrvrSTATAE,stat_names)
   return res;
}

/**************************************************************************/

char * io_stat_to_str(CtrDrvrIoStatus iostat) {

static char *io_names[CtrDrvrIoSTATAE] = { "XE",    "XI",    "V1",     "V2",
					   "S1",    "S2",    "X1",     "X2",
					   "O1",    "O2",    "O3",     "O4",
					   "O5",    "O6",    "O7",     "O8",
					   "ID",    "DH",    "SP",     "XM",
					   "TM" };

   return enum_to_str((int) iostat,CtrDrvrIoSTATAE,io_names);
}

/**************************************************************************/

char *interrupt_to_str(CtrDrvrInterruptMask imsk) {

static char *inter_names[CtrDrvrInterruptSOURCES] = {"Ch0","Ch1","Ch2","Ch3",
						     "Ch4","Ch5","Ch6","Ch7",
						     "Ch8","Pll","Gmt","1Hz",
						     "1Kz","Mch" };

   return enum_to_str((int) imsk,CtrDrvrInterruptSOURCES,inter_names);
}

/*****************************************************************/

char *counter_mask_to_str(CtrDrvrCounterMask cmsk) {

static char *cmask_names[CtrDrvrCOUNTER_MASKS] = {"Ctim", "Cntr1","Cntr2","Cntr3",
						  "Cntr4","Cntr5","Cntr6","Cntr7",
						  "Cntr8","40Mh", "ExCk1","ExCk2" };

   return enum_to_str((int) cmsk,CtrDrvrCOUNTER_MASKS,cmask_names);
}

/*****************************************************************/

char *counter_start_to_str(CtrDrvrCounterStart cstrt) {

static char *cstart_names[CtrDrvrCounterSTARTS] = {"Nor",  "Ext1", "Ext2", "Chnd",
						   "Self", "Remt", "Pps",  "Chnd+Stop"};

   return to_str((int) cstrt,CtrDrvrCounterSTARTS,cstart_names);
}

/*****************************************************************/

char *counter_mode_to_str(CtrDrvrCounterMode mode) {

static char *mode_names[CtrDrvrCounterMODES] = {"Once", "Mult", "Brst", "Mult+Brst"};

   return enum_to_str((int) mode,CtrDrvrCounterMODES,mode_names);
}

/*****************************************************************/

char *counter_clock_to_str(CtrDrvrCounterClock cclk) {

static char *clock_names[CtrDrvrCounterCLOCKS] = {"1KHz", "10MHz", "40MHz", "Ext1", "Ext2", "Chnd" };

   return enum_to_str((int) cclk,CtrDrvrCounterCLOCKS,clock_names);
}

/*****************************************************************/

char *polarity_to_str(CtrDrvrPolarity pol) {

#define POLARITIES 2
static char *polarity_names[POLARATIES] = {"TTL","BAR"};

   return int_to_str(pol,POLARITIES,polarity_names);
}

/*****************************************************************/

char *onzero_to_str(CtrDrvrCounterOnZero onz) {

#define ON_ZEROS 2
static char *onzero_names[ON_ZEROS] = {"Bus","Out"};

   return enum_to_str((int) onz,ON_ZEROS,onzero_names);
}

/*****************************************************************/

char *trigger_condition_to_str(CtrDrvrTriggerCondition tcon) {

static char tcon_names[CtrDrvrTriggerCONDITIONS] = {"NoChk", "Equ", "LAnd"};

   return int_to_str((int) tcon,CtrDrvrTriggerCONDITIONS,tcon_names);
}

/*****************************************************************/

char * ccv_to_str(ctr_ccv_s *ccv, ctr_ccv_fields_t flds) {

#define CCV_FIELDS 14

int i, msk, w;

char *cp, tmp[128];
static char res[512];

   bzero((void *) res,512);
   bzero((void *) tmp,128);

   for (i=0; i<CCV_FIELDS; i++) {
      msk = 1 << i;
      if (msk & ccm) {
	 switch (msk) {
	    case CTR_CCV_ENABLE:
	       if (ccv->enable) sprintf(tmp,"Enb");
	       else             sprintf(tmp,"Dis");
	    break;

	    case CTR_CCV_START:
	       sprintf(tmp,"STR:%s",counter_start_to_str(ccv->start));
	    break;

	    case CTR_CCV_MODE:
	       sprintf(tmp,"MOD:%s",counter_mode_to_str(ccv->mode));
	    break;

	    case CTR_CCV_CLOCK:
	       sprintf(tmp,"CLK:%s",counter_clock_to_str(ccv->clock));
	    break;

	    case CTR_CCV_PULSE_WIDTH:
	       w = ccv->pulse_width * 25;
	       if      (w >= 1000000) { w = w/1000000; cp = "ms"; }
	       else if (w >= 1000   ) { w = w/1000;    cp = "us"; }
	       else                   {                cp = "ns"; }
	       sprintf(tmp,"PWD:%d%s",w,cp);
	    break;

	    case CTR_CCV_DELAY:
	       sprintf(tmp,"DLY:%d",ccv->delay);
	    break;

	    case CTR_CCV_COUNTER_MASK:
	       sprintf(tmp,"OMS:%s",counter_mask_to_str(ccv->counter_mask));
	    break;

	    case CTR_CCV_POLARITY:
	       sprintf(tmp,"POL:%s",polarity_to_str(ccv->polarity));
	    break;

	    case CTR_CCV_CTIM:
	       sprintf(tmp,"CTM:%d",ccv->ctim);
	    break;

	    case CTR_CCV_PAYLOAD:
	       sprintf(tmp,"PLD:%d",ccv->payload);
	    break;

	    case CTR_CCV_CMP_METHOD:
	       sprintf(tmp,"CMP:%s",ccv->cmp_method));
	    break;

	    case CTR_CCV_GRNUM:
	       sprintf(tmp,"GNM:%d",,ccv->grnum);
	    break;

	    case CTR_CCV_GRVAL:
	       sprintf(tmp,"GVL:%d",,ccv->grnum);
	    break;

	    case CTR_CCV_TGNUM:
	       sprintf(tmp,"TGM:%d",,ccv->tgnum);
	    break;


	    default:
	    break;
	 }
	 strcat(res,tmp);
	 strcat(res," ");
	 bzero((void *) tmp,128);
	 }
      }
   }
   return res;
}

/*****************************************************************/
/* Commands used in the test program.                            */
/*****************************************************************/

int change_text_editor(int arg) {
static int eflg = 0;

   arg++;
   if (eflg++ > 4) eflg = 1;

   if      (eflg == 1) editor = "e";
   else if (eflg == 2) editor = "emacs";
   else if (eflg == 3) editor = "nedit";
   else if (eflg == 4) editor = "vi";

   printf("Editor: %s\n",get_file(editor));
   return arg;
}

/*****************************************************************/

int change_working_directory(int arg) {
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
      strcat(localconfigpath,"/libctrtest.config");
      if (yes_no("Change libctrtest config to:",localconfigpath))
	 configpath = localconfigpath;
      else
	 configpath = NULL;
   }

   sprintf(txt,"%s %s",get_file(editor),configpath);
   printf("\n%s\n",txt);
   system(txt);
   printf("\n");
   return(arg);
}

/*****************************************************************/

int NextCounter(int arg) {

CtrDrvrHardwareType ht;
int max_ch = 4;

   arg++;
   if (ctr_get_type(h,&ht) < 0)
      perror("ctr_get_type");
      return arg;
   }
   if (ht == CtrDrvrHardwareTypeCTRV) max_ch = 8;
   else if (ht == CtrDrvrHardwareTypeCTRP) max_ch = 3;

   counter++;
   if (counter > max_ch) counter = 1;
   return arg;
}

/*****************************************************************/

int NextModule(int arg) {
int mcnt;

   arg++;
   mcnt = ctr_get_module_count(h);
   if (mcnt < 0) {
      perror("ctr_get_module_count");
      return arg;
   }

   module++;
   if (module > mcnt) module = 1;
   return arg;
}

/*****************************************************************/

int GetStatus(int arg) {
CtrDrvrStatus *stat

   arg++;
   if (ctr_get_status(h,&stat) < 0) {
      perror("ctr_get_status");
      return arg;
   }
   printf("Status:%s\n",status_to_str(stat);
   return arg;
}

/*****************************************************************/

int GetSetCounter(int arg) {
ArgVal   *v;
AtomType  at;

CtrDrvrHardwareType ht;
int max_ch = 4;

   arg++;

   if (ctr_get_type(h,&ht) < 0)
      perror("ctr_get_type");
      return arg;
   }
   if (ht == CtrDrvrHardwareTypeCTRV) max_ch = 8;
   else if (ht == CtrDrvrHardwareTypeCTRP) max_ch = 3;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      cnt = v->Number;
      if ((cnt>0) && (cnt<=max_ch)) counter = cnt;
   }
   printf("Cntr:%d Selected\n",counter);
   return arg;
}

/*****************************************************************/

int GetSetModule(int arg) {
ArgVal   *v;
AtomType  at;

int mcnt;

   arg++;
   mcnt = ctr_get_module_count(h);
   if (mcnt < 0) {
      perror("ctr_get_module_count");
      return arg;
   }

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      mod = v->Number;
      if (mod <= mcnt) module = mod;
   }

   printf("Mod:%d Selected\n",module);
   return arg;
}

/*****************************************************************/

static int debug_level = 0;

int SwDeb(int arg) {
ArgVal   *v;
AtomType  at;

   arg++;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      debug_level = v->Number;
      if (ctr_set_debug_level(h, debug_level) < 0) {
	 perror("ctr_set_debug_level");
	 return arg;
      }
   }
   debug_level = ctr_get_debug_level(h);
   printf("Debg:%d\n",debug_level);
   return arg;
}

/*****************************************************************/

static int time_out = 2000;

int GetSetTmo(int arg) { /* Arg=0 => Timeouts disabled, else Arg = Timeout */
ArgVal   *v;
AtomType  at;

   arg++;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      time_out = v->Number;
      if (ctr_set_timeout(h, time_out) < 0) {
	 perror("ctr_set_timeout");
	 return arg;
      }
   }
   if (ctr_get_timeout(h) < 0) {
      perror("ctr_get_timeout");
      return arg;
   }
   if (time_out > 0) printf("Timo:%dms Enabled\n",(int) time_out);
   else              printf("Timo:Disabled\n");
   return arg;
}

/*****************************************************************/

static int queue_flag = 0;

int GetSetQue(int arg) { /* Arg=Flag */
ArgVal   *v;
AtomType  at;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      queue_flag = v->Number;
      if (ctr_set_queue_flag(h,queue_flag) < 0) {
	 perror("ctr_set_queue_flag");
	 return arg;
      }
   }
   queue_flag = ctr_get_queue_flag(h);
   if (queue_flag < 0) {
      perror("ctr_get_queue_flag");
      return arg;
   }
   if (queue_flag) printf("Qflg:On Interrupts will be queued\n");
   else            printf("Qflg:OFF No interrupt queue\n");
   return arg;
}

/*****************************************************************/

int GetVersion(int arg) {

CtrDrvrTime t;
CtrDrvrVersion dver;

   arg++;
   bzero ((void *) &t, sizeof(CtrDrvrTime));

   printf("This:%s %s\n",__DATE__, __TIME__);
   if (ctr_get_version(h,&dver) < 0) {
      perror("ctr_get_version");
      return arg;
   }
   t.Second = dver.DrvrVersion;
   printf("Drvr:%d %s\n",t.Second,time_to_string(&t));
   t.Second = dver.VhdlVersion;
   printf("Vhdl:%d %s\n",t.Second,time_to_string(&t));
   return arg;
}

/*****************************************************************/

int GetUtc(int arg) {
ArgVal   *v;
AtomType  at;

CtrDrvrCTime *ctr_time;

   arg++;
   if (ctr_get_time(h,&ctr_time) < 0) {
      perror("ctr_get_time");
      return arg;
   }
   printf("Time:%s\n",time_to_string(&t));
   return arg;
}

/*****************************************************************/

static int connected = 0;
static char *class_names[3] = {"Hard","Ctim","Ptim"};

int WaitInterrupt(int arg) { /* msk */
ArgVal   *v;
AtomType  at;

int i, msk;

CtrDrvrConnectionClass ctr_class;
int equip;
struct ctr_interrupt_s ctr_interrupt;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 for (i=0; i<CtrDrvrInterruptSOURCES; i++) {
	    msk = 1 << i;
	    printf("%s 0x%04X ",interrupt_to_str(msk),msk);
	    if ((i%4) == 0)  printf("\n");
	 }
	 printf("\n<hardware mask> P<ptim object> C<ctim object>\n");
	 return arg;
      }
   }

   if (at == Numeric) {                 /* Hardware mask ? */
      arg++;

      ctr_class = CtrDrvrConnectionClassHARD;

      if (v->Number) {
	 equip = v->Number;
	 if (ctr_connect(h,ctr_class,equip) < 0) {
	    perror("ctr_connect");
	    return arg;
	 }
      } else {
	 if (ctr_disconnect(h,ctr_class,0) < 0) {
	    perror("ctr_disconnect");
	    return arg;
	 }
	 connected = 0;
	 printf("Disconnected from all interrupts\n");
	 return arg;
      }
   }

   if (at == Alpha) {
      if ((v->Text[0] == 'P') || (v->Text[0] == 'p')) { /* Ptim equipment ? */
	 arg++;

	 ctr_class = CtrDrvrConnectionClassPTIM;

	 v = &(vals[arg]);
	 at = v->Type;
	 if (at == Numeric) {
	    equip = v->Number;
	    arg++;
	    v = &(vals[arg]);
	    at = v->Type;

	    if (ctr_connect(h,ctr_class,equip) < 0) {
	       perror("ctr_connect");
	       return arg;
	    }
	    connected++;
	 }
      } else if ((v->Text[0] == 'C') || (v->Text[0] == 'c')) { /* Ctim equipment ? */

	 arg++;

	 ctr_class = CtrDrvrConnectionClassCTIM;

	 v = &(vals[arg]);
	 at = v->Type;
	 if (at == Numeric) {
	    equip = v->Number;
	    arg++;
	    v = &(vals[arg]);
	    at = v->Type;

	    if (ctr_connect(h,ctr_class,equip) < 0) {
	       perror("ctr_connect");
	       return arg;
	    }
	    connected++;
	 }
      }
   }

   if (connected == 0) {
      fprintf(stderr,"libctrtest:Error:WaitInterrupt:No connections to wait for\n");
      return arg;
   }

   if (ctr_wait(h,&ctr_interrupt) < 0) {
      perror("ctr_wait");
      return arg;
   }

   if      (ctr_interrupt.ctr_class == CtrDrvrConnectionClassHARD) printf("Hard:");
   else if (ctr_interrupt.ctr_class == CtrDrvrConnectionClassPTIM) printf("Ptim:");
   else if (ctr_interrupt.ctr_class == CtrDrvrConnectionClassCTIM) printf("Ctim:");
   else                                                            printf("xxxx:");
   printf("Equp:0x%04X Pyld:0x%04X Modn:%d\n",
	  ctr_interrupt.equip,
	  ctr_interrupt.payload,
	  ctr_interrupt.modnum);
   printf("    :Onz:C%d %s Trg:C%d %s Str:C%d %s\n",
	  ctr_interrupt.onzero.CTrain,time_to_str(ctr_interrupt.onzero.Time),
	  ctr_interrupt.trigger.CTrain,time_to_str(ctr_interrupt.trigger.Time),
	  ctr_interrupt.start.CTrain,time_to_str(ctr_interrupt.start.Time));
   return arg;
}

/*****************************************************************/

int SimulateInterrupt(int arg) { /* msk */
ArgVal   *v;
AtomType  at;

int i, equip;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 for (i=0; i<CtrDrvrInterruptSOURCES; i++) {
	    msk = 1 << i;
	    printf("%s 0x%04X ",interrupt_to_str(msk),msk);
	    if ((i%4) == 0)  printf("\n");
	 }
	 printf("\n<hardware mask> P<ptim object> C<ctim object>\n");
	 return arg;
      }
   }

   if (at == Numeric) {                 /* Hardware mask ? */
      v = &(vals[arg]);
      at = v->Type;
      equip = v->Number;
      arg++;
      if (ctr_simulate_interrupt(h,CtrDrvrConnectionClassHARD,equip) < 0) {
	 perror("ctr_simulate_interrupt");
	 return arg;
      }
      return arg
   }

   if (at == Alpha) {
      if ((v->Text[0] == 'P') || (v->Text[0] == 'p')) { /* Ptim equipment ? */
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;
	 if (at == Numeric) {
	    equip = v->Number;
	    if (ctr_simulate_interrupt(h,CtrDrvrConnectionClassPTIM,equip) < 0) {
	       perror("ctr_simulate_interrupt");
	       return arg;
	    }
	    return arg
	 }
      }

      if ((v->Text[0] == 'C') || (v->Text[0] == 'c')) { /* Ctim equipment ? */
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;
	 if (at == Numeric) {
	    equip = v->Number;
	    if (ctr_simulate_interrupt(h,CtrDrvrConnectionClassCTIM,equip) < 0) {
	       perror("ctr_simulate_interrupt");
	       return arg;
	    }
	    return arg
	 }
      }
   }
   fprintf(stderr,"libctrtest:SimulateInterrupt:Error:Bad interrupt spec\n");
   return arg;
}

/*****************************************************************/

static char *eccv_help =

"<CrLf>                 Next action           \n"
"/<Action Number>       Open action for edit  \n"
"?                      Print this help text  \n"
".                      Exit from the editor  \n"
"i<CTIM>                Change CTIM trigger number  CTIM                                           \n"
"f<Payload>             Change payload              Payload                                        \n"
"t<telegram number>     Change telegram number      0/CPS,1/PSB,2/LEI,3/ADE,4/SPS,5/LHC,6/SCT,7/FCT\n"
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


void EditCcvs(int ltim) {

struct ctr_ccv_s ccv[32];
int n, sze, idx, ctim, pyld, tnum, gnum, gval, strt, clck, pwdt, dlay, enbl;
char c, *cp, *ep, txt[128];


   for (idx=0; idx<32; idx++) {
      if (ctr_get_ccv(h,ltim,idx,&ccv[idx]) < 0) {
	 if (errno == ERANGE) break;
	 perror("ctr_get_ccv");
	 return;
      }
   }

   sze = idx + 1;
   idx = 0;

   while (1) {
      printf("%s :",ccv_to_str(&ccv[idx]));
      fflush(stdout);

      bzero((void *) txt, TXT_SZ); n = 0; c = 0;
      cp = ep = txt;
      while ((c != '\n') && (n < 128)) c = txt[n++] = (char) getchar();

      while (1) {
	 switch (*cp++) {

	    case '\n':
	       if (ctr_set_ccv(h,ltim,idx,&ccv[idx],0x3FFF) < 0)
		  perror("ctr_set_ccv");
		  return;
	       }
	       if (idx++ >= sze) {
		  idx = 0;
		  printf("\n");
	       }
	    break;

	    case '/':
	       idx = strtoul(cp,&ep,0);
	       if (cp != ep) {
		  cp = ep;
		  if (idx++ >= sze) idx = 0;
	       }
	    break;

	    case '?':
	       printf("%s\n",eccv_help);
	    break;

	    case '.': return;

	    case 'i':
	       ctim = strtoul(cp,&ep,0); cp = ep;
	       if (ctim) ccv[idx].ctim = ctim;
	    break;

	    case 'f':
	       pyld = strtoul(cp,&ep,0); cp = ep;
	       ccv[idx] = pyld;
	    break;

	    case 't':
	       tnum = strtoul(cp,&ep,0); cp = ep;
	       ccv[idx].tgnum = tnum;
	    break;

	    case 'n':
	       gnum = strtoul(cp,&ep,0); cp = ep;
	       ccv[idx].grnum = gnum;
	    break;

	    case 'g':
	       gval = strtoul(cp,&ep,0); cp = ep;
	       ccv[idx].grval = gval;
	    break;

	    case 's':
	       strt = strtoul(cp,&ep,0); cp = ep;
	       ccv[idx].start = strt;
	    break;

	    case 'k':
	       clck = strtoul(cp,&ep,0); cp = ep;
	       ccv[idx].clock = clck;
	    break;

	    case 'o':
	       mode = strtoul(cp,&ep,0); cp = ep;
	       ccv[idx].mode = mode;
	    break;

	    case 'w':
	       pwdt = strtoul(cp,&ep,0); cp = ep;
	       ccv[idx].pulse_width = pwdt;
	    break;

	    case 'v':
	       dlay = strtoul(cp,&ep,0); cp = ep;
	       ccv[idx].delay = dlay;
	    break;

	    case 'e':
	       enbl = strtoul(cp,&ep,0); cp = ep;
	       ccv[idx].enable = enbl;
	    break;

	    case 'p':
	       poly = strtoul(cp,&ep,0); cp = ep;
	       ccv[idx].polarity = poly;
	    break;

	    case 'q':
	       cmsk = strtoul(cp,&ep,0); cp = ep;
	       ccv[idx].counter_mask = cmsk;
	    break;

	    default: break;
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
ctr_Ccv ccv;
ctr_CcvMask ccm;

   bzero((void *) &ccv, sizeof(ctr_Ccv));
   ccm = 0;
   psize = 0;

   if (CheckErr(ctr_GetAllPtimObjects(ptimlist,&psize,PSIZE))) {

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
	    if (CheckErr(ctr_GetPtimObject(id,&mod,&cnt,&dim)) == 0) {
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
		     fprintf(stderr,"libctrtest:Error:No PTIM ID defined\n");
		     break;
		  }
		  cp = ep;

		  mod = strtoul(cp,&ep,0);
		  if (cp == ep) {
		     fprintf(stderr,"libctrtest:Error:No Mod defined\n");
		     break;
		  }
		  cp = ep;
		  if (mod == 0) mod = module;

		  cnt = strtoul(cp,&ep,0);
		  if (cp == ep) {
		     fprintf(stderr,"libctrtest:Error:No Cntr defined\n");
		     break;
		  }
		  cp = ep;

		  dim = strtoul(cp,&ep,0);
		  if (cp == ep) dim = 1;
		  cp = ep;

		  for (i=0; i<psize; i++) {
		     if (nid == ptimlist[i]) {
			fprintf(stderr,"libctrtest:Error:That PTIM:%d already exists\n",(int) nid);
			return;
		     }
		  }

		  if (CheckErr(ctr_CreatePtimObject(nid,mod,cnt,dim))) {
		     printf(">>>Ptm:%d Mo:%d Ch:%d Sz:%d Created\n",
			    (int) nid,(int) mod,(int) cnt,(int) dim);
		     ptimlist[psize] = nid;
		     psize++;
		     ix++;
		     id = ptimlist[ix];

		     CheckErr(ctr_Get(nid,1,0,0,&ccm,&ccv));
		     ccv.Enable     = ctr_CcvDEFAULT_ENABLE;
		     ccv.Start      = ctr_CcvDEFAULT_START;
		     ccv.Mode       = ctr_CcvDEFAULT_MODE;
		     ccv.Clock      = ctr_CcvDEFAULT_CLOCK;
		     ccv.PulsWidth  = ctr_CcvDEFAULT_PULSE_WIDTH;
		     ccv.Delay      = ctr_CcvDEFAULT_DELAY;
		     ccv.OutputMask = 1<<cnt;
		     ccv.Polarity   = ctr_CcvDEFAULT_POLARITY;
		     ccv.Ctim       = 0;
		     ccv.Payload    = ctr_CcvDEFAULT_PAYLOAD;

		     ccv.Machine    = ctr_CcvDEFAULT_MACHINE;
		     ccv.GrNum      = 0;
		     ccv.GrVal      = ctr_CcvDEFAULT_GRVAL;

		     for (i=0; i<dim; i++) {
			if (CheckErr(ctr_Set(nid,i+1,0,0,ccm,&ccv))) {
			   ccv.GrVal++;
			} else {
			   fprintf(stderr,"libctrtest:Error:Can't set ccv for PTIM:%d Line%d\n",(int) nid,i+1);
			   break;
			}
		     }
		  } else {
		     fprintf(stderr,"libctrtest:Error:Cant create PTIM:%d\n",(int) nid);
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

ctr_CcvMask ccm;
ctr_Ccv ccv;

int i, n;
char c, *cp, *ep;
char txt[TXT_SZ];
unsigned long str, clk, mde, plw, dly, enb, pol, oms, rfl;

   while (1) {
      if (CheckErr(ctr_GetRemote(mod,cnt,&rfl,&ccm,&ccv))) {
	 if (rfl) {
	    if (ccv_to_str(ccm,&ccv,0)) {
	       printf("Cntr:%d %s : ",(int) cnt, res);
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
		  if (ccm) if (!CheckErr(ctr_RemoteControl(rfl,mod,cnt,0,ccm,&ccv))) return;
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
		  if (str < ctr_STARTS) {
		     ccv.Start = str;
		     ccm |= ctr_CcvMaskSTART;
		     break;
		  }
	       break;

	       case 'k':
		  clk = strtoul(cp,&ep,0); cp = ep;
		  if (clk < ctr_CLOCKS) {
		     ccv.Clock = clk;
		     ccm |= ctr_CcvMaskCLOCK;
		     break;
		  }
	       break;

	       case 'o':
		  mde = strtoul(cp,&ep,0); cp = ep;
		  if (mde < ctr_MODES) {
		     ccv.Mode = mde;
		     ccm |= ctr_CcvMaskMODE;
		     break;
		  }
	       break;

	       case 'w':
		  plw = strtoul(cp,&ep,0); cp = ep;
		  ccv.PulsWidth = plw;
		  ccm |= ctr_CcvMaskPWIDTH;
	       break;

	       case 'v':
		  dly = strtoul(cp,&ep,0); cp = ep;
		  ccv.Delay = dly;
		  ccm |= ctr_CcvMaskDELAY;
	       break;

	       case 'e':
		  enb = strtoul(cp,&ep,0); cp = ep;
		  ccv.Enable = enb;
		  ccm |= ctr_CcvMaskENABLE;
	       break;

	       case 'p':
		  pol = strtoul(cp,&ep,0); cp = ep;
		  if (pol == 1) ccv.Polarity = ctr_PolarityTTL_BAR;
		  else          ccv.Polarity = ctr_PolarityTTL;
		  ccm |= ctr_CcvMaskPOLARITY;
	       break;

	       case 'q':
		  oms = strtoul(cp,&ep,0);
		  if (cp == ep) oms = 1 << cnt;
		  cp = ep;
		  ccv.OutputMask = oms & 0xFFF;
		  ccm |= ctr_CcvMaskOMASK;
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
      CheckErr(ctr_RemoteControl(rfl,module,counter,0,0,NULL));
   }

   if (CheckErr(ctr_GetRemote(module,counter,&rfl,NULL,NULL))) {
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
	    if (msk & ctr_RemoteBITS) {
	       printf("0x%02X %s\n",(int) msk,RemNames[i]);
	    } else break;
	 }
      }
      return arg;
   }

   rcm = 0;
   if (at == Numeric) {
      rcm = v->Number & ctr_RemoteBITS;
      arg++;
      v = &(vals[arg]);
      at = v->Type;
   }

   if (CheckErr(ctr_GetRemote(module,counter,&rfl,NULL,NULL))) {
      if (rfl) {
	 if (CheckErr(ctr_RemoteControl(rfl,module,counter,rcm,0,NULL))) {
	    printf("Send:Cntr:%d Mod:%d Cmd:0x%02X OK\n",(int) counter,(int) module,(int) rcm);
	    return arg;
	 }
      } else fprintf(stderr,"libctrtest:Error:Cntr:%d Mod:%d Not in REMOTE\n",(int) counter,(int) module);
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
   if (CheckErr(ctr_ConnectPayload(C_EVENT,ctime,module))) {
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

   if (!CheckErr(ctr_GetAllCtimObjects(ctimlist,&csize,CSIZE))) {
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

      if (!CheckErr(ctr_GetAllCtimObjects(ctimlist,&csize,CSIZE))) {
	  printf("Error: Can't get CTIM list\n");
	  return;
      }

      if (csize) {

	 eqp = ctimlist[i];
	 if (!CheckErr(ctr_GetCtimObject(eqp,&frm))) {
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
	       if (!CheckErr(ctr_CreateCtimObject(eqp,frm))) {
		  printf("Error: Cant create/modify CTIM object: %d\n",(int) eqp);
		  return;
	       }
	       printf(">>>Ctm:%d Fr:0x%08X : Frame changed\n",
		      (int) eqp,
		      (int) frm);
	    break;

	    case 'x':
	       if (!CheckErr(ctr_CreateCtimObject(eqp,0))) {
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
	       if (!CheckErr(ctr_CreateCtimObject(eqp,frm))) {
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

static char *LemoName[] = {"CH1","CH2","CH3","CH4","CH5","CH6","CH7","CH8","ST1","ST2","CK1","CK2"};

int DoIo(int arg) {
ArgVal   *v;
AtomType  at;

unsigned long lemos, lmask;
ctr_Error err;
int i;
char txt[128];

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   lemos = 0; lmask = 0;

   if (at == Numeric) {
      lemos = (ctr_Lemo) v->Number;

      arg++;
      v = &(vals[arg]);
      at = v->Type;

      if (at == Numeric) {
	 lmask = (ctr_Lemo) v->Number;

	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 sprintf(txt,"Lemos:0x%X Mask:0x%X",(int) lemos, (int) lmask);
	 if (yes_no("WARNING:Set CTR Output polarities:",txt)) {
	    err = ctr_SetOutputs(module,lemos,lmask);
	    if (err != ctr_ErrorSUCCESS) {
	       printf("Error:%s; Can't Set CTR Outputs:%s\n",ctr_ErrorToString(err),txt);
	       return arg;
	    }
	 }
      }
   }

   err = ctr_GetIoStatus(module,(ctr_Lemo *) &lemos);
   if (err != ctr_ErrorSUCCESS) {
      printf("Error:%s; Can't get CTR Inputs\n",ctr_ErrorToString(err));
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
ctr_ModuleStats stats;
ctr_Error ter;

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

      ter = ctr_SetPllLocking(module,pllflag);
      if (ter != ctr_ErrorSUCCESS) {
	 printf("Error:%s; Can't set PLL locking\n",ctr_ErrorToString(ter));
	 return arg;
      }
   }

   ter = ctr_GetModuleStats(module,&stats);
   if (ter != ctr_ErrorSUCCESS) {
      printf("Error:%s; Can't read module ststistice\n",ctr_ErrorToString(ter));
      return arg;
   }

   if (stats.Cst.Valid == 0) {
      printf("Error:Pll lock control not available on module\n");
      return arg;
   }

   printf("Cst.Stat:0x%X %s\n",
	  (int) stats.Cst.Stat,
	  io_stat_to_str(stats.Cst.Stat));

   if (stats.Cst.Stat & ctr_CstStatUtcPllEnabled)
      printf("PLL Slow locking\n");
   else
      printf("PLL Brutal locking\n");

   return arg;
}
