/* ============================================================= */
/* Send packets to a Socket                                      */
/* Do it with streams in server client form                      */
/* Julian Lewis AB/CO/HT 23rd Feb 2006 Julian.Lewis@CERN.ch      */
/* For Email the Subject line should contain the string "nospam" */
/* ============================================================= */

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
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <signal.h>
#include <wait.h>
#include <unistd.h>

#include <err/err.h>      /* Error handling */
#include <tgv/tgv.h>

#include <TimLib.h>
#include <TimServer.h>

extern pid_t getpgid(pid_t pid);

static int ssock = 0;

static int connected = 0;
static int seqnum    = 0;
static int errors    = 0;
static int debug     = 0;
static int remote    = 0;
static int qflag     = 0;
static int tmout     = 20000;

char *options[OPTIONS] = {"-p","-s","-eC","-eP","-d","-allow_remote"};

/* ============================================================= */
/* Open a UDP port for TimServer data                            */
/* ============================================================= */

int OpenPort(unsigned short port) {

struct sockaddr_in sin;

int s, on, off, val;

   if (!ssock) {

      s = socket(PF_INET, SOCK_STREAM, 0);
      if (s < 0) {
	 fprintf(stdout,"TimServer:OpenPort:Socket:Error:Cant open sockets\n");
	 perror("TimServer:OpenPort:Socket:errno");
	 return FAIL;
      }
      ssock = s;
      on = 1;
      off = 0;
      val = 5;
      setsockopt (ssock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (int));
      setsockopt (ssock, SOL_TCP, TCP_DEFER_ACCEPT, &off, sizeof (int));
      setsockopt (ssock, SOL_TCP, TCP_QUICKACK, &on, sizeof (int));
      setsockopt (ssock, SOL_TCP, TCP_KEEPCNT, &on, sizeof (int));
      setsockopt (ssock, SOL_TCP, TCP_KEEPIDLE, &val, sizeof (int));
      setsockopt (ssock, SOL_TCP, TCP_KEEPINTVL, &val, sizeof (int));
      setsockopt (ssock, SOL_TCP, TCP_LINGER2, &val, sizeof (int));
      setsockopt (ssock, SOL_TCP, TCP_NODELAY, &on, sizeof (int));
      setsockopt (ssock, SOL_TCP, TCP_SYNCNT, &val, sizeof (int));

      bzero((void *) &sin, SinSIZE);
      sin.sin_family = PF_INET;
      sin.sin_port = htons(port);
      sin.sin_addr.s_addr = htonl(INADDR_ANY);
      if (bind(s,(struct sockaddr *) &sin, SinSIZE) < 0) {
	 fprintf(stdout,"TimServer:OpenPort:Bind:Error:Cant bind port:%d\n",port);
	 perror("TimServer:OpenPort:Bind:errno");
	 close(s);
	 ssock = 0;
	 return FAIL;
      }
      if (listen(s,BACK_LOG) < 0) {
	 fprintf(stdout,"TimServer:Listen:Error:Port:%d\n",port);
	 perror("TimServer:OpenPort:Listen:errno");
	 close(s);
	 ssock = 0;
	 return FAIL;
      }
   }
   return ssock;
}

/* ============================================================= */
/* Get Configuration data for tim lib                            */
/* ============================================================= */

char *configpath = NULL;
char localconfigpath[128];  /* After a CD */

static char path[128];

/* ============================================================= */

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
	    configpath = "/dsc/bin/tim/timtest.config";
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

/* ============================================================= */
/* Convert a TimLib time in milliseconds to a string routine.    */
/* Result is a pointer to a static string representing the time. */
/*    the format is: Thu-18/Jan/2001 08:25:14.967                */
/*                   day-dd/mon/yyyy hh:mm:ss.ddd                */
/* ============================================================= */

