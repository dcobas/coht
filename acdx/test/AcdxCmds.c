/* ****************************************************************************** */
/* ACDX test program, calls driver                                                */
/* Julian Lewis AB/CO/HT Julian.Lewis@cern.ch July 2008                           */
/* ****************************************************************************** */

#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>   /* ctime */
#include <AcdxCmds.h>

#ifndef COMPILE_TIME
#define COMPILE_TIME 0
#endif

double round(double x);

static char *editor = "e";

/* ============================= */

char *defaultconfigpath = "/usr/local/drivers/acdx/acdxtest.config";

char *configpath = NULL;
char localconfigpath[128];  /* After a CD */

static short    func[TOTAL_POINTS];
static unsigned long srise, sftop, sfall;
static unsigned long func_valid = 0;
static unsigned long func_size = 0;

static unsigned long freq = 3000;
static unsigned long ampl = 500;
static unsigned long rise = 200000;
static unsigned long ftop = 200000;
static unsigned long fall = 200000;


/* ============================= */

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
      configpath = "./acdxtest.config";
      gpath = fopen(configpath,"r");
      if (gpath == NULL) {
	 configpath = "/dsc/local/data/acdxtest.config";
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

/* ============================= */

void GetPlotFile() {
char txt[128];
   sprintf(txt,"%s.%s",path,FuncNames[Funcn-1]);
   strcpy(path,txt);
}

/* ============================= */

static void IErr(char *name, int *value) {

   if (value != NULL)
      printf("AcdxDrvr: [%02d] ioctl=%s arg=%d :Error\n",
	     (int) acdx, name, (int) *value);
   else
      printf("AcdxDrvr: [%02d] ioctl=%s :Error\n",
	     (int) acdx, name);
   perror("AcdxDrvr");
}

/* ============================= */

volatile char *TimeToStr(unsigned int t) {

static char tbuf[128];

char tmp[128];
char *yr, *ti, *md, *mn, *dy;

    bzero((void *) tbuf, 128);
    bzero((void *) tmp, 128);

    if (t) {

	ctime_r ((time_t *) &t, tmp); /* Day Mon DD HH:MM:SS YYYY\n\0 */

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

	sprintf (tbuf, "%s-%s/%s/%s %s"  , dy, md, mn, yr, ti);
    }
    else {
	sprintf (tbuf, "--- Zero ---");
    }
    return (tbuf);
}

/* ============================= */
/* News                          */

int News(int arg) {

char sys[128], npt[128];

   arg++;

   if (GetFile("acdx_news")) {
      strcpy(npt,path);
      sprintf(sys,"%s %s",GetFile(editor),npt);
      printf("\n%s\n",sys);
      system(sys);
      printf("\n");
   }
   return(arg);
}

/* ============================= */

static int yesno=1;

static int YesNo(char *question, char *name) {
int yn, c;

   if (yesno == 0) return 1;

   printf("%s: %s\n",question,name);
   printf("Continue (Y/N):"); yn = getchar(); c = getchar();
   if ((yn != (int) 'y') && (yn != 'Y')) return 0;
   return 1;
}

/* ============================= */
/* Launch a task                 */

static void Launch(char *txt) {

#if 0
pid_t child;

   printf("Launch:%s\n",txt);

   if ((child = fork()) == 0) {
      if ((child = fork()) == 0) {
	 close(acdx);
	 system(txt);
	 exit (127);
      }
      exit (0);
   }
#else
   system(txt);
#endif

}

/* ============================= */

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

/* ============================= */

static int DefaultFile = 0;

int GetPath(int arg,char *defnam) {
ArgVal   *v;
AtomType  at;

char fname[128], *cp;
int n, earg;

   for (earg=arg; earg<pcnt; earg++) {
      v = &(vals[earg]);
      if ((v->Type == Close)
      ||  (v->Type == Terminator)) break;
   }

   v = &(vals[arg]);
   at = v->Type;
   if ((v->Type == Close)
   ||  (v->Type == Terminator)) {

      DefaultFile = 1;
      GetFile(defnam);

   } else {

      DefaultFile = 0;
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
      strcpy(path,fname);
   }
   return earg;
}

/* ============================= */

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
      strcat(localconfigpath,"/acdxtest.config");
      if (YesNo("Change acdxtest config to:",localconfigpath))
	 configpath = localconfigpath;
      else
	 configpath = NULL;
   }

   cp = GetFile(editor);
   sprintf(txt,"%s %s",cp,configpath);
   printf("\n%s\n",txt);
   system(txt);
   printf("\n");
   return(arg);
}
/* ============================= */

void WriteResults(char *func1, char *func2, double ohms, double degrees, double ohmsr, double ohmsi,
		  double freq, double wlen, double pvolts, double pamps,
		  double miliohms, double microhens, double gain, double corpvolts, double corpamps) {
FILE *fp;

   GetFile("AcdxStats");
   umask(0);
   fp = fopen(path,"a");
   if (fp == NULL) {
      printf("WriteResults:Can't open file: %s for write\n",path);
      return;
   }

   fprintf(fp,"#FuncName: %s <=> %s\n",func1,func2);
   fprintf(fp,"#Ohms,Degrees,OhmsReal,OhmsImag,Frequency,"
	      "WaveLength,PeakVolts,PeakAmps,MiliOhms,MicroHenries,"
	      "Gain,CorrectedPeakVolts,CorrectedPeakAmps\n");

   fprintf(fp,"%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
	      ohms,degrees,ohmsr,ohmsi,freq,
	      wlen,pvolts,pamps,miliohms,microhens,
	      gain,corpvolts,corpamps);

   fclose(fp);
}

/* ========================================= */
/* Write file in proper floating point units */

void WriteFileCalibrated() {

unsigned long i;
FILE *fp = NULL;
double fac;

   if ((Funcn-1 == AcdxFunctionMAGNET_VOLTAGE)
   ||  (Funcn-1 == AcdxFunctionLEFT_OUTPUT_VOLTAGE)
   ||  (Funcn-1 == AcdxFunctionRIGHT_OUTPUT_VOLTAGE)) {
      fac = (10.0/(float) 0x7fff) * AcdxOUTPUT_VOLTAGE_FACTOR;
   } else if ((Funcn -1) == AcdxFunctionMAGNET_CURRENT) {
      fac = (10.0/(float) 0x7fff) * AcdxMAGNET_CURRENT_FACTOR;
   } else {
      fac = (10.0/(float) 0x7fff) * AcdxOUTPUT_CURRENT_FACTOR;
   }

   GetFile("AcdxAqn"); GetPlotFile();
   printf("Writing calibrated data to:%s\n",path);
   umask(0);
   fp = fopen(path,"w");
   if (fp == NULL) {
      printf("Can't open file: %s for write\n",path);
      return;
   }

   for (i=0; i<func_size; i++) fprintf(fp,"%lu %f\n", i*AQN_MICROS_PER_POINT, func[i]*fac);
   fclose(fp);
   printf("Written:%d CALIBRATED values to:%s OK\n",(int) func_size,path);

}

