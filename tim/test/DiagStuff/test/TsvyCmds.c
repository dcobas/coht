/* ============================================================================== */
/* Test program for TimSurvey; Commands part                                      */
/* Julian Lewis 7th June 2007                                                     */
/* ============================================================================== */

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
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/file.h>
#include <a.out.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <dscrt/dscrtlib.h>

#define SinSIZE (sizeof(struct sockaddr_in))

/* No outgoing packet to TimSurvey can exceed this size */

#define OUT_BUF_SZ 132
static char out_buf[OUT_BUF_SZ];

/* No incomming packet from TimSurvey will exceed this size */

#define PADDING 16
#define IN_BUF_SZ 1100
static char in_buf[IN_BUF_SZ + PADDING];

typedef enum {
   TsHELP,
   TsKILL,
   TsPING,
   TsLIST,
   TsSTRINGS,
   TsSAMPLE,
   TsGET_STATUS,
   TsGET_STATIC,
   TsGET_DYNAMIC,
   TsDEBUG,
   TsGET_VERSION,
   TsRESET,
   TsCMDS
 } TsCmd;

static char *TsCmdNames[TsCMDS]  = {
   "Help",
   "Kill",
   "Ping",
   "List",
   "Strings",
   "Sampling",
   "GetStatus",
   "GetStatic",
   "GetDynamic",
   "Debug",
   "Version",
   "Reset"
 };

static char *TsFileNames[TsCMDS] = {
   "hlp",
   "kil",
   "png",
   "lst",
   "str",
   "smp",
   "rst",
   "ccv",
   "aqn",
   "dbg",
   "ver",
   "xeh"
 };

/* Handle packet streams simultaneously from multiple DSCs */

#define DSCS 10
#define DSC_NAME_SZ 31
#define CMD_NAME_SZ 15
#define MAX_PKTS    100
#define MAX_BUFS    10

typedef struct {
   char DscName[DSC_NAME_SZ +1];
   char CmdName[CMD_NAME_SZ +1];
   int  Size;                     /* Total size after getting an EOF */
   int  Valid;                    /* Number of valid packets recieved */
   char *Bufs[MAX_PKTS];
 } BufEntry;

BufEntry bufs[MAX_BUFS];

/* ==================================== */
/* Open a UDP port for SendSocket data  */

static int domain    = AF_INET;
static int send_sock = 0;

int OpenSendPort(unsigned short port) {

struct sockaddr_in sin;
int    s;

   if (!send_sock) {
      s = socket(domain, SOCK_DGRAM, 0);
      if (s < 0) {
	 fprintf(stderr,"tsvytest:OpenSendPort:Error:Cant open sockets\n");
	 perror("tsvytest:OpenSendPort:errno");
	 return 0;
      }
      send_sock = s;

      bzero((void *) &sin, SinSIZE);
      sin.sin_family = domain;
      sin.sin_port = htons(port);
      sin.sin_addr.s_addr = htonl(INADDR_ANY);
      if (bind(s,(struct sockaddr *) &sin, SinSIZE) < 0) {
	 fprintf(stderr,"tsvytest:OpenSendPort:Error:Cant bind port:%d\n",port);
	 perror("tsvytest:OpenSendPort:errno");
	 shutdown(send_sock,SHUT_RDWR);
	 close(send_sock);
	 send_sock = 0;
	 return 0;
      }
   }
   return 1;
}

/* ============================= */
/* Send packet to a port via UDP */

int SendToPort(char *ip, unsigned short port, char *pkt) {

int    cc;
struct sockaddr_in sin;

   if (send_sock) {
      bzero((void *) &sin, SinSIZE);
      sin.sin_family = domain;
      sin.sin_port = htons(port);
      sin.sin_addr.s_addr = inet_addr(ip);
      cc = sendto(send_sock,
		  pkt,
		  strlen(pkt),
		  0,
		  (struct sockaddr *) &sin,
		  SinSIZE);

      if (cc < 0) {
	 fprintf(stderr,
		"tsvytest:SendToPort:Error:Cant sendto port:%d at:%s\n",
		 port,
		 ip);
	 perror("tsvytest:SendToPort:errno");
	 return 0;
      }
      return 1;
   }
   return 0;
}

/* ==================================== */
/* Open a UDP port for RecvSocket data  */

static int recv_sock = 0;

int OpenRecvPort(unsigned short port) {

struct sockaddr_in sin;

int s;

   if (!recv_sock) {
      s = socket(domain, SOCK_DGRAM, 0);
      if (s < 0) {
	 fprintf(stderr,"tsvytest:OpenRecvPort:Error:Cant open sockets\n");
	 perror("tsvytest:OpenRecvPort:errno");
	 return 0;
      }
      recv_sock = s;

      bzero((void *) &sin, SinSIZE);
      sin.sin_family = domain;
      sin.sin_port = htons(port);
      sin.sin_addr.s_addr = htonl(INADDR_ANY);
      if (bind(s,(struct sockaddr *) &sin, SinSIZE) < 0) {
	 fprintf(stderr,"tsvytest:OpenRecvPort:Error:Cant bind port:%d\n",port);
	 perror("tsvytest:OpenRecvPort:errno");
	 shutdown(recv_sock,SHUT_RDWR);
	 close(recv_sock);
	 recv_sock = 0;
	 return 0;
      }
   }
   return 1;
}

/* ============================ */
/* Receive packets from socket  */

int RecvFromPort(unsigned short port, char *pkt, int len) {

int                cc;
socklen_t          from;
struct sockaddr_in sin;

   if (recv_sock) {
      bzero((void *) &sin, SinSIZE);
      from = sizeof(sin);
      cc = recvfrom(recv_sock,
		    pkt,
		    len,
		    0,
		    (struct sockaddr *) &sin,
		    &from);
      if (cc < 0) {
	 fprintf(stderr,
		"tsvytest:RecvFromPort:Error:Cant recvfrom port:%d\n",
		 port);
	 perror("tsvytest:RecvFromPort:errno");
	 return 0;
      }
      return 1;
   }
   return 0;
}

