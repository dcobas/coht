/* ============================================================= */
/* Send packets to Gran Saso                                     */
/* Julian Lewis AB/CO/HT 23rd Feb 2006 Julian.Lewis@CERN.ch      */
/* For Email the Subject line should contain the string "nospam" */
/* ============================================================= */

#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/time.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <err/err.h>      /* Error handling */

#include <TimLib.h>
#include <CNGS.h>

#define SinSIZE (sizeof(struct sockaddr_in))

extern int timlib_real_utc;

static int ssock     = 0;
static int connected = 0;
static int seqnum    = 0;
static int errors    = 0;
static int debug     = 0;

static TgmLineNameTable ltab;

typedef enum {
   HELP,
   PORT,
   TARGET,
   CTIM,
   DEBUG,
   OPTIONS
 } Options;

char *options[OPTIONS] = {"help","port","target","ctim","debug"};

/* ============================================================= */

int OpenPort(unsigned short port) {

struct sockaddr_in sin;

int s;

   if (!ssock) {

      bzero((void *) &sin, SinSIZE);

      sin.sin_family      = AF_INET;
      sin.sin_port        = htons(port);
      sin.sin_addr.s_addr = htonl(INADDR_ANY);

      s = socket(AF_INET, SOCK_DGRAM, 0);
      if (s < 0) {
	 fprintf(stderr,"SendCNGS:OpenPort:Error:Cant open AF_INET sockets\n");
	 perror("SendCNGS:OpenPort:errno");
	 return CngsFAIL;
      }
      ssock = s;

      if (bind(s,(struct sockaddr *) &sin, SinSIZE) < 0) {
	 fprintf(stderr,"SendCNGS:OpenPort:Error:Cant bind port:%d\n",port);
	 perror("SendCNGS:OpenPort:errno");
	 return CngsFAIL;
      }
   }
   return ssock;
}

/* ============================================================= */

int SendToPort(char *target_ip, unsigned short target_port, CngsPacket *pkt) {

int cc;
struct sockaddr_in sin;

   if (ssock) {

      bzero((void *) &sin, SinSIZE);

      sin.sin_family      = AF_INET;
      sin.sin_port        = htons(target_port);
      sin.sin_addr.s_addr = inet_addr(target_ip);

      pkt->MagicNumber = CngsMAGIG_NUMBER;

      cc = sendto(ssock,
		  (char *) pkt,
		  sizeof(CngsPacket),
		  0,
		  (struct sockaddr *) &sin,
		  SinSIZE);
      if (cc < 0) {
	 fprintf(stderr,
		"SendCNGS:SendToPort:Error:Cant sendto port:%d at:%s\n",
		 target_port,
		 target_ip);
	 perror("SendCNGS:SendToPort:errno");
	 return CngsFAIL;
      }
      if (debug) printf("SendCNGS:SendToPort:Debug:Sent:%d packets\n",(int) pkt->SequenceNumber);
      return CngsOK;
   }
   return CngsFAIL;
}

/* ============================================================= */

int ConnectToCtim(unsigned long ctim) {

TimLibError err;

   err = TimLibInitialize(TimLibDevice_ANY);
   if (err != TimLibErrorSUCCESS) {
      fprintf(stderr,"SendCNGS:Error:%s\n",TimLibErrorToString(err));
      if (err == TimLibErrorIO) perror("SendCNGS:TimLib:errno");
      return CngsFAIL;
   }

   err = TimLibQueue(1,0);
   if (err != TimLibErrorSUCCESS) {
      fprintf(stderr,"SendCNGS:Error:%s\n",TimLibErrorToString(err));
      if (err == TimLibErrorIO) perror("SendCNGS:TimLib:errno");
      return CngsFAIL;
   }

   err = TimLibConnect(TimLibClassCTIM,ctim,0);
   if (err != TimLibErrorSUCCESS) {
      fprintf(stderr,"SendCNGS:Error:%s\n",TimLibErrorToString(err));
      if (err == TimLibErrorIO) perror("SendCNGS:TimLib:errno");
      return CngsFAIL;
   }
   connected++;
   return connected;
}