/* ============================= */

int SwDeb(int arg) { /* Arg!=0 => On, else Off */
ArgVal   *v;
AtomType  at;
int debug;

   arg++;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      if (v->Number) debug = v->Number;
      else           debug = 0;
      if (ioctl(acdx,AcdxDrvrSET_SW_DEBUG,&debug) < 0) {
	 IErr("SET_SW_DEBUG",&debug);
	 return arg;
      }
   }
   debug = -1;
   if (ioctl(acdx,AcdxDrvrGET_SW_DEBUG,&debug) < 0) {
      IErr("GET_SW_DEBUG",NULL);
      return arg;
   }
   if (debug > 0)
      printf("SoftwareDebug: [%d] Enabled\n",debug);
   else
      printf("SoftwareDebug: [%d] Disabled\n",debug);

   return arg;
}

/* ============================= */

void EditMemory(int address) {

AcdxDrvrRawIoBlock iob;
unsigned long array[2];
unsigned long addr, *data;
char c, *cp, str[128];
int n, radix, nadr;

   printf("EditMemory: [?]Help [/]Open [CR]Next [.]Exit [x]Hex [d]Dec\n\n");

   data = array;
   addr = address;
   radix = 16;
   c = '\n';

   while (1) {

      iob.Size = 1;
      iob.Offset = addr;
      iob.UserArray = array;

      if (ioctl(acdx,AcdxDrvrRAW_READ,&iob) < 0) {
	 IErr("RAW_READ",NULL);
	 break;
      }

      if (radix == 16) {
	 if (c=='\n') printf("%s 0x%04x: 0x%08x ",MapNames[(int) addr],(int) addr,(int) *data);
      } else {
	 if (c=='\n') printf("%s %04d: %5d ",MapNames[(int) addr],(int) addr,(int) *data);
      }

      c = (char) getchar();

      if (c == '/') {
	 bzero((void *) str, 128); n = 0;
	 while ((c != '\n') && (n < 128)) c = str[n++] = (char) getchar();
	 nadr = strtoul(str,&cp,radix);
	 if (cp != str) addr = nadr;
      }

      else if (c == 'x')  { radix = 16; if (addr) addr--; continue; }
      else if (c == 'd')  { radix = 10; if (addr) addr--; continue; }
      else if (c == '.')  { c = getchar(); printf("\n"); break; }
      else if (c == 'q')  { c = getchar(); printf("\n"); break; }
      else if (c == '\n') { addr += 1; if (addr >= MAP_LONGS) {addr = 0; printf("\n----\n");} }
      else if (c == '?')  { printf("[?]Help [/]Open [CR]Next [.]Exit [x]Hex [d]Dec\n"); }

      else {
	 bzero((void *) str, 128); n = 0;
	 str[n++] = c;
	 while ((c != '\n') && (n < 128)) c = str[n++] = (char) getchar();
	 *data = strtoul(str,&cp,radix);
	 if (cp != str) {
	    iob.Size = 1;
	    if (ioctl(acdx,AcdxDrvrRAW_WRITE,&iob) < 0) {
	       IErr("RAW_WRITE",NULL);
	       break;
	    }
	 }
      }
   }
}

/* ============================= */

int MemIo(int arg) { /* Address */
ArgVal   *v;
AtomType  at;

   printf("Raw Io: Address space: BAR2: Acdx hardware space\n");

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

/* ============================= */

int AcdxListIo(AcdxIoItem *list, int length) {

AcdxDrvrRawIoBlock iob;
unsigned long array[2];
int i;

   for (i=0; i<length; i++) {

      usleep(1000);
      if (list[i].Address >= AcdxADDRESSES) break;
      iob.Size = 1;
      iob.UserArray = array;
      iob.Offset = list[i].Address;
      array[0] = list[i].Data;
      if (ioctl(acdx,AcdxDrvrRAW_WRITE,&iob) < 0) {
	 IErr("RAW_WRITE:address",(int *) &list[i].Address);
	 return 0;
      }
   }
   return 1;
}

/* ============================= */

double GetAmplitude(unsigned long t) {
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

/* ============================= */

short FuncToShort(double val) {

double dac;
short dval;

   dac = val / 10000.0;
   dac = dac * (double) 0x7FFF;
   dval = (short) round(dac);
   return dval;
}

/* =================================== */

int IncFreq(int arg) {
ArgVal   *v;
AtomType  at;

unsigned long i;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Numeric) {
      i = v->Number;
      if (i+freq < 500000) freq += i;
   }
   printf("Frequency:%d Hertz\n",(int) freq);
   return arg;
}

/* =================================== */

int SetParams(int arg) { /* Freq(Hz), Amplitude(Millivolts 0..20000), RiseTime(us), FlatTop(us), FallTime(us) */
ArgVal   *v;
AtomType  at;

unsigned long i;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Frequency(Hz),  Amplitude(mV), RiseTime(uS), FlatTop(uS), FallTime(uS) OutFileName\n");
	 printf("0..500000Hz     0..10000mV     0..300000uS   0..300000uS  0..300000uS  <path>\n");

	 return arg;
      }
   }

   if (at == Numeric) {

      i = v->Number;
      if (i > 500000) {
	 printf("Bad frequency:%d [0..500000]\n",(int) i);
	 return arg;
      }
      freq = i;

      arg++;
      v = &(vals[arg]);
      at = v->Type;

      if (at == Numeric) {

	 i = v->Number;
	 if (i > 10000) {
	    printf("Bad Amplitude:%d [0..10000mV]\n",(int) i);
	    return arg;
	 }
	 ampl = i;

	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 if (at == Numeric) {

	    i = v->Number;
	    if (i > 300000) {
	       printf("Bad RiseTime:%d [0..300000us]\n",(int) i);
	       return arg;
	    }
	    rise = i;

	    arg++;
	    v = &(vals[arg]);
	    at = v->Type;

	    if (at == Numeric) {

	       i = v->Number;
	       if (i > 300000) {
		  printf("Bad FlatTopTime:%d [0..300000us]\n",(int) i);
		  return arg;
	       }
	       ftop = i;

	       arg++;
	       v = &(vals[arg]);
	       at = v->Type;

	       if (at == Numeric) {

		  i = v->Number;
		  if (i > 300000) {
		     printf("Bad FallTime:%d [0..300000us]\n",(int) i);
		     return arg;
		  }
		  fall = i;

		  arg++;
		  v = &(vals[arg]);
		  at = v->Type;
	       }
	    }
	 }
      }
   }
   printf("Frequency:%d Hz Amplitude:%d mV RiseTime:%d uS FlatTop:%d uS FallTime:%d uS\n",
	  (int) freq, (int) ampl, (int) rise, (int) ftop, (int) fall);
   return arg;
}

