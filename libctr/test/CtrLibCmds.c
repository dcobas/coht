/* ****************************************************************************** */
/* libctr test program                                                            */
/* Julian Lewis Mon 30th April 2012                                               */
/* ****************************************************************************** */

#include <libctr.h>

/**************************************************************************/

static CtrDrvrCounter counter = CtrDrvrCounter1;
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
/* Convert a ctr_ time in milliseconds to a string routine.             */
/* Result is a pointer to a static string representing the time.          */
/*    the format is: Thu-18/Jan/2001 08:25:14.967                         */
/*                   day-dd/mon/yyyy hh:mm:ss.ddd                         */

char *time_to_string(CtrDrvrTime *t) {

static char tbuf[128];

char tmp[128];
char *yr, *ti, *md, *mn, *dy;

struct timeval utime;

   bzero((void *) tbuf, 128);
   bzero((void *) tmp, 128);

   ctr_ctime_to_unix(t,&utime);

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

   return tbuf;
}

/*****************************************************************/
/* Convert an int to a name string                               */

static char *int_to_str(int idx, int sze, char **names) {
static char res[128];

   bzero((void *) res,128);
   if (idx < sze) strcat(res,names[idx]);
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
   if (!strlen(res)) strcat(res,":");
   return res;
}

/**************************************************************************/

char *sts_to_string(CtrDrvrStatus sts) {
static char res[256];

static char *stat_names[CtrDrvrSTATAE] = {"GmtOk", "PllOk", "XCk1Ok","XCk2Ok",
					  "TestOk","Enb",   "HtdcIn","Ctri",
					  "Ctrp",  "Ctrv",  "IntrOk","BusOk"};
   bzero((void *) res,256);
   strcat(res,enum_to_str((int) sts,CtrDrvrSTATAE,stat_names));
   return res;
}

/**************************************************************************/

char * io_stat_to_str(CtrDrvrIoStatus iostat) {

static char *io_names[CtrDrvrIoSTATAE] = { "BeamEng", "ExtIo",    "V1Pcb",     "V2Pcb",
					   "Start1",  "Start2",   "XClk1",     "XClk2",
					   "Out1",    "Out2",     "Out3",      "Out4",
					   "Out5",    "Out6",     "Out7",      "Out8",
					   "IDChip",  "DebHis",   "PllEn",     "ExtMem",
					   "TemPres" };

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

   return int_to_str((int) cstrt,CtrDrvrCounterSTARTS,cstart_names);
}

/*****************************************************************/

char *counter_mode_to_str(CtrDrvrCounterMode mode) {

static char *mode_names[CtrDrvrCounterMODES] = {"Once", "Mult", "Brst", "Mult+Brst"};

   return int_to_str((int) mode,CtrDrvrCounterMODES,mode_names);
}

/*****************************************************************/

char *counter_clock_to_str(CtrDrvrCounterClock cclk) {

static char *clock_names[CtrDrvrCounterCLOCKS] = {"1KHz", "10MHz", "40MHz", "Ext1", "Ext2", "Chnd" };

   return int_to_str((int) cclk,CtrDrvrCounterCLOCKS,clock_names);
}

/*****************************************************************/

char *polarity_to_str(CtrDrvrPolarity pol) {

#define POLARITIES 2
static char *polarity_names[POLARITIES] = {"TTL","BAR"};

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

static char *tcon_names[CtrDrvrTriggerCONDITIONS] = {"NoChk", "Equ", "LAnd"};

   return int_to_str((int) tcon,CtrDrvrTriggerCONDITIONS,tcon_names);
}

/*****************************************************************/

#define REMC_FIELDS (CTR_CCV_ENABLE       | CTR_CCV_START       | CTR_CCV_MODE  | \
		     CTR_CCV_CLOCK        | CTR_CCV_PULSE_WIDTH | CTR_CCV_DELAY | \
		     CTR_CCV_COUNTER_MASK | CTR_CCV_POLARITY)

#define PTIM_FIELDS (CTR_CCV_ENABLE       | CTR_CCV_START       | CTR_CCV_MODE  | \
		     CTR_CCV_CLOCK        | CTR_CCV_PULSE_WIDTH | CTR_CCV_DELAY | \
		     CTR_CCV_COUNTER_MASK | CTR_CCV_POLARITY    | CTR_CCV_CTIM  | \
		     CTR_CCV_PAYLOAD      | CTR_CCV_CMP_METHOD  | CTR_CCV_GRNUM | \
		     CTR_CCV_GRVAL        | CTR_CCV_TGNUM)

