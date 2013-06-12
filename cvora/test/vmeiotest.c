/**************************************************************************/
/* Vmeio test                                                             */
/* Julian Lewis 26 Oct 2010                                               */
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

/**************************************************************************/
/* Code section from here on                                              */
/**************************************************************************/

/* Print news on startup if not zero */

#define NEWS 1

#define HISTORIES 24
#define CMD_BUF_SIZE 128

static char history[HISTORIES][CMD_BUF_SIZE];
static char *cmdbuf = &(history[0][0]);
static int  cmdindx = 0;
static char prompt[32];
static char *pname = NULL;

#include "Cmds.h"
#include "GetAtoms.c"
#include "PrintAtoms.c"
#include "DoCmd.c"
#include "Cmds.c"
#include "VmeioCmds.c"

/**************************************************************************/
/* Prompt and do commands in a loop                                       */
/**************************************************************************/

int main(int argc,char *argv[]) {

char *cp, *ep;
char host[49];
char tmpb[CMD_BUF_SIZE];

   printf("vmeiotest: See <news> command\n");
   printf("vmeiotest: Type h for help\n");

   pname = argv[0];
   printf("%s: Compiled %s %s\n",pname,__DATE__,__TIME__);

   bzero((void *) host,49);
   gethostname(host,48);

   if (argc > 1)
      strcat(dev_name,argv[1]);
   else
      strcat(dev_name,"vmeio");

   if ((vmeio[lun] = cvora_open_name(lun,dev_name)) == NULL) {
      printf("Warning: Can't open support library\n");
      perror(DRIVER_NAME);
   }

   memlen = 1024;
   mem = malloc(memlen);
   get_window_parameters(lun);
   read_regs(dev_name);

   while (True) {

      cmdbuf = &(history[cmdindx][0]);
      if (strlen(cmdbuf)) printf("{%s} ",cmdbuf);
      fflush(stdout);

      if (vmeio) sprintf(prompt,"%s:%s[%02d]",host,dev_name,cmdindx+1);
      else     sprintf(prompt,"%s:NoDriver:Vmeio[%02d]",host,cmdindx+1);
      printf("%s",prompt);

      bzero((void *) tmpb,CMD_BUF_SIZE);
      if (fgets(tmpb,CMD_BUF_SIZE,stdin)==NULL) exit(1);

      cp = tmpb;
      if (*cp == '!') {
	 cmdindx = strtoul(++cp,&ep,0) -1;
	 cp = ep;
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