/* =================================== */

#ifndef PI
#define PI M_PI
#endif

int SinWave(int arg) { /* Path */

double omega, tee;

unsigned long i, spts;
unsigned long array[2];
AcdxDrvrRawIoBlock iob;

FILE *fp;

   arg++;

   bzero((void *) func, sizeof(short)*TOTAL_POINTS);

   iob.Size = 1;
   iob.UserArray = array;
   iob.Offset = AcdxSmpClkHz;

   spts = SIN_POINTS_PER_SECOND;

   if (ioctl(acdx,AcdxDrvrRAW_READ,&iob) < 0) {
      IErr("RAW_READ SmpClkHz", NULL);
      spts = SIN_POINTS_PER_SECOND;
   }
   spts = array[0];
   if ((spts < SIN_POINTS_PER_SECOND - 512)
   ||  (spts > SIN_POINTS_PER_SECOND + 512)) {
      printf("WARNING:Sample clock out of range:%07dHz\n",(int) spts);
      spts = SIN_POINTS_PER_SECOND;
   }
   printf("Using sample clock frequency:%dHz\n",(int) spts);

   arg = GetPath(arg,"AcdxFunc");

   printf("Function:Freq:%dHz Amplitude:%dmV Rise:%duS FlatTop:%duS Fall:%duS OutFile:%s\n",
	  (int) freq, (int) ampl, (int) rise, (int) ftop, (int) fall, path);

   umask(0);
   fp = fopen(path,"w");
   if (fp == NULL) {
      printf("Can't open file: %s for write\n",path);
      return arg;
   }

   omega = 2.0 * PI * (double) freq;
   for (i=0; i<TOTAL_POINTS; i++) {
      tee = (double) i / spts;
      func[i] = FuncToShort(sin(omega * tee) * GetAmplitude(i));
      fprintf(fp,"%lu %d\n", i*SIN_MICROS_PER_POINT, (int) func[i]);
   }

   fclose(fp);
   func_valid = 1;
   return arg;
}

/* ============================= */

int PutFunc(int arg) {

AcdxMap *map = NULL;
AcdxDrvrRawIoBlock iob;
unsigned long array[2], back;
unsigned long zbtaddr, zbtdata;
int i, j, errcnt;
FILE *fp;

   arg++; j = arg;
   arg = GetPath(arg,"AcdxFunc");

   if ((func_valid == 0) && (j == arg)) {
      printf("No valid function in memory\n");
      return arg;
   }

   if ((func_valid) && (j == arg)) {
      printf("Using function in memory\n");
   } else {
      arg = j+1;
      printf("Reading function from file:%s\n",path);
      fp = fopen(path,"r");
      if (fp == NULL) {
	 printf("Can't open file: %s for read\n",path);
	 return arg;
      }
      for (i=0; i<TOTAL_POINTS; i++) {
	 fscanf(fp,"%d %d\n", &j, (int *) &(func[i]));
	 if (j != i) {
	    printf("File read error at line:%d\n",i);
	    return arg;
	 }
      }

      fclose(fp);
      func_valid = 1;
   }

   zbtaddr = (unsigned long) ((unsigned long) &(map->ZBTSRAMAddr)) / sizeof(long);
   zbtdata = (unsigned long) ((unsigned long) &(map->ZBTSRAMData)) / sizeof(long);
   errcnt  = 0;

   printf("Load & Check");
   for (i=0; i<TOTAL_POINTS; i+=2) {

      if ((i%50000)==0) printf(".");
      fflush(stdout);

      iob.Size = 1;
      iob.UserArray = array;
      iob.Offset = zbtaddr;
      array[0] = i>>1;
      if (ioctl(acdx,AcdxDrvrRAW_WRITE,&iob) < 0) {
	 IErr("RAW_WRITE zbtaddr",(int *) &zbtaddr);
	 break;
      }

      iob.Size = 1;
      iob.UserArray = array;
      iob.Offset = zbtdata;

      array[0] = ( (0x0000FFFF & (func[i  ]      ))
	       |   (0xFFFF0000 & (func[i+1] << 16)) );
      back = array[0];

      if (ioctl(acdx,AcdxDrvrRAW_WRITE,&iob) < 0) {
	 IErr("RAW_WRITE zbtdata",(int *) &zbtdata);
	 break;
      }

      array[0] = 0;
      if (ioctl(acdx,AcdxDrvrRAW_READ,&iob) < 0) {
	 IErr("RAW_READ",NULL);
	 break;
      }
      if (back != array[0]) {
	 errcnt++;
	 if (errcnt < 10) printf("Error:Address:%d Wrote:0x%X Read:0x%X\n",i,(int) back,(int) array[0]);
      }
   }
   printf("\n");
   printf("Writtten and Readback:%d Errors:%d\n",i,errcnt);

   return arg;
}

/* ============================= */

#define AcdxIoItemLIST_LENGTH 1024
static AcdxIoItem iolist[AcdxIoItemLIST_LENGTH];
static int        iolist_len = 0;

/* ============================= */

