/* ****************************************************************************** */
/* VMIC-5565 Reflective memory test program using the XMEM device driver.         */
/* Julian Lewis Tue 18th October 2004. First Version implemented.                 */
/* ****************************************************************************** */

extern int errno;

static char *editor = "e";

#define OBUF_SIZE 1023
static unsigned char obuf[OBUF_SIZE +1];

#define IBUF_SIZE 4095
static unsigned char ibuf[IBUF_SIZE +1];

#define ID_SIZE 3
static char id[ID_SIZE] = { '0', '1', 0 };

static int pdebug = 0;

#define STX 0x02
#define ETX 0x03

/**************************************************************************/

#define AMP_PATH "/dsrc/drivers/acdipole/rs485/amplifier"

#define CONFIG_FILE_NAME "Amp.conf"

char *defaultconfigpath = AMP_PATH CONFIG_FILE_NAME;
char *configpath = NULL;
char localconfigpath[128];  /* After a CD */

static char path[128];

/**************************************************************************/
/* Look in the config file to get paths to data needed by this program.   */
/* If no config path has been set up by the user, then ...                */
/* First try the current working directory, if not found try TEMP, and if */
/* still not found try CONF                                               */


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
      configpath = defaultconfigpath;
      gpath = fopen(configpath,"r");
      if (gpath == NULL) {
	 configpath = NULL;
	 sprintf(path,"./%s",name);
	 return path;
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

/**************************************************************************/
/* This routine handles getting File Names specified optionally on the    */
/* command line where an unusual file is specified not handled in GetFile */

char *GetFileName(int *arg) {

ArgVal   *v;
AtomType  at;
char     *cp;
int       n, earg;

   /* Search for the terminator of the filename or command */

   for (earg=*arg; earg<pcnt; earg++) {
      v = &(vals[earg]);
      if ((v->Type == Close)
      ||  (v->Type == Terminator)) break;
   }

   n = 0;
   bzero((void *) path,128);

   v = &(vals[*arg]);
   at = v->Type;
   if ((v->Type != Close)
   &&  (v->Type != Terminator)) {

      bzero((void *) path, 128);

      cp = &(cmdbuf[v->Pos]);
      do {
	 at = atomhash[(int) (*cp)];
	 if ((at != Seperator)
	 &&  (at != Close)
	 &&  (at != Terminator))
	    path[n++] = *cp;
	 path[n] = 0;
	 cp++;
      } while ((at != Close) && (at != Terminator));
   }
   if (n) {
      *arg = earg;
      return path;
   }
   return NULL;
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

volatile char *TimeToStr(time_t tod) {

static char tbuf[128];

char tmp[128];
char *yr, *ti, *md, *mn, *dy;

    bzero((void *) tbuf, 128);
    bzero((void *) tmp, 128);

    if (tod) {
	ctime_r (&tod, tmp);

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

    } else sprintf(tbuf, "--- Zero ---");

    return (tbuf);
}

/*****************************************************************/

void MyStrCpy(char *dst, char *src) {

int i;

   bzero((void *) dst, IBUF_SIZE);

   for (i=0; i<IBUF_SIZE; i++) {
      dst[i] = src[i];
      if (src[i] == 0) return;
   }
   dst[IBUF_SIZE -1] = 0;
}
/*****************************************************************/

unsigned int ByteToInt(char msb, char lsb) {

char txt[3], *ep;

  txt[0] = msb; txt[1] = lsb; txt[3] = 0;
  return strtoul(txt,&ep,10);
}

/*****************************************************************/

unsigned int CharToInt(char *ptab, int nbChar) {
/*
char txt[2], *ep;

  txt[0] = msb; txt[1] = lsb;
  printf("%c%c\n",txt[0],txt[1]);
  return strtoul(txt,&ep,16);
*/
  unsigned int myInt;
  char tab[nbChar+1],*ep;
  int i;

  for(i = 0;i < nbChar;i++)
  {
    tab[i] = ptab[i];
 //   printf("\ntab[%d]=%d %x",i,tab[i],tab[i]);
  }
  tab[nbChar] = 0;

  myInt = strtoul(tab,&ep,16);
  //printf("\nint=%d\nhex=%x\n\n",myInt,myInt);
  return myInt;
}


/*****************************************************************/

#define ITEMS 16
#define ITEM_LEN 128

static unsigned int   item = 0;
static unsigned char *items[ITEMS];
static unsigned char  icpy[IBUF_SIZE +1];
static int initialized = 0;

void InitItems() {
int i;

   if (initialized) return;

   for (i=0; i<ITEMS; i++) {
      items[i] = malloc(ITEM_LEN);
      if (items[i] == NULL) {
	 fprintf(stderr,"amptest:FATAL ERROR:Can't allocate memory\n");
	 perror("amptest");
	 exit(1);
      }
   }
}

/*****************************************************************/

int ParseIbuf(char sep) {

unsigned char *cp;
unsigned char *ep, cks, c;
int i, err, len;

   InitItems();
   bcopy((void *) ibuf, icpy, IBUF_SIZE);

   len = 0;
   for (i=0; i<IBUF_SIZE; i++) {
      c = icpy[i];
      if (c == 0) icpy[i] = ' ';
      if (c == ETX) {
	 icpy[i] = 0;
	 len = i + 1;
	 break;
      }
   }

   err = 0;
   if (len > 4) {
      cks = 0; for (i=0; i<len -2; i++) cks ^= ibuf[i];
      if (cks != ibuf[i]) err++;
      else                icpy[i] = 0;
   } else err++;
   if (err) return 0; /* Illegal string length or checksum error */

   cp = &(icpy[4]);          /* Skip STX 99 c */
   for (i=0; i<ITEMS; i++) {
      ep = index(cp,sep);
      if (ep) {
	 *ep = 0;
	 MyStrCpy(items[i],cp);
	 cp = ++ep;
      } else break;
   }
   MyStrCpy(items[i],cp); item = i+1;
   return item;
}

/*****************************************************************/

void DoCommand(char cc, unsigned int pcnt, unsigned char *pars) {

size_t sz;
int i;
unsigned char cks;

   tcflush(amp, TCIOFLUSH);

   bzero((void *) obuf, OBUF_SIZE);
   obuf[strlen(obuf)] = STX;
   obuf[strlen(obuf)] = id[0]; obuf[2] = id[1];
   obuf[strlen(obuf)] = cc;
   if (pars) {
      for (i=0; i<pcnt; i++)
	 obuf[strlen(obuf)] = pars[i];
   }
   cks = 0;
   for (i=0; i<strlen(obuf); i++) cks ^= obuf[i];
   obuf[strlen(obuf)] = cks;
   obuf[strlen(obuf)] = ETX;
   if (pdebug) {
      printf("Send:");
      for (i=0; i<pdebug; i++) {
	 printf("%02x",(unsigned) obuf[i]);
      }
      printf("\n");
      printf("Send:");
      for (i=0; i<strlen(obuf); i++) {
	 if (isprint(obuf[i])) printf(" %c",obuf[i]);
	 else                  printf("  ");
      }
      printf("\n");
      printf("Send:");
      for (i=0; i<strlen(obuf); i++) {
	 printf(" %x",obuf[i]);
      }
      printf("\n");
   }
   sz = write(amp,obuf,strlen(obuf));
   if (sz != strlen(obuf)) {
      fprintf(stderr,"testamp: Write error\n");
      perror("testamp");
   }
   bzero((void *) ibuf, IBUF_SIZE);
   tcflush(amp, TCOFLUSH);
   usleep(100000);

   sz = 0;
   do {
      sz += read(amp,&ibuf[sz],(IBUF_SIZE - sz));
   } while ((errno == EAGAIN) && (sz < IBUF_SIZE));
   tcflush(amp, TCIFLUSH);

   if (pdebug) {
      printf("Recv:");
      for (i=0; i<pdebug; i++) {
	 printf("%02x",(unsigned) ibuf[i]);
      }
      printf("\n");
      printf("Recv:");
      for (i=0; i<4; i++) printf("  ");
      for (i=0; i<pdebug; i++) printf("%02d",i);
      printf("\n");
      printf("Recv:");
      for (i=0; i<strlen(ibuf); i++) {
	 if (isprint(ibuf[i])) printf(" %c",ibuf[i]);
	 else                  printf("  ");
      }
      printf("\n");
      printf("Recv:");
      for (i=0; i<strlen(ibuf); i++) {
	printf(" %x",ibuf[i]);
      }
      printf("\n");
   }
}

/*****************************************************************/

int GetVersion(int arg) {

   arg++;
   printf("%s Compiled: %s %s\n", "amptest", __DATE__, __TIME__);
   DoCommand('I',0,NULL);

   if (ParseIbuf(',')) {
      printf("FirmwareVersion:%s\n",items[0]);
      printf("DeviceID       :%s\n",items[1]);
      printf("SerialNumber   :%s\n",items[2]);
   } else {
      printf("Illegal string or checksum error\n");
      printf("%s\n",ibuf);
   }
   return arg;
}

/*****************************************************************/

int InputLevel(int arg) {

char *cp;

   arg++;
   DoCommand('L',0,NULL);
   if (ParseIbuf(',')) {
      cp = items[0];
      printf("Level Input1:%c%c Input2:%c%c\n",cp[2],cp[3],cp[4],cp[5]);
      printf("Level Otput1:%c%c Otput2:%c%c V-meters\n",cp[6],cp[7],cp[8],cp[9]);

   } else {
      printf("Illegal string or checksum error\n");
      printf("%s\n",ibuf);
   }
   return arg;
}

/*****************************************************************/

#if 0
typedef struct {
   Kn char[2];      /* 0         */
   V1 char[2];      /* 2         */
   V2 char[2];      /* 4         */
   Mutes char[2];   /* 6         */
   T12 char[2];     /* 8         */
   Pr char;         /* 10        */
   Rd char;         /* 11        */
   Sp char[2];      /* 12        */
   Npr char[2];     /* 14        */
   Z1h char;        /* 16        */
   Z1l char;        /* 17        */
   Z2h char;        /* 18        */
   Z2l char;        /* 19        */
   G1 char[2];      /* 20        */
   G2 char[2];      /* 22        */
   OV1 char[2];     /* 24        */
   OV2 char[2];     /* 26        */
   MC char[2];      /* 28        */
   LM char;         /* 30        */
   Cnth char;       /* 31        */
   Cntl char;       /* 32        */
   Boards char;     /* 33        */
   Isel char;       /* 34        */
   Idletimeh char;  /* 35        */
   Idletimel char;  /* 36        */
   Dspcnth char;    /* 37        */
   Dspcntl char;    /* 38        */
   Dspch1 char[4];  /* 39        */
   Dspch2 char[4]'  /* 43        */
   Dspcom char[4];  /* 47 ... 50 */
 } myStruc;
#endif

int GetStatus(int arg) {

char *cp, *m1, *m2;
unsigned int v;

   arg++;
   DoCommand('S',0,NULL);
   if (ParseIbuf(',')) {
      cp = items[0];

      printf("\n");

      printf("DigitalAmp:%c%c\n",cp[0],cp[1]);

      printf("Volume Ch1:%c%c Ch2:%c%c\n",
      /*     Chan 1       Chan 2     */
	     cp[2],cp[3], cp[4],cp[5]);

      v = CharToInt(&cp[6],2);
      if (0x1 & v) m1="ON"; else m1="OFF";
      if (0x2 & v) m2="ON"; else m2="OFF";
      printf("Mutes Mute1:%s Mute2:%s\n",m1,m2);

      v = CharToInt(&cp[8],2);
      printf("Temperature:%d Degrees C\n",v);

      v = CharToInt(&cp[10],1);
      if (0x01 & v) m1 = "Protection"; else m1 = "OK";
      printf("Procection Ch1:%s ",m1);
      if (0x02 & v) m1 = "Hardware"; else m1 = "Software";
      printf("%s\n",m1);

      if (0x10 & v) m2 = "Protection"; else m2 = "OK";
      printf("Procection Ch2:%s ",m2);
      if (0x20 & v) m2 = "Hardware"; else m2 = "Software";
      printf("%s\n",m2);

      v = CharToInt(&cp[12],2);
      if (0x01 & v) m1 = "Present"; else m1 = "Absent";
      printf("Mains:%s ",m1);
      if (0x02 & v) m1 = "ON"; else m1 = "OFF";
      printf("OnOffLastSet:%s ",m1);
      if (0x04 & v) m1 = "Ready"; else m1 = "NotReady";
      printf("Channels1/2:%s ",m1);
      if (0x08 & v) m1 = "ON"; else m1 = "OFF";
      printf("OnOffDeviceStatus:%s\n",m1);

      if (0x10 & v) m1 = "Idle"; else m1 = "NotIdle";
      printf("Ch1:%s ",m1);
      if (0x20 & v) m1 = "Idle"; else m1 = "NotIdle";
      printf("Ch2:%s\n",m1);

      v = CharToInt(&cp[14],2);
      if (0x01 & v) m1 = "Present"; else m1 = "Absent";
      if (0x02 & v) m2 = "Present"; else m2 = "Absent";
      printf("InuptSignals Ch1:%s Ch2:%s\n",m1,m2);

      v = CharToInt(&cp[16],2);
      printf("Protection time from last switch:%d\n",v);

      v = CharToInt(&cp[18],4);
      printf("Load impedance Ch1:%d Ohms\n",v);
      v = CharToInt(&cp[22],4);
      printf("Load impedance Ch2:%d Ohms\n",v);

      v = CharToInt(&cp[26],2);
      printf("Gain Ch1:%d [dB] ",v);
      v = CharToInt(&cp[28],2);
      printf("Ch2:%d [dB]\n",v);

      v = CharToInt(&cp[30],2);
      printf("MaxOutputVoltage Ch1:%d [V] ",v);
      v = CharToInt(&cp[32],2);
      printf("Ch2:%d [V]\n",v);

      v = CharToInt(&cp[34],2);
      printf("MaxMainsCurrent:%d [A]\n",v);

      v = CharToInt(&cp[38],4);
      printf("ModCounter:%d\n",v);

   } else {
      printf("Illegal string or checksum error\n");
      printf("%s\n",ibuf);
   }
   return arg;
}

int GetTemp(int arg) {

char *cp;
unsigned int v;

   arg++;
   DoCommand('S',0,NULL);
   if (ParseIbuf(',')) {
      cp = items[0];
      v = CharToInt(&cp[8],2);
      printf("Temperature:%d Degrees C\n",v);
   } else {
      printf("Illegal string or checksum error\n");
      printf("%s\n",ibuf);
   }
   return arg;
}

int SetOnOff(int arg) {
ArgVal   *v;
AtomType  at;
unsigned char onof;
unsigned char param[2];

int i;

   arg++;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
     arg++;
     onof = v->Number;
      if ((onof == 0) || (onof == 1)) {
         param[0] = '1';
	 param[1] = onof + 0x30;
	 DoCommand('c',2,param);
	 if (ParseIbuf(',')) {
	    for (i=0; i<item; i++) printf("item:%d:%s\n",i,items[i]);
	 } else {
	    printf("Illegal string or checksum error\n");
	    printf("%s\n",ibuf);
	 }
	 return arg;
      }
   }
   printf("Bad OnOff value:%d Should be:[0..1]\n",(unsigned) onof);
   return arg;
}

int Mute(int arg) {
ArgVal   *v;
AtomType  at;
unsigned char onof;
unsigned char param[2];

int i;

   arg++;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
     arg++;
     onof = v->Number;
      if ((onof == 0) || (onof == 1)) {
      	 param[0] = '0';
	 param[1] = onof + 0x30;
	 DoCommand('m',2,param);
	 if (ParseIbuf(',')) {
	    for (i=0; i<item; i++) printf("item:%d:%s\n",i,items[i]);
	 } else {
	    printf("Illegal string or checksum error\n");
	    printf("%s\n",ibuf);
	 }
	 return arg;
      }
   }
   printf("Bad Mute value:%d Should be:[0..1]\n",(unsigned) onof);
   return arg;
}

int GetSetDebug(int arg) {
ArgVal   *v;
AtomType  at;

   arg++;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
     arg++;
     pdebug = v->Number;
   }
   printf("DebugLevel:%d\n",pdebug);
   return arg;
}

int SetAmpAddr(int arg) {
ArgVal   *v;
AtomType  at;
unsigned char addr;

   arg++;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
     arg++;
     addr = v->Number;
     if (addr == 1) {
      	 id[0] = '0';
	 id[1] = '1';
	 printf("Amplifier address : %d\n", addr);
	 return arg;
      }
      else if(addr == 2){
         id[0] = '0';
	 id[1] = '2';
	 printf("Amplifier address : %d\n", addr);
	 return arg;
      }
   }
   printf("Bad Address value:%d Should be:[1..2]\n",(unsigned) addr);
   return arg;
}
