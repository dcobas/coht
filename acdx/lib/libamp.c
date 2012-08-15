/* ================================================ */
/* Simply library interface to the AC Dipole system */
/* Amplifier                                        */
/* Julian Lewis AB/CO/HT Fri 17th July 2009         */
/* ================================================ */

#include <sys/termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <errno.h>        /* Error numbers */
#include <sys/file.h>
#include <a.out.h>
#include <ctype.h>
#include <time.h>
#include <sys/sem.h>

#include <libamp.h>
#include <libampP.h>

static int amp = 0;

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

static int slow = 0; // Seed of IO

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

static char *GetRouteName(char *name) {
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

static char *GetFile(char *name) {
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
	       fprintf(stderr,"libamp:Warning:%s Not found:All paths are: ./\n",CONFIG_FILE_NAME);
	    }
	    return path;
	 }
	 if (warnpath) {
	    warnpath = 0;
	    fprintf(stderr,"libamp:Warning:%s Not found\n",defaultconfigpath);
	    fprintf(stderr,"libamp:Warning:%s Used in local directory\n",CONFIG_FILE_NAME);
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

// =====================================================================
// Set options

static int SetOptions(int baudrate,
		      int databits,
		      char *parity,
		      char *stop,
		      int softwareHandshake,
		      int hardwareHandshake) {

struct termios newtio;
int mcs=0;
speed_t _baud=0;

   bzero((void *) &newtio, sizeof(struct termios));

   if (tcgetattr(amp, &newtio) != 0) {
      fprintf(stderr,"libamp:SetOptions:Error\n");
      perror("tcgetattr");
      return 0;
   }

   switch (baudrate) {
      case 600:
	 _baud=B600;
	 break;
      case 1200:
	 _baud=B1200;
	 break;
      case 2400:
	 _baud=B2400;
	 break;
      case 4800:
	 _baud=B4800;
	 break;
      case 9600:
	 _baud=B9600;
	 break;
      case 19200:
	 _baud=B19200;
	 break;
      case 38400:
	 _baud=B38400;
	 break;
      case 57600:
	 _baud=B57600;
	 break;
      case 115200:
	 _baud=B115200;
	 break;
      case 230400:
	 _baud=B230400;
	 break;
      case 460800:
	 _baud=B460800;
	 break;
      case 576000:
	 _baud=B576000;
	 break;
      case 921600:
	 _baud=B921600;
	 break;
      default: break;
   }
   cfsetospeed(&newtio, (speed_t)_baud);
   cfsetispeed(&newtio, (speed_t)_baud);

   if ( (databits == 7)
   &&   ((strcmp(parity,"Mark" ) == 0)
   ||    (strcmp(parity,"Space") == 0)) ) databits = 8;

   switch (databits) {
      case 5:
	 newtio.c_cflag = (newtio.c_cflag & ~CSIZE) | CS5;
	 break;
      case 6:
	 newtio.c_cflag = (newtio.c_cflag & ~CSIZE) | CS6;
	 break;
      case 7:
	 newtio.c_cflag = (newtio.c_cflag & ~CSIZE) | CS7;
	 break;
      case 8:
      default:
	 newtio.c_cflag = (newtio.c_cflag & ~CSIZE) | CS8;
	 break;
   }
   newtio.c_cflag |= CLOCAL | CREAD;

   newtio.c_cflag &= ~(PARENB | PARODD);
   if (strcmp(parity,"Even") == 0)
      newtio.c_cflag |= PARENB;
   else if (strcmp(parity,"Odd") == 0)
      newtio.c_cflag |= (PARENB | PARODD);

   newtio.c_cflag &= ~CRTSCTS;

   if (strcmp(stop,"2") == 0)
      newtio.c_cflag |= CSTOPB;
   else
      newtio.c_cflag &= ~CSTOPB;

    newtio.c_iflag=IGNBRK;

   if (softwareHandshake)
      newtio.c_iflag |= IXON | IXOFF;
   else
      newtio.c_iflag &= ~(IXON|IXOFF|IXANY);

   newtio.c_lflag=0;
   newtio.c_oflag=0;
   newtio.c_cc[VTIME]=1;
   newtio.c_cc[VMIN]=60;

   if (tcsetattr(amp, TCSANOW, &newtio) != 0) {
      fprintf(stderr,"libamp:SetOptions:Error\n");
      perror("tcsetattr");
      return 0;
   }

   ioctl(amp, TIOCMGET, &mcs);
   mcs |= TIOCM_RTS;
   ioctl(amp, TIOCMSET, &mcs);

   if (tcgetattr(amp, &newtio) != 0) {
      fprintf(stderr,"libamp:SetOptions:Error\n");
      perror("tcgetattr");
      return 0;
   }

   if (hardwareHandshake)
      newtio.c_cflag |= CRTSCTS;
   else
      newtio.c_cflag &= ~CRTSCTS;

   if (tcsetattr(amp, TCSANOW, &newtio) != 0) {
      fprintf(stderr,"libamp:SetOptions:Error\n");
      perror("tcgetattr");
      return 0;
   }
   return 1;
}

// =====================================================================
// Open the ttyMI0 PCI RS485 driver for the powersoft amplifier control

#define DEFAULT_AMP_DEVICE "/dev/ttyMI0"

static int AmpOpen() {

char *cp;

   cp = GetRouteName(GetFile("AmpDevice"));
   if ((cp == NULL) || (*cp == '.')) cp = DEFAULT_AMP_DEVICE;

   fprintf(stderr,"libamp:Opening:AmpDevice:%s",cp);

   if ((amp = open(cp,O_RDWR|O_SYNC|O_NDELAY,0)) > 0) {
      if (SetOptions(19200,8,"None","1",0,0) > 0) return amp;
      if (close(amp) != 0) perror("close");
   }
   amp = 0;

   return amp;
}

// =====================================================================
// Try to recover from an error

int AmpRecover() {
char *cp;

   cp = GetRouteName(GetFile("AmpDevice"));

   tcflush(amp, TCIOFLUSH);
   if (close(amp) != 0) {
      fprintf(stderr,"libamp:Error:Closing:AmpDevice:%s",cp);
      perror("close");
   }
   AmpOpen();
   return amp;
}

// ==========================================================

static int IsSpecial(char c) {
int i;
   for (i=0; i<SPECIAL_CHARS; i++) {
      if (SpecialChars[i] == c) return 1;
   }
   return 0;
}

// ==========================================================

static void EscapeOBuf() {

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
   fprintf(stderr,"libamp:Error:EscapeBuffer:Illegal size or not delimited by STX ETX\n");
}

// ==========================================================

static void UnEscapeIBuf() {

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
   fprintf(stderr,"libamp:Error:UnEscapeBuffer:Illegal size or not delimited by STX ETX\n");
}

// ==========================================================
// Reverse search for ETX at end of string to get size

static int GetBufSize(char *buf, int sz) {

int i;

   if (buf[0] == STX) {
      for (i=sz-1; i>0; i--) {
	 if (buf[i] == ETX) return i+1;
	 if (buf[i] != 0) break;
      }
   }
   fprintf(stderr,"libamp:Error:GetBufSize:Not delimited by STX ETX\n");
   return sz;
}

// ==========================================================

static char CalcCheckSum(PktType pkty, GenericPacket *pkt, int psz) {
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

// =====================================================

static char *Serialize(GenericPacket *pkt, int psz) {

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

   return OBuf;
}

// =====================================================

static char *SendRecvCommand(char cid, char *par, int psz) {

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
	 OBufSize = GetBufSize(OBuf,OBUF_SIZE);
	 EscapeOBuf();

	 if (OBufSize != write(amp,OBuf,OBufSize)) {
	    fprintf(stderr,"libamp:Error:Cant write to amp\n");
	    perror("write");
	 }
	 //tcflush(amp, TCOFLUSH);
	 tcdrain(amp);

	 if (slow) usleep(slow);

	 tmo = 0; IBufSize = 0; wexit = 0;
	 do {

	    usleep(100000); // It slow give it 100 ms between reads (10^5 us)

	    ibinc = read(amp,&IBuf[IBufSize],(IBUF_SIZE - IBufSize));
	    if (ibinc > 0) {
	       IBufSize += ibinc;
	       wexit++;
	    } else if (wexit) break;

	    if (tmo++ > 20) {
	       fprintf(stderr,"libamp:Error:Amplifier has not responed after 2 seconds\n");
	       perror("read");

	       amp = AmpRecover();
	       if (amp == 0) {
		  fprintf(stderr,"libamp:Error:Can't recover from amplifier error\n");
		  perror("open");
	       }
	       return NULL;
	    }

	 } while ((errno == EAGAIN) && (IBufSize < IBUF_SIZE)); // Wait for data

	 IBufSize = GetBufSize(IBuf,IBufSize);

	 UnEscapeIBuf();

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
	    printf("libamp:Reply:Not terminated by ETX\n");
	 }
	 return Payload;
      }
   }
   return NULL;
}