int PlayFile(int arg, char *defnam) {

FILE *fp = NULL;

   arg = GetPath(arg,defnam);

   if (YesNo("Play file:",path)) {

      fp = fopen(path,"r");
      if (fp == NULL) {
	 printf("Can't open file: %s for read\n",path);
	 return arg;
      }

      bzero((void *) iolist, sizeof(AcdxIoItem)*AcdxIoItemLIST_LENGTH);
      iolist_len = 0;

      while ((feof(fp) == 0) && (iolist_len <  AcdxIoItemLIST_LENGTH)) {
	 fscanf(fp,"0x%X 0x%X\n",(int *) &(iolist[iolist_len].Address), (int *) &(iolist[iolist_len].Data));
	 iolist_len++;
      }
      fclose(fp);
      printf("Read:%d rows from:%s\n",(int) iolist_len, path);
      AcdxListIo(iolist, iolist_len);
      printf("Played OK\n");
   }
   return arg;
}

/* ============================= */

int PllConfig(int arg) {

   arg++;
   arg = PlayFile(arg,"AcdxConfig");
   return arg;
}

/* ============================= */

static FILE *bfp = NULL;
#define PREAM_SIZE 13
static unsigned char pream[PREAM_SIZE] = { 0,  9,
					   15, 240,
					   15, 240,
					   15, 240,
					   15, 240,
					   0,  0,
					   1 };

#define BIT_BUF_LEN 0x1000000
static unsigned char bitbuf[BIT_BUF_LEN];
static unsigned long buflen = 0;

/* =============================== */

int ReadPreamble() {
int i;
unsigned char fc;

   for (i=0; i<PREAM_SIZE; i++) {
      fc = (char) fgetc(bfp);
      if (pream[i] != fc) {
	 printf("ReadPreamble:Error:Preamble[%d] File[%d] Index:%d\n",(int) pream[i], (int) fc, i);
	 return 0;
      }
   }
   printf("ReadPreamble:OK\n");
   return 1;
}

/* =============================== */

unsigned int ReadSectionHeader() {
unsigned char fc;
unsigned short len, msw, lsw;

   fc = (char) fgetc(bfp);
   if (feof(bfp)) {
      printf("ReadSectionHeader:Unexpected EOF\n");
      return 0;
   }

   if ((fc >= 'a') && (fc <= 'd')) {
      msw = (unsigned short) fgetc(bfp);
      lsw = (unsigned short) fgetc(bfp);
      len = (msw << 8) | lsw;
      printf("ReadSectionHeader:%c:%d:OK\n",fc,(int) len);
      return (unsigned int) len;
   }
   printf("ReadSectionHeader:Bad section char:%c\n",fc);
   return 0;
}

/* ============================= */

unsigned int ReadSection(char *buf, unsigned int len) {

int i;

   for (i=0; i<len; i++) {
      if (feof(bfp)) {
	 printf("\nReadSection:Unexpected EOF\n");
	 return 0;
      }
      buf[i] = fgetc(bfp);
      if (isprint((int) buf[i])) printf("%c",buf[i]);
   }
   printf("\nReadSection:OK\n");
   return 1;
}

/* ============================= */

unsigned int ReadLength() {
unsigned char fc;
unsigned int len, lnb[4];

int i;

   buflen = 0;
   fc = (char) fgetc(bfp);
   if (feof(bfp)) {
      printf("ReadLength:Unexpected EOF\n");
      return 0;
   }
   if (fc == 'e') {
      for (i=0; i<4; i++) {
	 if (feof(bfp)) {
	    printf("ReadLength:Unexpected EOF\n");
	    return 0;
	 }
	 lnb[i] = (unsigned int) fgetc(bfp);
      }
      len = (lnb[0] << 24) | (lnb[1] << 16) | (lnb[2] << 8) | lnb[3];
      printf("ReadLength:Bytes:%c:0x%X\n",fc,len);
      if (len > BIT_BUF_LEN) {
	 printf("ReadLength:File too big > 0x%X\n",BIT_BUF_LEN);
	 return 0;
      }
      for (i=0; i<len; i++) {
	 if (feof(bfp)) {
	    printf("ReadLength:Unexpected EOF\n");
	    return 0;
	 }
	 bitbuf[i] = fgetc(bfp);
      }
      printf("ReadLength:Buffer read OK\n");
      buflen = len;
      return len;
   }
   printf("ReadLength:Bad Section header:%c\n",fc);
   return 0;
}

/* ============================= */

int WriteFPGA() {

unsigned long lock = getpid();
int n = 0, resid = buflen, sz = 0;
AcdxDrvrFpgaChunk chnk;

   if (buflen) {
      if (ioctl(acdx,AcdxDrvrLOCK,&lock) < 0) {
	 IErr("LOCK",(int *) &lock);
	 return 0;
      }

      if (ioctl(acdx,AcdxDrvrOPEN_USER_FPGA,NULL) < 0) {
	 IErr("OPEN_USER_FPGA",NULL);
	 ioctl(acdx,AcdxDrvrUNLOCK,&lock);
	 return 0;
      }

      resid = buflen;
      while (resid > 0) {

	 if (resid < AcdxDrvrCHUNK_SIZE) sz = resid;
	 else                            sz = AcdxDrvrCHUNK_SIZE;

	 bcopy((void *) &(bitbuf[n]), (void *) chnk.Chunk, sz);
	 chnk.Size = sz;

	 n += sz;
	 resid -= sz;

	 if (ioctl(acdx,AcdxDrvrWRITE_FPGA_CHUNK,&chnk) < 0) {
	    if (resid > 0) IErr("WRITE_FPGA_CHUNK",NULL);
	    break;
	 }
      }

      ioctl(acdx,AcdxDrvrCLOSE_USER_FPGA,NULL);
      ioctl(acdx,AcdxDrvrUNLOCK,&lock);

      return 1;
   }
   printf("Bit stream buffer empty\n");
   return 0;
}

/* ============================= */

int LoadBitFile(char *fname) {

char ftag[100], fpart[256], fdate[100], ftime[100];

   if (ReadPreamble()) {
      if (ReadSection(ftag,ReadSectionHeader())) {
	 if (ReadSection(fpart,ReadSectionHeader())) {
	    if (ReadSection(fdate,ReadSectionHeader())) {
	       if (ReadSection(ftime,ReadSectionHeader())) {
		  if (ReadLength()) {
		     if (YesNo("LoadBitFile:WriteFPGA:",fname)) {
			return WriteFPGA();
		     }
		  }
	       }
	    }
	 }
      }
   }
   printf("LoadBitFile:Error\n");
   return 0;
}

/* ============================= */

