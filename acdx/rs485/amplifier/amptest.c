/**************************************************************************/
/* Amp  test                                                              */
/* Julian Lewis 15th Nov 2002                                             */
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
#include <sys/sem.h>

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
static int  amp;

#include "Cmds.h"
#include "GetAtoms.c"
#include "PrintAtoms.c"
#include "DoCmd.c"
#include "Cmds.c"
#include "AmpOpen.c"
#include "AmpCmds.c"

// =====================================================

int AmpUnBlock();
int AmpBlock();

#define KEY 41793632
union sem_un { int               val;
	       struct semid_ds  *buff;
	       unsigned short   *array;
	       struct seminfo   *__buf;      /* buffer for IPC_INFO */
	       void             *__pad;
 };

int AmpIsBlocked() {

int key = KEY;
int sd;
int cnt;
union  sem_un arg;

   sd  = semget(key,1,0);
   if (sd == -1) return AmpUnBlock(); // Go create it

   bzero((char *) &arg, sizeof(arg));
   cnt = semctl(sd,0,GETVAL,arg);

   if (cnt == 0) return 1;    // Blocked
   else          return 0;    // Not blocked
}

// =====================================================

int AmpBlock() {

int key = KEY;
int sd;
union  sem_un arg;
struct sembuf sop;

  sd  = semget(key,1,0666|IPC_CREAT);
  if (sd == -1) {
  fail:
    return -1; /* can not start */
  }

  /* Unblock semaphore */

  bzero((char *) &arg, sizeof(arg));
  arg.val = 1;
  if (semctl(sd,0,SETVAL,arg) == -1) goto fail;

  /* Block it with UNDO on kill */

  bzero((char *) &sop, sizeof(sop));
  sop.sem_num = 0;
  sop.sem_op  = -1;
  sop.sem_flg = SEM_UNDO;
  if (semop(sd,&sop,1) == -1) goto fail;
  return 0;
}

// =====================================================

int AmpUnBlock() {

int key = KEY;
int sd;
union  sem_un arg;

  sd  = semget(key,1,0666|IPC_CREAT);
  if (sd == -1) {
  fail:
    return -1; /* can not start */
  }

  /* Unblock semaphore */

  bzero((char *) &arg, sizeof(arg));
  arg.val = 1;
  if (semctl(sd,0,SETVAL,arg) == -1) goto fail;

  return 0;
}

/**************************************************************************/
/* Prompt and do commands in a loop                                       */
/**************************************************************************/

int main(int argc,char *argv[]) {

char *cp, *ep;
char host[49];
char tmpb[CMD_BUF_SIZE];
int tmo;

#if NEWS
   printf("amptest: See <news> command\n");
   printf("amptest: Type h for help\n");
#endif

   pname = argv[0];
   printf("%s: Compiled %s %s\n",pname,__DATE__,__TIME__);

   tmo = 0;
   while (AmpIsBlocked()) {
      usleep(100000);
      if (tmo++ > 50) {
	 printf("Amplifier is blocked by another program\n");
	 if (YesNo("Force","Reservation")) AmpUnBlock();
	 else exit(1);
      }
   }
   AmpBlock();
   printf("Amplifier reservation taken, all other programs blocked\n");


   amp = AmpOpen();
   if (amp == 0) {
      printf("\nWARNING: Could not open driver");
      printf("\n\n");
   } else {
      printf("Driver opened OK\n\n");
   }

   bzero((void *) host,49);
   gethostname(host,48);

   while (True) {

      if (amp) sprintf(prompt,"%s:Amp[%02d]",host,cmdindx+1);
      else     sprintf(prompt,"%s:NoDriver:Amp[%02d]",host,cmdindx+1);

      cmdbuf = &(history[cmdindx][0]);
      if (strlen(cmdbuf)) printf("{%s} ",cmdbuf);
      printf("%s",prompt);

      bzero((void *) tmpb,CMD_BUF_SIZE);
      if (gets(tmpb)==NULL) exit(1);

      cp = &(tmpb[0]);
      if (*cp == '!') {
	 cmdindx = strtoul(++cp,&ep,0) -1; cp = ep;
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
