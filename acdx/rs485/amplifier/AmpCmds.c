/* *************************************** */
/* Amplifier control program               */
/* Julian Lewis Mon 28th September 2009    */
/* *************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <AmpCmdP.h>

extern int errno;

static char *editor = "e";

// ------------------------
// Input and output buffers

#define OBUF_SIZE 1023
static int  OBufSize = 0;
static char OBuf[OBUF_SIZE +1];

#define IBUF_SIZE 0x1000
static int  IBufSize = 0;
static char IBuf[IBUF_SIZE +1];

// -------------------------------------------------------------------------
// Escaped buffer (where special characters are replaced by escape sequences)
// we copy to and from here when replacing specials

#define ESC_BUF_SIZE 4096
static int  EscBufSize = 0;
static char EscBuf[ESC_BUF_SIZE];

// -------------------
// Amplifier ID string

#define ID_SIZE 3
static char Id[ID_SIZE] = { '0', '1', 0 };
static int AmpIndx = 0;

static int pdebug = 0;
static int slow = 0;

// ------------------------------------------------------------------------
// Special characters can not be part of a data stream and must be replaced
// by an "esacping strategy" see PowerSoft manual Addendum-5

#define SPECIAL_CHARS 5

#define STX 0x02
#define ETX 0x03
#define ESC 0x1B
#define OPB 0x7B
#define CLB 0x7D

static char SpecialChars[SPECIAL_CHARS] = { STX, ETX, ESC, OPB, CLB };

// ------------------
// Packet description

#define PAYLOAD_SIZE 4096

static GenericPacket Pkt;
static int PayloadSize = 0;
static char Payload[PAYLOAD_SIZE];

// --------------------------------------------------------------
// To get relative time we nee the amps working time, one per amp

static int WorkingTime[2] = { 0, 0 };

// ---------------------------------------------------------------------
// Wave samples, actually this isn't implemented on the amp firmware yet

static int WaveSampLen = 0;
static int WaveSamp[800];
static int Blk = 0;

// ---------------------------------------------
// Comfiguration data files used in this program

#define AMP_PATH "/usr/local/drivers/acdx/test/"

#define CONFIG_FILE_NAME "amptest.config"

char *defaultconfigpath = AMP_PATH CONFIG_FILE_NAME;
char *configpath = NULL;
char localconfigpath[128];  /* After a CD */

static char path[128];

// -----------------------------------------------------------------------
// Get the root name of a file

char *GetRouteName(char *name) {
int i, j;
static char res[128];

   if (name == NULL) return NULL;

   bzero((void *) res, 128);
   i = strlen(name); if (i > 128) i = 128 -1;

   for (; i>=0; i--) if (name[i] == '/') break;

   for (j=0; j<i; j++) res[j] = name[j];
   return res;
}

// -----------------------------------------------------------------------
// Look in the config file to get paths to data needed by this program.
// If no config path has been set up by the user, then ...
// First try the current working directory, if not found try TEMP, and if
// still not found try CONF

static int warnpath = 1;

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
	 configpath = localconfigpath;
	 sprintf(configpath,"./%s",CONFIG_FILE_NAME);
	 gpath = fopen(configpath,"r");
	 if (gpath == NULL) {
	    configpath = NULL;
	    sprintf(path,"./%s",name);
	    if (warnpath) {
	       warnpath = 0;
	       fprintf(stderr,"Warning:%s Not found: using local directory\n",CONFIG_FILE_NAME);
	    }
	    return path;
	 }
	 if (warnpath) {
	    warnpath = 0;
	    fprintf(stderr,"Warning:%s Not found\n",defaultconfigpath);
	    fprintf(stderr,"Warning:%s is in local directory\n",CONFIG_FILE_NAME);
	 }
      }
   }

   printf("%s: ",path);
   bzero((void *) path,128);

   while (1) {
      if (fgets(txt,128,gpath) == NULL) { perror("fgets"); break; }
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

// ------------------------------------------------------------------------
// This routine handles getting File Names specified optionally on the
// command line where an unusual file is specified not handled in GetFile

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

/* ============================= */
/* News                          */

int News(int arg) {

char sys[128], npt[128];

   arg++;

   if (GetFile("amp_news")) {
      strcpy(npt,path);
      sprintf(sys,"%s %s",GetFile(editor),npt);
      printf("\n%s\n",sys);
      system(sys);
      printf("\n");
   }
   return(arg);
}

// ---------------------------------------
// Get confirmation from user

static int yesno=1;

static int YesNo(char *question, char *name) {
int yn, c;

   if (yesno == 0) return 1;

   printf("%s: %s\n",question,name);
   printf("Continue (Y/N):"); yn = getchar(); c = getchar();
   if ((yn != (int) 'y') && (yn != 'Y')) return 0;
   return 1;
}

// ------------------------------------
// Change text editor

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

// ------------------------------------
// Convert time to a string

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

// =====================================================================

unsigned int CharToInt(char *ptab, int nbChar) {

unsigned int myInt;
char tab[nbChar+1],*ep;
int i;

  for(i = 0;i < nbChar;i++)
  {
    tab[i] = ptab[i];
  }
  tab[nbChar] = 0;

  myInt = strtoul(tab,&ep,16);
  return myInt;
}

// =====================================================================

#define ITEMS 16
#define ITEM_LEN 128

static unsigned int   item = 0;
static char *items[ITEMS];
static char  icpy[IBUF_SIZE +1];
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

int ParseIBuf(char sep) {

char *cp;
char *ep, cks, c;
int i, err, len;

   InitItems();
   bcopy((void *) IBuf, icpy, IBUF_SIZE);

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
      cks = 0; for (i=0; i<len -2; i++) cks ^= IBuf[i];
      if (cks != IBuf[i]) err++;
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

// ==========================================================

int IsSpecial(char c) {
int i;
   for (i=0; i<SPECIAL_CHARS; i++) {
      if (SpecialChars[i] == c) return 1;
   }
   return 0;
}

// ==========================================================

void EscapeOBuf() {

int i, j;
unsigned char c;

   if ((OBufSize > 0) && (OBuf[0] == STX) && (OBuf[OBufSize-1] == ETX)) {

      bzero((void *) EscBuf, ESC_BUF_SIZE);
      EscBufSize = 0;
      EscBuf[0] = STX;

      for (i=1,j=1; i<OBufSize-1; i++) {
	 c = OBuf[i];
	 if (IsSpecial(c)) {
	    EscBuf[j++] = ESC;
	    EscBuf[j++] = c + 0x40;
	 } else {
	    EscBuf[j++] = OBuf[i];
	 }
      }
      EscBuf[j++] = ETX;

      for (i=0; i<j; i++) OBuf[i] = EscBuf[i];
      EscBufSize = j;
      OBufSize = j;
      return;
   }
   fprintf(stderr,"EscapeBuffer:Illegal size or not delimited by STX ETX\n");
}

// ==========================================================

void UnEscapeIBuf() {

int i, j;
unsigned char c;

   if ((IBufSize > 0) && (IBuf[0] == STX) && (IBuf[IBufSize-1] == ETX)) {

      for (i=0; i<IBufSize; i++) EscBuf[i] = IBuf[i];
      EscBufSize = IBufSize;

      bzero((void *) IBuf, IBUF_SIZE);
      IBufSize = 0;
      IBuf[0] = STX;

      for (i=1,j=1; i<EscBufSize-1; i++) {
	 c = EscBuf[i];
	 if (c == ESC) {
	    i++;
	    IBuf[j++] = EscBuf[i] - 0x40;
	 } else {
	    IBuf[j++] = EscBuf[i];
	 }
      }
      IBuf[j++] = ETX;
      IBufSize = j;
      return;
   }
   fprintf(stderr,"UnEscapeBuffer:Illegal size or not delimited by STX ETX\n");
}

// ==========================================================
// Reverse search for ETX at end of string to get size

int GetBufSize(char *buf, int sz) {

int i;

   if (buf[0] == STX) {
      for (i=sz-1; i>0; i--) {
	 if (buf[i] == ETX) return i+1;
	 if (buf[i] != 0) break;
      }
   }
   fprintf(stderr,"GetBufSize:Not delimited by STX ETX\n");
   return sz;
}

// ==========================================================

char CalcCheckSum(PktType pkty, GenericPacket *pkt, int psz) {
int i;
char cks;

   cks = 0;

   cks ^= pkt->Head;
   cks ^= pkt->DeviceId[0];
   cks ^= pkt->DeviceId[1];
   cks ^= pkt->CmdId;

   if (pkty == PktReply) {
      cks ^= pkt->ModleId[0];
      cks ^= pkt->ModleId[1];
   }

   if (pkt->Payload) {
      for (i=0; i<psz; i++) {
	 cks ^= pkt->Payload[i];
      }
   }

   if ((cks == 0) || (cks == STX) || (cks == ETX)) cks += 0x0A;

   return cks;
}

// ==========================================================

void PrintPayload(char *buf, int sz) {

int i;
unsigned char c;

   printf("Payload:\n[  ");
   for (i=0; i<sz; i++) {
      c = buf[i];
      if (isalnum(c))   printf("%03d:%c[0x%02X] ",i,(int) c, (int) c);
      else              printf("%03d:.[0x%02X] ",i,(int) c);
      if (((i+1)%6)==0) printf("\n   ");
   }
   printf("]\n");
}

// ==========================================================

void DisplayPkt(PktType pkty, GenericPacket *pkt, int sz) {

char cks;

   if (pdebug == 0) return;

   if (pkty == PktGeneric) printf("Send: ");
   if (pkty == PktReply  ) printf("Rply: ");

   if (pkt->Head == STX) printf("Head[STX] ");
   else                  printf("\n%d (Bad header byte)\n",(int) pkt->Head);

   printf("DeviceId[%c%c] ",pkt->DeviceId[0],pkt->DeviceId[1]);
   printf("Command[%c] ",pkt->CmdId);

   if (pkty == PktReply)
      printf("ModelId[%c%c] ",pkt->ModleId[0],pkt->ModleId[1]);

   if (pkt->Payload) {
      if (pkty == PktGeneric) printf("\nSend: ");
      if (pkty == PktReply  ) printf("\nRply: ");

      PrintPayload(pkt->Payload,sz);
   }

   cks = CalcCheckSum(pkty,pkt,sz);
   if (cks == pkt->Checksum)
      printf("Checksum[OK(0x%X)] ",cks);
   else
      printf("\nChecksum[0x%X] Bad:Should be:[0x%X]\n",(int) pkt->Checksum, (int) cks);

   if (pkt->Tail == ETX) printf("Tail[ETX]\n");
   else                  printf("\n%d (Bad tail byte)\n",(int) pkt->Tail);

   printf("\n");
}

// =====================================================

void PrintBuffer(char *buf, char *name, int len) {

int i, sz;

   if (pdebug) {
      printf("%s:[",name);
      sz = GetBufSize(buf,len);
      for (i=0; i<sz; i++) {
	 printf("%02X",((int) buf[i]) & 0xFF);
      }
      printf("]\n");
   }

   if (pdebug > 1) {
      printf("%s:[",name);
      sz = GetBufSize(buf,len);
      for (i=0; i<sz; i++) {
	 if (isalnum(buf[i])) printf(" %c",buf[i]);
	 else                 printf("..");
      }
      printf("]\n");
   }
}

// =====================================================

char *Serialize(GenericPacket *pkt, int psz) {

int i, j;

   bzero((void *) OBuf, OBUF_SIZE);

   i = 0;
   OBuf[i++] = pkt->Head;
   OBuf[i++] = pkt->DeviceId[0];
   OBuf[i++] = pkt->DeviceId[1];
   OBuf[i++] = pkt->CmdId;

   if (pkt->Payload) for (j=0; j<psz; j++) OBuf[i++] = pkt->Payload[j];

   OBuf[i++] = pkt->Checksum;
   OBuf[i++] = pkt->Tail;

   PrintBuffer(OBuf,"OBuf",OBUF_SIZE);
   return OBuf;
}

// =====================================================

char *SendRecvCommand(char cid, char *par, int psz) {

CmdDesc *cmdsc;
int i, bi, cm, tmo, ibinc, wexit;
char chr, cks;

   tcflush(amp, TCIOFLUSH);

   for (cm=0; cm<COMMANDS; cm++) {
      cmdsc = &(cmdtb[cm]);
      if (cid == cmdsc->Id) {

	 Pkt.Head = STX;
	 Pkt.Tail = ETX;

	 Pkt.DeviceId[0] = Id[0];
	 Pkt.DeviceId[1] = Id[1];

	 Pkt.CmdId = cid;

	 if (par) Pkt.Payload = par;
	 else     Pkt.Payload = NULL;

	 Pkt.Checksum = CalcCheckSum(PktGeneric,&Pkt,psz);

	 Serialize(&Pkt,psz);
	 DisplayPkt(PktGeneric,&Pkt,psz);

	 OBufSize = GetBufSize(OBuf,OBUF_SIZE);
	 EscapeOBuf();

	 if (OBufSize != write(amp,OBuf,OBufSize)) {
	    fprintf(stderr,"testamp: Write error\n");
	    perror("testamp");
	 }
	 //tcflush(amp, TCOFLUSH);
	 //tcdrain(amp);

	 if (slow) usleep(slow);

	 tmo = 0; IBufSize = 0; wexit = 0;
	 do {

	    usleep(100000); // It slow give it 100 ms between reads (10^5 us)

	    ibinc = read(amp,&IBuf[IBufSize],(IBUF_SIZE - IBufSize));
	    if (ibinc > 0) {
	       IBufSize += ibinc;
	       wexit++;
	    } else if (wexit) break;

	    if (tmo++ > 50) {
	       fprintf(stderr,"Amplifier has not responed after 5 seconds\n");
	       perror("read");

	       amp = AmpRecover();
	       if (amp == 0) {
		  fprintf(stderr,"FATAL: Can't recover from error\n");
		  perror("open");
	       }
	       return NULL;
	    }

	 } while ((errno == EAGAIN) && (IBufSize < IBUF_SIZE)); // Wait for data

	 IBufSize = GetBufSize(IBuf,IBufSize);

	 PrintBuffer(IBuf,"ERawIBuf",IBufSize);
	 UnEscapeIBuf();
	 PrintBuffer(IBuf,"UEscIBuf",IBufSize);

	 bi = 0;

	 Pkt.Head        = IBuf[bi++];
	 Pkt.DeviceId[0] = IBuf[bi++];
	 Pkt.DeviceId[1] = IBuf[bi++];
	 Pkt.CmdId       = IBuf[bi++];
	 Pkt.ModleId[0]  = IBuf[bi++];
	 Pkt.ModleId[1]  = IBuf[bi++];

	 bzero((void *) Payload, PAYLOAD_SIZE);
	 for (i=0; bi<IBufSize-2; bi++) Payload[i++] = IBuf[bi];
	 PayloadSize = i;

	 cks = IBuf[IBufSize-2];
	 chr = IBuf[IBufSize-1];

	 if (i) Pkt.Payload = Payload;
	 else   Pkt.Payload = NULL;

	 Pkt.Checksum = cks;
	 cks = CalcCheckSum(PktReply,&Pkt,i);
	 if (cks != Pkt.Checksum) {
	    printf("Reply:Checksum error\n");
	 }

	 Pkt.Tail = chr;
	 if (chr != ETX) {
	    printf("Reply:Not terminated by ETX\n");
	 }

	 DisplayPkt(PktReply,&Pkt,PayloadSize);

	 return Payload;
      }
   }
   return NULL;
}

// =====================================================

int GetMeanTemperature(int arg) {

ArgVal   *v;
AtomType  at;
char par[16];

int i, t1, t2, period;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Get mean temperature over a period:[0..10]\n");
	 printf("   00: From:   0: To: Current\n");
	 for (i=1; i<=10; i++) {
	    printf("   %02d: From:%4d To:%4d Hours\n",i,(i-1)*1000,i*1000);
	 }

	 return arg;
      }
   }

   period = 0;

   if (at == Numeric) {
      arg++;
      v = &(vals[arg]);
      at = v->Type;

      period = v->Number;
      if ((period < 0) || (period > 10)) {
	 fprintf(stderr,"GetMeanTemperature: Illegal Time Period:%02d\n",period);
	 period = 0;
      }
   }

   sprintf(par,"%02d",period);
   if (SendRecvCommand('B',par,2)) {
      t1 = CharToInt(&(Payload[0]),2);
      t2 = CharToInt(&(Payload[2]),2);
      printf("Mean temp:%d-%d %s for period:%d\n",t1,t2,Payload,period);
   }
   return arg;
}