int LoadFpga(int arg) {

   arg++;

   arg = GetPath(arg,"Acdx.bit");

   bfp = fopen(path,"r");
   if (bfp == NULL) {
      printf("Can't open file: %s for read\n",path);
      return arg;
   }

   if (LoadBitFile(path)) printf("Loaded OK\n");
   else                   printf("Load ERROR\n");

   fclose(bfp);

   return arg;
}

/* ============================= */

int GetAqn(int arg) { /* Function number, Number of functions */

unsigned int word;
AcdxMap *map = NULL;
AcdxDrvrRawIoBlock iob;
unsigned long array[2];
unsigned long sramaddr, sramdata;
unsigned long i, j;
FILE *fp = NULL;
short data;

   arg++;

   arg = GetPath(arg,"AcdxAqn"); GetPlotFile();
   printf("Acquiring function:%d from:%d functions to:%s\n",Funcn,Funcs,path);

   sramaddr = (unsigned long) ((unsigned long) &(map->SRAMAddr)) / sizeof(long);
   sramdata = (unsigned long) ((unsigned long) &(map->SRAMData)) / sizeof(long);

   func_valid = 0; func_size = 0;

   for (i=0,j=0; i<(TOTAL_POINTS); i+=Funcs,j++) {

      word = i+Funcn-1;
      array[0] = word>>1;

      iob.Size = 1;
      iob.UserArray = array;
      iob.Offset = sramaddr;
      if (ioctl(acdx,AcdxDrvrRAW_WRITE,&iob) < 0) {
	 IErr("RAW_WRITE zbtaddr",(int *) &sramaddr);
	 break;
      }

      iob.Size = 1;
      iob.UserArray = array;
      iob.Offset = sramdata;
      if (ioctl(acdx,AcdxDrvrRAW_READ,&iob) < 0) {
	 IErr("RAW_READ zbtdata",(int *) &sramdata);
	 break;
      }

      if ((Funcn-1) & 1) data = 0xFFFF & (array[0] >> 16);
      else               data = 0xFFFF & (array[0]      );

      func[j] = data;
      func_size = j+1;
   }

   if (func_size) func_valid = 1;

   umask(0);
   fp = fopen(path,"w");
   if (fp == NULL) {
      printf("Can't open file: %s for write\n",path);
      return arg;
   }
   for (i=0; i<func_size; i++) fprintf(fp,"%lu %d\n", i*AQN_MICROS_PER_POINT, (int) func[i]);
   fclose(fp);
   printf("Written:%d values to:%s OK\n",(int) func_size,path);

   return arg;
}

/* ============================= */

int GetVersion(int arg) {

AcdxDrvrVersion vers;

   arg++;

   printf("acdxtest:Version:[%u]%s\n",COMPILE_TIME,TimeToStr(COMPILE_TIME));
   if (ioctl(acdx,AcdrDrvrGET_VERSION,&vers) < 0) {
      IErr("GET_VERSION",NULL);
      return arg;
   }
   printf("acdxdrvr:Version:[%u]%s\n",(int) vers.DrvrVersion,TimeToStr(vers.DrvrVersion));
   printf("acdxFPGA:Version:[0x%X]\n",(int) vers.FpgaVersion);
   return arg;
}

/* ============================= */

int GetTemperature(int arg) {

AcdxDrvrTemperature tprt;

   arg++;

   if (ioctl(acdx,AcdxDrvrGET_TEMPERATURE,&tprt) < 0) {
      IErr("GET_TEMPERATURE",NULL);
      return arg;
   }
   printf("Temperature:%f-C,[%d]\n",(float) (tprt.Temperature*0.0625),(int) tprt.Temperature);
   printf("TempWarning:%d\n",(int) tprt.TempWarning);
   printf("TempFailure:%d\n",(int) tprt.TempFailure);

   return arg;
}

/* ============================= */

static char *statnames[32] = {

/* 0  */ "SoftReset",
/* 1  */ "ADCRunOn",
/* 2  */ "InhibExTrig",
/* 3  */ "SofTrig",

/* 4  */ "b4",
/* 5  */ "b5",
/* 6  */ "b6",
/* 7  */ "b7",

/* 8  */ "b8",
/* 9  */ "b9",
/* 10 */ "b10",
/* 11 */ "b11",

/* 12 */ "RelayOpen",
/* 13 */ "AmpON",
/* 14 */ "b14",
/* 15 */ "Armed",

/* 16 */ "DACRun",
/* 17 */ "b17",
/* 18 */ "Running",
/* 19 */ "AqnInProgress",

/* 20 */ "DCMLocked",
/* 21 */ "InhibOneMin",
/* 22 */ "PLLLocked",
/* 23 */ "b23",

/* 24 */ "b24",
/* 25 */ "b25",
/* 26 */ "b26",
/* 27 */ "b27",

/* 28 */ "b28",
/* 29 */ "b29",
/* 30 */ "b30",
/* 31 */ "b31" };

int ReadStatus(int arg) {

unsigned long stat, msk;
int i;

   arg++;

   if (ioctl(acdx,AcdxDrvrGET_STATUS_CONTROL,&stat) < 0) {
      IErr("GET_STATUS_CONTROL",NULL);
      return arg;
   }

   stat = stat & AcdxDrvrStatusControlRD_BIT_MASK;

   printf("AcdxStatus: 0x%6X: ", (int) stat);

   for (i=0; i<32; i++) {
      msk = 1 << i;
      if (stat & msk) printf("%s ",statnames[i]);
   }
   printf("\n");
   return arg;
}

/* ============================= */