char *ccv_to_str(struct ctr_ccv_s *ccv, ctr_ccv_fields_t flds) {

#define CCV_FIELDS 14

int i, msk, w;

char *cp, tmp[128];
static char res[512];

   bzero((void *) res,512);
   bzero((void *) tmp,128);

   for (i=0; i<CCV_FIELDS; i++) {
      msk = 1 << i;
      if (msk & flds) {
	 switch (msk) {
	    case CTR_CCV_ENABLE:
	       if (ccv->enable) sprintf(tmp,"e:%s ",onzero_to_str(ccv->enable));
	       else             sprintf(tmp,"e:Dis ");
	    break;

	    case CTR_CCV_START:
	       sprintf(tmp,"s:%s ",counter_start_to_str(ccv->start));
	    break;

	    case CTR_CCV_MODE:
	       sprintf(tmp,"o:%s ",counter_mode_to_str(ccv->mode));
	    break;

	    case CTR_CCV_CLOCK:
	       sprintf(tmp,"k:%s ",counter_clock_to_str(ccv->clock));
	    break;

	    case CTR_CCV_PULSE_WIDTH:
	       w = ccv->pulse_width * 25;
	       if      (w >= 1000000) { w = w/1000000; cp = "ms"; }
	       else if (w >= 1000   ) { w = w/1000;    cp = "us"; }
	       else                   {                cp = "ns"; }
	       sprintf(tmp,"w:%d%s ",w,cp);
	    break;

	    case CTR_CCV_DELAY:
	       sprintf(tmp,"v:%d ",ccv->delay);
	    break;

	    case CTR_CCV_COUNTER_MASK:
	       sprintf(tmp,"m:%s ",counter_mask_to_str(ccv->counter_mask));
	    break;

	    case CTR_CCV_POLARITY:
	       sprintf(tmp,"p:%s ",polarity_to_str(ccv->polarity));
	    break;

	    case CTR_CCV_CTIM:
	       if (ccv->ctim)
		  sprintf(tmp,"i:%d ",ccv->ctim);
	    break;

	    case CTR_CCV_PAYLOAD:
	       if (ccv->payload)
		  sprintf(tmp,"f:0x%X ",ccv->payload);
	    break;

	    case CTR_CCV_CMP_METHOD:
	       if (ccv->cmp_method)
		  sprintf(tmp,"c:%s ",trigger_condition_to_str(ccv->cmp_method));
	    break;

	    case CTR_CCV_GRNUM:
	       if (ccv->cmp_method)
		  sprintf(tmp,"n:%d ",ccv->grnum);
	    break;

	    case CTR_CCV_GRVAL:
	       if (ccv->cmp_method)
		  sprintf(tmp,"g:%d ",ccv->grnum);
	    break;

	    case CTR_CCV_TGNUM:
	       if (ccv->cmp_method)
		  sprintf(tmp,"t:%d ",ccv->tgnum);
	    break;

	    default:
	    break;
	 }
	 strcat(res,tmp);
	 bzero((void *) tmp,128);
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

int get_max_ch() {

CtrDrvrHardwareType ht;
int max_ch = 4;

   if (ctr_get_type(h,&ht) < 0) {
      perror("ctr_get_type");
      return max_ch;
   }

   if      (ht == CtrDrvrHardwareTypeCTRV) max_ch = 8;
   else if (ht == CtrDrvrHardwareTypeCTRP) max_ch = 3;

   return max_ch;
}

/*****************************************************************/

int NextCounter(int arg) {

   arg++;

   counter++;
   if (counter > get_max_ch()) counter = 1;
   printf("Cntr:%d Selected\n",counter);
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
   printf("Mod:%d Selected\n",module);
   return arg;
}

/*****************************************************************/

int GetSetEnable(int arg) {
ArgVal   *v;
AtomType  at;
int flag;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      flag = v->Number;
      if (ctr_set_enable(h,flag) < 0) {
	 perror("ctr_set_enable");
	 return arg;
      }
   }

   if ((flag = ctr_get_enable(h)) < 0) {
      perror("ctr_get_enable");
      return arg;
   }
   if (flag) printf("Module:%d Enabled\n" ,module);
   else      printf("Module:%d Disabled\n",module);
   return arg;
}

/*****************************************************************/

int GetSetInputDelay(int arg) {
ArgVal   *v;
AtomType  at;
int delay;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      delay = v->Number;
      if (ctr_set_input_delay(h,delay) < 0) {
	 perror("ctr_set_input_delay");
	 return arg;
      }
   }

   if ((delay = ctr_get_input_delay(h)) < 0) {
      perror("ctr_get_input_delay");
      return arg;
   }

   printf("Input delay:%d Ticks = %dns\n",delay, delay*25);

   return arg;
}

/*****************************************************************/

int GetSetCableId(int arg) {
ArgVal   *v;
AtomType  at;
int cid;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      cid = v->Number;
      if (ctr_set_cable_id(h,cid) < 0) {
	 perror("ctr_set_cable_id");
	 return arg;
      }
   }

   if (ctr_get_cable_id(h,&cid) < 0) {
      perror("ctr_get_cable_id");
      return arg;
   }

   printf("Cable ID:%d [0x%04X]\n",cid,cid);

   return arg;
}

/*****************************************************************/