// =====================================================

int GetTurnOnCount(int arg) {

ArgVal   *v;
AtomType  at;

int cnt;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Gets the number of times the amplifier has been powered on\n");
	 return arg;
      }
   }

   if (SendRecvCommand('A',NULL,0)) {
      cnt = CharToInt(&(Payload[0]),6);
      printf("Turn On Count:%d %s\n",cnt,Payload);
   }
   return arg;
}

// =====================================================

typedef struct {           /* Reply to E */
   char TimeFrstTurnOn[6]; /* 0  */
   char ChanId        [2]; /* 6  */
   char Type          [2]; /* 8  */
   char Next          [4]; /* 10 */
 } AmpRecord;

int GetEventHistory(int arg) {

ArgVal   *v;
AtomType  at;

char event[16];
AmpRecord *arec;
int et, cnt, i, evt, ago, ton;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Gets the recorded event list from the amplifier\n");
	 printf("Takes argument: Count: the number of events to print\n");
	 printf("Takes argument: EventId: The starting event ID (See U)\n");
	 printf("If you run the N command, the relative time is displaye aswell\n");
	 return arg;
      }
   }

   cnt = 10;
   if (at == Numeric) {
      cnt = v->Number;
      if ((cnt < 1) || (cnt > 0xFFFF)) {
	 fprintf(stderr,"GetEventHistory: Illegal Count:%02d\n",cnt);
	 cnt = 10;
      }

      arg++;
      v = &(vals[arg]);
      at = v->Type;
   }

   i = 0; arec = (AmpRecord *) Payload;

   evt = 0xFFFF;
   if (at == Numeric) {
      evt = v->Number;
      if (evt > 0xFFFF) {
	 fprintf(stderr,"GetEventHistory: Illegal event:%d\n",evt);
	 evt = 0xFFFF;
      }

      arg++;
      v = &(vals[arg]);
      at = v->Type;
   }

   i = 0; arec = (AmpRecord *) Payload;
   sprintf(event,"%04X",evt);

   while (1) {
      i++;

      if (SendRecvCommand('E',event,4)) {

	 printf("\n---------\nEvent:%d Id:%s\n",i,event);
	 printf("TimeTurnOn:%d-Minutes\n",CharToInt(arec->TimeFrstTurnOn,6));
	 ton = CharToInt(arec->TimeFrstTurnOn,6);
	 printf("TimeStamp :%d-Minutes",ton);
	 if (WorkingTime[AmpIndx]) {
	    ago = WorkingTime[AmpIndx] - ton;
	    printf(" [%d] Minutes ago =  Hrs[%d] Days[%d]",ago, ago/60, ago/(60*24));
	 }
	 printf("\n");
	 printf("ChannelId :%d (0=All)\n",CharToInt(arec->ChanId,2));

	 et = CharToInt(arec->Type,2);
	 printf("EventType :%d ",et);
	 switch (et) {
	    case 0: printf("ShortCircuit-Chan1\n");
	    break;
	    case 1: printf("ShortCircuit-Chan2\n");
	    break;
	    case 2: printf("Temperature-Warning\n");
	    break;
	    case 3: printf("Temperature-Critical\n");
	    break;
	    case 4: printf("+15V Fault\n");
	    break;
	    case 5: printf("-15V Fault\n");
	    break;
	    case 6: printf("+5V Analog Fault\n");
	    break;
	    case 7: printf("External voltage PowerControl Hub Fault\n");
	    break;
	    case 8: printf("+Ve power supply rail bus voltage Fault Chan1\n");
	    break;
	    case 9: printf("+Ve power supply rail bus voltage Fault Chan2\n");
	    break;
	    case 10: printf("-Ve power supply rail bus voltage Fault Chan1\n");
	    break;
	    case 11: printf("-Ve power supply rail bus voltage Fault Chan2\n");
	    break;
	    case 12: printf("Clock 192KHz system Fault\n");
	    break;
	    case 13: printf("Chan1 protection Clear\n");
	    break;
	    case 14: printf("Chan2 protection Clear\n");
	    break;
	    default:
	    break;
	 }
	 printf("NextEvent :%d\n",CharToInt(arec->Next,4));
	 strncpy(event,arec->Next,4);
	 if (strcmp(event,"0000") == 0) break;
	 if (i>=cnt) break;
      }
   }
   return arg;
}

/*****************************************************************/

int GetVersion(int arg) {

   arg++;
   printf("%s Compiled: %s %s\n", "amptest", __DATE__, __TIME__);
   if (SendRecvCommand('I',NULL,0)) {

      if (ParseIBuf(',')) {
	 printf("FirmwareVersion:%s\n",items[0]);
	 printf("DeviceID       :%s\n",items[1]);
	 printf("SerialNumber   :%s\n",items[2]);
      } else {
	 printf("Illegal string or checksum error\n");
	 printf("%s\n",IBuf);
      }
   }
   return arg;
}