int SetControl(int arg) {

ArgVal   *v;
AtomType  at;

unsigned long ctrl, stat, msk;
int i, setbits;
unsigned long lock = getpid();
char *cp;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   setbits = 1;
   if (at == Operator) {

      if (v->OId == OprPL) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 setbits = 1;

      } else if (v->OId == OprMI) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 setbits = 0;

      } else if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Allowed Control bits:0x%06X\n",AcdxDrvrStatusControlWR_BIT_MASK);
	 for (i=0; i<32; i++) {
	    msk = 1 << i;
	    if (msk & AcdxDrvrStatusControlWR_BIT_MASK) {
	       printf("0x%06X: %s\n",(int) msk, statnames[i]);
	    }
	 }
	 printf("\n");
	 printf("ctrl + <bits> ;Set   bits\n");
	 printf("ctrl - <bits> ;Clear bits\n");

	 return arg;
      }
   }

   ctrl = 0;
   if (at == Numeric) {
      arg++;
      ctrl = v->Number;
      if ((ctrl & AcdxDrvrStatusControlWR_BIT_MASK) != ctrl) {
	 printf("Illegal control bit pattern:0x%06X\n", (int) ctrl);
	 return arg;
      }
   }

   if (ctrl == 0) {
      printf("No control value supplied\n");
      return arg;
   }

   if (ioctl(acdx,AcdxDrvrGET_STATUS_CONTROL,&stat) < 0) {
      IErr("GET_STATUS_CONTROL",NULL);
      return arg;
   }

   if (setbits) {stat |=  ctrl; cp = "AcdxControl:Set:"; }
   else         {stat &= ~ctrl; cp = "AcdxControl:Clr:"; }
   stat &= AcdxDrvrStatusControlWR_BIT_MASK;

   printf("%s: 0x%0X ", cp, (int) ctrl);
   for (i=0; i<32; i++) {
      msk = 1 << i;
      if (ctrl & msk) printf("%s ",statnames[i]);
   }
   printf("\n");

   if (ioctl(acdx,AcdxDrvrLOCK,&lock) < 0) {
      IErr("LOCK",(int *) &lock);
      return arg;
   }

   if (ioctl(acdx,AcdxDrvrSET_STATUS_CONTROL,&stat) < 0) {
      IErr("LOCK",(int *) &ctrl);
   }

   ioctl(acdx,AcdxDrvrUNLOCK,&lock);

   return arg;
}

/* ============================= */

int SetArmed(int arg) {

unsigned long stat;
unsigned long lock = getpid();

   arg++;

   if (ioctl(acdx,AcdxDrvrLOCK,&lock) < 0) {
      IErr("LOCK",(int *) &lock);
      return arg;
   }

   stat = 0x51A002;
   if (ioctl(acdx,AcdxDrvrSET_STATUS_CONTROL,&stat) < 0) {
      IErr("SET_STATUS_CONTROL",(int *) &stat);
   }

   ioctl(acdx,AcdxDrvrUNLOCK,&lock);
   printf("Armed OK\n");
   return arg;
}

/* ============================= */

int NextFunc(int arg) {

   arg++;

   Funcn++; if (Funcn > Funcs) Funcn = 1;
   printf("Function:[%d] %s\n",Funcn,FuncNames[Funcn -1]);
   return arg;
}

/* ============================= */

int PrevFunc(int arg) {

   arg++;

   Funcn--; if (Funcn < 1) Funcn = Funcs;
   printf("Function:[%d] %s\n",Funcn,FuncNames[Funcn -1]);
   return arg;
}

/* ============================= */

int SetFunc(int arg) {

ArgVal   *v;
AtomType  at;
unsigned int funcn;

   arg++;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
      arg++;
      funcn = v->Number;
      if ((funcn < 1) || (funcn > Funcs)) {
	 printf("Illegal function number:%d [1..%d]\n",funcn,Funcs);
	 return arg;
      }
      Funcn = funcn;
   }

   printf("Function:[%d] %s\n",Funcn,FuncNames[Funcn -1]);
   return arg;
}

/* ============================= */

int SetAqnFuncs(int arg) {

ArgVal   *v;
AtomType  at;
unsigned int i;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Numeric) {
      i = v->Number;
      if ((i > AcdxFUNCTIONS) || (i < 1)) {
	 printf("Bad number of functions:%d\n",i);
	 return arg;
      }
      Funcs = i;
      if (Funcn > i) Funcn=i;
   }
   printf("AqnFunctions:%d:",Funcs);
   for (i=0; i<Funcs; i++)
      printf("[%d:%s] ",i+1,FuncNames[i]);
   printf("\n");
   return arg;
}

/* ============================= */

int Plot(int arg) {

FILE *fp;
char *cp, txt[128], plt[128];

   arg++;

   cp = getenv("DISPLAY");
   if (cp) {
      GetFile("AcdxPlotAqn");
      strcpy(plt,path);
      umask(0);
      fp = fopen(path,"w");
      if (fp == NULL) {
	 printf("Can't open file: %s for write\n",path);
	 return arg;
      }
      arg = GetPath(arg,"AcdxAqn");
      if (DefaultFile) {
	 GetPlotFile();
	 fprintf(fp,"set title \"%s\"\n",Titles[Funcn -1]);
      }
      fprintf(fp,"plot \"%s\" with line\n",path);
      fprintf(fp,"pause -1 \"Hit carrige return to continue\"\n");
      fclose(fp);
      printf("Plotting %s on display:%s\n",path,cp);
      sprintf(txt,"xterm -e /usr/bin/gnuplot %s\n",plt);
      Launch(txt);
   } else printf("Environment variable DISPLAY not set\n");
   return arg;
}

/* ============================= */

static unsigned long tlow = 200000;
static unsigned long thig = 400000;

int TimeWindow(int arg) {

ArgVal   *v;
AtomType  at;
unsigned int tl, th;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Numeric) {
      tl = v->Number;

      arg++;
      v = &(vals[arg]);
      at = v->Type;

      if (at == Numeric) {
	 th = v->Number;
	 arg++;
	 if ((tl < th) && (tl+th < TOTAL_POINTS)) {
	    tlow = tl;
	    thig = th;
	 }
      }
   }

   printf("TimeWindow:low:%lu uSec hig:%lu uSec\n",tlow,thig);
   return arg;
}

/* ============================= */

double mod(double v) {
   if (v < 0) return -v;
   return v;
}

static double tzc = 0.0;

void InterZc(unsigned long p1, unsigned long p2) {

double y1, y2, x1, x2;

   y1 = (double) func[p1];
   y2 = (double) func[p2];
   x1 = (double) p1;
   x2 = (double) p2;

   tzc = (( (x2 - x1) / mod(y2 - y1) ) * mod(y1)) + x1;
}

/* ============================= */

typedef enum {
   Rising   = 1,
   Falling  = 2,
   CrossPos = 3,
   CrossNeg = 4,
   TurnTop  = 5,
   TurnBot  = 6,
   Flat     = 7
 } SegmentType;