int GetSetP2Byte(int arg) {
ArgVal   *v;
AtomType  at;
int p2b;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      p2b = v->Number;
      if (ctr_set_p2_output_byte(h,p2b) < 0) {
	 perror("ctr_set_p2_output_byte");
	 return arg;
      }
   }

   if ((p2b = ctr_get_p2_output_byte(h)) < 0) {
      perror("ctr_get_p2_output_byte");
      return arg;
   }

   if (p2b) printf("P2-Byte:%d Bits:%d..%d\n",p2b, (p2b-1)*8, (p2b*8));
   else     printf("P2-Byte:Not set\n");

   return arg;
}

/*****************************************************************/

int GetSetPllLockMethod(int arg) {
ArgVal   *v;
AtomType  at;
int flag;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      flag = v->Number;
      if (ctr_set_pll_lock_method(h,flag) < 0) {
	 perror("ctr_set_pll_lock_method");
	 return arg;
      }
   }

   if ((flag = ctr_get_pll_lock_method(h)) < 0) {
      perror("ctr_get_pll_lock_method");
      return arg;
   }
   if (flag) printf("Module:%d: %d=Slow lock method\n" ,module,flag);
   else      printf("Module:%d: 0=Brutal lock method\n",module);
   return arg;
}

/*****************************************************************/

int MemTest(int arg) {

int addr, wpat, rpat, res;

   arg++;

   res = ctr_memory_test(h,&addr,&wpat,&rpat);
   if (res < 0) {
      if (errno) {
	 perror("ctr_memory_test");
	 return arg;
      }
      printf("MemoryTest:FAIL addr:0x%X wrote:0x%X read:0x%X\n",addr,wpat,rpat);
      return arg;
   }
   printf("MemoryTest:PASS\n");

   return arg;
}

/*****************************************************************/

int ListClients(int arg) {

CtrDrvrClientList pids;
CtrDrvrClientConnections ccon;
CtrDrvrConnection *con;
int i, j, pid;

   arg++;

   pid = getpid();

   if (ctr_get_client_pids(h,&pids) < 0) {
      perror("ctr_get_client_pids");
      return arg;
   }

   for (i=0; i<pids.Size; i++) {
      if (ctr_get_client_connections(h,pids.Pid[i],&ccon) < 0) {
	 perror("ctr_get_client_connections");
	 return arg;
      }
      printf("PID:%04d",pids.Pid[i]);
      if (pid == pids.Pid[i]) printf(" <== this");
      printf("\n");
      for (j=0; j<ccon.Size; j++) {
	 con = &ccon.Connections[j];
	 printf("   [Mod:%02d Eqp:%04d 0x%04X Cls:",
	       con->Module,
	       con->EqpNum,
	       con->EqpNum);
	 if      (con->EqpClass == CtrDrvrConnectionClassHARD) printf("Hard");
	 else if (con->EqpClass == CtrDrvrConnectionClassPTIM) printf("Ltim");
	 else if (con->EqpClass == CtrDrvrConnectionClassCTIM) printf("Ctim");
	 else                                                  printf("?:%d",(int) con->EqpClass);
	 printf("]\n");
      }
      printf("\n");
   }
   return arg;
}

/*****************************************************************/

int GetStatus(int arg) {
CtrDrvrStatus sts;
CtrDrvrIoStatus ios;
CtrDrvrModuleStats stats;

   arg++;
   if (ctr_get_status(h,&sts) < 0) {
      perror("ctr_get_status");
      return arg;
   }
   printf("\n");
   printf("Hws:%s\n", sts_to_string(sts));

   if (ctr_get_io_status(h,&ios) < 0) {
      perror("ctr_get_io_status");
      return arg;
   }
   printf("\n");
   printf("Ios:%s\n", io_stat_to_str(ios));

   if (ctr_get_stats(h, &stats) < 0) {
      perror("ctr_get_stats");
      return arg;
   }

   printf("\n");

   if (stats.PllErrorThreshold)                printf("PllErrorThreshold    : %d\n",stats.PllErrorThreshold);
   if (stats.PllDacLowPassValue)               printf("PllDacLowPassValue   : %d\n",stats.PllDacLowPassValue);
   if (stats.PllDacCICConstant)                printf("PllDacCICConstant    : %d\n",stats.PllDacCICConstant);
   if (stats.PllMonitorCICConstant)            printf("PllMonitorCICConstant: %d\n",stats.PllMonitorCICConstant);
   if (stats.PhaseDCM)                         printf("PhaseDCM             : %d\n",stats.PhaseDCM);
   if (stats.UtcPllPhaseError)                 printf("UtcPllPhaseError     : %d\n",stats.UtcPllPhaseError);
   if (stats.Temperature)                      printf("Temperature          : %d\n",stats.Temperature);
   if (stats.MsMissedErrs)                     printf("MsMissedErrs         : %d\n",stats.MsMissedErrs);
					       printf("LastMsMissed         : %s\n",time_to_string(&stats.LastMsMissed));
   if (stats.PllErrors)                        printf("PllErrors            : %d\n",stats.PllErrors);
					       printf("LastPllError         : %s\n",time_to_string(&stats.LastPllError));
   if (stats.MissedFrames)                     printf("MissedFrames         : %d\n",stats.MissedFrames);
					       printf("LastFrameMissed      : %s\n",time_to_string(&stats.LastFrameMissed));
   if (stats.BadReceptionCycles)               printf("BadReceptionCycles   : %d\n",stats.BadReceptionCycles);
   if (stats.ReceivedFrames)                   printf("ReceivedFrames       : %d\n",stats.ReceivedFrames);
   if (stats.SentFramesEvent)                  printf("SentFramesEvent      : %d\n",stats.SentFramesEvent);
   if (stats.UtcPllErrs)                       printf("UtcPllErrs           : %d\n",stats.UtcPllErrs);

   return arg;
}