// ==========================================================

typedef struct {
   char OutCurrent1[2];
   char OutCurrent2[2];
   char OutVoltage1[2];
   char OutVoltage2[2];
 } AmpLevel;

int Levels(int arg) {

AmpLevel *alev;
int ol1, ol2, ov1, ov2;

   arg++;
   if (SendRecvCommand('L',NULL,0)) {

      alev = (AmpLevel *) Payload;

      ol1 = CharToInt(alev->OutCurrent1,2);
      ol2 = CharToInt(alev->OutCurrent2,2);
      ov1 = CharToInt(alev->OutVoltage1,2);
      ov2 = CharToInt(alev->OutVoltage2,2);

      printf("OutCurrent 1-%d 2-%d OutVoltage 1-%d 2-%d\n",ol1, ol2, ov1, ov2);
   }
   return arg;
}

// ==========================================================

int ReadStatBlk() {

   if (SendRecvCommand('S',NULL,0)) {
      bcopy((void *) Payload, &StatBlk, sizeof(AmpStatus));
      StatBlkValid = 1;
      return 1;
   }
   return 0;
}

// ==========================================================

void DisplayStatus() {

char *m1, *m2;
unsigned int v;

   if (StatBlkValid) {

      printf("\n");
      printf("DigitalAmp:%c%c\n",Id[0],Id[1]);

      printf("OutAttenuation Ch1:%c%c Ch2:%c%c\n",
	     StatBlk.OutAten1[0], StatBlk.OutAten1[1],
	     StatBlk.OutAten2[0], StatBlk.OutAten2[1]);

      v = CharToInt(StatBlk.Mute,2);
      if (0x1 & v) m1="ON"; else m1="OFF";
      if (0x2 & v) m2="ON"; else m2="OFF";
      printf("Mutes Mute1:%s Mute2:%s\n",m1,m2);

      v = CharToInt(StatBlk.Temp12,2);
      printf("Temperature:%d Degrees C\n",v);

      v = CharToInt(StatBlk.Protect,2);
      if (0x01 & v) m1 = "Protection"; else m1 = "OK";
      printf("Procection Ch1:%s ",m1);
      if (0x02 & v) m1 = "Hardware"; else m1 = "Software";
      printf("%s\n",m1);
      if (0x10 & v) m2 = "Protection"; else m2 = "OK";
      printf("Procection Ch2:%s ",m2);
      if (0x20 & v) m2 = "Hardware"; else m2 = "Software";
      printf("%s\n",m2);

      v = CharToInt(StatBlk.Ready,2);
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

      v = CharToInt(StatBlk.SignalPres,2);
      if (0x01 & v) m1 = "Present"; else m1 = "Absent";
      if (0x02 & v) m2 = "Present"; else m2 = "Absent";
      printf("InuptSignals Ch1:%s Ch2:%s\n",m1,m2);

      v = CharToInt(StatBlk.ProtectCount,2);
      printf("Protection count:%d\n",v);

      v = CharToInt(StatBlk.Impedance1,4);
      printf("Load impedance Ch1:%d Ohms\n",v);
      v = CharToInt(StatBlk.Impedance2,4);
      printf("Load impedance Ch2:%d Ohms\n",v);

      v = CharToInt(StatBlk.Gain1,2);
      printf("Gain Ch1:%d [dB] ",v);
      v = CharToInt(StatBlk.Gain2,2);
      printf("Ch2:%d [dB]\n",v);

      v = CharToInt(StatBlk.OutVoltage1,2);
      printf("MaxOutputVoltage Ch1:%d [V] ",v);
      v = CharToInt(StatBlk.OutVoltage2,2);
      printf("Ch2:%d [V]\n",v);

      v = CharToInt(StatBlk.MaxMainsCur,2);
      printf("MaxMainsCurrent:%d [A]\n",v);

      v = CharToInt(StatBlk.Limiter,2);
      printf("Limiter:%d ",v);
      printf("Ch1:Limt:"); if (v & 0x01) printf("Active "); else printf("Off ");
      printf("Ch2:Limt:"); if (v & 0x02) printf("Active "); else printf("Off ");
      printf("Ch1:Gate:"); if (v & 0x04) printf("Active "); else printf("Off ");
      printf("Ch2:Gate:"); if (v & 0x08) printf("Active "); else printf("Off ");
      printf("\n");

      v = CharToInt(StatBlk.ModCounter,4);
      printf("ModCounter:%d\n",v);

      v = CharToInt(StatBlk.Boards,2);
      printf("Boards: ");
      if (0x01 & v) printf("KFRONT ");
      if (0x02 & v) printf("KCNTRL ");
      if (0x04 & v) printf("KKAESOP ");
      if (0x08 & v) printf("KDSP ");
      if (0x10 & v) printf("KSPARE ");
      printf("\n");

      v = CharToInt(StatBlk.InputRout,2);
      printf("InputRouting: ");
      if (v == 0x00) printf("ANALOG=>OUT ");
      if (v == 0x01) printf("ANALOG=>DSP=>OUT ");
      if (v == 0x02) printf("AES3=>OUT ");
      if (v == 0x03) printf("AES3=>DSP=>OUT ");
      if (v == 0x04) printf("KAESOP=>OUT ");
      if (v == 0x05) printf("KAESOP=>DSP=>OUT ");
      printf("\n");

      v = CharToInt(StatBlk.Idletime,4);
      printf("IdleTime:%d\n",v);

      v = CharToInt(StatBlk.DspModCount,4);
      printf("DspModCounter:%d\n",v);

      v = CharToInt(StatBlk.DspCh1Crc,4);
      printf("DspCrc1:%d\n",v);
      v = CharToInt(StatBlk.DspCh2Crc,4);
      printf("DspCrc2:%d\n",v);
      v = CharToInt(StatBlk.DspComCrc,4);
      printf("DspCrc0:%d\n",v);

   } else {
      printf("DisplayStatus:No valid status in memory, read it first\n");
   }
}

// ==========================================================

int GetStatus(int arg) {

   arg++;
   ReadStatBlk();
   printf("OK\n");
   return arg;
}

// ==========================================================

int ShowStatus(int arg) {
   arg++;
   DisplayStatus();
   return arg;
}

// ==========================================================

int GetTemp(int arg) {

unsigned int v;

   arg++;
   if (ReadStatBlk()) {
      v = CharToInt(StatBlk.Temp12,2);
      printf("Temperature:%d Degrees C\n",v);
   }
   return arg;
}

// ==========================================================

int SetOnOff(int arg) {
ArgVal   *v;
AtomType  at;
char onof;
char param[4];

int ack;

   arg++;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
     arg++;
     onof = v->Number;
      if ((onof == 0) || (onof == 1)) {
	 param[0] = Id[1];
	 param[1] = onof + 0x30;
	 param[2] = 0;
	 if (SendRecvCommand('c',param,2)) {

	    ack = (int) Payload[0];
	    printf("Response:");
	    if      (ack == ACK)  printf("ACK=OK");
	    else if (ack == NACK) printf("NACK=NotOK");
	    else if (ack == INV)  printf("INV=InvalidRequest");
	    else if (ack == OOR)  printf("OOR=OutOfRange");
	    else                  printf("Unexpected:%d This shouldn't happen",ack);
	    printf("\n");
	 }
	 return arg;
      }
   }
   printf("Bad OnOff value:%d Should be:[0..1]\n",(unsigned) onof);
   return arg;
}

// ==========================================================

int Mute(int arg) {
ArgVal   *v;
AtomType  at;
char onof;
char param[4];

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
	 param[2] = 0;
	 if (SendRecvCommand('m',param,2)) {
	    if (ParseIBuf(',')) {
	       for (i=0; i<item; i++) printf("item:%d:%s\n",i,items[i]);
	    } else {
	       printf("Illegal string or checksum error\n");
	       printf("%s\n",IBuf);
	    }
	 }
	 return arg;
      }
   }
   printf("Bad Mute value:%d Should be:[0..1]\n",(unsigned) onof);
   return arg;
}

// ==========================================================

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

// ==========================================================

int SetAmpAddr(int arg) {
ArgVal   *v;
AtomType  at;
char addr;

   arg++;
   v = &(vals[arg]);
   at = v->Type;
   if (at == Numeric) {
     arg++;
     addr = v->Number;
     if (addr == 1) {
	 Id[0] = '0';
	 Id[1] = '1';
	 AmpIndx = 0;
	 printf("Amplifier address : %d\n", addr);
	 return arg;
      }
      else if(addr == 2){
	 Id[0] = '0';
	 Id[1] = '2';
	 AmpIndx = 1;
	 printf("Amplifier address : %d\n", addr);
	 return arg;
      }
   }
   printf("Bad Address value:%d Should be:[1..2]\n",(unsigned) addr);
   return arg;
}

// ==========================================================

int GetVoltageClockTempStat(int arg) {

ArgVal   *v;
AtomType  at;

int auxv, aux5v, busv, istat, led;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Gets device voltage, clock and temperature led status\n");
	 return arg;
      }
   }

   if (SendRecvCommand('T',NULL,0)) {

      auxv   = CharToInt(&Payload[0],2);
      aux5v  = CharToInt(&Payload[2],2);
      busv   = CharToInt(&Payload[4],2);
      istat  = CharToInt(&Payload[6],2);
      led    = CharToInt(&Payload[8],2);

      printf("AuxVoltage  :%d\n",auxv);
      printf("Aux5V       :%d\n",aux5v);
      printf("BusVoltage  :%d\n",busv);
      printf("InternalStat:");
      if (istat & 0x01) printf("LED-Active ");
      if (istat & 0x02) printf("VAux-OK "); else printf("VAux-Fault ");
      if (istat & 0x04) printf("IBGTConvertor-Active "); else printf("IBGTConvertor-InActive ");
      if (istat & 0x08) printf("BOOST-Active "); else printf("BOOST-InActive ");
      printf("\n");

      printf("LEDStatus   :");
      if (led & 0x01) printf("NotActive ");
      if (led & 0x02) printf("Blinking ");
      if (led & 0x04) printf("Active ");
      printf("\n");
   }
   return arg;
}

// ====================================================