SegmentType TestPoint(unsigned long p1,unsigned long p2) {

short v1,v2;
SegmentType st;

   v1 = func[p1];
   v2 = func[p2];

   if (v1 == v2) st = Flat;
   if (v2 >  v1) st = Rising;
   if (v1 >  v2) st = Falling;

   if ((v1 <  0) && (v2 >= 0)) { st = CrossPos; InterZc(p1,p2); }
   if ((v1 >= 0) && (v2 <  0)) { st = CrossNeg; InterZc(p1,p2); }

   return st;
}

/* ============================= */

#define LONGEST_SEGMENT 1000

SegmentType DetectSegment(unsigned long p1,unsigned long p2,unsigned long *p) {

unsigned long i, j, k;
SegmentType os, ns;

   os = 0; ns = 0;
   for (i=p1; i<p2; i++) {
      if ((os) && (os != ns)) break;
      os = ns;
      for (j=i+1,k=0; j<p2; j++,k++) {
	 ns = TestPoint(i,j);
	 if ((k > LONGEST_SEGMENT) || (ns != Flat)) break;
      }
   }
   if ((os == Rising ) && (ns == Falling)) ns = TurnTop;
   if ((os == Falling) && (ns == Rising )) ns = TurnBot;

   *p = i;
   return ns;
}

/* ============================= */

int Scan(int arg) {

FILE *fp;
unsigned long i, oi, j, points;
SegmentType st;
double ocpos, ncpos;
double ocneg, ncneg;
unsigned long maxcnt, mincnt;
double maxsum, minsum;
unsigned long cycles, tend;
double wlpsum, wlnsum;
double fac;
char *units;
AcdxFunctionParams *pars;

   ocpos =0.0; ncpos =0.0; wlpsum=0.0;
   ocneg =0.0; ncneg =0.0; wlnsum=0.0;
   maxsum=0.0; minsum=0.0;
   maxcnt=0;   mincnt=0;

   arg++;

   GetFile("AcdxAqn"); GetPlotFile();
   fp = fopen(path,"r");
   if (fp == NULL) {
      printf("Can't open file: %s for read\n",path);
      return arg;
   }

   for (i=0; i<TOTAL_POINTS; i++) {
      if (feof(fp)) break;
      fscanf(fp,"%lu %d\n", &j, (int *) &(func[i]));
      if (j != i*AQN_MICROS_PER_POINT) {
	 printf("File read error at line:%lu\n",i);
	 fclose(fp);
	 return arg;
      }
   }
   fclose(fp);

   GetFile("AcdxInflPts"); GetPlotFile();
   umask(0);
   fp = fopen(path,"w");
   if (fp == NULL) {
      printf("Can't open file: %s for write\n",path);
      return arg;
   }

   pars = &(fpars[Funcn -1]);
   pars->Tzc = 0.0;
   pars->Valid = 0;

   cycles = 0; points = 0;
   tend = thig/AQN_MICROS_PER_POINT; i = tlow/AQN_MICROS_PER_POINT;
   while(i < tend) {
      oi = i;
      st = DetectSegment(oi,tend,&i);
      if (i <= oi) {
	 printf("File seems to contain noise\n");
	 break;
      }
      switch (st) {
	 case Rising:
	 case Falling:
	    break;

	 case CrossPos:
	    fprintf(fp,"%f %d\n",(double) (tzc*AQN_MICROS_PER_POINT) ,0); points++;
	    ocpos = ncpos; ncpos = tzc;
	    if (ocpos) {
	      cycles++;
	      wlpsum += (ncpos - ocpos);
	      if (pars->Tzc == 0.0) pars->Tzc = (tzc * (double) AQN_MICROS_PER_POINT);
	    }
	 break;

	 case CrossNeg:
	    fprintf(fp,"%f %d\n",(double) (tzc*AQN_MICROS_PER_POINT) ,0); points++;
	    ocneg = ncneg; ncneg = tzc;
	    if (ocneg) wlnsum += (ncneg - ocneg);
	 break;

	 case TurnTop:
	    fprintf(fp,"%f %d\n",(double) (i*AQN_MICROS_PER_POINT) ,func[i]); points++;
	    maxcnt++; maxsum+=func[i];
	 break;

	 case TurnBot:
	    fprintf(fp,"%f %d\n",(double) (i*AQN_MICROS_PER_POINT) ,func[i]); points++;
	    mincnt++; minsum+=func[i];
	 break;

	 default:
	    printf("Warning:Scan terminated on turning point above X-axis\n");
	 break;
      }
   }
   fclose(fp);

   units = "Amps";
   if ((Funcn-1 == AcdxFunctionMAGNET_VOLTAGE)
   ||  (Funcn-1 == AcdxFunctionLEFT_OUTPUT_VOLTAGE)
   ||  (Funcn-1 == AcdxFunctionRIGHT_OUTPUT_VOLTAGE)) {
      fac = (10.0/(float) 0x7fff) * AcdxOUTPUT_VOLTAGE_FACTOR;
      units = "Vlts";
   } else if ((Funcn -1) == AcdxFunctionMAGNET_CURRENT) {
      fac = (10.0/(float) 0x7fff) * AcdxMAGNET_CURRENT_FACTOR;
   } else {
      fac = (10.0/(float) 0x7fff) * AcdxOUTPUT_CURRENT_FACTOR;
   }

   printf("Written:%lu Inflection points to:%s\n",points,path);

   pars->Valid         = 1;
   pars->ScaledMaxVal  = (maxsum / (double) maxcnt) * fac;
   pars->ScaledMinVal  = (minsum / (double) mincnt) * fac;
   pars->PosWaveLength =  wlpsum / (double) (AQN_POINTS_PER_SECOND * cycles);
   pars->NegWaveLength =  wlnsum / (double) (AQN_POINTS_PER_SECOND * cycles);

   WriteFileCalibrated();

   return arg;
}

/* ============================= */