/*****************************************************************/

int GetSetCounter(int arg) {
ArgVal   *v;
AtomType  at;
int cnt;

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      cnt = v->Number;
      if (cnt<=get_max_ch()) counter = cnt;
   }
   printf("Cntr:%d Selected\n",counter);
   return arg;
}

/*****************************************************************/

int GetSetModule(int arg) {
ArgVal   *v;
AtomType  at;

int cbid, mcnt, mod, i;
struct ctr_module_address_s module_address;

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

   if (ctr_get_cable_id(h, &cbid) < 0) {
      perror("ctr_get_cable_id");
      return arg;
   }
   printf("Modn:%d CbId:0x%04X\n",module,cbid);

   if (ctr_get_module_address(h, &module_address) < 0) {
      perror("ctr_get_module_address");
      return arg;
   }

   printf("Mmap:0x%p\n",module_address.memory_map);
   if (module_address.jtag_address)
      printf("Jtag:0x%p\n",module_address.jtag_address);

   for (i=0; i<4; i++)
      printf("Spec:%d 0x%X\n",i,module_address.specific[i]);

   return arg;
}

/*****************************************************************/

int GetTelegram(int arg) {
ArgVal   *v;
AtomType  at;

int i, tgm = 1;
short grps[32];

   arg++;

   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      tgm = v->Number;
   }

   bzero((void *) grps, 64);
   if (ctr_get_telegram(h, tgm, (short *) grps) < 0) {
      perror("ctr_get_telegram");
      return arg;
   }

   for (i=0; i<32; i++) {
      printf("g:%02d v:0x%04X ",i+1,(unsigned int) (0xFFFF & grps[i]));
      if (((i+1)%4) == 0) printf("\n");
   }
   printf("\n");
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
   if (debug_level) printf("Debug:Level:%d ON\n",debug_level);
   else             printf("Debug:OFF\n");
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

CtrDrvrCTime ctr_time;

   arg++;
   if (ctr_get_time(h,&ctr_time) < 0) {
      perror("ctr_get_time");
      return arg;
   }
   printf("Time:%s\n",time_to_string(&ctr_time.Time));
   return arg;
}

/*****************************************************************/

static int connected = 0;

int WaitInterrupt(int arg) { /* msk */
ArgVal   *v;
AtomType  at;

int i, msk, qsz;

CtrDrvrConnectionClass ctr_class;
int equip, payld;
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
	 printf("\n<hardware mask> P<ptim object> C<ctim object>[<payload>]\n");
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
	 connected++;
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

	    if (at == Numeric) {
	       payld = v->Number;
	       arg++;

	       if (ctr_connect_payload(h,equip,payld) < 0) {
		  perror("ctr_connect_payload");
		  return arg;
	       }
	    }

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
   qsz = ctr_get_queue_size(h);
   if (qsz < 0) {
      perror("ctr_get_queue_size");
      return arg;
   }

   if      (ctr_interrupt.ctr_class == CtrDrvrConnectionClassHARD) printf("Hard:");
   else if (ctr_interrupt.ctr_class == CtrDrvrConnectionClassPTIM) printf("Ltim:");
   else if (ctr_interrupt.ctr_class == CtrDrvrConnectionClassCTIM) printf("Ctim:");
   else                                                            printf("xxxx:");

   printf("Equp:%03d Pyld:0x%04X Modn:%d QuSz:%d\n",
	  ctr_interrupt.equip,
	  ctr_interrupt.payload,
	  ctr_interrupt.modnum,
	  qsz);

   printf("    :Onz:C%04d %s\n"
	  "    :Trg:C%04d %s\n"
	  "    :Str:C%04d %s\n",
	  ctr_interrupt.onzero.CTrain,  time_to_string(&ctr_interrupt.onzero.Time),
	  ctr_interrupt.trigger.CTrain, time_to_string(&ctr_interrupt.trigger.Time),
	  ctr_interrupt.start.CTrain,   time_to_string(&ctr_interrupt.start.Time));

   printf("\n");
   return arg;
}

/*****************************************************************/