int GetLastEvent(int arg) {

ArgVal   *v;
AtomType  at;

char event[16];
AmpRecord *arec;
int et, ton, ago;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Gets the last recorded event from the amplifier\n");
	 printf("If you run the N command, the relative time is displaye aswell\n");

	 return arg;
      }
   }

   arec = (AmpRecord *) Payload;

   if (SendRecvCommand('U',NULL,0)) {

      bzero((void *) event, 16);
      strncpy(event,arec->Next,4);

      printf("LastEvent :Id:%s\n",event);
      ton = CharToInt(arec->TimeFrstTurnOn,6);
      printf("TimeStamp :%d-Minutes",ton);
      if (WorkingTime[AmpIndx]) {
	 ago = WorkingTime[AmpIndx] - ton;
	 printf(" [%d] Minutes ago =  Hrs[%d] Days[%d]",ago, ago/60, ago/(60*24));
      }
      printf("\n");
      printf("ChannelId :%d (0=All)\n",CharToInt(arec->ChanId,2));

      et = CharToInt(arec->Type,2);
      printf("EventType :%d ",et);
      switch (et) {
	 case 0: printf("ShortCircuit-Chan1\n");
	 break;
	 case 1: printf("ShortCircuit-Chan2\n");
	 break;
	 case 2: printf("Temperature-Warning\n");
	 break;
	 case 3: printf("Temperature-Critical\n");
	 break;
	 case 4: printf("+15V Fault\n");
	 break;
	 case 5: printf("-15V Fault\n");
	 break;
	 case 6: printf("+5V Analog Fault\n");
	 break;
	 case 7: printf("External voltage PowerControl Hub Fault\n");
	 break;
	 case 8: printf("+Ve power supply rail bus voltage Fault Chan1\n");
	 break;
	 case 9: printf("+Ve power supply rail bus voltage Fault Chan2\n");
	 break;
	 case 10: printf("-Ve power supply rail bus voltage Fault Chan1\n");
	 break;
	 case 11: printf("-Ve power supply rail bus voltage Fault Chan2\n");
	 break;
	 case 12: printf("Clock 192KHz system Fault\n");
	 break;
	 case 13: printf("Chan1 protection Clear\n");
	 break;
	 case 14: printf("Chan2 protection Clear\n");
	 break;
	 default:
	 break;
      }
      printf("NextEvent :%d\n",CharToInt(arec->Next,4));
   }
   return arg;
}

// ====================================================

int GetWorkingTime(int arg) {

ArgVal   *v;
AtomType  at;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Get amplifier working time since first started in minutes\n");
	 printf("   A side effect of this command sets this program's\n");
	 printf("   internal time variable for reading event history\n");
	 return arg;
      }
   }

   if (SendRecvCommand('N',NULL,0)) {

      WorkingTime[AmpIndx] = CharToInt(Payload,6);

      printf("Amplifier-%d:Working-Time:%d Minutes\n",AmpIndx+1,WorkingTime[AmpIndx]);
   }
   return arg;
}

// ====================================================

int GetProtectCount(int arg) {

ArgVal   *v;
AtomType  at;

int pcnt;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Gets the amplifier protection count\n");
	 return arg;
      }
   }

   if (SendRecvCommand('P',NULL,0)) {

      pcnt = CharToInt(Payload,6);

      printf("Amplifier-%d:Protection Count:%d\n",AmpIndx+1,pcnt);
   }
   return arg;
}

// ====================================================

int DeviceOnOff(int arg) {

ArgVal   *v;
AtomType  at;
int ack, onof;
char param[4];

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Turns on or off a device\n");
	 printf("You supply the on off value 1=on, 0=off\n");
	 return arg;
      }
   }

   if (at == Numeric) {
     arg++;
     onof = v->Number;
      if ((onof == 0) || (onof == 1)) {
	 param[0] = onof + 0x30;
	 param[1] = 0;
	 if (SendRecvCommand('p',param,1)) {

	    ack = (int) Payload[0];
	    printf("Response:");
	    if      (ack == ACK)  printf("ACK=OK");
	    else if (ack == NACK) printf("NACK=NotOK");
	    else if (ack == INV)  printf("INV=InvalidRequest");
	    else if (ack == OOR)  printf("OOR=OutOfRange");
	    else                  printf("Unexpected:%d This shouldn't happen",ack);
	    printf("\n");
	 }
	 return arg;
      }
   }
   printf("DeviceOnOff:No parameter supplied\n");
   return arg;
}

// ====================================================

int ResetModule(int arg) {

ArgVal   *v;
AtomType  at;
int ack, mod;
char param[4];

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Reset an amplifier module\n");
	 printf("You supply the module:\n");
	 printf("   1 = module 1 channels 1 and 2\n");
	 printf("   2 = module 2 channels 3 and 4\n");
	 printf("   3 = module 3 channels 5 and 6\n");

	 return arg;
      }
   }

   if (at == Numeric) {
     arg++;
     mod = v->Number;
      if ((mod > 0) && (mod < 4)) {
	 param[0] = mod + 0x30;
	 param[1] = 0;
	 if (YesNo("Reset","Amplifier") &&  SendRecvCommand('r',param,1)) {

	    ack = (int) Payload[0];
	    printf("Response:");
	    if      (ack == ACK)  printf("ACK=OK");
	    else if (ack == NACK) printf("NACK=NotOK");
	    else if (ack == INV)  printf("INV=InvalidRequest");
	    else if (ack == OOR)  printf("OOR=OutOfRange");
	    else                  printf("Unexpected:%d This shouldn't happen",ack);
	    printf("\n");
	 }
	 return arg;
      }
   }
   printf("ResetModule:No parameter supplied\n");
   return arg;
}

// ====================================================

int SetFakeTemp(int arg) {

ArgVal   *v;
AtomType  at;
int ack, ftm;
char param[4];

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Set fake temperature for tests\n");
	 printf("You supply the temp 00..255 deg Centigrade\n");
	 return arg;
      }
   }

   if (at == Numeric) {
     arg++;
     ftm = v->Number;
      if ((ftm >= 0) && (ftm <= 0xFF)) {

	 sprintf(param,"%02X",ftm); param[2] = 0;
	 if (SendRecvCommand('t',param,2)) {

	    ack = (int) Payload[0];
	    printf("Response:");
	    if      (ack == ACK)  printf("ACK=OK");
	    else if (ack == NACK) printf("NACK=NotOK");
	    else if (ack == INV)  printf("INV=InvalidRequest");
	    else if (ack == OOR)  printf("OOR=OutOfRange");
	    else                  printf("Unexpected:%d This shouldn't happen",ack);
	    printf("\n");
	 }
	 return arg;
      }
   }
   printf("SetFakeTemp:No parameter supplied\n");
   return arg;
}

// ====================================================

int SetOutAtten(int arg) {

ArgVal   *v;
AtomType  at;
int ack, chn, aten;
char param[4];

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Set the output attenuation for a channel\n");
	 printf("   You supply the channel 0=All 1..6\n");
	 printf("   You supply the attenuation 0..30 dB\n");
	 return arg;
      }
   }

   if (at == Numeric) {
      arg++;
      chn = v->Number;
      v = &(vals[arg]);
      at = v->Type;
      if ((chn >= 0) && (chn <= 6)) {
	 arg++;
	 if (at == Numeric) {
	    arg++;
	    aten = v->Number;
	    if ((aten >= 0) && (aten <= 30)) {

	       sprintf(param,"%1X%02X",chn,aten); param[3] = 0;
	       if (SendRecvCommand('v',param,3)) {

		  ack = (int) Payload[0];
		  printf("Response:");
		  if      (ack == ACK)  printf("ACK=OK");
		  else if (ack == NACK) printf("NACK=NotOK");
		  else if (ack == INV)  printf("INV=InvalidRequest");
		  else if (ack == OOR)  printf("OOR=OutOfRange");
		  else                  printf("Unexpected:%d This shouldn't happen",ack);
		  printf("\n");
	       }
	       return arg;
	    }
	 }
      }
   }
   printf("SetOutAtten:No/OutRange parameter supplied\n");
   return arg;
}

// ====================================================

int SetOutAttenVariable(int arg) {

ArgVal   *v;
AtomType  at;
int i, ack, aten;
char param[32+1];

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Set the output attenuation for variable number of channels\n");
	 printf("   You supply the attenuation 0..30 dB list\n");
	 return arg;
      }
   }

   i = 0;
   while ((at == Numeric) && (i<=30)) {
      arg++;
      aten = v->Number;

      v = &(vals[arg]);
      at = v->Type;

      if ((aten >= 0) && (aten <= 29)) {
	 sprintf(&(param[i]),"%02X",aten);
	 i += 2; param[i] = 0;
      } else {
	 printf("Attenuation value:%d = %d (Out of range:[0..30])\n",(i/2)+1,aten);
	 return arg;
      }
   }

   if (i == 0) {
      printf("No attenuation values supplied\n");
      return arg;
   }

   if (SendRecvCommand('w',param,i)) {

      ack = (int) Payload[0];
      printf("Response:");
      if      (ack == ACK)  printf("ACK=OK");
      else if (ack == NACK) printf("NACK=NotOK");
      else if (ack == INV)  printf("INV=InvalidRequest");
      else if (ack == OOR)  printf("OOR=OutOfRange");
      else                  printf("Unexpected:%d This shouldn't happen",ack);
      printf("\n");
   }
   return arg;
}

// ====================================================

int GetWave(int arg) {

ArgVal   *v;
AtomType  at;
int freq, insr, otsr, comp, ovrf, i, j;
char *cp;
char param[32+1];

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;
	 printf("Get wave sample test tone\n");
	 printf("   You supply the frequency:[20..20000Hz]\n");
	 printf("   You supply the input routing for sinwave...\n");
	 printf("      Bit:0=1 => Ch1 negative\n");
	 printf("      Bit:1=2 => Ch1 posative\n");
	 printf("      Bit:2=4 => Ch2 negative\n");
	 printf("      Bit:3=8 => Ch2 posative\n");
	 printf("   You supply the output routing for sinwave...\n");
	 printf("      Val:0 => Ch1 Voltage\n");
	 printf("      Val:1 => Ch2 Voltage\n");
	 printf("      Val:2 => Ch1 Current\n");
	 printf("      Val:3 => Ch2 Current\n");
	 printf("   You supply the computation ...\n");
	 printf("      Val:0 => Coarse\n");
	 printf("      Val:1 => Fine\n");
	 return arg;
      }
   }

   freq = 0;
   if (at == Numeric) {
      arg++;
      freq = v->Number;
      v = &(vals[arg]);
      at = v->Type;
   }
   if ((freq < 20) || (freq > 20000)) {
      printf("Frequency out of range:%d[20..20000]\n",freq);
      return arg;
   }

   insr = 0xFF;
   if (at == Numeric) {
      arg++;
      insr = v->Number;
      v = &(vals[arg]);
      at = v->Type;
   }
   if ((insr & 0xF) != insr) {
      printf("Input sinwave routing out of range:%d[0..15]\n",insr);
      return arg;
   }

   otsr = 0xFF;
   if (at == Numeric) {
      arg++;
      otsr = v->Number;
      v = &(vals[arg]);
      at = v->Type;
   }
   if (otsr > 3) {
      printf("Output sinwave routing out of range:%d[0..3]\n",otsr);
      return arg;
   }
   comp = 2;
   if (at == Numeric) {
      arg++;
      comp = v->Number;
      v = &(vals[arg]);
      at = v->Type;
   }
   if (comp > 1) {
      printf("Computation out of range:%d[0..1]\n",comp);
      return arg;
   }

   bzero((void *) param, 33);
   sprintf(param,"%0X4%0X2%0X2%0X2",freq,insr,otsr,comp);
   if (SendRecvCommand('D',param,10)) {

      ovrf = CharToInt(Payload,2);
      if (ovrf) {
	 printf("Computation failed, Overflow:%d\n",ovrf);
	 return arg;
      }

      cp = &Payload[2];
      for (i=0; i<800; i++) {

	 j = i << 1;
	 if (j>=strlen(cp)) break;

	 WaveSamp[i] = CharToInt(&cp[j],2);
	 WaveSampLen = i;
      }

      printf("Wave Sample taken, Samples:%d OK\n",WaveSampLen);
   }
   return arg;
}

