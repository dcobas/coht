/* ============================================================= */
/* Pull data from PushTcp server                                 */
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
#include <signal.h>

#include <PushTcp.h>

static int ssock = 0;
static int pok   = 1;

char server[64];
char *options[OPTIONS] = {"-p","-s","-d","-allow_remote"};

/* ============================================================= */
/* Open a UDP port for PullTcp data                              */
/* ============================================================= */

int OpenPort(unsigned short port) {

struct sockaddr_in sin;

int s;

   if (!ssock) {

      s = socket(PF_INET, SOCK_STREAM, 0);
      if (s < 0) {
	 fprintf(stderr,"PullTcp:OpenPort:Socket:Error:Cant open sockets\n");
	 perror("PullTcp:OpenPort:Socket:errno");
	 return FAIL;
      }
      ssock = s;

      bzero((void *) &sin, SinSIZE);
      sin.sin_family = PF_INET;
      sin.sin_port = htons(port);
      sin.sin_addr.s_addr = inet_addr(server);
      if (connect(s,(struct sockaddr *) &sin, SinSIZE) < 0) {
	 fprintf(stderr,"PullTcp:OpenPort:Connect:Error:%d\n",port);
	 perror("PullTcp:OpenPort:Connect:errno");
	 close(ssock);
	 ssock = 0;
	 return FAIL;
      }
   }
   return ssock;
}

/* ============================================================= */
/* Catch broken pipe signals when client dies                    */
/* ============================================================= */

void sigpipehand(int signo) {
   pok = 0;
   fprintf(stderr,"PullTcp: SigPipe: Connection shutdown: Exit\n");
   exit(0);
}

/* ==================================================================== */
/* Main: Proccess arguments and recieve events and print them for ever. */
/* ==================================================================== */

int main(int argc,char *argv[]) {

unsigned short port;

struct sigaction action, old;
char spkt[128];
int       i;
char      *cp,
	  *ep;

   /* Set up defaults */

   port = DEFAULT_PORT;
   strcpy(server,"127.0.0.1");
   bzero((void *) spkt, 128);

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

      printf("PullTcp: No such option: %s\n",argv[i]);
      exit(1);
   }

   if (OpenPort(port)) {

      fprintf(stderr,"PullTcp: Up and Running OK\n");

      bzero((void *) &action, sizeof(action));
      action.sa_handler = sigpipehand;
      sigaction(SIGPIPE, &action, &old);

      while (pok) {

	 bzero((void *) spkt, 128);

	 if (read(ssock,&spkt,128) <= 0) pok = 0;
	 else                            printf("%s\n",spkt);
      }
   }

   if (ssock) close(ssock);

   fprintf(stderr,"PullTcp: ABORTED\n");

   exit(1);
}
