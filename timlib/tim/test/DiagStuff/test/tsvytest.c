/* ====================================================================== */
/* Timing Survey Test Program                                             */
/* Julian Lewis Thu 7th June 2007                                         */
/* ====================================================================== */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <errno.h>        /* Error numbers */
#include <sys/file.h>
#include <a.out.h>
#include <ctype.h>

#include <err/err.h>      /* Error handling */

#define HISTORIES 24
#define CMD_BUF_SZ 128
#define HOST_NAME_SZ 48

#define SEND_PORT 2001
#define RECV_PORT 2002

static char history[HISTORIES][CMD_BUF_SZ];
static char *cmdbuf = &(history[0][0]);
static unsigned int  cmdindx = 0;
static char prompt[32];
static char *pname = NULL;

#include "Cmds.h"
#include "GetAtoms.c"
#include "PrintAtoms.c"
#include "DoCmd.c"
#include "Cmds.c"
#include "TsvyCmds.c"

/* ====================================================================== */
/* Prompt and do commands in a loop                                       */
/* ====================================================================== */

int main(int argc,char *argv[]) {

char *cp, *ep;
char host[HOST_NAME_SZ];
char tmpb[CMD_BUF_SZ];

   cp = NULL; ep = NULL;

   printf("tsvytest: See <news> command\n");
   printf("tsvytest: Type h for help\n");

   pname = argv[0];
   printf("%s: Compiled %s %s\n",pname,__DATE__,__TIME__);

   bzero((void *) host,HOST_NAME_SZ);
   gethostname(host,HOST_NAME_SZ);

   bzero((void *) dscname, DSC_NAME_SZ);
   bzero((void *) dscipad, DSC_NAME_SZ);

   if (OpenSendPort(SEND_PORT) == 0) exit(1);
   if (OpenRecvPort(RECV_PORT) == 0) exit(1);

   while (True) {

      cmdbuf = &(history[cmdindx][0]);
      if (strlen(cmdbuf)) printf("{%s} ",cmdbuf);
      fflush(stdout);

      sprintf(prompt,"%s:%s:Tsvy[%02d]",host,dscname,cmdindx+1);
      printf("%s",prompt);

      bzero((void *) tmpb,CMD_BUF_SZ);
      if (gets(tmpb)==NULL) exit(1);

      cp = &(tmpb[0]);

      if (*cp == '!') {
	 ++cp;
	 cmdindx = strtoul(cp,&cp,0) -1;
	 if (cmdindx >= HISTORIES) cmdindx = 0;
	 cmdbuf = &(history[cmdindx][0]);
	 continue;
      } else if (*cp == '.') {
	 printf("Execute:%s\n",cmdbuf); fflush(stdout);
      } else if ((*cp == '\n') || (*cp == '\0')) {
	 cmdindx++;
	 if (cmdindx >= HISTORIES) { printf("\n"); cmdindx = 0; }
	 cmdbuf = &(history[cmdindx][0]);
	 continue;
      } else if (*cp == '?') {
	 printf("History:\n");
	 printf("\t!<1..24> Goto command\n");
	 printf("\tCR       Goto next command\n");
	 printf("\t.        Execute current command\n");
	 printf("\this      Show command history\n");
	 continue;
      } else {
	 cmdindx++; if (cmdindx >= HISTORIES) { printf("\n"); cmdindx = 0; }
	 strcpy(cmdbuf,tmpb);
      }
      bzero((void *) val_bufs,sizeof(val_bufs));
      GetAtoms(cmdbuf);
      DoCmd(0);
   }
}