// ====================================================

int SetInputRoute(int arg) {

ArgVal   *v;
AtomType  at;
int irot, ack;
char param[4];

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Set input routing\n");
	 printf("   You supply the route:[0..5]\n");
	 printf("      00: Analogue source => output\n");
	 printf("      01: Analogue source => DSP => output\n");
	 printf("      02: AES3 source => output\n");
	 printf("      03: AES3 source => DSP => output\n");
	 printf("      04: KAESOP source => output (Not implemented)\n");
	 printf("      05: KAESOP source => DSP => output\n");

	 return arg;
      }
   }

   if (at == Numeric) {
      arg++;
      irot = v->Number;
      if ((irot !=4) && (irot <=5)) {

	 sprintf(param,"%02X",irot); param[2] = 0;
	 if (SendRecvCommand('Z',param,2)) {

	    ack = (int) Payload[0];
	    printf("Response:");
	    if      (ack == ACK)  printf("ACK=OK");
	    else if (ack == NACK) printf("NACK=NotOK");
	    else if (ack == INV)  printf("INV=InvalidRequest");
	    else if (ack == OOR)  printf("OOR=OutOfRange");
	    else                  printf("Unexpected:%d This shouldn't happen",ack);
	    printf("\n");
	 }
	 return arg;
      }
   }
   printf("SetInputRoute:No route supplied\n");
   return arg;
}

// ====================================================

int SetBrownOut(int arg) {

ArgVal   *v;
AtomType  at;
int brno, ack;
char param[4];

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Enable brown out detection\n");
	 printf("   You supply the detection...\n");
	 printf("      00: Disabled\n");
	 printf("      01: Enabled\n");

	 return arg;
      }
   }

   if (at == Numeric) {
      arg++;
      brno = v->Number;
      if (brno <= 1) {

	 sprintf(param,"%1X",brno); param[1] = 0;
	 if (SendRecvCommand('b',param,1)) {

	    ack = (int) Payload[0];
	    printf("Response:");
	    if      (ack == ACK)  printf("ACK=OK");
	    else if (ack == NACK) printf("NACK=NotOK");
	    else if (ack == INV)  printf("INV=InvalidRequest");
	    else if (ack == OOR)  printf("OOR=OutOfRange");
	    else                  printf("Unexpected:%d This shouldn't happen",ack);
	    printf("\n");
	 }
	 return arg;
      }
   }
   printf("SetBrownOut:No enable/disable supplied\n");
   return arg;
}

// ====================================================

int SetHiMains(int arg) {

ArgVal   *v;
AtomType  at;
int vlts, ack;
char param[6];

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Set High value for mains calibration\n");
	 printf("   You supply the voltage[0..300]\n");
	 return arg;
      }
   }

   if (at == Numeric) {
      arg++;
      vlts = v->Number;
      if (vlts <= 300) {

	 sprintf(param,"%04X",vlts); param[4] = 0;
	 if (SendRecvCommand('h',param,4)) {

	    ack = (int) Payload[0];
	    printf("Response:");
	    if      (ack == ACK)  printf("ACK=OK");
	    else if (ack == NACK) printf("NACK=NotOK");
	    else if (ack == INV)  printf("INV=InvalidRequest");
	    else if (ack == OOR)  printf("OOR=OutOfRange");
	    else                  printf("Unexpected:%d This shouldn't happen",ack);
	    printf("\n");
	 }
	 return arg;
      }
   }
   printf("SetHiMains:No voltage supplied\n");
   return arg;
}

// ====================================================

int SetLoMains(int arg) {

ArgVal   *v;
AtomType  at;
int vlts, ack;
char param[6];

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Set Low value for mains calibration\n");
	 printf("   You supply the voltage[0..300]\n");
	 return arg;
      }
   }

   if (at == Numeric) {
      arg++;
      vlts = v->Number;
      if (vlts <= 300) {

	 sprintf(param,"%04X",vlts); param[4] = 0;
	 if (SendRecvCommand('o',param,4)) {

	    ack = (int) Payload[0];
	    printf("Response:");
	    if      (ack == ACK)  printf("ACK=OK");
	    else if (ack == NACK) printf("NACK=NotOK");
	    else if (ack == INV)  printf("INV=InvalidRequest");
	    else if (ack == OOR)  printf("OOR=OutOfRange");
	    else                  printf("Unexpected:%d This shouldn't happen",ack);
	    printf("\n");
	 }
	 return arg;
      }
   }
   printf("SetLoMains:No voltage supplied\n");
   return arg;
}

// ====================================================

char *GetEnumName(int eval, int val) {

EnumBinding *eb;
EnumNames   *en;

int i, j;

   for (i=0; i<ENUM_NAMES; i++) {
      en = &ETab[i];
      if (en->EnumValue == eval) {
	 for (j=0; j<ENUM_BINDINGS; j++) {
	    eb = &(en->Bindings[j]);
	    if (eb->Value == val) return eb->Name;
	 }
      }
   }
   return "??";
}

// ====================================================

void DisplayEnumNames(int eval) {

EnumBinding *eb;
EnumNames   *en;

int i, j;

   for (i=0; i<ENUM_NAMES; i++) {
      en = &ETab[i];
      if (en->EnumValue == eval) {
	 for (j=0; j<ENUM_BINDINGS; j++) {
	    eb = &(en->Bindings[j]);
	    if (strlen(eb->Name)) printf("%d=%s\n",eb->Value,eb->Name);
	 }
      }
   }
}

// ====================================================

void DisplayChannelParameters(int chan) {

ChannelParameters *dsp;
Limiter *lim;
Crossover *cro;
Parametric *pmr;

unsigned short *sp;
unsigned char c;
float f;
int i;

   dsp = (ChannelParameters *) &(ChanPars[chan]);

   printf("Board:%c%c\n",dsp->Board[0],dsp->Board[1]);
   printf("SubCommand:%c\n",dsp->SubCommand);

   sp = (unsigned short *) dsp->Tag;
   printf("Tag:0x%04X\n",(int) *sp);
   sp = (unsigned short *) dsp->Version;
   printf("Version:0x%04X (%d)\n",(int) *sp, (int) *sp);

   c = dsp->PhaseReversed;;
   printf("PhaseReversed:%d=",(int) c);
   if (c) printf("Reversed");
   else   printf("NotReversed");
   printf("\n");

   c = dsp->PeakLimitEnabled;
   printf("PeakLimitEnabled:%d=",(int) c);
   if (c) printf("Enabled");
   else   printf("Disabled");
   printf("\n");

   c = dsp->PowerLimitEnabled;
   printf("PowerLimitEnabled:%d=",(int) c);
   if (c) printf("Enabled");
   else   printf("Disabled");
   printf("\n");

   sp = (unsigned short *) dsp->GEqBypass;
   printf("GEqBypass:%d=",(int) *sp);
   if (*sp == 0) printf("Graphic Equalizer bypassed");
   else          printf("Graphic Equalizer in use");
   printf("\n");

   sp = (unsigned short *) dsp->PresetProtection;
   printf("PresetProtection:%d=",(int) *sp);
   if (*sp == 10) printf("No protection");
   else           printf("Undefined value");
   printf("\n");

   sp = (unsigned short *) dsp->Delay;
   f = (1000000.0/96000.0) * ((float) (*sp));
   printf("Delay:%d Units 1/96000 Seconds = %f micro Seconds\n",(int) *sp, f);

   lim = (Limiter *) &(dsp->Peak);
   sp = (unsigned short *) lim->Threshold;
   printf("PeakLimiter:Threshold:%d Volts. ",(int) *sp);
   sp = (unsigned short *) lim->Attack;
   printf("Attack:%d milli Seconds. ",(int) *sp);
   sp = (unsigned short *) lim->Decay;
   printf("Decay:%d milli Seconds.\n",(int) *sp);

   sp = (unsigned short *) dsp->AuxDelay;
   printf("AuxDelay:%d milli Seconds\n",(int) *sp);

   lim = (Limiter *) &(dsp->Power);
   sp = (unsigned short *) lim->Threshold;
   printf("PowerLimiter:Threshold:%d Watts. ",(int) *sp);
   sp = (unsigned short *) lim->Attack;
   printf("Attack:%d milli Seconds. ",(int) *sp);
   sp = (unsigned short *) lim->Decay;
   printf("Decay:%d milli Seconds.\n",(int) *sp);

   sp = (unsigned short *) dsp->Gain;
   printf("Gain:%d dB/10 reduction\n",(int) *sp);

   sp = (unsigned short *) dsp->DampingFactor;
   printf("DampingFactor:%d Ohms\n",(int) *sp);

   sp = (unsigned short *) dsp->DampingMode;
   printf("DampingMode:%d=",(int) *sp);
   if      (*sp == 0) printf("Off");
   else if (*sp == 1) printf("On (Manual)");
   else if (*sp == 2) printf("Auto");
   else               printf("Illegal value");
   printf("\n");

   sp = (unsigned short *) dsp->FiltersDefinition;
   printf("FiltersDefinition:%d=",(int) *sp);
   if      (*sp == 1) printf("Powersoft");
   else if (*sp == 2) printf("XTA");
   else if (*sp == 3) printf("BSS Symetric");
   else if (*sp == 4) printf("BSS Asymetric");
   else if (*sp == 5) printf("DBX Constant");
   else if (*sp == 6) printf("DBX Adaptive");
   else if (*sp == 7) printf("Lake");
   else if (*sp == 8) printf("Crown");
   else               printf("Illegal value");
   printf("\n");

   for (i=0; i<2; i++) {
      cro = (Crossover *) &(dsp->CrossoverFilters[i]);

      c = cro->Enabled;
      printf("CrossopverFilters[%02d]:%d=",i, (int) c);
      if (c == 0) printf("Inactive. ");
      else        printf("Active. ");

      sp = (unsigned short *) cro->Align;
      printf("Alignment:%d=",(int) *sp);
      if      (*sp == 0) printf("Butterworth. ");
      else if (*sp == 1) printf("Bessel. ");
      else if (*sp == 2) printf("Linkwitz-Riley. ");
      else if (*sp == 3) printf("FIR. ");
      else if (*sp == 4) printf("FIR+ ");
      else               printf("Illegav value. ");

      sp = (unsigned short *) cro->Slope;
      printf("Slope:%02d dB/Octave. ",(int) *sp);

      sp = (unsigned short *) cro->Frequency;
      printf("Frequency:%d Hertz (Cutoff)",(int) *sp);

      printf("\n");
   }

   for (i=0; i<16; i++) {
      pmr = (Parametric *) &(dsp->ParametricFilters[i]);

      c = pmr->Enabled;
      printf("ParametricFilters[%02d]:%d=",i, (int) c);
      if (c == 0) printf("Inactive. ");
      else        printf("Active. ");

      sp = (unsigned short *) pmr->Type;
      printf("Type:0x%02X=",(int) *sp);
      if      (*sp == 0x00) printf("Peaking. ");
      else if (*sp == 0x0B) printf("Low shelving. ");
      else if (*sp == 0x0C) printf("High shelving. ");
      else if (*sp == 0x0D) printf("Low-pass EQ. ");
      else if (*sp == 0x0E) printf("High-pass EQ. ");
      else if (*sp == 0x0F) printf("Band-pass. ");
      else if (*sp == 0x10) printf("Band-stop. ");
      else if (*sp == 0x11) printf("All-pass. ");
      else                  printf("Illegav value. ");

      sp = (unsigned short *) pmr->Frequency;
      printf("Frequency:%d Hertz. ",(int) *sp);

      sp = (unsigned short *) pmr->Q;
      printf("Q:%d Q/10. ",(int) *sp);

      sp = (unsigned short *) pmr->Slope;
      printf("Slope:%d Slope/10. ",(int) *sp);

      printf("\n");
   }
}

