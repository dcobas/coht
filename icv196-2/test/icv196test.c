#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sched.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <dlfcn.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <sys/mman.h>

#include <icv196.h>
#include <libicv196.h>

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
#include "icv196Cmds.c"

/**************************************************************************/
/* Prompt and do commands in a loop                                       */
/**************************************************************************/

int main(int argc,char *argv[])
{
	int rp, pr, dev;
	char *cp;
	char host[49];
	char tmpb[CMD_BUF_SIZE];

	pname = argv[0];
	printf("%s: Compiled %s %s\n",pname,__DATE__,__TIME__);

	if (argc > 1)
		dev = strtoul(argv[1],&cp,0);

	do_open(dev);
	show_dev(1);

	bzero((void *) host,49);
	gethostname(host,48);

	while (True) {

		sprintf(prompt,"%s:Icv196.%d[%02d]",host,get_lun(),cmdindx+1);
		printf("%s",prompt);

		cmdbuf = &(history[cmdindx][0]);

		bzero((void *) tmpb,CMD_BUF_SIZE);
		if (fgets(tmpb,CMD_BUF_SIZE,stdin) == NULL) break;

		cp = &(tmpb[0]);
		pr = 0;           /* Dont print a history */
		rp = 0;           /* Dont repeat a command */

		while ((*cp == '-')
		||     (*cp == '+')
		||     (*cp == '.')
		||     (*cp == '!')) {

			pr = 1;        /* Print command on */

			if (*cp == '!') {
				cp++;
				cmdindx = strtoul(cp,&cp,0) -1;
				if (cmdindx >= HISTORIES) cmdindx = 0;
				if (cmdindx < 0) cmdindx = HISTORIES -1;
				rp = 1;
				break;
			}

			if (*cp == '-') {
				if (--cmdindx < 0) cmdindx = HISTORIES -1;
				cmdbuf = &(history[cmdindx][0]);
			}

			if (*cp == '+') {
				if (++cmdindx >= HISTORIES) cmdindx = 0;
				cmdbuf = &(history[cmdindx][0]);
			}

			if (*cp == '.') {
				rp = 1;
				break;
			}

			cp++;
		}
		if (pr) {
			printf("{%s}\t ",cmdbuf); fflush(stdout);
			if (!rp) continue;
		}
		if (!rp) strcpy(cmdbuf,tmpb);

		bzero((void *) val_bufs,sizeof(val_bufs));
		GetAtoms(cmdbuf);
		DoCmd(0);

		if ((!rp) && (++cmdindx >= HISTORIES)) cmdindx = 0;
	}
	exit(0);
}
