/**************************************************************************/
/* TimLib test program                                                    */
/* Julian Lewis 25th April 2005                                           */
/**************************************************************************/

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
#include <time.h>

#include <err/err.h>      /* Error handling */

#include <libctr.h>

/**************************************************************************/
/* Code section from here on                                              */
/**************************************************************************/

#define NEWS 1

#define HISTORIES 24
#define CMD_BUF_SIZE 128
#define PROMPT_LEN 64
#define HOST_LEN 16

static char history[HISTORIES][CMD_BUF_SIZE];
static char *cmdbuf = &(history[0][0]);
static unsigned int cmdindx = 0;

void *h;

#include <libctr.h>

#include "Cmds.h"
#include "GetAtoms.c"
#include "PrintAtoms.c"
#include "DoCmd.c"
#include "Cmds.c"
#include "CtrLibCmds.c"

/**************************************************************************/
/* Prompt and do commands in a loop                                       */
/**************************************************************************/

int main(int argc,char *argv[]) {

char host[HOST_LEN];
char tmpb[CMD_BUF_SIZE];
char prmt[PROMPT_LEN];
char *dname = NULL;

char *cp;

int cnt;
CtrDrvrHardwareType typ;

   printf("libctrtest: compiled %s %s libctr.a version:%s\n",__DATE__,__TIME__,ctr_get_ldver());

   h = ctr_open(NULL);
   if ((long) h == -1) {
      perror("ctr_open");
      exit(1);
   }

   if (ctr_get_type(h, &typ) < 0) {
      perror("ctr_get_type");
      exit(1);
   }
   if      (typ == CtrDrvrHardwareTypeCTRP) dname = "ctrp";
   else if (typ == CtrDrvrHardwareTypeCTRI) dname = "ctri";
   else if (typ == CtrDrvrHardwareTypeCTRV) dname = "ctrv";
   else {
      fprintf(stderr,"libctr: Unknown CTR device type:%d\n",(int) typ);
      exit(1);
   }

   cnt = ctr_get_module_count(h);
   if (cnt < 0) {
      perror("ctr_get_module_count");
      exit(1);
   }

   gethostname(host,HOST_LEN);

   while (1) {

      cmdbuf = &(history[cmdindx][0]);
      if (strlen(cmdbuf)) printf("{%s} ",cmdbuf);
      fflush(stdout);

      sprintf(prmt,"libctr:%s:%s[%02d]",host,dname,cmdindx+1);
      printf("%s",prmt);

      bzero((void *) tmpb,CMD_BUF_SIZE);
      if (fgets(tmpb, CMD_BUF_SIZE, stdin)==NULL) exit(1);

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