// ====================================================

void DisplayCommonParameters() {

CommonParameters *dsp;

unsigned short *sp;
unsigned char c;

   printf("\nDSP Common parameters\n");

   dsp = (CommonParameters *) &ComPars;

   printf("Board:%c%c\n",dsp->Board[0],dsp->Board[1]);
   printf("SubCommand:%c\n",dsp->SubCommand);

   sp = (unsigned short *) dsp->Tag;
   printf("Tag:0x%04X\n",(int) *sp);

   sp = (unsigned short *) dsp->Version;
   printf("Version:0x%04X (%d)\n",(int) *sp, (int) *sp);

   c = dsp->DSPBypass;
   printf("DSPBypass:%d=",(int) c);
   if (c) printf("Bypassed");
   else   printf("NoBypass");
   printf("\n");

   sp = (unsigned short *) dsp->Input;
   printf("Input:%d=",(int) *sp);
   if      (*sp == 0) printf("ADC");
   else if (*sp == 1) printf("AESEBU");
   else if (*sp == 2) printf("AUX");
   else if (*sp == 3) printf("KAESOP");
   else if (*sp == 4) printf("MUTE");
   else               printf("Illegal value");
   printf("\n");

   sp = (unsigned short *) dsp->MainDelay;
   printf("MainDelay:%d milliseconds\n",(int) *sp);

   sp = (unsigned short *) dsp->SoundSpeed;
   printf("SoundSpeed:%d Meters per Second\n",(int) *sp);

   sp = (unsigned short *) dsp->AES3Gain;
   printf("AES3Gain:%d dB/10\n",(int) *sp);

   sp = (unsigned short *) dsp->Source;
   printf("Source:%d=",(int) *sp);
   if      (*sp == 0) printf("Both channels");
   else if (*sp == 1) printf("Channel-1");
   else if (*sp == 2) printf("Channel-2");
   else               printf("Illegal value");
   printf("\n");

   sp = (unsigned short *) dsp->NoLinkBehaviour;
   printf("NoLinkBehaviour:%d=",(int) *sp);
   if      (*sp == 0) printf("Mute");
   else if (*sp == 1) printf("Auto");
   else if (*sp == 2) printf("Analog");
   else               printf("Illegal value");
   printf("\n");
}

// ====================================================

int GetDspParams(int arg) {

ArgVal   *v;
AtomType  at;
int blk;
char param[6];

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Get DSP parameters\n");
	 printf("   You supply which parameters to get...\n");
	 printf("      00: DSP Chn1 parameters block\n");
	 printf("      01: DSP Chn2 parameters block\n");
	 printf("      02: DSP Common parameters block\n");
	 return arg;
      }
   }

   if (at == Numeric) {
      arg++;
      blk = v->Number;
      if (blk <= 2) {
	 Blk = blk;

	 sprintf(param,"03C%02X",blk); param[5] = 0;

	 slow = 1000000;
	 if (SendRecvCommand('x',param,5)) {
	    if (blk != 2) {
	       bcopy(Payload,&(ChanPars[blk]),sizeof(ChannelParameters));
	       ChanParsValid[blk] = 1;
	    } else {
	       bcopy(Payload,&ComPars,sizeof(CommonParameters));
	       ComParsValid = 1;
	    }
	    printf("OK\n");
	 }
	 slow = 0;
	 return arg;
      }
   }
   printf("GetDspParams:No DSP parameter block supplied\n");
   return arg;
}

// ====================================================

char *GetDspParamStr(int pidx, int chan, int indx) {

static char parst[128];

KDspPar *kpar;

   printf("%02d:",pidx);

   if ((pidx < 0) || (pidx >= KPARS)) {
      printf("Invalid\n");
      return NULL;
   }

   kpar = &(Kpars[pidx]);
   bzero((void *) parst, 128);

   if ((kpar->Index != -1) && (kpar->Channel != -1)) {

      sprintf(parst,
	      "Name:%s:Type:0x%02X:Index:%d:Channel:%d:Default:%d:Max:%d:Min:%d:Units:%s",
	      kpar->Name,
	      kpar->Type,
	      indx,
	      chan,
	      kpar->Default,
	      kpar->Max,
	      kpar->Min,
	      kpar->Units);

   } else if (kpar->Index != -1) {

      sprintf(parst,
	      "Name:%s:Type:0x%02X:Index:%d:Default:%d:Max:%d:Min:%d:Units:%s",
	      kpar->Name,
	      kpar->Type,
	      indx,
	      kpar->Default,
	      kpar->Max,
	      kpar->Min,
	      kpar->Units);

   } else if (kpar->Channel != -1) {

      sprintf(parst,
	      "Name:%s:Type:0x%02X:Channel:%d:Default:%d:Max:%d:Min:%d:Units:%s",
	      kpar->Name,
	      kpar->Type,
	      chan,
	      kpar->Default,
	      kpar->Max,
	      kpar->Min,
	      kpar->Units);

   } else {

      sprintf(parst,
	      "Name:%s:Type:0x%02X:Default:%d:Max:%d:Min:%d:Units:%s",
	      kpar->Name,
	      kpar->Type,
	      kpar->Default,
	      kpar->Max,
	      kpar->Min,
	      kpar->Units);

   }

   return parst;
}

// ====================================================

char *GetDspParamAddress(int pidx, int chan, int indx) {

static char astr[8], *cp;

KDspPar *kpar;

   if ((pidx < 0) || (pidx >= KPARS)) return NULL;

   kpar = &(Kpars[pidx]);
   bzero((void *) astr, 8);

   cp = &(astr[strlen(astr)]);
   sprintf(cp,"%02X",(kpar->Type & 0xFF));

   if (kpar->Index != -1) {
      if ((indx < 0) || (indx > kpar->Index)) return NULL;

      cp = &(astr[strlen(astr)]);
      sprintf(cp,"%02X",(indx & 0xFF));
   }

   if (kpar->Channel != -1) {
      if ((chan < 0) || (chan > kpar->Channel)) return NULL;

      cp = &(astr[strlen(astr)]);
      sprintf(cp,"%02X",(chan & 0xFF));
   }

   return astr;
}

// ====================================================

char *GetNextDspParamAddress(int *pidx, int *chan, int *indx) {

KDspPar *kpar;

   if ((*pidx < 0) || (*pidx >= KPARS)) {
      *pidx = 0;
      *indx = 0;
      *chan = 0;
      return GetDspParamAddress(*pidx,*chan,*indx);
   }

   kpar = &(Kpars[*pidx]);

   if (kpar->Index != -1) {
      if (*indx < kpar->Index) {
	 *indx = *indx +1;
	 return GetDspParamAddress(*pidx,*chan,*indx);
      }
   }
   *indx = 0;

   if (kpar->Channel != -1) {
      if (*chan < kpar->Channel) {
	 *chan = *chan +1;
	 return GetDspParamAddress(*pidx,*chan,*indx);
      }
   }
   *chan = 0;

   *pidx = *pidx + 1;
   if (*pidx >= KPARS) *pidx = 0;
   return GetDspParamAddress(*pidx,*chan,*indx);
}

// ====================================================

int GetPayloadDataOffset(char *cmd) {

char *cp;
int i;

   for (i=0; i<PAYLOAD_SIZE - strlen(cmd); i++) {
      cp = &(Payload[i]);
      if (strncmp(cp,cmd,strlen(cmd)) == 0) return i+strlen(cmd);
   }
   return strlen(cmd);
}

// ====================================================

void DisplayDspAddressMap() {
int i;

   printf("DSP AddressMap:\n");
   for (i=0; i<KPARS; i++) {
      printf("%28s:%02d-00-00\t",Kpars[i].Name,i);
      if (i & 1) printf("\n");
   }
   printf("\n");
}

// ====================================================

int EditDspParams(int arg) {

ArgVal   *v;
AtomType  at;

int chan = 0;   // Channel
int indx = 0;   // Index
int pidx = 0;   // Parameter number

char *astr = NULL; // amp address string
char *pstr = NULL; // DSP parameter string

int n, data, ack;
int radix = 10;
char c, *cp, str[128];

char param[128];
KDspPar *kpar;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Edit live DSP parameters on amplifier\n");
	 printf("\n");

	 printf("This DSP editor uses the G and H sub commands\n");
	 printf("Here CR means carrige return char\n");
	 printf("Here <addr> means: <par>-<chan>-<indx>\n");
	 printf("\n");

	 printf("   .CR           Exit editor\n");
	 printf("   <par val>CR   Change parameter\n");
	 printf("   CR            Next parameter code\n");
	 printf("   /<addr>CR     Goto parameter at <addr>\n");
	 printf("   x             Hexadecimal\n");
	 printf("   y             Decimal\n");
	 printf("   ?             Help\n");
	 printf("   *             Set to default\n");
	 printf("\n");

	 DisplayDspAddressMap();
	 return arg;
      }
   }

   if (at == Numeric) {
      arg++;
      pidx = v->Number;
   }