int SimulateInterrupt(int arg) { /* msk */
ArgVal   *v;
AtomType  at;

int i, msk, equip;

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
      return arg;
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
	    return arg;
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
	    return arg;
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
"c<Compare method>      Change telegram compare     0/NoChk,1/Equality,2/Logical and               \n"
"t<telegram number>     Change telegram number      0/CPS,1/PSB,2/LEI,3/ADE,4/SPS,5/LHC,6/SCT,7/FCT\n"
"n<Trigger Group>       Change trigger group number Group Number                                   \n"
"g<Trigger group value> Change trigger group value  Group Value                                    \n"
"s<Start>               Change Start                1KHz/Ext1/Ext2/Chnd/Self/Remt/Pps/Chnd+Stop    \n"
"k<Clock>               Change Clock                1KHz/10MHz/40MHz/Ext1/Ext2/Chnd                \n"
"o<Mode>                Change Mode                 Once/Mult/Brst/Mult+Brst                       \n"
"w<Pulse Width>         Change Pulse Width          Pulse Width                                    \n"
"v<Delay>               Change Delay                Delay                                          \n"
"e<enable>              Change OnZero behaviour     0/NoOut,1/Bus,2/Out,3/OutBus                   \n"
"p<polarity>            Change polarity             TTL/TTL_BAR/TTL                                \n"
"m<outmask>             Change output mask          1<<cntr 0x200/40MHz 0x400/ExCk1 0x800/ExCk2    \n"
"q                      Exit from the editor  \n";

void edit_ccvs(int ltim) {

struct ctr_ccv_s ccv[64];
int i, n, sze, idx, ctim, pyld, cmpm, tnum, gnum, gval, strt, mode, clck, pwdt, dlay, enbl, poly, cmsk, nxt;
char c, *cp, *ep, txt[128];

CtrDrvrPtimObjects obs;
CtrDrvrPtimBinding *ob = NULL;

ctr_ccv_fields_t cf = 0;

   if (ctr_list_ltim_objects(h, &obs) < 0) {
      perror("ctr_list_ltim_objects");
      return;
   }

   for (i=0; i<obs.Size; i++) {
      ob = &obs.Objects[i];
      if (ob->EqpNum == ltim) break;
      else ob = NULL;
   }

   if (!ob) {
      fprintf(stderr,"edit_ccvs:Ltim:%d Not found\n",ltim);
      return;
   }

   sze = ob->Size;  if (sze > 64) sze = 64;
   for (idx=0; idx<sze; idx++) {
      if (ctr_get_ccv(h,ltim,idx,&ccv[idx]) < 0) {
	 perror("ctr_get_ccv");
	 return;
      }
   }

   cf = 0; idx = 0;
   while (1) {

      if (cf) {
	 if (ctr_set_ccv(h,ltim,idx,&ccv[idx],cf) < 0) {
	    perror("ctr_set_ccv");
	    return;
	 }
	 cf = 0;
	 if (ctr_get_ccv(h,ltim,idx,&ccv[idx]) < 0) {
	    perror("ctr_get_ccv");
	    return;
	 }
      }
      printf("%s:",ccv_to_str(&ccv[idx],PTIM_FIELDS));
      fflush(stdout);

      bzero((void *) txt,128); n = 0; c = 0;
      cp = ep = txt;
      while ((c != '\n') && (n < 128)) {
	 c = (char) getchar();
	 txt[n++] = c;
      }

      nxt = 0;
      while (1) {

	 if ((*cp == '\n') || (*cp == 0)) {
	    if (!nxt) {
	       if (++idx >= sze) {
		  idx = 0;
		  printf("\n");
	       }
	       nxt = 0;
	    }
	    break;
	 }

	 c = *cp++;

	 if (c == '/') {
	    n = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       if (n < sze) {
		  idx = n;
		  nxt = 1;
	       } else printf("Error: bad index\n");
	    } else printf("Error: no index\n");
	 }

	 else if (c == '?') printf("%s\n",eccv_help);
	 else if (c == '.') return;
	 else if (c == 'q') return;

	 else if (c == 'i') {
	    ctim = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       if (ctim) ccv[idx].ctim = ctim;
	       cf |= CTR_CCV_CTIM;
	       break;
	    } else printf("Error: no ctim\n");
	 }

	 else if (c == 'f') {
	    pyld = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       ccv[idx].payload = pyld;
	       cf |= CTR_CCV_PAYLOAD;
	       break;
	    } else printf("Error: no payload\n");
	 }

	 else if (c == 'c') {
	    cmpm = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       ccv[idx].cmp_method = cmpm;
	       cf |= CTR_CCV_CMP_METHOD;
	       break;
	    } else printf("Error: no compare method\n");
	 }

	 else if (c =='t') {
	    tnum = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       ccv[idx].tgnum = tnum;
	       cf |= CTR_CCV_TGNUM;
	       break;
	    } else printf("Error: no telegram number\n");
	 }

	 else if (c == 'n') {
	    gnum = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       ccv[idx].grnum = gnum;
	       cf |= CTR_CCV_GRNUM;
	       break;
	    } else printf("Error: no group number\n");
	 }

	 else if (c == 'g') {
	    gval = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       ccv[idx].grval = gval;
	       cf |= CTR_CCV_GRVAL;
	       break;
	    } else printf("Error: no group value\n");
	 }

	 else if (c == 's') {
	    strt = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       ccv[idx].start = strt;
	       cf |= CTR_CCV_START;
	       break;
	    } else printf("Error: no start\n");
	 }

	 else if (c == 'k') {
	    clck = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       ccv[idx].clock = clck;
	       cf |= CTR_CCV_CLOCK;
	       break;
	    } else printf("Error: bo clock\n");
	 }

	 else if (c == 'o') {
	    mode = strtoul(cp,&ep,0); cp = ep;
	    if (cp != ep) {
	       cp = ep;
	       ccv[idx].mode = mode;
	       cf |= CTR_CCV_MODE;
	       break;
	    } else printf("Error: no mode\n");
	 }

	 else if (c == 'w') {
	    pwdt = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       ccv[idx].pulse_width = pwdt;
	       cf |= CTR_CCV_PULSE_WIDTH;
	       break;
	    } else printf("Error: no pulse width\n");
	 }

	 else if (c == 'v') {
	    dlay = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       ccv[idx].delay = dlay;
	       cf |= CTR_CCV_DELAY;
	       break;
	    } else printf("Error: no delay value\n");
	 }

	 else if (c == 'e') {
	    enbl = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       ccv[idx].enable = enbl;
	       cf |= CTR_CCV_ENABLE;
	       break;
	    } else printf("Error: no enable flag\n");
	 }

	 else if (c == 'p') {
	    poly = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       ccv[idx].polarity = poly;
	       cf |= CTR_CCV_POLARITY;
	       break;
	    } else printf("Error: no polarity\n");
	 }

	 else if (c == 'm') {
	    cmsk = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       ccv[idx].counter_mask = cmsk;
	       cf |= CTR_CCV_COUNTER_MASK;
	       break;
	    } else printf("Error: no counter mask\n");
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
"y<Id>,<Cntr>,<Size> Create PTIM equipment \n";

