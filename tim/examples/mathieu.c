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

#define ENERGY_EVENT 525

int main(int argc,char *argv[]) {

TimLibError err;
TimLibTime     onzero;
unsigned long energy;
int i;

   err = TimLibInitialize(TimLibDevice_ANY); /*initialize the hardware*/
   if (err) {
   	printf("Error: %s\n", TimLibErrorToString(err));
	exit(err);
   }

   err = TimLibConnect(TimLibClassCTIM,ENERGY_EVENT,0);
   if (err) printf("%s\n",TimLibErrorToString(err));

   for (i=1; i<10; i++) {

      err = TimLibWait(NULL,NULL,NULL,NULL,&onzero,
		       NULL,NULL,NULL,&energy,NULL,
		       NULL,NULL,NULL);
      if (err) printf("%s\n\n",TimLibErrorToString(err));
      printf("Energy:%d\n",(int) energy * 120 );
   }

   printf("Time UTC: %ds\n", (int) onzero.Second);
   exit(0);
}