loop: while (1) {

      slow = 100000;

      if ((pidx < 0) || (pidx >= KPARS)) {
	 pidx = 0;
	 indx = 0;
	 chan = 0;
      }
      kpar = &(Kpars[pidx]);

      if (kpar->Index   == -1) indx = 0;
      if (kpar->Channel == -1) chan = 0;

      if (radix == 10) printf("%02d-%02d-%02d:%s:",pidx,chan,indx,kpar->Name);
      else             printf("0x%02X-0x%02X-0x%02X:%s:",pidx,chan,indx,kpar->Name);

      astr = GetDspParamAddress(pidx,chan,indx);
      sprintf(param,"03G%s",astr);
      if (SendRecvCommand('x',param,strlen(param)) == NULL) {
	 pstr = GetDspParamStr(pidx,chan,indx);
	 printf("Error:%s\n",pstr);
	 break;
      }

      data = strtol(&(Payload[GetPayloadDataOffset("03G")]),&cp,16);
      if (radix == 10) printf("%d %s "    ,data, kpar->Units);
      else             printf("0x%08X %s ",data, kpar->Units);
      if (strcmp(kpar->Units,"Enum") == 0) printf("%s ",GetEnumName(kpar->Type,data));

      c = (char) getchar();

      if      (c == '\n') { astr = GetNextDspParamAddress(&pidx,&chan,&indx); }
      else if (c == '.' ) { slow = 0; c = getchar(); printf("\n"); break; }
      else if (c == 'x' ) { radix = 16; c = 0; getchar(); continue; }
      else if (c == 'y' ) { radix = 10; c = 0; getchar(); continue; }
      else if (c == '?' ) {
	 printf("%s\n",GetDspParamStr(pidx,chan,indx));
	 if (strcmp(kpar->Units,"Enum") == 0) {
	    DisplayEnumNames(kpar->Type);
	 }
	 c = 0; getchar();
	 goto loop;
      } else if (c == '/')  {

	 pidx = 0;
	 chan = 0;
	 indx = 0;

	 if (c == '\n') { continue; }

	 bzero((void *) str, 128);
	 n = 0; while (n < 128) {
	    c = getchar();
	    if (c == '\n') goto loop;
	    if (c == '-' ) break;
	    str[n++] = c;
	    pidx = strtoul(str,&cp,radix);
	 }

	 bzero((void *) str, 128);
	 n = 0; while (n < 128) {
	    c = getchar();
	    if (c == '\n') goto loop;
	    if (c == '-' ) break;
	    str[n++] = c;
	    chan = strtoul(str,&cp,radix);
	 }

	 bzero((void *) str, 128);
	 n = 0; while (n < 128) {
	    c = getchar();
	    if (c == '\n') goto loop;
	    if (c == '-' ) break;
	    str[n++] = c;
	    indx = strtoul(str,&cp,radix);
	 }

      } else {

	 bzero((void *) str, 128); n = 0;
	 str[n++] = c;

	 while ((c != '\n') && (n < 128) && (c != '*')) c = str[n++] = (char) getchar();

	 if (c == '*') {getchar(); printf("\n"); data = kpar->Default;}
	 else          data = strtoul(str,&cp,radix);

	 sprintf(param,"03H%s%04X",astr,data);

	 if (SendRecvCommand('x',param,strlen(param))) {
	    ack = (int) Payload[GetPayloadDataOffset("03H")];
	    printf("Response:");
	    if      (ack == ACK)  printf("ACK=OK");
	    else if (ack == NACK) printf("NACK=NotOK");
	    else if (ack == INV)  printf("INV=InvalidRequest");
	    else if (ack == OOR)  printf("OOR=OutOfRange");
	    else                  printf("Unexpected:%d This shouldn't happen",ack);
	    printf("\n");
	 }
      }
   }
   return arg;
}

// ====================================================

int SetDspParams(int arg) {

ArgVal   *v;
AtomType  at;
int i, lp, blk, len, ack, off;
char *cp;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Set DSP parameters\n");
	 printf("   You supply which parameters to get...\n");
	 printf("      00: DSP Chn1 parameters block\n");
	 printf("      01: DSP Chn2 parameters block\n");
	 printf("      02: DSP Common parameters block\n");
	 return arg;
      }
   }

   blk = -1;
   if (at == Numeric) {
      arg++;
      blk = v->Number;
      if (blk > 2) {
	 fprintf(stderr,"Error:Illegal DSP binary block number:%d Not in range:[0..2]\n",blk);
	 return arg;
      }
   }

   if (blk < 0) {
      blk = Blk;
      printf("Using block:%d\n",blk);
   } else Blk = blk;

   if ( ((blk == 2) && (ComParsValid != 1))
   ||   (ChanParsValid[blk] != 1) ) {
      fprintf(stderr,"Error:Parameters binary block:%d must first be read from amp/file\n",blk);
      return arg;
   }

   if (blk == 2) {
      cp = (char *) &(ComPars.Tag);
      len = sizeof(CommonParameters) -3;
      ComPars.SubCommand       = 'j';
   } else {
      cp = (char *) &(ChanPars[blk].Tag);
      len = sizeof(ChannelParameters) -3;
      ChanPars[blk].SubCommand = 'j';
   }

   bzero((void *) Payload, PAYLOAD_SIZE);
   sprintf(Payload,"03j%02X",blk);
   lp = strlen(Payload);

   for (i=0; i<len; i++) Payload[lp+i] = cp[i];

   slow = 1000000;

   if (SendRecvCommand('x',Payload,lp+len)) {
      off = GetPayloadDataOffset("03j");
      ack = (int) Payload[off];
      printf("Response:");
      if      (ack == ACK)  printf("ACK=OK");
      else if (ack == NACK) printf("NACK=NotOK");
      else if (ack == INV)  printf("INV=InvalidRequest");
      else if (ack == OOR)  printf("OOR=OutOfRange");
      else                  printf("Unexpected:%d This shouldn't happen",ack);
      printf("\n");
   }
   slow = 0;
   return arg;
}

// ====================================================

int WriteParamsFile(int arg) {

ArgVal   *v;
AtomType  at;

FILE *fp;
int blk, len, cc;
char *cp;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Write DSP parameter block to a file\n");
	 printf("   You supply which parameters block to be written\n");
	 printf("      00: DSP Chn1 parameters block\n");
	 printf("      01: DSP Chn2 parameters block\n");
	 printf("      02: DSP Common parameters block\n");
	 printf("   You optionally supply the target file\n");
	 printf("      No name: Implies use the default in amptest.config\n");
	 printf("      Path:    Write to the file specified in path\n");
	 printf("\n");
	 printf("N.B.: the DSP parameters must have been read into memory\n");
	 printf("      using the binary DSP read command: See help\n");
	 return arg;
      }
   }

   blk = -1;
   if (at == Numeric) {
      arg++;
      blk = v->Number;
      if (blk > 2) {
	 fprintf(stderr,"Error:Illegal DSP binary block number:%d Not in range:[0..2]\n",blk);
	 return arg;
      }
   }
   if (blk < 0) {
      blk = Blk;
      printf("Using block:%d\n",blk);
   } else Blk = blk;

   if ( ((blk == 2) && (ComParsValid != 1))
   ||   (ChanParsValid[blk] != 1) ) {
      fprintf(stderr,"Error:Parameters binary block:%d must first be read from amp\n",blk);
      return arg;
   }

   cp = GetFileName(&arg);
   if (cp == NULL) cp = GetFile("AmpDspBin");
   if ((cp) && (strcmp(cp,"RefAmpDspBin") == 0)) cp = GetFile("RefAmpDspBin");
   sprintf(path,"%s%d",path,blk);

   umask(0);
   fp = fopen(path,"w");
   if (fp == NULL) {
      fprintf(stderr,"WriteParamsFile:Can't open file: %s for write\n",path);
      perror("fopen");
      return arg;
   }

   if (blk == 2) { cp = (char *) &(ComPars);       len = sizeof(CommonParameters);  }
   else          { cp = (char *) &(ChanPars[blk]); len = sizeof(ChannelParameters); }

   cc = fwrite(cp,len,1,fp);
   if (cc != 1) {
      fprintf(stderr,"Error:Write failed cc:%d should be:%d\n",cc,1);
      perror("fwrite");
      fclose(fp);
      return arg;
   }

   printf("WriteParamsFile: Written:%d bytes to:%s OK\n",len,path);
   fclose(fp);
   return arg;
}

// ====================================================

int ReadParamsFile(int arg) {

ArgVal   *v;
AtomType  at;

FILE *fp;
int blk, len, cc;
char *cp;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Read DSP parameter block from a file\n");
	 printf("   You supply which parameters block to be read\n");
	 printf("      00: DSP Chn1 parameters block\n");
	 printf("      01: DSP Chn2 parameters block\n");
	 printf("      02: DSP Common parameters block\n");
	 printf("   You optionally supply the source file\n");
	 printf("      No name: Implies use the default in amptest.config\n");
	 printf("      Path:    Read from the file specified in path\n");
	 return arg;
      }
   }

   blk = -1;
   if (at == Numeric) {
      arg++;
      blk = v->Number;
      if (blk > 2) {
	 fprintf(stderr,"Error:Illegal DSP binary block number:%d Not in range:[0..2]\n",blk);
	 return arg;
      }
   }
   if (blk < 0) {
      blk = Blk;
      printf("Using block:%d\n",blk);
   } else Blk = blk;

   cp = GetFileName(&arg);
   if ((cp) && (strcmp(cp,"RefAmpDspBin") == 0)) cp = GetFile("RefAmpDspBin");
   if (cp == NULL) cp = GetFile("AmpDspBin");
   sprintf(path,"%s%d",path,blk);

   umask(0);
   fp = fopen(path,"r");
   if (fp == NULL) {
      fprintf(stderr,"ReadParamsFile:Can't open file: %s for read\n",path);
      perror("fopen");
      return arg;
   }

   if (blk == 2) { cp = (char *) &(ComPars);       len = sizeof(CommonParameters);  }
   else          { cp = (char *) &(ChanPars[blk]); len = sizeof(ChannelParameters); }

   cc = fread(cp,len,1,fp);
   if (cc != 1) {
      fprintf(stderr,"Error:Read failed cc:%d should be:%d\n",cc,1);
      perror("fread");
      fclose(fp);
      return arg;
   }
   if   (blk == 2) ComParsValid       = 1;
   else            ChanParsValid[blk] = 1;

   printf("ReadParamsFile: Read:%d bytes from:%s OK\n",len,path);
   fclose(fp);
   return arg;
}