void edit_ltim(int ltim) {

int n, i, idx, nid, cnt, dim, nxt;
char c, *cp, *ep, txt[128];

CtrDrvrPtimObjects obs;
CtrDrvrPtimBinding *ob = NULL;

   if (ctr_list_ltim_objects(h, &obs) < 0) {
      perror("ctr_list_ltim_objects");
      return;
   }

   if (ltim) {
      for (idx=0; idx<obs.Size; idx++) {
	 ob = &obs.Objects[idx];
	 if (ob->EqpNum == ltim) break;
	 else ob = NULL;
      }
      if (!ob) {
	 fprintf(stderr,"edit_ccvs:Ltim:%d Not found\n",ltim);
	 return;
      }
   } else {
      idx = 0;
      ob = &obs.Objects[idx];
   }

   while (1) {
      if (obs.Size == 0) printf(">>>Ltim:None defined : ");
      else {
	 printf("%02d:Ltm:%d Mo:%d Ch:%d Sz:%d Ix:%d: ",
		idx,
		(int) ob->EqpNum,
		(int) ob->ModuleIndex+1,
		(int) ob->Counter,
		(int) ob->Size,
		(int) ob->StartIndex);

	 fflush(stdout);
      }

      bzero((void *) txt,128); n = 0; c = 0;
      cp = ep = txt;
      while ((c != '\n') && (n < 128)) {
	 c = (char) getchar();
	 txt[n++] = c;
      }

      nxt = 0;
      while (1) {
	 c = *cp++;
	 if ((c == '\n') || (c == 0)) {
	    if (!nxt) {
	       if (++idx >= obs.Size) {
		  idx = 0;
		  printf("\n");
	       }
	       nxt = 0;
	    }
	    ob = &obs.Objects[idx];
	    break;
	 }
	 else if (c == '/') {
	    n = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       nxt = 1;
	       if (n < obs.Size) idx = n;
	       else printf("Error: Bad index\n");
	    } else printf("Error: No index\n");
	 }
	 else if (c == '.') return;
	 else if (c == 'q') return;
	 else if (c == '?') printf("%s\n",eptim_help);
	 else if (c == 'a') edit_ccvs(ob->EqpNum);
	 else if (c == 'y') {
	    nid = strtoul(cp,&ep,0);
	    if (cp == ep) {
	       fprintf(stderr,"libctrtest:Error:No PTIM ID defined\n");
	       break;
	    } cp = ep;

	    cnt = strtoul(cp,&ep,0);
	    if (cp == ep) {
	       fprintf(stderr,"libctrtest:Error:No Cntr defined\n");
	       break;
	    } cp = ep;

	    dim = strtoul(cp,&ep,0);
	    if (cp == ep) dim = 1;
	    cp = ep;

	    for (i=0; i<obs.Size; i++) {
	       if (nid == obs.Objects[i].EqpNum) {
		  fprintf(stderr,"libctrtest:Error:That PTIM:%d already exists\n",(int) nid);
		  return;
	       }
	    }

	    if (ctr_create_ltim(h,nid,cnt,dim) < 0) {
	       perror("ctr_create_ltim");
	       return;
	    }
	    if (ctr_list_ltim_objects(h,&obs) < 0) {
	       perror("ctr_list_ltim_objects");
	       return;
	    }
	    for (idx=0; idx<obs.Size; idx++) {
	       ob = &obs.Objects[i];
	       if (ob->EqpNum == nid) break;
	    }
	    break;
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
"e<enable>              Change OnZero behaviour     0/NoOut,1/Bus,2/Out,3/OutBus                \n"
"p<polarity>            Change polarity             TTL/TTL_BAR/TTL                             \n"
"m<outmask>             Change output mask          1<<cntr 0x200/40MHz 0x400/ExCk1 0x800/ExCk2 \n"
"q                      Exit from the editor  \n";

static void edit_remote(int cnt) {

struct ctr_ccv_s ccv;
ctr_ccv_fields_t cf = 0;

int n, rfl, ch, strt, clck, mode, pwdt, dlay, enbl, poly, cmsk;
char c, *cp, *ep, txt[128];

   if (cnt) ch = cnt;
   else     ch = 1;

   rfl = ctr_get_remote(h,ch,&ccv);
   if (rfl < 0) {
      perror("ctr_get_remote");
      return;
   }

   while (1) {

      if (cf) {
	 if (ctr_set_remote(h,rfl,ch,0,&ccv,cf) < 0) {
	    perror("ctr_set_remote");
	    return;
	 }
	 cf = 0;
      }

      rfl = ctr_get_remote(h,ch,&ccv);
      if (rfl < 0) {
	 perror("ctr_get_remote");
	 return;
      }

      printf("rem:%d ch:%d %s",rfl,ch,ccv_to_str(&ccv,REMC_FIELDS));
      fflush(stdout);

      if (!rfl) {
	 printf("\n");
	 if (ch++ >= get_max_ch()) return;
	 continue;
      }

      bzero((void *) txt,128); n = 0; c = 0;
      cp = ep = txt;
      while ((c != '\n') && (n < 128)) {
	 c = (char) getchar();
	 txt[n++] = c;
      }

      while (1) {

	 c = *cp++;

	 if ((c == '\n') || (c == 0)) {
	    if (ch++ >= get_max_ch()) {
	       ch = 1;
	       printf("\n");
	    }
	    break;
	 }

	 else if (c == '/') {
	    n = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       if (n > get_max_ch()) printf("Error: bad channel\n");
	       else ch = n;
	       break;
	    } else printf("Error: no channel\n");
	 }

	 else if (c == '?') printf("%s\n",erem_help);
	 else if (c == '.') return;
	 else if (c == 'q') return;

	 else if (c == 's') {
	    strt = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       ccv.start = strt;
	       cf |= CTR_CCV_START;
	       break;
	    } printf("Error: no start\n");
	 }

	 else if (c == 'k') {
	    clck = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       ccv.clock = clck;
	       cf |= CTR_CCV_CLOCK;
	       break;
	    } else printf("Error: no clock\n");
	 }

	 else if (c == 'o') {
	    mode = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       ccv.mode = mode;
	       cf |= CTR_CCV_MODE;
	       break;
	    } else printf("Error: no mode\n");
	 }

	 else if (c == 'w') {
	    pwdt = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       ccv.pulse_width = pwdt;
	       cf |= CTR_CCV_PULSE_WIDTH;
	       break;
	    } else printf("Error: no pulse width\n");
	 }

	 else if (c == 'v') {
	    dlay = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       ccv.delay = dlay;
	       cf |= CTR_CCV_DELAY;
	       break;
	    } else printf("Error: no delay\n");
	 }

	 else if (c == 'e') {
	    enbl = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       ccv.enable = enbl;
	       cf |= CTR_CCV_ENABLE;
	       break;
	    } else printf("Error: no enable flag\n");
	 }

	 else if (c == 'p') {
	    poly = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       ccv.polarity = poly;
	       cf |= CTR_CCV_POLARITY;
	       break;
	    } else printf("Error: no polarity\n");
	 }

	 else if (c == 'm') {
	    cmsk = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       ccv.counter_mask = cmsk;
	       cf |= CTR_CCV_COUNTER_MASK;
	       break;
	    } else printf("Error: no counter mask\n");
	 }
      }
   }
}

