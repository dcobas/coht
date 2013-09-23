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

#include <TimLib.h>

int main(int argc,char *argv[]) {

TimLibError err;
TimLibTime     onzero;

   err = TimLibInitialize(TimLibDevice_CTR); /*initialize the hardware*/
   if (err) {
   	printf("Error: %s\n", TimLibErrorToString(err));
	exit(err);
   }

   err = TimLibConnect(TimLibClassHARDWARE,TimLibHardwareCOUNTER_1,0);
   if (err) printf("%s\n",TimLibErrorToString(err));

   err = TimLibWait(NULL,NULL,NULL,NULL,&onzero,
		    NULL,NULL,NULL,NULL,NULL,
		    NULL,NULL,NULL);
   if (err) printf("%s\n\n",TimLibErrorToString(err));

   printf("Time UTC: %ds\n", (int) onzero.Second);
   exit(0);
}