/* ============================================================= */
/* Get the time of day from the system clock in milliseconds.    */
/* ============================================================= */

CngsTime *GetSystemClock(void) {

static CngsTime result;

struct timeval tv;
struct timezone tz;

   tz.tz_dsttime = 0;
   tz.tz_minuteswest = 0;
   gettimeofday (&tv, &tz);

   result.Second  = tv.tv_sec;
   result.Nano    = tv.tv_usec * 1000;
   result.CTrain  = 0;
   result.Machine = 4;


   return &result;
}
/* ============================================================= */

void AddTimeMs(TimLibTime *ts, unsigned long ms, CngsTime *td) {

unsigned long sc, rm;

   sc = ms / 1000;
   rm = ms % 1000;

   bcopy((void *) ts, (void *) td, sizeof(TimLibTime));

   td->CTrain += ms;
   td->Second += sc;
   td->Nano   += rm * NS_IN_MS;
   while (td->Nano >= NS_IN_SEC) {
      td->Nano -= NS_IN_SEC;
      td->Second++;
   }
}

/* ============================================================= */

int WaitForConnection(CngsPacket *pkt) {

TimLibError   err;
TimLibClass   iclss;
TimLibTime    trigger;
TimLibTime    cstamp;
unsigned long payload, ctim, dn, gn, gv;
CngsTime      *tod;

   if (connected) {
      err = TimLibWait(&iclss,    /* Class of interrupt */
		       &ctim,     /* PTIM CTIM or hardware mask */
		       NULL,      /* Ptim line number 1..n or 0 */
		       NULL,      /* Hardware source of interrupt */
		       NULL,      /* Time of interrupt/output */
		       &trigger,  /* Time of counters load */
		       NULL,      /* Time of counters start */
		       NULL,      /* CTIM trigger equipment ID */
		       &payload,  /* Payload of trigger event */
		       NULL,      /* Module that interrupted */
		       NULL,      /* Number of missed interrupts */
		       NULL,      /* Remaining interrupts on queue */
		       NULL);     /* Corresponding TgmMachine */

      tod = GetSystemClock();     /* Get local system time of send */

      if (err != TimLibErrorSUCCESS) {
	 fprintf(stderr,"SendCNGS:Error:%s\n",TimLibErrorToString(err));
	 if (err == TimLibErrorIO) perror("SendCNGS:TimLib:errno");
	 return CngsFAIL;
      }

      if (iclss == TimLibClassCTIM) {

	 err = TimLibGetTgmInfo(trigger,&cstamp,NULL,NULL);
	 if (err != TimLibErrorSUCCESS) {
	    fprintf(stderr,"SendCNGS:Error:%s\n",TimLibErrorToString(err));
	    if (err == TimLibErrorIO) perror("SendCNGS:TimLib:errno");
	    return CngsFAIL;
	 }

	 bcopy((void *) &cstamp,  (void *) &(pkt->StartCycle), sizeof(CngsTime));
	 bcopy((void *) tod,      (void *) &(pkt->SendTime),   sizeof(CngsTime));

	 if (ctim == CngsSYNC)
	    bcopy((void *) &trigger, (void *) &(pkt->BasicPeriod), sizeof(CngsTime));
	 else {
	    bcopy((void *) &trigger, (void *) &(pkt->Extraction),  sizeof(CngsTime));
	    pkt->Extraction.Nano += CngsFW_EXTRACTION_NS;
	    if (pkt->Extraction.Nano >= NS_IN_SEC) {
	       pkt->Extraction.Nano -= NS_IN_SEC;
	       pkt->Extraction.Second +=1;
	    }
	 }
	 pkt->SequenceNumber = seqnum++;

	 if ((payload > 0) && (payload <= ltab.Size))
	    strcpy((char *) pkt->CycleName,ltab.Table[payload -1].Name);
	 else pkt->CycleName[0] = 0;
	 if (debug) printf("User:%s\n",pkt->CycleName);

	 gn = TgmGetGroupNumber(TgmSPS,"DURN");
	 if (gn) {
	    TimLibGetGroupValueFromStamp((TimLibTime) cstamp,gn,0,&gv);
	    if (gv) {
	       dn = gv * TgmGetBPWidth(); /* Duration in ms of cycle */
	       AddTimeMs(&cstamp,dn,&(pkt->EndCycle));
	    }
	 }


	 return CngsOK;
      }
   }
   return CngsFAIL;
}