/*****************************************************************/

int Config(int arg) { /* Remote flag */

   arg++;

   edit_remote(counter);
   return arg;
}

/*****************************************************************/

int GetSetRemote(int arg) { /* Remote flag */
ArgVal   *v;
AtomType  at;

int rfl;

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
      if (ctr_set_remote(h,rfl,counter,0,NULL,0) < 0) {
	 perror("ctr_set_remote");
	 return arg;
      }
   }

   if ((rfl = ctr_get_remote(h,counter,NULL)) < 0) {
      perror("ctr_get_remote");
      return arg;
   }

   if (rfl == 0) printf("Cntr:%d Mod:%d Under CTR-Trigger control\n",(int) counter,(int) module);
   else          printf("Cntr:%d Mod:%d Under Remote control\n",(int) counter,(int) module);
   return arg;
}

/*****************************************************************/

static char *RemNames[CtrDrvrREMOTES] = {"Load","Stop","Start","OutNow","BusNow"};
#define RBITS ((1<<CtrDrvrREMOTES)-1)

int SetRemoteCmd(int arg) { /* Remote flag */
ArgVal   *v;
AtomType  at;

int i;
unsigned long msk, rfl;
CtrDrvrRemote rcm;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Cmd:\n");
	 for (i=0; i<CtrDrvrREMOTES; i++) {
	    msk = 1 << i;
	    if (msk & RBITS) {
	       printf("0x%02X %s\n",(int) msk,RemNames[i]);
	    } else break;
	 }
      }
      return arg;
   }

   rcm = 0;
   if (at == Numeric) {
      rcm = v->Number & RBITS;
      arg++;
      v = &(vals[arg]);
      at = v->Type;
   }

   if ((rfl = ctr_get_remote(h,counter,NULL)) < 0) {
      perror("ctr_get_remote");
      return arg;
   }
   if (rfl) {
      if (ctr_set_remote(h,rfl,counter,rcm,NULL,0) < 0) {
	 perror("ctr_set_remote");
	 return arg;
      }
      return arg;
   }
   printf("Counter:%d Not in remote\n",counter);
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
   if (ctr_connect_payload(h,C_EVENT,ctime) < 0) {
      perror("ctr_connect_payload");
      return arg;
   }
   connected++;
   printf("Connected to C-%d\n",ctime);

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
   edit_ltim(ptim);
   return arg;
}

