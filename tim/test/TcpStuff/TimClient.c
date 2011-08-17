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
#include <TimServer.h>

static int ssock     = 0;
static int errors    = 0;

char server[IP_NAME_SIZE];
char *options[OPTIONS] = {"-p","-s","-eC","-eP","-d"};

/* ============================================================= */
/* Errlog handler                                                */
/* ============================================================= */

static Boolean error_handler(class,text)
ErrClass class;
char *text; {

   errors++;
   fprintf(stderr,"TimClient:Errors:%d %s\n",errors,text);
   return(False);
}

/* ============================================================= */
/* Open a UDP port for TimClient data                           */
/* ============================================================= */

int OpenPort(unsigned short port) {

struct sockaddr_in sin;

int s;

   if (!ssock) {

      s = socket(PF_INET, SOCK_STREAM, 0);
      if (s < 0) {
	 fprintf(stderr,"TimClient:OpenPort:Socket:Error:Cant open sockets\n");
	 perror("TimClient:OpenPort:Socket:errno");
	 return FAIL;
      }
      ssock = s;

      bzero((void *) &sin, SinSIZE);
      sin.sin_family = PF_INET;
      sin.sin_port = htons(port);
      sin.sin_addr.s_addr = inet_addr(server);
      if (connect(s,(struct sockaddr *) &sin, SinSIZE) < 0) {
	 fprintf(stderr,"TimClient:OpenPort:Connect:Error:%d\n",port);
	 perror("TimClient:OpenPort:Connect:errno");
	 close(ssock);
	 ssock = 0;
	 return FAIL;
      }
   }
   return ssock;
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
   strcpy(server,"127.0.0.1");
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

      if (strncmp(argv[i],options[SERVER],strlen(options[SERVER])) == 0) {
	 i++;
	 cp = argv[i];
	 if (cp) strcpy(server,cp);
	 continue;
      }

      printf("TimClient: No such option: %s\n",argv[i]);
      exit(1);
   }

   ErrSetHandler((ErrHandler) error_handler);

   if (OpenPort(port)) {

      fprintf(stderr,"TimClient: Up and Running OK\n");

      while (errors < MAX_ERRORS) {

	 bzero((void *) spkt, PACKET_STRING_SIZE);

	 if (read(ssock,&spkt,PACKET_STRING_SIZE) <= 0) {
	    sleep(1);
	    errors++;
	 } else
	    printf("%s\n",spkt);
      }
   } else errors++;

   if (ssock) close(ssock);

   fprintf(stderr,"TimClient: ABORTED, after ErrorCount:%d\n",errors);

   exit(1);
}