char *TimeToStr(TimLibTime *t) {

static char tbuf[128];

char tmp[128];
char *yr, *ti, *md, *mn, *dy;

   bzero((void *) tbuf, 128);
   bzero((void *) tmp, 128);

   if (t->Second) {
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

/* ============================================================= */
/* Convert an Ltim to its name                                    */
/* ============================================================= */

PtmNames ptm_names[128];
static char ptm_name_txt[128];
int ptm_names_size = 0;

/* ============================================================= */

char *GetPtmName(unsigned long eqp) {

char *cp, *ep;
int i;
FILE *inp;

   if (ptm_names_size == 0) {
      GetFile("ltim.obnames");
      inp = fopen(path,"r");
      if (inp) {
	 while (fgets(ptm_name_txt,128,inp) != NULL) {
	    cp = ep = ptm_name_txt;
	    ptm_names[ptm_names_size].Eqp = strtoul(cp,&ep,0);
	    if (cp == ep) continue;
	    for (i=strlen(ep); i>=0; i--) {
	       if (ep[i] == '\n') {
		  ep[i] = 0;
		  break;
	       }
	    }
	    strcpy(ptm_names[ptm_names_size++].Name,++ep);
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

/* ============================================================= */
/* Convert a packet to a string                                  */
/* ============================================================= */

char *PacketToStr(AcqPacket *pkt) {

TgvName tname;
char *evn;
char *cln;
char aqtim[128], cytim[128];

static char spkt[PACKET_STRING_SIZE];

   evn = cln = "?";

   if (pkt->Class == TimLibClassPTIM) {
      evn = GetPtmName(pkt->Equipment);
      cln = "PTIM";
   }

   if (pkt->Class == TimLibClassCTIM) {
      TgvGetNameForMember(pkt->Equipment,&tname);
      evn = (char *) tname;
      cln = "CTIM";
   }

   bzero((void *) spkt, PACKET_STRING_SIZE);

   strcpy(aqtim,TimeToStr(&(pkt->Acquisition)));
   strcpy(cytim,TimeToStr(&(pkt->StartCycle)));

   sprintf(spkt,"eventName      :%s\n"
		"eventId        :%d\n"
		"eventClass     :%s\n"
		"oCounter       :%d\n"
		"acqC           :%d\n"
		"acqUTC         :%d\n"
		"acqNano        :%d\n"
		"cycleUTC       :%d\n"
		"cycleNano      :%d\n"
		"eventPayload   :0x%04X (%d)\n"
		"cycleUserName  :%s\n"
		"acqTimeString  :%s\n"
		"cycleTimeString:%s\n",
		      evn,
		(int) pkt->Equipment,
		      cln,
		(int) pkt->SequenceNumber,
		(int) pkt->Acquisition.CTrain,
		(int) pkt->Acquisition.Second,
		(int) pkt->Acquisition.Nano,
		(int) pkt->StartCycle.Second,
		(int) pkt->StartCycle.Nano,
		(int) pkt->Payload,
		(int) pkt->Payload,
		      pkt->CycleName,
		      aqtim,
		      cytim);

   if (debug) printf("\n%s\n",spkt);

   return spkt;
}

/* ============================================================= */
/* Connect to a CTIM equipment                                   */
/* ============================================================= */

int ConnectToCtim(unsigned long ctim) {

TimLibError err;

   err = TimLibInitialize(TimLibDevice_ANY);
   if (err != TimLibErrorSUCCESS) {
      fprintf(stdout,"TimServer:Error:%s\n",TimLibErrorToString(err));
      if (err == TimLibErrorIO) perror("TimServer:TimLib:errno");
      return FAIL;
   }

   err = TimLibQueue(qflag,tmout);
   if (err != TimLibErrorSUCCESS) {
      fprintf(stdout,"TimServer:Error:%s\n",TimLibErrorToString(err));
      if (err == TimLibErrorIO) perror("TimServer:TimLib:errno");
      return FAIL;
   }

   err = TimLibConnect(TimLibClassCTIM,ctim,0);
   if (err != TimLibErrorSUCCESS) {
      fprintf(stdout,"TimServer:Error:%s\n",TimLibErrorToString(err));
      if (err == TimLibErrorIO) perror("TimServer:TimLib:errno");
      return FAIL;
   }
   connected++;
   return connected;
}

/* ============================================================= */
/* Connect to a PTIM = Ltim equipment                            */
/* ============================================================= */

int ConnectToPtim(unsigned long ptim) {

TimLibError err;

   err = TimLibInitialize(TimLibDevice_ANY);
   if (err != TimLibErrorSUCCESS) {
      fprintf(stdout,"TimServer:Error:%s\n",TimLibErrorToString(err));
      if (err == TimLibErrorIO) perror("TimServer:TimLib:errno");
      return FAIL;
   }

   err = TimLibQueue(qflag,tmout);
   if (err != TimLibErrorSUCCESS) {
      fprintf(stdout,"TimServer:Error:%s\n",TimLibErrorToString(err));
      if (err == TimLibErrorIO) perror("TimServer:TimLib:errno");
      return FAIL;
   }

   err = TimLibConnect(TimLibClassPTIM,ptim,0);
   if (err != TimLibErrorSUCCESS) {
      fprintf(stdout,"TimServer:Error:%s\n",TimLibErrorToString(err));
      if (err == TimLibErrorIO) perror("TimServer:TimLib:errno");
      return FAIL;
   }
   connected++;
   return connected;
}

/* ============================================================= */
/* Wait for a connected interrupt                                */
/* ============================================================= */

int WaitForConnection(AcqPacket *pkt) {

char          *cp, txt[32];
TimLibError   err;
TimLibClass   iclss;
TimLibTime    trigger;
TimLibTime    output;
TimLibTime    cstamp;
unsigned long payload, equp;

   if (connected) {
      err = TimLibWait(&iclss,    /* Class of interrupt */
		       &equp,     /* PTIM CTIM or hardware mask */
		       NULL,      /* Ptim line number 1..n or 0 */
		       NULL,      /* Hardware source of interrupt */
		       &output,   /* Time of interrupt/output */
		       &trigger,  /* Time of counters load */
		       NULL,      /* Time of counters start */
		       NULL,      /* CTIM trigger equipment ID */
		       &payload,  /* Payload of trigger event */
		       NULL,      /* Module that interrupted */
		       NULL,      /* Number of missed interrupts */
		       NULL,      /* Remaining interrupts on queue */
		       NULL);     /* Corresponding TgmMachine */

      if (err != TimLibErrorSUCCESS) {
	 fprintf(stdout,"TimServer:Error:%s\n",TimLibErrorToString(err));
	 if (err == TimLibErrorIO) perror("TimServer:TimLib:errno");
	 return FAIL;
      }

      pkt->SequenceNumber = seqnum++;
      pkt->Equipment = equp;
      pkt->Class     = iclss;

      bzero((void *) &cstamp, sizeof(TimLibTime));
      err = TimLibGetTgmInfo(trigger,&cstamp,NULL,NULL);
      if (err != TimLibErrorSUCCESS) {
	 fprintf(stdout,"TimServer:Error:%s\n",TimLibErrorToString(err));
	 if (err == TimLibErrorIO) perror("TimServer:TimLib:errno");
	 return FAIL;
      }
      bcopy((void *) &cstamp,  (void *) &(pkt->StartCycle), sizeof(TimLibTime));

      if (iclss == TimLibClassCTIM) {
	 bcopy((void *) &trigger, (void *) &(pkt->Acquisition),sizeof(TimLibTime));
      } else {
	 bcopy((void *) &output, (void *) &(pkt->Acquisition),sizeof(TimLibTime));
      }

      pkt->Payload = payload;
      if ((payload>0) && (payload<=32)) cp = (char *) TgmGetLineName(TgmSPS,"USER",payload);
      else {
	 sprintf(txt,"0x%04X (Legacy)",(int) payload);
	 cp = txt;
      }
      if (cp) {
	 strcpy(pkt->CycleName,cp);
	 if (cp != txt) free(cp);
      } else pkt->CycleName[0] = 0;

      return OK;
   }
   return FAIL;
}

/* ============================================================= */
/* Errlog handler                                                */
/* ============================================================= */

static Boolean error_handler(class,text)
ErrClass class;
char *text; {

   errors++;
   fprintf(stdout,"TimServer:Errors:%d %s\n",errors,text);
   return(False);
}

/* ============================================================= */
/* Catch broken pipe signals when client dies                    */
/* ============================================================= */

static int pok = 1;

void sigpipehand(int signo) {
   pok = 0;
}

/* ============================================================= */
/* Catch Sig Child to clean up zombies                           */
/* ============================================================= */

void sigchildhand(int signo) {
   waitpid(-1,NULL,WNOHANG);
}

/* ============================================================= */
/* Catch Sig HUP from wreboot                                    */
/* ============================================================= */

void sighuphand(int signo) {
   exit(kill(0,SIGHUP));
}

/* ============================================================= */
/* Other signals                                                 */
/* ============================================================= */

void sigother(int signo) {
   printf("TimServer:Caught signal: %d: Shutting down\n",signo);
   sighuphand(SIGHUP);
}

/* ============================================================= */
/* Main: Proccess arguments and send events for ever             */
/* ============================================================= */

int main(int argc,char *argv[]) {

unsigned short port;
int            events;
unsigned long  event [MAX_EVENTS];
TimLibClass    tclass[MAX_EVENTS];

struct sockaddr_in client;
struct sigaction action, old;
pid_t child;

TgvName   tname;
AcqPacket pkt;

int   len, on, i, clsock;
char  *cp, *ep;
pid_t pid;

   if (argc <= 1) {
      printf("TimServer: Parameters are ...\n\n"
	     "   -p  <PORT>        ;Sets the target host port number\n"
	     "   -eC <CTIM NUMBER> ;Set a CTIM event (OEX_EN_CT=988 OEX_WE_CT=987)\n"
	     "   -eP <PTIM NUMBER  ;Set a PTIM/LTIM equipment (on this host)>\n"
	     "   -d                ;Set Debug printing on\n"
	     "   -allow_remote     ;Allow remote computers to connect to socket\n"
	    );
      exit(1);
   }

   /* Set up defaults */

   port = DEFAULT_PORT;

   events=0;
   bzero((void *) event,  MAX_EVENTS * sizeof(unsigned long));
   bzero((void *) tclass, MAX_EVENTS * sizeof(unsigned long));
   bzero((void *) &pkt, sizeof(AcqPacket));

   /* Process command line arguments */

   for (i=1; i<argc; i++) {

      cp = NULL;

      if (strncmp(argv[i],options[PORT],strlen(options[PORT])) == 0) {
	 i++;
	 cp = argv[i];
	 if (cp) port = (unsigned short) strtoul(cp,&ep,0);
	 continue;
      }

      if (strncmp(argv[i],options[CTIM],strlen(options[CTIM])) == 0) {
	 i++;
	 cp = argv[i];
	 if (cp) {
	    tclass[events  ] = TimLibClassCTIM;
	    event [events++] = strtoul(cp,&ep,0);
	 }
	 continue;
      }

      if (strncmp(argv[i],options[PTIM],strlen(options[PTIM])) == 0) {
	 i++;
	 cp = argv[i];
	 if (cp) {
	    tclass[events  ] = TimLibClassPTIM;
	    event [events++] = strtoul(cp,&ep,0);
	 }
	 continue;
      }

      if (strncmp(argv[i],options[DEBUG],strlen(options[DEBUG])) == 0) {
	 debug = 1;
	 continue;
      }

      if (strncmp(argv[i],options[REMOTE],strlen(options[REMOTE])) == 0) {
	 remote = 1;
	 continue;
      }

      printf("TimServer: No such option: %s\n",argv[i]);
      exit(1);
   }

   if (events) {
      fprintf(stdout,
	     "TimServer: Sartup: Port:%d ", port);

      for (i=0; i<events; i++) {
	 if (tclass[i] == TimLibClassPTIM) {
	    cp = GetPtmName(event[i]);
	    if (cp) fprintf(stdout,"PTIM:%s(%d) ",cp,(int) event[i]);
	    else {
	       fprintf(stdout,"\nTimServer: No such PTIM:%d\n",(int) event[i]);
	       exit(1);
	    }
	 } else {
	    if (TgvGetNameForMember(event[i],&tname)) cp = (char *) tname;
	    if (cp) fprintf(stdout,"CTIM:%s(%d) ",cp,(int) event[i]);
	    else {
	       fprintf(stdout,"\nTimServer: No such CTIM:%d\n",(int) event[i]);
	       exit(1);
	    }
	 }
      }
   } else {
      fprintf(stdout,"TimServer: No events specified, FATAL, exit\n");
      exit(1);
   }

   setpgid (pid,getpgid(0));

   ErrSetHandler((ErrHandler) error_handler);

   bzero((void *) &action, sizeof (action));
   action.sa_handler = (void (*)(int)) sigchildhand;
   sigaction(SIGCHLD, &action, &old);

   bzero((void *) &action, sizeof (action));
   action.sa_handler = (void (*)(int)) sighuphand;
   sigaction(SIGHUP, &action, &old);

   if (OpenPort(port)) {

      fprintf(stdout,"TimServer: Up and Running OK\n");

      while(1) {

	 while(1) {
	    len = sizeof(client);
	    clsock = accept(ssock,(struct sockaddr *) &client,&len);
	    if (clsock == -1) {
	       if (errno == EINTR) continue;

	       fprintf(stdout,"TimServer:Main:Accept:%d\n",errno);
	       perror("TimServer:Main:Accept:");
	       exit(kill(0,SIGHUP));
	    }
	    if (remote == 0) {
	       if (strcmp(LOOPBACK,inet_ntoa(client.sin_addr)) == 0) break;
	       printf("TimServer:Main:RejectConnection:%s\n",inet_ntoa(client.sin_addr));
	    } else break;
	    close(clsock);
	 }

	 on = 1;
	 setsockopt (clsock, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof (int));

	 child = fork();
	 if (child == 0) {
	    fprintf(stdout,"TimServer:Serve client:%s\n"
			  ,inet_ntoa(client.sin_addr));

	    for (i=0; i<events; i++) {
	       if (tclass[i] == TimLibClassPTIM) {
		  if (ConnectToPtim(event[i]) == FAIL) {
		     fprintf(stdout,"TimServer:Can't connect to PTIM:%d\n",(int) event[i]);
//                   close(clsock);
//                   exit(1);
		     continue;
		  }
	       } else {
		  if (ConnectToCtim(event[i]) == FAIL) {
		     fprintf(stdout,"TimServer:Can't connect to CTIM:%d\n",(int) event[i]);
//                   close(clsock);
//                   exit(1);
		     continue;
		  }
	       }
	    }

	    bzero((void *) &action, sizeof(action));
	    action.sa_handler = sigpipehand;
	    sigaction(SIGPIPE, &action, &old);

	    TgmAttach(TgmSPS,TgmTELEGRAM);

	    do {
	       if (WaitForConnection(&pkt)) {
		  cp = PacketToStr(&pkt);
		  len = write(clsock,cp,strlen(cp));
		  if (len <=0 ) pok = 0;
	       }
	    } while (pok);

	    fprintf(stdout,"TimServer:Terminate client:%s\n"
			  ,inet_ntoa(client.sin_addr));
	    close(clsock);
	    exit(0);
	 }

	 setpgid (child, getpgid (0));

	 bzero((void *) &action, sizeof (action));
	 action.sa_handler = (void (*)(int)) sigother;

	 sigaction(SIGINT, &action, &old);
	 sigaction(SIGQUIT, &action, &old);

	 sigaction(SIGSTOP, &action, &old);
	 sigaction(SIGCONT, &action, &old);

	 waitpid (-1, NULL, WNOHANG);
      }
   }
   fprintf(stdout,"TimServer:Main:Exit\n");
   exit(errno);
}
