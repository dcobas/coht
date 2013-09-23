/* ============================================================= */
/* Recieve packets as text from source socket and print them.    */
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
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <err/err.h>      /* Error handling */
#include <tgv/tgv.h>

#include <TimLib.h>
#include <SendSocket.h>

static int ssock     = 0;
static int errors    = 0;
static int domain    = PF_UNIX;

char *options[OPTIONS] = {"-p","-t","-inet","-eC","-eP","-d"};

/* ============================================================= */
/* Errlog handler                                                */
/* ============================================================= */

static Boolean error_handler(class,text)
ErrClass class;
char *text; {

   errors++;
   fprintf(stderr,"SendSocket:Errors:%d %s\n",errors,text);
   return(False);
}

/* ============================================================= */
/* Open a UDP port for RecvSocket data                           */
/* ============================================================= */

int OpenPort(unsigned short port) {

struct sockaddr_un snm;
struct sockaddr_in sin;

int s, size;
char   fnam[FILE_NAME_SIZE];

   if (!ssock) {

      s = socket(domain, SOCK_DGRAM, 0);
      if (s < 0) {
	 fprintf(stderr,"SendSocket:OpenPort:Error:Cant open sockets\n");
	 perror("SendSocket:OpenPort:errno");
	 return FAIL;
      }
      ssock = s;

      if (domain == PF_INET) {
	 bzero((void *) &sin, SinSIZE);
	 sin.sin_family = domain;
	 sin.sin_port = htons(port);
	 sin.sin_addr.s_addr = htonl(INADDR_ANY);
	 if (bind(s,(struct sockaddr *) &sin, SinSIZE) < 0) {
	    fprintf(stderr,"SendSocket:OpenPort:Error:Cant bind port:%d\n",port);
	    perror("SendSocket:OpenPort:errno");
	    close(ssock);
	    ssock = 0;
	    return FAIL;
	 }
      } else {
	 sprintf(fnam,"/tmp/SktRcv%d",(int) port);
	 snm.sun_family = PF_LOCAL;
	 strncpy (snm.sun_path, fnam, sizeof (snm.sun_path));
	 unlink(fnam);
	 size = SUN_LEN(&snm);
	 if (bind(ssock,(struct sockaddr *) &snm,size) < 0) {
	    fprintf(stderr,"SendSocket:OpenPort:Error:Cant bind port:%d\n",port);
	    perror("SendSocket:OpenPort:errno");
	    close(ssock);
	    ssock = 0;
	    return FAIL;
	 }
      }
   }
   return ssock;
}

/* ============================================================= */
/* Receive packets from socket                                   */
/* ============================================================= */

int RecvFromPort(unsigned short source_port, char *spkt) {

int cc;
char fnam[FILE_NAME_SIZE];
socklen_t from;

struct sockaddr_un snm;
struct sockaddr_in sin;

   if (ssock) {

      if (domain == PF_INET) {
	 bzero((void *) &sin, SinSIZE);
	 sin.sin_family = domain;
	 sin.sin_port = htons(source_port);
	 cc = recvfrom(ssock,
		       (char *) spkt,
		       PACKET_STRING_SIZE,
		       0,
		       (struct sockaddr *) &sin,
		       &from);
      } else {
	 sprintf(fnam,"/tmp/SktSnd%d",(int) source_port);
	 snm.sun_family = PF_LOCAL;
	 strncpy (snm.sun_path, fnam, sizeof (snm.sun_path));
	 from = SUN_LEN(&snm);
	 cc = recvfrom(ssock,
		       (char *) spkt,
		       PACKET_STRING_SIZE,
		       0,
		       (struct sockaddr *) &snm,
		       &from);
      }
      if (cc < 0) {
	 fprintf(stderr,
		"RecvSocket:RecvFromPort:Error:Cant recvfrom port:%d\n",
		 source_port);
	 perror("RecvSocket:RecvFromPort:errno");
	 return FAIL;
      }
      return OK;
   }
   return FAIL;
}

/* ==================================================================== */
/* Main: Proccess arguments and recieve events and print them for ever. */
/* ==================================================================== */

int main(int argc,char *argv[]) {

unsigned short port;

char spkt[PACKET_STRING_SIZE];
int       i;
char      *cp,
	  *ep;

   /* Set up defaults */

   port = 1234;

   bzero((void *) spkt, PACKET_STRING_SIZE);

   /* Process command line arguments */

   for (i=1; i<argc; i++) {

      cp = NULL;

      if (strncmp(argv[i],options[PORT],strlen(options[PORT])) == 0) {
	 i++;
	 cp = argv[i];
	 if (cp) port = (unsigned short) strtoul(cp,&ep,0);
	 continue;
      }

      if (strncmp(argv[i],options[DOMAIN],strlen(options[DOMAIN])) == 0) {
	 domain = PF_INET;
	 continue;
      }

      printf("RecvSocket: No such option: %s\n",argv[i]);
      exit(1);
   }

   ErrSetHandler((ErrHandler) error_handler);

   if (OpenPort(port)) {

      fprintf(stderr,"RecvSocket: Up and Running OK\n");

      while (errors < MAX_ERRORS) {

	 bzero((void *) spkt, PACKET_STRING_SIZE);

	 if (RecvFromPort(port,spkt) == FAIL) {
	    sleep(1);
	    errors++;
	 } else
	    printf("%s\n",spkt);
      }
   } else errors++;

   fprintf(stderr,"RecvSocket: ABORTED, after ErrorCount:%d\n",errors);

   exit(1);
}