// ====================================================

static int GetDspParams(int blk) {

char param[6];

   sprintf(param,"03C%02X",blk); param[5] = 0;

   slow = 1000000;
   if (SendRecvCommand('x',param,5)) {
      slow = 0;
      if (blk != 2) {
	 bcopy(Payload,&(ChanPars[blk]),sizeof(ChannelParameters));
	 ChanParsValid[blk] = 1;
      } else {
	 bcopy(Payload,&ComPars,sizeof(CommonParameters));
	 ComParsValid = 1;
      }
      return 1;
   }
   return 0;
}

// ====================================================

static int CompareParamsFile(int blk) {

FILE *fp;
int i, len, cc, bad;
char *cp;

   if ( ((blk == 2) && (ComParsValid != 1))
   ||   (ChanParsValid[blk] != 1) ) {
      fprintf(stderr,"libamp:Error:Parameters binary block:%d must first be read from amp\n",blk);
      return 0;
   }

   cp = GetFile("RefAmpDspBin");
   sprintf(path,"%s%d",path,blk);

   umask(0);
   fp = fopen(path,"r");
   if (fp == NULL) {
      fprintf(stderr,"libamp:Error:CompareParamsFile:Can't open file:%s for read\n",path);
      perror("fopen");
      return 0;
   }

   if (blk == 2) { cp = (char *) &(ComPars);       len = sizeof(CommonParameters);  }
   else          { cp = (char *) &(ChanPars[blk]); len = sizeof(ChannelParameters); }

   cc = fread(Payload,len,1,fp);
   if (cc != 1) {
      fprintf(stderr,"libamp:Error:Read failed cc:%d should be:%d\n",cc,1);
      perror("fread");
      fclose(fp);
      return 0;
   }

   bad = 0;
   for (i=3; i<len; i++) {
      if (Payload[i] != cp[i]) {
	 if (bad < 4) {
	    fprintf(stderr,"libamp:Error:Mismatch:Byte:%d File:0x%X - %d Memory:0x%X - %d\n",
		    i,
		    (int) Payload[i],
		    (int) Payload[i],
		    (int) cp[i],
		    (int) cp[i]);
	 }
	 bad++;
      }
   }
   fclose(fp);
   if (bad) return 0;
   return 1;
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

// ====================================================

int CompareStatusFile() {

FILE *fp;
int cc;
char *cp;

AmpStatus *ref;

   cp = GetFile("RefAmpStatus");
   umask(0);
   fp = fopen(path,"r");
   if (fp == NULL) {
      fprintf(stderr,"CompareStatusFile:Can't open file: %s for read\n",path);
      perror("fopen");
      return 0;
   }

   ref = (AmpStatus *) Payload;

   cc = fread(ref,sizeof(AmpStatus),1,fp);
   if (cc != 1) {
      fprintf(stderr,"Error:Read failed cc:%d should be:%d\n",cc,1);
      perror("fread");
      fclose(fp);
      return 0;
   }

   if ( (strncmp(ref->OutAten1     ,StatBlk.OutAten1    ,2) != 0)
   ||   (strncmp(ref->OutAten2     ,StatBlk.OutAten2    ,2) != 0)
   ||   (strncmp(ref->Mute         ,StatBlk.Mute        ,2) != 0)
   ||   (strncmp(ref->Protect      ,StatBlk.Protect     ,2) != 0)
   ||   (strncmp(ref->Ready        ,StatBlk.Ready       ,2) != 0)
   ||   (strncmp(ref->SignalPres   ,StatBlk.SignalPres  ,2) != 0)
   ||   (strncmp(ref->Gain1        ,StatBlk.Gain1       ,2) != 0)
   ||   (strncmp(ref->Gain2        ,StatBlk.Gain2       ,2) != 0)
   ||   (strncmp(ref->OutVoltage1  ,StatBlk.OutVoltage1 ,2) != 0)
   ||   (strncmp(ref->OutVoltage2  ,StatBlk.OutVoltage2 ,2) != 0)
   ||   (strncmp(ref->MaxMainsCur  ,StatBlk.MaxMainsCur ,2) != 0)
   ||   (strncmp(ref->Limiter      ,StatBlk.Limiter     ,2) != 0)
   ||   (strncmp(ref->InputRout    ,StatBlk.InputRout   ,2) != 0) ) return 0;

   return 1;
}

// =====================================================

int AmpUnBlock();
int AmpBlock();

#define KEY 41793632
union sem_un { int               val;
	       struct semid_ds  *buff;
	       unsigned short   *array;
	       struct seminfo   *__buf;      /* buffer for IPC_INFO */
	       void             *__pad;
 };

int AmpIsBlocked() {

int key = KEY;
int sd;
int cnt;
union  sem_un arg;

   sd  = semget(key,1,0);
   if (sd == -1) return AmpUnBlock(); // Go create it

   bzero((char *) &arg, sizeof(arg));
   cnt = semctl(sd,0,GETVAL,arg);

   if (cnt == 0) return 1;    // Blocked
   else          return 0;    // Not blocked
}

// =====================================================

int AmpBlock() {

int key = KEY;
int sd;
union  sem_un arg;
struct sembuf sop;

  sd  = semget(key,1,0666|IPC_CREAT);
  if (sd == -1) {
  fail:
    return -1; /* can not start */
  }

  /* Unblock semaphore */

  bzero((char *) &arg, sizeof(arg));
  arg.val = 1;
  if (semctl(sd,0,SETVAL,arg) == -1) goto fail;

  /* Block it with UNDO on kill */

  bzero((char *) &sop, sizeof(sop));
  sop.sem_num = 0;
  sop.sem_op  = -1;
  sop.sem_flg = SEM_UNDO;
  if (semop(sd,&sop,1) == -1) goto fail;
  return 0;
}

// =====================================================

int AmpUnBlock() {

int key = KEY;
int sd;
union  sem_un arg;

  sd  = semget(key,1,0666|IPC_CREAT);
  if (sd == -1) {
  fail:
    return -1; /* can not start */
  }

  /* Unblock semaphore */

  bzero((char *) &arg, sizeof(arg));
  arg.val = 1;
  if (semctl(sd,0,SETVAL,arg) == -1) goto fail;

  return 0;
}

// =====================================================

int AmpIsOn() {                 /* Returns value of Amp-On status */

int i, bad;

   if (!amp) amp = AmpOpen();

   while (AmpIsBlocked()) usleep(10000);  /* Wait for IsConfig to complete */
   AmpBlock();

   bad = 0;
   for (i=1; i<=2; i++) {
      sprintf(Id,"%02X",i);
      if (SendRecvCommand('A',NULL,0) == 0) bad++;
   }
   AmpUnBlock();

   if (bad) return 0;
   return 1;
}

// =====================================================

int AmpIsConfigured() {         /* Returns value of Amp-Configured status */

int i, blk, bad;

   if (!amp) amp = AmpOpen();

   while (AmpIsBlocked()) usleep(10000);  /* Wait for IsConfig to complete */
   AmpBlock();

   bad = 0;
   for (i=1; i<=2; i++) {
      sprintf(Id,"%02X",i);
      for (blk=0; blk<3; blk++) {
	 if (GetDspParams(blk) == 0) bad++;
	 if (CompareParamsFile(blk) == 0) bad++;
      }
      if (ReadStatBlk() == 0) bad++;
      if (CompareStatusFile() == 0) bad++;
   }
   AmpUnBlock();

   if (bad) return 0;
   return 1;
}