/* ====================================== */
/* Get file paths from configuration file */

#define LN 128

static char *defaultconfigpath = "/mcr/tim/tsvytest.config";
static char *configpath = NULL;
static char  localconfigpath[LN];
static char  path[LN];

char *GetFile(char *name) {
FILE *gpath = NULL;
char txt[LN];
int i, j;

   if (configpath) {
      gpath = fopen(configpath,"r");
      if (gpath == NULL) {
	 configpath = NULL;
      }
   }

   if (configpath == NULL) {
      configpath = "./tsvytest.config";
      gpath = fopen(configpath,"r");
      if (gpath == NULL) {
	 configpath = "/dsc/local/data/tsvytest.config";
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

   bzero((void *) path,LN);

   while (1) {
      if (fgets(txt,LN,gpath) == NULL) break;
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

/* ================================================= */
/* Get a buffer corresponding to a DSC and a command */

BufEntry *GetBuf(char *dnm, char *cmn) {

BufEntry *bep;
int i;

   if ((strlen(dnm) + strlen(cmn)) == 0) return NULL;

   for (i=0; i<MAX_BUFS; i++) {
      bep = &(bufs[i]);
      if ((strcmp(dnm,bep->DscName) == 0)
      &&  (strcmp(cmn,bep->CmdName) == 0)) return bep;
   }
   return NULL;
}

/* ============================================= */
/* Release a buffer and claim back storage space */

void RelBuf(char *dnm, char *cmn) {

BufEntry *bep;
int i;
char *cp;

   bep = GetBuf(dnm,cmn);
   if (bep) {
      for (i=0; i<bep->Size; i++) {
	 cp = bep->Bufs[i];
	 if (cp) free(cp);
      }
      bzero((void *) bep, sizeof(BufEntry));
      return;
   }
}

/* =================== */
/* Release all buffers */

void RelAll() {

BufEntry *bep;
int i;

   for (i=0; i<MAX_BUFS; i++) {
      bep = &(bufs[i]);
      RelBuf(bep->DscName,bep->CmdName);
   }
}

/* ======================= */
/* Get an empty new buffer */

BufEntry *NewBuf(char *dnm, char *cmn) {

BufEntry *bep;
int i;

   if (GetBuf(dnm,cmn)) return NULL; /* Already exists */

   for (i=0; i<MAX_BUFS; i++) {
      bep = &(bufs[i]);
      if ((strlen(bep->DscName) == 0)
      &&  (strlen(bep->CmdName) == 0)) {
	 strcpy(bep->DscName,dnm);
	 strcpy(bep->CmdName,cmn);
	 return bep;
      }
   }
   return NULL;
}

/* ======================== */
/* Print a buffer to stdout */

int PrintBuf(char *dnm, char *cmn) {

BufEntry *bep;
char *cp;
int i;

   bep = GetBuf(dnm,cmn);
   if (bep) {
      printf("%s.%s [%d.%d]\n",
	     bep->DscName,
	     bep->CmdName,
	     (int) bep->Size,
	     (int) bep->Valid);
      for (i=0; i<bep->Size; i++) {
	 cp = bep->Bufs[i];
	 if (cp) {
	    printf("%s",cp);
	 }
      }
      return 1;
   } else printf("No buffer for:%s.%s\n",dnm,cmn);
   return 0;
}

/* ====================================================================== */
/* Store packets in buffers.                                              */
/* enp is true when the EOF packet arrives, it should contain pkn packets */
/* otherwise write buf into memory at position pkn.                       */

int StorePacket(int enp, int pkn, char *dnm, char *cmn, char *buf) {

BufEntry *bep;
char *cp;

   bep = GetBuf(dnm,cmn);
   if ((bep == NULL) && (enp == 0)) bep = NewBuf(dnm,cmn);
   if (bep == NULL) return 0;

   if (enp) {
      bep->Size = pkn; /* Size it should be */
      return 1;
   }

   cp = (char *) malloc(IN_BUF_SZ + PADDING);
   if (cp) {
      bzero((void *) cp, IN_BUF_SZ + PADDING);
      bep->Bufs[pkn -1] = cp;
      strcpy(bep->Bufs[pkn -1],buf);
      bep->Valid++;    /* Actual size so far */
      return 1;
   }
   return 0;
}

/* ======================================================================= */
/* Poll the recv socket to see if any data is waiting.                     */
/* If nothing arrives after 1 Second we assume the transfers are finished. */

int PollRecv(int delay) {

fd_set rdset;
struct timeval tmo;
int nfds, cc;

   FD_ZERO(&rdset);
   FD_SET(recv_sock,&rdset);
   nfds = recv_sock + 1;
   tmo.tv_sec = delay; tmo.tv_usec = 0;
   cc = select(nfds,&rdset,NULL,NULL,&tmo);
   if (cc < 0) {
      fprintf(stderr,"PollRecv:(Select returned an error)\n");
      perror("PollRecv");
      return 0;
   }
   if (FD_ISSET(recv_sock,&rdset)) return cc;
   return 0;
}

/* ===================================================== */
/* Get incomming packets and save them in memory buffers */
/* All incomming packets have a first line like this ... */
/* PKT:<number>:<dsc name>:<command name>                */
/* EOF:<number>:<dsc name>:<command name>                */
/* The PKT: entry denotes the current packet             */
/* The EOF: entry denotes the total number of packets    */
/* Packets can arrive in any order                       */

#define FIRST_WAIT 120
#define THEN_WAIT 1

int GetPackets() {

int i, pkn, enp, res, tmo;
char c, *cp, *ep;
char dsc_name[DSC_NAME_SZ + 1];
char cmd_name[CMD_NAME_SZ + 1];

   res = 0; tmo = FIRST_WAIT;

   while (PollRecv(tmo)) {

      tmo = THEN_WAIT;

      bzero((void *) in_buf, IN_BUF_SZ + PADDING);
      RecvFromPort(RECV_PORT, in_buf, IN_BUF_SZ);

      enp = -1;
      if (strncmp("PKT:",in_buf,4) == 0) enp = 0;
      if (strncmp("EOF:",in_buf,4) == 0) enp = 1;
      if (enp == -1) continue;

      cp = &(in_buf[4]);
      pkn = strtoul(cp,&ep,0);
      if ((pkn == 0) || (cp == ep)) continue;

      cp = ep +1;
      bzero((void *) dsc_name, sizeof(dsc_name));
      for (i=0; i<DSC_NAME_SZ; i++) {
	 c = *cp++; if ((c == ':') || (c == '\n') || (c == 0)) break;
	 dsc_name[i] = c;
      }
      if (strlen(dsc_name) == 0) continue;

      bzero((void *) cmd_name, sizeof(cmd_name));
      if (c == ':') {
	 for (i=0; i<CMD_NAME_SZ; i++) {
	    c = *cp++; if ((c == ':') || (c == '\n') || (c == 0)) break;
	    cmd_name[i] = c;
	 }
      }
      if (strlen(cmd_name) == 0) continue;

      res += StorePacket(enp,pkn,dsc_name,cmd_name,cp);
   }

   if (res) return res;

   fprintf(stderr,"GetPackets:(Nothing received)\n");
   return 0;
}

/* ========================= */
/* Save packets to disc file */

int FWritePackets(char *dnm, char *cmn, char *fpath) {

FILE *fp;
BufEntry *bep;
int i, cnt;
char *cp;

   bep = GetBuf(dnm,cmn);
   if (bep == NULL) return 0;

   umask(0);
   unlink(fpath);
   fp = fopen(fpath,"w");
   if (fp == NULL) {
      printf("SavePackets: Can't open output file: %s for write\n",fpath);
      perror("SavePackets");
      return 0;
   }

   for (i=0, cnt=0; i<bep->Size; i++) {
      cp = bep->Bufs[i];
      if (cp) {
	 if (fwrite(cp,strlen(cp),1,fp) != 1) break;
	 cnt++;
      }
   }
   fclose(fp);
   return cnt;
}

/* ============================== */
/* Restore packets from disc file */

int FReadPackets(char *dnm, char *cmn, char *fpath) {

FILE *fp;
BufEntry *bep;
int i, cc, cnt, enp;
char tmp[IN_BUF_SZ];

   bep = NewBuf(dnm,cmn);
   if (bep == NULL) return 0;

   fp = fopen(fpath,"r");
   if (fp == NULL) {
      printf("RestPackets: Can't open input file: %s for read\n",fpath);
      perror("RestPackets");
      return 0;
   }

   enp = 0;
   for (i=0, cnt=0; i<MAX_PKTS; i++) {
      bzero((void *) tmp, IN_BUF_SZ);
      cc = fread(tmp, IN_BUF_SZ, 1, fp); cnt++;
      if (StorePacket(enp,i+1,dnm,cmn,tmp) == 0) break;
      if (cc <= 0) break;
   }
   enp = 1;
   fclose(fp);
   return StorePacket(enp,cnt,dnm,cmn,tmp);
}

/* ========================== */
/* Print out any program news */

static char *editor = "e";

int News(int arg) {

char sys[LN], npt[LN];

   arg++;

   if (GetFile("tsvy_news")) {
      strcpy(npt,path);
      sprintf(sys,"%s %s",GetFile(editor),npt);
      printf("\n%s\n",sys);
      system(sys);
      printf("\n");
   }
   return(arg);
}

/* ================================ */
/* Yes or No question asker routine */

static int yesno=1;

static int YesNo(char *question, char *name) {
int yn, c;

   if (yesno == 0) return 1;

   printf("%s: %s\n",question,name);
   printf("Continue (Y/N):"); yn = getchar(); c = getchar();
   if ((yn != (int) 'y') && (yn != 'Y')) return 0;
   return 1;
}

/* ============= */
/* Launch a task */

static void Launch(char *txt) {
pid_t child;

   if ((child = fork()) == 0) {
      if ((child = fork()) == 0) {
	 close(send_sock);
	 close(recv_sock);
	 system(txt);
	 exit(127);
      }
      exit (0);
   }
   while(waitpid(-1, NULL, WNOHANG) > 0);
}

/* ================================= */
/* Commands used in the test program */

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

/* =================================================== */
/* Change location of, or edit, the configuration file */

int ChangeDirectory(int arg) {
ArgVal   *v;
AtomType  at;
char txt[LN], fname[LN], *cp;
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

      bzero((void *) fname, LN);

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

/* ======================== */
/* Add a dsc to the dsclist */

static char dscname[DSC_NAME_SZ];
static char dscipad[DSC_NAME_SZ];

static int  dscs = 0;
static char dscnames[DSCS][DSC_NAME_SZ];
static char dscipads[DSCS][DSC_NAME_SZ];

int AddDsc(char *dnm, char *ipa, int flag) {

int i;

   if (strlen(dnm) && strlen(ipa)) {
      for (i=0; i<dscs; i++) {
	 if (strcmp(dnm,dscnames[i]) == 0) {
	    return 0;
	 }
      }

      if (dscs >= DSCS) {
	 printf("DSC: List is full\n");
	 return 0;
      }

      strcpy(dscnames[dscs],dnm);
      strcpy(dscipads[dscs],ipa);
      dscs++;
   }

   if (flag) {
      for (i=0; i<dscs; i++) {
	 printf("%2d:%s:%s",i+1,dscnames[i],dscipads[i]);
	 if (strcmp(dscname,dscnames[i]) == 0) printf(" <==");
	 printf("\n");
      }
   }
   return dscs;
}

/* ========================= */
/* Remove a DSC from program */

int RemoveDsc(char *dnm, int flag) {

int i, j;

   if (strlen(dnm)) {
      for (i=0; i<dscs; i++) {
	 if (strcmp(dnm,dscnames[i]) == 0) {
	    for (j=i; j<dscs; j++) {
	       strcpy(dscnames[j],dscnames[j+1]);
	       strcpy(dscipads[j],dscipads[j+1]);
	    }
	    dscs--;
	    bzero((void *) dscipads[dscs], DSC_NAME_SZ);
	    bzero((void *) dscnames[dscs], DSC_NAME_SZ);
	    break;
	 }
      }
   }
   strcpy(dscname,dscnames[0]);
   strcpy(dscipad,dscipads[0]);

   if (flag) {
      for (i=0; i<dscs; i++) {
	 printf("%2d:%s:%s",i+1,dscnames[i],dscipads[i]);
	 if (strcmp(dscname,dscnames[i]) == 0) printf(" <==");
	 printf("\n");
      }
   }
   return dscs;
}

/* ============================== */
/* Launch TimSurvey on target DSC */

int LaunchSurvey(char *name) {

char txt[LN];

   sprintf(out_buf,"Ping");
   SendToPort(dscipad, SEND_PORT, out_buf);
   while (PollRecv(1)) {
      bzero((void *) in_buf, IN_BUF_SZ + PADDING);
      RecvFromPort(RECV_PORT, in_buf, IN_BUF_SZ);
      sprintf(txt,"EOF:0:%s:Ping\n",dscname);
      if (strcmp(txt,in_buf) == 0) return 1;
   }

   sprintf(txt,"ssh -x -k -f %s \"prio 20 /usr/local/bin/TimSurvey&\"& exit",name);
   Launch(txt);
   sleep(1);
   return 1;
}

/* ============================= */
/* Send commands out to dsc list */

int SendCommand() {

int i;

   for (i=0; i<dscs; i++) {
      if (SendToPort(dscipads[i], SEND_PORT, out_buf) == 0) return 0;
   }
   return 1;
}

/* =========================== */
/* Set the working DSC by name */

int GetSetDsc(int arg) {

ArgVal   *v;
AtomType  at;
int i, n, earg;

struct hostent *hp;

char *cp, **cpp, tmp[DSC_NAME_SZ], txt[DSC_NAME_SZ];

   arg++;
   for (earg=arg; earg<pcnt; earg++) {
      v = &(vals[earg]);
      if ((v->Type == Close)
      ||  (v->Type == Terminator)) break;
   }

   bzero((void *) tmp, DSC_NAME_SZ);

   v = &(vals[arg]);
   at = v->Type;
   if ((v->Type != Close)
   &&  (v->Type != Terminator)) {
      n = 0;
      cp = &(cmdbuf[v->Pos]);
      do {
	 at = atomhash[(int) (*cp)];
	 if ((at != Seperator)
	 &&  (at != Close)
	 &&  (at != Terminator)) tmp[n++] = *cp;
	 tmp[n] = 0;
	 cp++;
      } while ((at != Close) && (at != Terminator));
   }

   if (strlen(tmp)) {
      hp = gethostbyname(tmp);
      if (hp == NULL) {
	 printf("Can't find ip address for host:%s\n",tmp);
	 return earg;
      }
      for (cpp=hp->h_aliases; *cpp; cpp++) printf("Alias:%s\n",*cpp);
      for (cpp=hp->h_addr_list; *cpp; cpp++) {
	 cp = (char *) inet_ntop(hp->h_addrtype,
				 (const void *) *cpp,
				 txt,
				 (socklen_t) sizeof(txt));
	 strcpy(dscname,hp->h_name);
	 strcpy(dscipad,cp);
	 break;
      }
      for (i=0; i<strlen(dscname); i++) {
	 if (dscname[i] == '.') {
	    dscname[i] = 0;
	    break;
	 }
      }
      AddDsc(dscname,dscipad,1);
      LaunchSurvey(dscname);
      return earg;
   }
   AddDsc("","",1);
   return earg;
}

/* ============================= */
/* Select the next DSC           */

int NextDsc(int arg) {

int i;

   arg++;

   if (dscs) {
      for (i=0; i<dscs; i++) {
	 if (strcmp(dscname,dscnames[i]) == 0) break;
      }
      if (++i >= dscs) i = 0;
      strcpy(dscname,dscnames[i]);
      strcpy(dscipad,dscipads[i]);
   }
   AddDsc("","",0);
   return arg;
}

/* =============== */
/* Get the version */

int GetVersion(int arg) {
int i;

   arg++;

   printf("tsvytest: Compiled: %s %s %s\n", "timtest", __DATE__, __TIME__);

   if (dscs) {
      sprintf(out_buf,"%s",TsCmdNames[TsGET_VERSION]);
      if (SendCommand()) {
	 if (GetPackets() == 0) return arg;
	 for (i=0; i<dscs; i++) {
	    if (PrintBuf(dscnames[i],TsCmdNames[TsGET_VERSION])) {
	       RelBuf(dscnames[i],TsCmdNames[TsGET_VERSION]);
	    }
	 }
      }
   }
   return arg;
}

/* ============================= */

int RemGetVersion(int arg) {

char fname[LN];
int i;

   arg++;

   if (dscs) {
      sprintf(out_buf,"%s",TsCmdNames[TsGET_VERSION]);
      if (SendCommand()) {
	 if (GetPackets() == 0) return arg;
	 for (i=0; i<dscs; i++) {
	    sprintf(fname,"/tmp/%s.%s",TsFileNames[TsGET_VERSION],dscnames[i]);
	    if (FWritePackets(dscnames[i],TsCmdNames[TsGET_VERSION],fname)) {
	       RelBuf(dscnames[i],TsCmdNames[TsGET_VERSION]);
	    }
	 }
      }
   }
   return arg;
}

/* ============================= */

int KillSurvey(int arg) {

   arg++;

   sprintf(out_buf,"%s",TsCmdNames[TsKILL]);
   SendToPort(dscipad, SEND_PORT, out_buf);
   while (PollRecv(1)) {
      bzero((void *) in_buf, IN_BUF_SZ + PADDING);
      RecvFromPort(RECV_PORT, in_buf, IN_BUF_SZ);
      printf("%s",in_buf);
   }
   return arg;
}

/* ============================= */

int ResetSurvey(int arg) {

   arg++;

   sprintf(out_buf,"%s",TsCmdNames[TsRESET]);
   SendToPort(dscipad, SEND_PORT, out_buf);
   while (PollRecv(1)) {
      bzero((void *) in_buf, IN_BUF_SZ + PADDING);
      RecvFromPort(RECV_PORT, in_buf, IN_BUF_SZ);
      printf("%s",in_buf);
   }
   return arg;
}

/* ============================================= */
/* Ping a DSC to see if TimSurvey daemon running */

int PingSurvey(int arg) {

char txt[LN], ok[DSCS];
int i, j;

   arg++;

   bzero((void *) ok, DSCS);

   for (i=0; i<dscs; i++) {
      sprintf(out_buf,"Ping");
      SendToPort(dscipads[i], SEND_PORT, out_buf);
      while (PollRecv(1)) {
	 bzero((void *) in_buf, IN_BUF_SZ + PADDING);
	 RecvFromPort(RECV_PORT, in_buf, IN_BUF_SZ);

	 for (j=0; j<=i; j++) {
	    sprintf(txt,"EOF:0:%s:Ping\n",dscnames[j]);
	    if (strcmp(txt,in_buf) == 0) {
	       printf("TimSurvey daemon alive on:%s\n",dscnames[j]);
	       ok[j] = 1;
	       break;
	    }
	 }
      }
   }
   for (i=0; i<dscs; i++) {
      if (ok[i] == 0) {
	 printf("No response from:%s\n",dscnames[i]);
	 continue;
      }
   }
   return arg;
}

/* =============================== */
/* Restart TimSurvey daemon on DSC */

int StartSurvey(int arg) {

   arg++;

   LaunchSurvey(dscname);
   printf("Started TimSurvey on:%s\n",dscname);
   return arg;
}

/* ============================= */

int GetStatus(int arg) {

int i;

   arg++;

   if (dscs) {
      sprintf(out_buf,"%s",TsCmdNames[TsGET_STATUS]);
      if (SendCommand()) {
	 if (GetPackets() == 0) return arg;
	 for (i=0; i<dscs; i++) {
	    if (PrintBuf(dscnames[i],TsCmdNames[TsGET_STATUS])) {
	       RelBuf(dscnames[i],TsCmdNames[TsGET_STATUS]);
	    }
	 }
      }
   }
   return arg;
}

/* ============================= */

int RemGetStatus(int arg) {

char fname[LN];
int i;

   arg++;

   if (dscs) {
      sprintf(out_buf,"%s",TsCmdNames[TsGET_STATUS]);
      if (SendCommand()) {
	 if (GetPackets() == 0) return arg;
	 for (i=0; i<dscs; i++) {
	    sprintf(fname,"/tmp/%s.%s",TsFileNames[TsGET_STATUS],dscnames[i]);
	    if (FWritePackets(dscnames[i],TsCmdNames[TsGET_STATUS],fname)) {
	       RelBuf(dscnames[i],TsCmdNames[TsGET_STATUS]);
	    }
	 }
      }
   }
   return arg;
}

/* ============================= */

unsigned long ccvmsk = 0;

int GetCcv(int arg) {

ArgVal   *v;
AtomType  at;

char fname[LN], cmtxt[LN];
int i;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Numeric) {
      ccvmsk = v->Number;
      arg++;
   }

   if (dscs) {
      sprintf(out_buf,"%s:%d",TsCmdNames[TsGET_STATIC],(int) ccvmsk);
      if (SendCommand()) {
	 if (GetPackets() == 0) return arg;
	 for (i=0; i<dscs; i++) {
	    sprintf(fname,"/tmp/%s.%s",TsFileNames[TsGET_STATIC],dscnames[i]);
	    if (FWritePackets(dscnames[i],TsCmdNames[TsGET_STATIC],fname)) {
	       RelBuf(dscnames[i],TsCmdNames[TsGET_STATIC]);
	    }
	    sprintf(cmtxt,"xterm 2>/dev/null -e %s %s",GetFile(editor),fname);
	    Launch(cmtxt);
	 }
      }
   }
   return arg;
}

/* ============================= */

int RemGetCcv(int arg) {

char fname[LN];

ArgVal   *v;
AtomType  at;

int i;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   if (at == Numeric) {
      ccvmsk = v->Number;
      arg++;
   }

   if (dscs) {
      sprintf(out_buf,"%s:%d",TsCmdNames[TsGET_STATIC],(int) ccvmsk);
      if (SendCommand()) {
	 if (GetPackets() == 0) return arg;
	 for (i=0; i<dscs; i++) {
	    sprintf(fname,"/tmp/%s.%s",TsFileNames[TsGET_STATIC],dscnames[i]);
	    if (FWritePackets(dscnames[i],TsCmdNames[TsGET_STATIC],fname)) {
	       RelBuf(dscnames[i],TsCmdNames[TsGET_STATIC]);
	    }
	 }
      }
   }
   return arg;
}

/* ============================= */

int GetAqn(int arg) {

char fname[LN], fsort[LN*4], cmtxt[LN];
int i;

   arg++;

   if (dscs) {
      sprintf(fsort,"/bin/sort -n ");
      sprintf(out_buf,"%s",TsCmdNames[TsGET_DYNAMIC]);
      if (SendCommand()) {
	 if (GetPackets() == 0) return arg;
	 for (i=0; i<dscs; i++) {
	    sprintf(fname,"/tmp/%s.%s",TsFileNames[TsGET_DYNAMIC],dscnames[i]);
	    if (FWritePackets(dscnames[i],TsCmdNames[TsGET_DYNAMIC],fname)) {
	       RelBuf(dscnames[i],TsCmdNames[TsGET_DYNAMIC]);
	       strcat(fsort,fname); strcat(fsort," ");
	    }
	    sprintf(cmtxt,"xterm 2>/dev/null -e %s %s",GetFile(editor),fname);
	    Launch(cmtxt);
	 }
	 if (dscs > 1) {
	    strcat(fsort,"> /tmp/aqn.sort");
	    printf("execute:%s\n",fsort);
	    system(fsort);
	    sprintf(fsort,"xterm 2>/dev/null -e %s /tmp/aqn.sort",GetFile(editor));
	    Launch(fsort);
	 }
      }
   }
   return arg;
}

/* ============================= */

int RemGetAqn(int arg) {

char fname[LN], fsort[LN];
int i;

   arg++;

   if (dscs) {
      sprintf(fsort,"/bin/sort -n ");
      sprintf(out_buf,"%s",TsCmdNames[TsGET_DYNAMIC]);
      if (SendCommand()) {
	 if (GetPackets() == 0) return arg;
	 for (i=0; i<dscs; i++) {
	    sprintf(fname,"/tmp/%s.%s",TsFileNames[TsGET_DYNAMIC],dscnames[i]);
	    if (FWritePackets(dscnames[i],TsCmdNames[TsGET_DYNAMIC],fname)) {
	       RelBuf(dscnames[i],TsCmdNames[TsGET_DYNAMIC]);
	       strcat(fsort,fname); strcat(fsort," ");
	    }
	 }
      }
      strcat(fsort,"> /tmp/aqn.sort");
      system(fsort);
   }
   return arg;
}

/* ============================= */

int GetLsp(int arg) {

int i;

   arg++;

   if (dscs) {
      sprintf(out_buf,"%s",TsCmdNames[TsLIST]);
      if (SendCommand()) {
	 if (GetPackets() == 0) return arg;
	 for (i=0; i<dscs; i++) {
	    if (PrintBuf(dscnames[i],TsCmdNames[TsLIST])) {
	       RelBuf(dscnames[i],TsCmdNames[TsLIST]);
	    }
	 }
      }
   }
   return arg;
}

/* ============================= */

int RemGetLsp(int arg) {

char fname[LN];
int i;

   arg++;

   if (dscs) {
      sprintf(out_buf,"%s",TsCmdNames[TsLIST]);
      if (SendCommand()) {
	 if (GetPackets() == 0) return arg;
	 for (i=0; i<dscs; i++) {
	    sprintf(fname,"/tmp/%s.%s",TsFileNames[TsLIST],dscnames[i]);
	    if (FWritePackets(dscnames[i],TsCmdNames[TsLIST],fname)) {
	       RelBuf(dscnames[i],TsCmdNames[TsLIST]);
	    }
	 }
      }
   }
   return arg;
}

/* ============================= */

int GetHelp(int arg) {

int i;

   arg++;

   if (dscs) {
      sprintf(out_buf,"%s",TsCmdNames[TsHELP]);
      if (SendCommand()) {
	 if (GetPackets() == 0) return arg;
	 for (i=0; i<dscs; i++) {
	    if (PrintBuf(dscnames[i],TsCmdNames[TsHELP])) {
	       RelBuf(dscnames[i],TsCmdNames[TsHELP]);
	    }
	 }
      }
   }
   return arg;
}

/* ============================= */

static unsigned long txton = 1;

int GetSetTxt(int arg) {

ArgVal   *v;
AtomType  at;
int i;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   txton = 1;
   if (at == Numeric) {
      if (v->Number) txton = 1;
      else           txton = 0;
   }

   if (dscs) {
      sprintf(out_buf,"%s:%d",TsCmdNames[TsSTRINGS],(int) txton);
      if (SendCommand()) {
	 if (GetPackets() == 0) return arg;
	 for (i=0; i<dscs; i++) {
	    if (PrintBuf(dscnames[i],TsCmdNames[TsSTRINGS])) {
	       RelBuf(dscnames[i],TsCmdNames[TsSTRINGS]);
	    }
	 }
      }
   }
   return arg;
}

/* ============================= */

static unsigned long rdebug = 0;

int GetSetDebug(int arg) {

ArgVal   *v;
AtomType  at;
int i;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   rdebug = 0;
   if (at == Numeric) {
      if (v->Number) rdebug = 1;
      else           rdebug = 0;
   }

   if (dscs) {
      sprintf(out_buf,"%s:%d",TsCmdNames[TsDEBUG],(int) rdebug);
      if (SendCommand()) {
	 if (GetPackets() == 0) return arg;
	 for (i=0; i<dscs; i++) {
	    if (PrintBuf(dscnames[i],TsCmdNames[TsDEBUG])) {
	       RelBuf(dscnames[i],TsCmdNames[TsDEBUG]);
	    }
	 }
      }
   }
   return arg;
}

/* ============================= */

static unsigned long smpmsk[DSCS];

int GetSetSampling(int arg) {

ArgVal   *v;
AtomType  at;
int i, dsci;

   arg++;
   v = &(vals[arg]);
   at = v->Type;

   dsci = -1;
   if (at == Numeric) {
      arg++;

      for (i=0; i<dscs; i++) {
	 if (strcmp(dscnames[i],dscname) == 0) {
	    dsci = i;
	    smpmsk[dsci] = v->Number;
	 }
      }
   }

   if (dscs) {
      if (dsci >= 0) {
	 sprintf(out_buf,"%s:%d",TsCmdNames[TsSAMPLE],(int) smpmsk[dsci]);
	 if (SendToPort(dscipads[dsci], SEND_PORT,out_buf)) {
	    if (GetPackets() == 0) return arg;
	    if (PrintBuf(dscnames[dsci],TsCmdNames[TsSAMPLE])) {
	       RelBuf(dscnames[dsci],TsCmdNames[TsSAMPLE]);
	    }
	 }
      }

      printf("DSC Sampling Masks:\n");
      for (i=0; i<dscs; i++) {
	 printf("%s:0x%04X\n",dscnames[i],(int) smpmsk[i]);
      }
   } else
      printf("No DSCs are defined\n");

   return arg;
}

/* ============================= */

int SaveAqn(int arg) {

char sname[LN], dname[LN];

   arg++;

   sprintf(sname,"/tmp/%s.%s",TsFileNames[TsGET_DYNAMIC],dscname);
   sprintf(dname,"/ps/dsc/mcr/data/tim/%s.%s",TsFileNames[TsGET_DYNAMIC],dscname);

   if (FReadPackets(dscname,TsCmdNames[TsGET_DYNAMIC],sname)) {
      printf("Read:%s OK\n",sname);
      if (FWritePackets(dscname,TsCmdNames[TsGET_DYNAMIC],dname)) {
	 printf("Written:%s OK\n",dname);
      }
   }
   RelBuf(dscname,TsCmdNames[TsGET_DYNAMIC]);

   return arg;
}

/* ============================= */

int RestAqn(int arg) {

char sname[LN], dname[LN];

   arg++;

   sprintf(dname,"/tmp/%s.%s",TsFileNames[TsGET_DYNAMIC],dscname);
   sprintf(sname,"/ps/dsc/mcr/data/tim/%s.%s",TsFileNames[TsGET_DYNAMIC],dscname);

   if (FReadPackets(dscname,TsCmdNames[TsGET_DYNAMIC],sname)) {
      printf("Read:%s OK\n",sname);
      if (FWritePackets(dscname,TsCmdNames[TsGET_DYNAMIC],dname)) {
	 printf("Written:%s OK\n",dname);
      }
   }
   RelBuf(dscname,TsCmdNames[TsGET_DYNAMIC]);
   sprintf(sname,"xterm 2>/dev/null -e %s %s",GetFile(editor),dname);
   Launch(sname);

   return arg;
}

/* ============================= */

int SaveCcv(int arg) {

char sname[LN], dname[LN];

   arg++;

   sprintf(sname,"/tmp/%s.%s",TsFileNames[TsGET_STATIC],dscname);
   sprintf(dname,"/ps/dsc/mcr/data/tim/%s.%s",TsFileNames[TsGET_STATIC],dscname);

   if (FReadPackets(dscname,TsCmdNames[TsGET_STATIC],sname)) {
      printf("Read:%s OK\n",sname);
      if (FWritePackets(dscname,TsCmdNames[TsGET_STATIC],dname)) {
	 printf("Written:%s OK\n",dname);
      }
   }
   RelBuf(dscname,TsCmdNames[TsGET_STATIC]);

   return arg;
}

/* ============================= */

int RestCcv(int arg) {

char sname[LN], dname[LN];

   arg++;

   sprintf(dname,"/tmp/%s.%s",TsFileNames[TsGET_STATIC],dscname);
   sprintf(sname,"/ps/dsc/mcr/data/tim/%s.%s",TsFileNames[TsGET_STATIC],dscname);

   if (FReadPackets(dscname,TsCmdNames[TsGET_STATIC],sname)) {
      printf("Read:%s OK\n",sname);
      if (FWritePackets(dscname,TsCmdNames[TsGET_STATIC],dname)) {
	 printf("Written:%s OK\n",dname);
      }
   }
   RelBuf(dscname,TsCmdNames[TsGET_STATIC]);
   sprintf(sname,"xterm 2>/dev/null -e %s %s",GetFile(editor),dname);
   Launch(sname);

   return arg;
}

/* ============================= */

int CompareAqn(int arg) {

char sname[LN], dname[LN], cmtxt[LN];

   arg++;

   sprintf(dname,"/tmp/%s.%s",TsFileNames[TsGET_DYNAMIC],dscname);
   sprintf(sname,"/ps/dsc/mcr/data/tim/%s.%s",TsFileNames[TsGET_DYNAMIC],dscname);
   printf("Compare AQN:%s Against Reference:%s\n",dname,sname);
   sprintf(cmtxt,"diff -abBi %s %s",sname,dname);
   printf("Execute:%s\n",cmtxt);
   system(cmtxt);
   return arg;
}

/* ============================= */

int CompareCcv(int arg) {

char sname[LN], dname[LN], cmtxt[LN];

   arg++;

   sprintf(dname,"/tmp/%s.%s",TsFileNames[TsGET_STATIC],dscname);
   sprintf(sname,"/ps/dsc/mcr/data/tim/%s.%s",TsFileNames[TsGET_STATIC],dscname);
   printf("Compare CCV:%s Against Reference:%s\n",dname,sname);
   sprintf(cmtxt,"diff -abBi %s %s",sname,dname);
   printf("Execute:%s\n",cmtxt);
   system(cmtxt);
   return arg;
}

/* ============================= */

int List(int arg) {

char cmtxt[LN];

   arg++;

   printf("\nTemp files Static: CCV\n");
   sprintf(cmtxt,"ls -l /tmp/ccv.*");
   system(cmtxt);
   printf("Temp files Dynamic: AQN\n");
   sprintf(cmtxt,"ls -l /tmp/aqn.*");
   system(cmtxt);
   printf("Temp files Status: RST\n");
   sprintf(cmtxt,"ls -l /tmp/rst.*");
   system(cmtxt);
   printf("Temp files Version: VER\n");
   sprintf(cmtxt,"ls -l /tmp/ver.*");
   system(cmtxt);

   printf("\nRefernce files Static: CCV\n");
   sprintf(cmtxt,"ls -l /ps/dsc/mcr/data/tim/ccv.*");
   system(cmtxt);
   printf("Reference files Dynamic: AQN\n");
   sprintf(cmtxt,"ls -l /ps/dsc/mcr/data/tim/aqn.*");
   system(cmtxt);

   return arg;
}

/* ============================= */

int Clean(int arg) {

char cmtxt[LN];

   printf("Cleaning ccv, aqn, rst, ver files from /tmp\n");

   sprintf(cmtxt,"rm -f /tmp/ccv.*");
   system(cmtxt);
   sprintf(cmtxt,"rm -f /tmp/aqn.*");
   system(cmtxt);
   sprintf(cmtxt,"rm -f /tmp/rst.*");
   system(cmtxt);
   sprintf(cmtxt,"rm -f /tmp/ver.*");
   system(cmtxt);
   return List(arg);
}

/* ======================= */
/* Save DSC list in a file */

int SaveDscList(int arg) {

ArgVal   *v;
AtomType  at;
int i, n, earg;

char *cp, fpath[LN], txt[LN];

FILE *fp;

   arg++;
   for (earg=arg; earg<pcnt; earg++) {
      v = &(vals[earg]);
      if ((v->Type == Close)
      ||  (v->Type == Terminator)) break;
   }

   bzero((void *) fpath, LN);

   v = &(vals[arg]);
   at = v->Type;
   if ((v->Type != Close)
   &&  (v->Type != Terminator)) {
      n = 0;
      cp = &(cmdbuf[v->Pos]);
      do {
	 at = atomhash[(int) (*cp)];
	 if ((at != Seperator)
	 &&  (at != Close)
	 &&  (at != Terminator)) fpath[n++] = *cp;
	 fpath[n] = 0;
	 cp++;
      } while ((at != Close) && (at != Terminator));
   }
   if (strlen(fpath) == 0) strcpy(fpath,"/tmp/dsc.lst");

   umask(0);
   unlink(fpath);
   fp = fopen(fpath,"w");
   if (fp == NULL) {
      printf("Can't open:%s for write\n",fpath);
      perror("Err");
      return earg;
   }
   sprintf(txt,"%d\n",dscs);
   if (fwrite(txt,strlen(txt),1,fp) != 1) {
      printf("Can't write to file:%s\n",fpath);
      perror("Err");
      fclose(fp);
      return earg;
   }
   for (i=0; i<dscs; i++) {
      sprintf(txt,"%s %s\n",dscnames[i],dscipads[i]);
      if (fwrite(txt,strlen(txt),1,fp) != 1) {
	 printf("Unexpected EOF when writing:%s\n",fpath);
	 perror("Err");
	 break;
      }
   }
   fclose(fp);
   return earg;
}

/* =========================== */
/* Load a DSC list from a file */

int LoadDscList(int arg) {

ArgVal   *v;
AtomType  at;
int i, j, n, earg, ndscs;

char *cp, *ep, fpath[LN], txt[LN];

FILE *fp;

   arg++;
   for (earg=arg; earg<pcnt; earg++) {
      v = &(vals[earg]);
      if ((v->Type == Close)
      ||  (v->Type == Terminator)) break;
   }

   bzero((void *) fpath, LN);

   v = &(vals[arg]);
   at = v->Type;
   if ((v->Type != Close)
   &&  (v->Type != Terminator)) {
      n = 0;
      cp = &(cmdbuf[v->Pos]);
      do {
	 at = atomhash[(int) (*cp)];
	 if ((at != Seperator)
	 &&  (at != Close)
	 &&  (at != Terminator)) fpath[n++] = *cp;
	 fpath[n] = 0;
	 cp++;
      } while ((at != Close) && (at != Terminator));
   }
   if (strlen(fpath) == 0) strcpy(fpath,"/tmp/dsc.lst");

   fp = fopen(fpath,"r");
   if (fp == NULL) {
      printf("Can't open:%s for read\n",fpath);
      perror("Err");
      return earg;
   }

   if (fgets(txt, LN, fp)) {
      cp = txt; ep = NULL;
      ndscs = strtoul(cp, &ep, 0);
   } else {
      printf("Can't read file:%s\n",fpath);
      perror("Err");
      fclose(fp);
      return earg;
   }

   for (i=0; i<ndscs; i++) {
      if (fgets(txt, LN, fp)) {
	 if (strlen(txt)) txt[strlen(txt) -1] = 0;
	 for (j=0; j<strlen(txt); j++) {
	    if (txt[j] == ' ') {

	       txt[j] = 0;
	       strcpy(dscname,&txt[0]);
	       strcpy(dscipad,&txt[j+1]);

	       AddDsc(dscname,dscipad,0);
	       break;
	    }
	 }
      } else {
	 printf("Unexpected EOF in:%s\n",fpath);
	 perror("Err");
	 break;
      }
   }
   fclose(fp);
   AddDsc("","",1);
   return earg;
}

/* =============================== */
/* Clear current DSC from the list */

int ClearDsc(int arg) {

   arg++;

   RemoveDsc(dscname,1);

   return arg;
}

/* ============================ */
/* Clear all dscs from the list */

int ClearAllDscs(int arg) {

int i;

   arg++;

   for (i=0; i<DSCS; i++) {
      bzero((void *) dscipads[i], DSC_NAME_SZ);
      bzero((void *) dscnames[i], DSC_NAME_SZ);
   }
   bzero((void *) dscname, DSC_NAME_SZ);
   bzero((void *) dscipad, DSC_NAME_SZ);

   dscs = 0;
   return arg;
}