// ====================================================

int DisplayDspParams(int arg) {

ArgVal   *v;
AtomType  at;

int blk;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Display DSP parameter block\n");
	 printf("   You supply which parameters block to be read\n");
	 printf("      00: DSP Chn1 parameters block\n");
	 printf("      01: DSP Chn2 parameters block\n");
	 printf("      02: DSP Common parameters block\n");
	 printf("\n");
	 printf("N.B.: The DSP parameters must be read into memory\n");
	 printf("      using the binary DSP read command: See help\n");
	 return arg;
      }
   }

   blk = -1;
   if (at == Numeric) {
      arg++;
      blk = v->Number;
      if (blk > 2) {
	 fprintf(stderr,"Error:Illegal DSP binary block number:%d Not in range:[0..2]\n",blk);
	 return arg;
      }
   }
   if (blk < 0) {
      blk = Blk;
      printf("Using block:%d\n",blk);
   } else Blk = blk;

   if ( ((blk == 2) && (ComParsValid != 1))
   ||   (ChanParsValid[blk] != 1) ) {
      fprintf(stderr,"Error:Parameters binary block:%d must first be read from amp\n",blk);
      return arg;
   }

   if (blk == 2) DisplayCommonParameters();
   else          DisplayChannelParameters(blk);

   return arg;
}

// ====================================================

void Mismatch(char *what) {

   printf("Mismatch:Amp:%c%c:%s\n",Id[0],Id[1],what);
}

// ====================================================

int CompareParamsFile(int arg) {

ArgVal   *v;
AtomType  at;

FILE *fp;
int i, blk, len, cc, bad;
char *cp;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Compare DSP parameter block to a file\n");
	 printf("   You supply which parameters block to be compared\n");
	 printf("      00: DSP Chn1 parameters block\n");
	 printf("      01: DSP Chn2 parameters block\n");
	 printf("      02: DSP Common parameters block\n");
	 printf("   You optionally supply the source file\n");
	 printf("      No name: Implies use the default in amptest.config\n");
	 printf("      Path:    Compare with the file specified in path\n");
	 printf("\n");
	 printf("N.B.: the DSP parameters must have been read into memory\n");
	 printf("      using the binary DSP read command: See help\n");
	 return arg;
      }
   }

   blk = -1;
   if (at == Numeric) {
      arg++;
      blk = v->Number;
      if (blk > 2) {
	 fprintf(stderr,"Error:Illegal DSP binary block number:%d Not in range:[0..2]\n",blk);
	 return arg;
      }
   }
   if (blk < 0) {
      blk = Blk;
      printf("Using block:%d\n",blk);
   } else Blk = blk;

   if ( ((blk == 2) && (ComParsValid != 1))
   ||   (ChanParsValid[blk] != 1) ) {
      fprintf(stderr,"Error:Parameters binary block:%d must first be read from amp\n",blk);
      return arg;
   }

   cp = GetFileName(&arg);
   if ((cp) && (strcmp(cp,"AmpDspBin") == 0)) cp = GetFile("AmpDspBin");
   if (cp == NULL) cp = GetFile("RefAmpDspBin");
   sprintf(path,"%s%d",path,blk);

   umask(0);
   fp = fopen(path,"r");
   if (fp == NULL) {
      fprintf(stderr,"CompareParamsFile:Can't open file: %s for read\n",path);
      perror("fopen");
      return arg;
   }

   if (blk == 2) { cp = (char *) &(ComPars);       len = sizeof(CommonParameters);  }
   else          { cp = (char *) &(ChanPars[blk]); len = sizeof(ChannelParameters); }

   cc = fread(Payload,len,1,fp);
   if (cc != 1) {
      fprintf(stderr,"Error:Read failed cc:%d should be:%d\n",cc,1);
      perror("fread");
      fclose(fp);
      return arg;
   }

   bad = 0;
   for (i=3; i<len; i++) {
      if (Payload[i] != cp[i]) {
	 printf("Mismatch:Byte:%d File:0x%X - %d Memory:0x%X - %d\n",
		i,
		(int) Payload[i],
		(int) Payload[i],
		(int) cp[i],
		(int) cp[i]);
	 bad++;
      }
   }
   printf("CompareParamsFile: Compared:%d bytes with File:%s Mismatches:%d ",len,path,bad);
   if (!bad) printf("- OK");
   printf("\n");
   fclose(fp);
   return arg;
}

// ====================================================

int WriteStatusFile(int arg) {

ArgVal   *v;
AtomType  at;

FILE *fp;
int cc;
char *cp;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Write status block to a file\n");
	 printf("   You optionally supply the target file\n");
	 printf("      No name: Implies use the default in amptest.config\n");
	 printf("      Path:    Write to the file specified in path\n");
	 printf("\n");
	 printf("N.B.: the status must have been read into memory\n");
	 printf("      using the read status command: See help\n");
	 return arg;
      }
   }

   cp = GetFileName(&arg);
   if ((cp) && (strcmp(cp,"RefAmpStatus") == 0)) cp = GetFile("RefAmpStatus");
   if (cp == NULL) cp = GetFile("AmpStatus");

   if (StatBlkValid) {
      umask(0);
      fp = fopen(path,"w");
      if (fp == NULL) {
	 fprintf(stderr,"WriteStatusFile:Can't open file: %s for write\n",path);
	 perror("fopen");
	 return arg;
      }

      cc = fwrite(&StatBlk,sizeof(AmpStatus),1,fp);
      if (cc != 1) {
	 fprintf(stderr,"Error:Write failed cc:%d should be:%d\n",cc,1);
	 perror("fwrite");
	 fclose(fp);
	 return arg;
      }
      fclose(fp);
      printf("WriteStatusFile: Written:Status to:%s OK\n",path);
   } else {
      printf("WriteStatusFile: No valid status in memory, read it first\n");
   }

   return arg;
}

// ====================================================

int ReadStatusFile(int arg) {

ArgVal   *v;
AtomType  at;

FILE *fp;
int cc;
char *cp;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Read reference status from a file\n");
	 printf("   You optionally supply the source file\n");
	 printf("      No name: Implies use the default in amptest.config\n");
	 printf("      Path:    Read from the file specified in path\n");
	 printf("\n");
	 return arg;
      }
   }

   cp = GetFileName(&arg);
   if ((cp) && (strcmp(cp,"RefAmpStatus") == 0)) cp = GetFile("RefAmpStatus");
   if (cp == NULL) cp = GetFile("AmpStatus");

   umask(0);
   fp = fopen(path,"r");
   if (fp == NULL) {
      fprintf(stderr,"ReadStatusFile:Can't open file: %s for read\n",path);
      perror("fopen");
      return arg;
   }

   cc = fread(&StatBlk,sizeof(AmpStatus),1,fp);
   if (cc != 1) {
      fprintf(stderr,"Error:Read failed cc:%d should be:%d\n",cc,1);
      perror("fread");
      fclose(fp);
      return arg;
   }

   StatBlkValid = 1;

   printf("ReadParamsFile: Read:Status from:%s OK\n",path);
   fclose(fp);
   return arg;
}

// ====================================================

int CompareStatusFile(int arg) {

ArgVal   *v;
AtomType  at;

FILE *fp;
int cc;
char *cp;

AmpStatus *ref;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Operator) {
      if (v->OId == OprNOOP) {
	 arg++;
	 v = &(vals[arg]);
	 at = v->Type;

	 printf("Compare status to a refernce file\n");
	 printf("   You optionally supply the source file\n");
	 printf("      No name: Implies use the default in amptest.config\n");
	 printf("      Path:    Compare with the file specified in path\n");
	 printf("\n");
	 printf("N.B. Status in memory must be valid\n");
	 return arg;
      }
   }

   cp = GetFileName(&arg);
   if ((cp) && (strcmp(cp,"AmpStatus") == 0)) cp = GetFile("AmpStatus");
   if (cp == NULL) cp = GetFile("RefAmpStatus");

   if (StatBlkValid) {

      umask(0);
      fp = fopen(path,"r");
      if (fp == NULL) {
	 fprintf(stderr,"CompareStatusFile:Can't open file: %s for read\n",path);
	 perror("fopen");
	 return arg;
      }

      ref = (AmpStatus *) Payload;

      cc = fread(ref,sizeof(AmpStatus),1,fp);
      if (cc != 1) {
	 fprintf(stderr,"Error:Read failed cc:%d should be:%d\n",cc,1);
	 perror("fread");
	 fclose(fp);
	 return arg;
      }

      if (strncmp(ref->OutAten1     ,StatBlk.OutAten1    ,2) != 0) Mismatch("OutAten1");
      if (strncmp(ref->OutAten2     ,StatBlk.OutAten2    ,2) != 0) Mismatch("OutAten2");
      if (strncmp(ref->Mute         ,StatBlk.Mute        ,2) != 0) Mismatch("Mute");
      if (strncmp(ref->Protect      ,StatBlk.Protect     ,2) != 0) Mismatch("Protection");
      if (strncmp(ref->Ready        ,StatBlk.Ready       ,2) != 0) Mismatch("Ready");
      if (strncmp(ref->SignalPres   ,StatBlk.SignalPres  ,2) != 0) Mismatch("SignalPresemce");
      if (strncmp(ref->Gain1        ,StatBlk.Gain1       ,2) != 0) Mismatch("Gain1");
      if (strncmp(ref->Gain2        ,StatBlk.Gain2       ,2) != 0) Mismatch("Gain2");
      if (strncmp(ref->OutVoltage1  ,StatBlk.OutVoltage1 ,2) != 0) Mismatch("OutVoltage1");
      if (strncmp(ref->OutVoltage2  ,StatBlk.OutVoltage2 ,2) != 0) Mismatch("OutVoltage2");
      if (strncmp(ref->MaxMainsCur  ,StatBlk.MaxMainsCur ,2) != 0) Mismatch("MaxMainsCurrent");
      if (strncmp(ref->Limiter      ,StatBlk.Limiter     ,2) != 0) Mismatch("Limiter");
      if (strncmp(ref->InputRout    ,StatBlk.InputRout   ,2) != 0) Mismatch("InputRoute");

      printf("End compare\n");

      return arg;
   }
   printf("CompareStatusFile:No valid status in memory\n");
   return arg;
}
