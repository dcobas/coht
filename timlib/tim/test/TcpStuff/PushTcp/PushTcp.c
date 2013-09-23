/* ============================================================= */
/* Push stdin out to a socket                                    */
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

#include <PushTcp.h>

extern pid_t getpgid(pid_t pid);

static int ssock = 0;

static int debug     = 0;
static int remote    = 0;

char *options[OPTIONS] = {"-p","-s","-d","-allow_remote"};

/* ============================================================= */
/* Open a UDP port for PushTcp data                              */
/* ============================================================= */

int OpenPort(unsigned short port) {

struct sockaddr_in sin;

int s, on, off, val;

   if (!ssock) {

      s = socket(PF_INET, SOCK_STREAM, 0);
      if (s < 0) {
	 fprintf(stderr,"PushTcp:OpenPort:Socket:Error:Cant open sockets\n");
	 perror("PushTcp:OpenPort:Socket:errno");
	 return FAIL;
      }
      ssock = s;
      on = 1;
      off = 0;
      val = 5;

      setsockopt (ssock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (int));

#ifdef __linux__
      setsockopt (ssock, SOL_TCP, TCP_DEFER_ACCEPT, &off, sizeof (int));
      setsockopt (ssock, SOL_TCP, TCP_QUICKACK, &on, sizeof (int));
      setsockopt (ssock, SOL_TCP, TCP_KEEPCNT, &on, sizeof (int));
      setsockopt (ssock, SOL_TCP, TCP_KEEPIDLE, &val, sizeof (int));
      setsockopt (ssock, SOL_TCP, TCP_KEEPINTVL, &val, sizeof (int));
      setsockopt (ssock, SOL_TCP, TCP_LINGER2, &val, sizeof (int));
      setsockopt (ssock, SOL_TCP, TCP_NODELAY, &on, sizeof (int));
      setsockopt (ssock, SOL_TCP, TCP_SYNCNT, &val, sizeof (int));
#endif

      bzero((void *) &sin, SinSIZE);
      sin.sin_family = PF_INET;
      sin.sin_port = htons(port);
      sin.sin_addr.s_addr = htonl(INADDR_ANY);
      if (bind(s,(struct sockaddr *) &sin, SinSIZE) < 0) {
	 fprintf(stderr,"PushTcp:OpenPort:Bind:Error:Cant bind port:%d\n",port);
	 perror("PushTcp:OpenPort:Bind:errno");
	 close(s);
	 ssock = 0;
	 return FAIL;
      }
      if (listen(s,BACK_LOG) < 0) {
	 fprintf(stderr,"PushTcp:Listen:Error:Port:%d\n",port);
	 perror("PushTcp:OpenPort:Listen:errno");
	 close(s);
	 ssock = 0;
	 return FAIL;
      }
   }
   return ssock;
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
   printf("PushTcp:Caught signal: %d: Shutting down\n",signo);
   sighuphand(SIGHUP);
}

/* ============================================================= */
/* Main: Proccess arguments and send events for ever             */
/* ============================================================= */

int main(int argc,char *argv[]) {

unsigned short port;

struct sockaddr_in client;
struct sigaction action, old;
pid_t child;

int   on, i, clsock;
unsigned int len;
char  *cp, *ep, buf[128];
pid_t pid = 0;

   if (argc <= 1) {
      printf("PushTcp: Parameters are ...\n\n"
	     "   -p  <PORT>        ;Sets the target host port number\n"
	     "   -d                ;Set Debug printing on\n"
	     "   -allow_remote     ;Allow remote computers to connect to socket\n"
	    );
      exit(1);
   }

   /* Set up defaults */

   port = DEFAULT_PORT;

   /* Process command line arguments */

   for (i=1; i<argc; i++) {

      cp = NULL;

      if (strncmp(argv[i],options[PORT],strlen(options[PORT])) == 0) {
	 i++;
	 cp = argv[i];
	 if (cp) port = (unsigned short) strtoul(cp,&ep,0);
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

      printf("PushTcp: No such option: %s\n",argv[i]);
      exit(1);
   }

   fprintf(stderr,"PushTcp: Sartup: Port:%d ", port);

   setpgid (pid,getpgid(0));

   bzero((void *) &action, sizeof (action));
   action.sa_handler = (void (*)(int)) sigchildhand;
   sigaction(SIGCHLD, &action, &old);

   bzero((void *) &action, sizeof (action));
   action.sa_handler = (void (*)(int)) sighuphand;
   sigaction(SIGHUP, &action, &old);

   if (OpenPort(port)) {

      fprintf(stderr,"PushTcp: Up and Running OK\n");

      while(1) {

	 while(1) {
	    len = sizeof(client);
	    clsock = accept(ssock,(struct sockaddr *) &client,&len);
	    if (clsock == -1) {
	       if (errno == EINTR) continue;

	       fprintf(stderr,"PushTcp:Main:Accept:%d\n",errno);
	       perror("PushTcp:Main:Accept:");
	       exit(kill(0,SIGHUP));
	    }
	    if (remote == 0) {
	       if (strcmp(LOOPBACK,inet_ntoa(client.sin_addr)) == 0) break;
	       printf("PushTcp:Main:RejectConnection:%s\n",inet_ntoa(client.sin_addr));
	    } else break;
	    close(clsock);
	 }

	 on = 1;
	 setsockopt (clsock, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof (int));

	 child = fork();
	 if (child == 0) {
	    fprintf(stderr,"PushTcp:Serve client:%s\n"
			  ,inet_ntoa(client.sin_addr));

	    bzero((void *) &action, sizeof(action));
	    action.sa_handler = sigpipehand;
	    sigaction(SIGPIPE, &action, &old);

	    while (pok) {
	       cp = buf;
	       if (fgets(cp,128,stdin)) {
		  len = write(clsock,cp,strlen(cp));
		  if (len <=0 ) pok = 0;
	       }
	    }

	    fprintf(stderr,"PushTcp:Terminate client:%s\n"
			  ,inet_ntoa(client.sin_addr));
	    close(clsock);
	    exit(0);
	 }

#ifdef __linux__
	 setpgid (pid,getpgid(0));
#else
	 setpgid (pid,getpgrp());
#endif

	 bzero((void *) &action, sizeof (action));
	 action.sa_handler = (void (*)(int)) sigother;

	 sigaction(SIGINT, &action, &old);
	 sigaction(SIGQUIT, &action, &old);

	 sigaction(SIGSTOP, &action, &old);
	 sigaction(SIGCONT, &action, &old);

	 waitpid (-1, NULL, WNOHANG);
      }
   }
   fprintf(stderr,"PushTcp:Main:Exit\n");
   exit(errno);
}