/*****************************************************************/

static char *ectim_help =

"<CrLf>        Next \n"
"/<Index>      Goto \n"
"?             Help \n"
".             Exit \n"
"x<id>         Destroy \n"
"y<Id>,<Frame> Create  \n";

void edit_ctim(int ctim) {

char c, *cp, *ep, txt[128];
int n, idx, new, frm, eqp, nxt;

CtrDrvrCtimObjects ctims;
CtrDrvrCtimBinding *cb = NULL;

   if (ctr_list_ctim_objects(h,&ctims) < 0) {
      perror("ctr_list_ctim_objects");
      return;
   }
   if (ctim) {
      for (idx=0; idx<ctims.Size; idx++) {
	 cb = &ctims.Objects[idx];
	 if (ctim == cb->EqpNum) break;
	 cb = NULL;
      }
   }
   if (ctim) {
      if (!cb) {
	 printf("No such ctim:%d\n",ctim);
	 return;
      }
   } else {
      idx = 0;
      cb = &ctims.Objects[idx];
   }

   while (1) {

      if (ctr_list_ctim_objects(h,&ctims) < 0) {
	 perror("ctr_list_ctim_objects");
	 return;
      }

      printf("%04d:ctim:%03d frame:0x%08X: ",idx,cb->EqpNum,cb->Frame.Long);
      fflush(stdout);

      bzero((void *) txt,128); n = 0; c = 0;
      cp = ep = txt;
      while ((c != '\n') && (n < 128)) {
	 c = (char) getchar();
	 txt[n++] = c;
      }
      nxt = 0;

      while (1) {
	 c = *cp++;
	 if ((c == '\n') || (c == 0)) {
	    if (!nxt) {
	       if (++idx >= ctims.Size) {
		  idx = 0;
		  printf("\n");
	       }
	       cb = &ctims.Objects[idx];
	    }
	    nxt = 0;
	    break;
	 }
	 else if (c == '/') {
	    n = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       if (n < ctims.Size) {
		  idx = n;
		  cb = &ctims.Objects[idx];
		  nxt = 1;
	       } else printf("Error:Bad index\n");
	    } else printf("Error:No index\n");
	 }
	 else if (c == '?') printf("%s\n",ectim_help);
	 else if (c == '.') return;
	 else if (c == 'q') return;
	 else if (c == 'x') {
	    eqp = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       if (ctr_destroy_ctim(h,eqp) < 0) {
		  perror("ctr_destroy_ctim");
		  return;
	       }
	       printf("Destroyed:ctim:%d\n",eqp);
	    } else printf("Error:No id\n");
	    break;
	 }
	 else if (c == 'y') {
	    new = strtoul(cp,&ep,0);
	    if (cp != ep) {
	       cp = ep;
	       frm = strtoul(cp,&ep,0);
	       if (cp != ep) {
		  cp = ep;
		  if (ctr_create_ctim(h,new,frm) < 0) {
		     perror("ctr_create_ctim");
		     return;
		  }
		  printf("Created:%04d:ctim:%d 0x%08X OK\n",ctims.Size,new,frm);
	       } else printf("Error:No frame\n");
	    } else printf("Error:No id\n");
	    break;
	 }
      }
   }
}

/*****************************************************************/

int GetSetCtim(int arg) { /* CTIM ID */
ArgVal   *v;
AtomType  at;

int ctim;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   ctim = 0;
   if (at == Numeric) {
      arg++;
      ctim = v->Number;
   }

   edit_ctim(ctim);

   return arg;
}