int Calc(int arg) {

ArgVal   *v;
AtomType  at;

FILE *fp;
unsigned int f1, f2;
AcdxFunctionParams *par1, *par2;
double zeta, phse, omega, freq, lfreq, hfreq, gain, hgain, lgain;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   f1 = 1;
   if (at == Numeric) {
      f1 = v->Number;
      if ((f1 != 1) && (f1 != 2) && (f1 != 3)) {
	 printf("Illegal function number:%d [1,2,3]\n",f1);
	 return arg;
      }
      arg++;
      v = &(vals[arg]);
      at = v->Type;
   }

   f2 = FuncNext[f1];

   par2 = &(fpars[f1 -1]);
   par1 = &(fpars[f2 -1]);

   if (par1->Valid == 0) {
      printf("Function:%d %s has no scan data available\n",f1,FuncNames[f1 -1]);
      return arg;
   }
   if (par2->Valid == 0) {
      printf("Function:%d %s has no scan data available\n",f2,FuncNames[f2 -1]);
      return arg;
   }

   printf("\nFunctions[%s <=> %s]\n",Titles[f1 -1],Titles[f2 -1]);

   omega = 2*PI/par1->PosWaveLength;                    /* Radiens per second */
   zeta  = (par1->ScaledMaxVal - par1->ScaledMinVal) /
	   (par2->ScaledMaxVal - par2->ScaledMinVal);   /* Ohms */
   phse  = ((par2->Tzc-par1->Tzc)/1000000.0)*omega;     /* Phase angle radiens (Always +ve) */
   freq  = 1.0/par1->PosWaveLength;

   printf("Zeta:Ohms:%f Degs:%f\n",zeta,(phse*180.0/PI));
   printf("Zeta:Ohms:(%f + i%f)\n",zeta*cos(phse),zeta*sin(phse));
   printf("Freq:Hrtz:%f Wlen:Secs:%f\n",freq,par1->PosWaveLength);
   printf("Peak:Vlts:%f Amps:%f\n",par1->ScaledMaxVal,par2->ScaledMaxVal);
   if (f1-1 == AcdxFunctionMAGNET_CURRENT)
      printf("Magn:MilliOhms:%f MicroHenrys:%f\n",1000*zeta*cos(phse),1000000*zeta*sin(phse)/omega);


   GetFile("AcdxFreqGain");
   fp = fopen(path,"r");
   if (fp == NULL) {
      printf("Can't open file: %s for read\n",path);
      return arg;
   }

   while (!feof(fp)){
      lfreq = hfreq; lgain = hgain;
      fscanf(fp,"%lf %lf\n", &hfreq, &hgain);
      if ((freq >= lfreq) && (freq <= hfreq)) break;
   }
   fclose(fp);

   gain = freq*hgain/hfreq;

   printf("Gain:%f at Frequency:%f Corrected:Peak:Vlts:%f Amps:%f\n",
	  gain,freq,par1->ScaledMaxVal/gain,par2->ScaledMaxVal/gain);

   WriteResults(Titles[f1 -1],                // func1
		Titles[f2 -1],                // func2
		zeta,                         // ohms
		(phse*180.0/PI),              // degrees
		zeta*cos(phse),               // ohmsr
		zeta*sin(phse),               // ohmsi
		freq,                         // freq
		par1->PosWaveLength,          // wlen
		par1->ScaledMaxVal,           // pvolts
		par2->ScaledMaxVal,           // pamps
		1000*zeta*cos(phse),          // miliohms
		1000000*zeta*sin(phse)/omega, // microhens
		gain,                         // gain
		par1->ScaledMaxVal/gain,      // corpvolts
		par2->ScaledMaxVal/gain);     // corpamps

   return arg;
}

/* ============================= */

int Macro(int arg) {

char txt[128];
FILE *fp;
int ln = 0;

   arg++;
   arg = GetPath(arg,"AcdxMacro");
   fp = fopen(path,"r");
   if (fp == NULL) {
      printf("Can't open macro file:%s for read\n",path);
      return arg;
   }

   while (fgets(txt,128,fp) != NULL) {
      ln++; printf("Line:%02d: %s\n",ln,txt);
      GetAtoms(txt);
      DoCmd(0);
   }
   return arg;
}

/* ============================= */
/* Low pass filter on buf        */

static double coefbuf[AcdxCOEFS];
static double cyclbuf[AcdxCOEFS];
static int coefsiz = 0;

int LowPass(int arg) {

FILE *fp;

int i, j, k, cyp;
double new;

   arg++;

   if (coefsiz == 0) {

      GetFile("AcdxLowPass");
      fp = fopen(path,"r");
      if (fp == NULL) {
	 printf("Can't open file: %s for read\n",path);
	 return arg;
      }

      while (coefsiz < AcdxCOEFS) {
	 if (feof(fp)) break;
	 fscanf(fp,"%lf\n",&coefbuf[coefsiz]);
	 coefsiz++;
      }

      printf("Read:%d Coeficients from:%s\n",coefsiz,path);
      fclose(fp);
   } else {
      printf("Using:%d coefficients from memory\n",coefsiz);
   }

   if (coefsiz==0) {
      printf("Coeficients not in memory\n");
      return arg;
   }

   GetFile("AcdxAqn"); GetPlotFile();
   fp = fopen(path,"r");
   if (fp == NULL) {
      printf("Can't open file: %s for read\n",path);
      return arg;
   }

   func_size = 0;
   func_valid = 0;

   for (i=0; i<TOTAL_POINTS; i++) {
      if (feof(fp)) break;
      fscanf(fp,"%d %hd\n", &j, &(func[i]));
      if (j != i*AQN_MICROS_PER_POINT) {
	 printf("File read error at line:%d\n",i);
	 fclose(fp);
	 return arg;
      }
      func_size++;
   }
   fclose(fp);

   if (func_size) func_valid = 1;

   cyp = coefsiz -1;
   bzero((void *) cyclbuf, (AcdxCOEFS*sizeof(double)));
   for (i=0; i<func_size; i++) {

      cyclbuf[cyp] = (double) func[i];

      new = 0;
      for (j=0; j<coefsiz; j++) {
	 k = cyp - j; if (k < 0) k+= coefsiz;
	 new += cyclbuf[k] * coefbuf[j];
      }
      if (--cyp < 0) cyp = coefsiz -1;

      func[i] = (short) new;
   }

   umask(0);
   fp = fopen(path,"w");
   if (fp == NULL) {
      printf("Can't open file: %s for write\n",path);
      return arg;
   }
   for (i=0; i<func_size; i++) fprintf(fp,"%d %hd\n", i*AQN_MICROS_PER_POINT, (int) func[i]);
   fclose(fp);
   printf("Written:%d smoothed values to:%s OK\n",(int) func_size,path);

   return arg;
}

/* ============================= */
/* Batch mode controls YesNo     */

int Batch(int arg) {

ArgVal   *v;
AtomType  at;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Numeric) {
      yesno = v->Number;
      arg++;
   }
   return arg;
}