/* ============================================================= */

static Boolean error_handler(class,text)
ErrClass class;
char *text; {

   errors++;
   fprintf(stderr,"SendCNGS:Errors:%d %s\n",errors,text);
   return(False);
}

/* ============================================================= */

int main(int argc,char *argv[]) {
int i;
char target[32], *cp, *ep;
unsigned long ctim;
unsigned short port;
CngsPacket pkt;

   timlib_real_utc = 1; /* Don't frig UTC to PS TG8 */

   ErrSetHandler((ErrHandler) error_handler);
   if (TgmAttach(TgmSPS,0xFFFF) != TgmSUCCESS) exit(1);

   if (TgmGetLineNameTable(TgmSPS,"USER",&ltab) != TgmSUCCESS) exit(1);

   port = CngsPORT;
   strcpy(target,CngsTARGET);
   ctim = CngsCTIM;
   bzero((void *) &pkt, sizeof(CngsPacket));

   for (i=1; i<argc; i++) {

      if (strcmp(argv[i],options[HELP]) == 0) {
	 printf("\nOptions are:\n\n");
	 for (i=0; i<OPTIONS; i++) {
	    printf("%s ",options[i]);
	    switch ((Options) i) {
	       case HELP:
		  printf("[print this help text]\n");
	       break;

	       case PORT:
		  printf("<UDP Port number to be used. Default:%d>\n",CngsPORT);
	       break;

	       case TARGET:
		  printf("<Target systems IP address. Default:%s>\n",CngsTARGET);
	       break;

	       case CTIM:
		  printf("<SPS extraction CTIM event ID. Default:%d>\n",CngsCTIM);
	       break;

	       case DEBUG:
		  printf("<Debug level: 0/None 1/Debug ON. Default:0>\n");
	       break;

	       default:
		  printf("For help type: SendCNGS help\n");
	    }
	 }
	 printf("\n\n");
	 exit(0);
      }

      else if (strcmp(argv[i],options[PORT]) == 0) {
	 i++;
	 cp = argv[i];
	 if (cp) port = (unsigned short) strtoul(cp,&ep,0);
	 continue;
      }

      else if (strcmp(argv[i],options[TARGET]) == 0) {
	 i++;
	 cp = argv[i];
	 if (cp) strcpy(target,cp);
	 continue;
      }

      else if (strcmp(argv[i],options[CTIM]) == 0) {
	 i++;
	 cp = argv[i];
	 if (cp) ctim = strtoul(cp,&ep,0);
	 continue;
      }

      else if (strcmp(argv[i],options[DEBUG]) == 0) {
	 i++;
	 cp    = argv[i];
	 debug = strtoul(cp,&ep,0);
	 continue;
      }

      else {
	 printf("No such option: %s\n",argv[i]);
	 printf("For help type: timtest %s\n",options[HELP]);
	 exit(1);
      }
   }

   fprintf(stderr,
	  "SendCNGS:Sartup parameters:Port:%d Target:%s Ctim:%d\n",
	   port,
	   target,
	   (int) ctim);

   if ( (OpenPort(port))
   &&   (ConnectToCtim(ctim))
   &&   (ConnectToCtim(CngsSYNC)) ) {

      fprintf(stderr,"SendCNGS: Up and running OK\n");

      while (errors < CngsMAX_ERRORS) {

	 if (WaitForConnection(&pkt)) {
	    if (!SendToPort(target,port,&pkt)) { sleep(1); errors++; }
	 } else { sleep(1); errors++; }
      }
   } else errors++;

   fprintf(stderr,"SendCNGS: ABORTED, after ErrorCount:%d\n",errors);

   exit(1);
}
