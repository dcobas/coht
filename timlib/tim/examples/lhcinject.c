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

#include <tgm/tgm.h>
#include <TimLib.h>

int main(int argc,char *argv[]) {

   unsigned long grpNb;
   unsigned long lhcsqVal, miscaVal, nextUser, ssc;
   TimLibError tErr;
   TgmLineNameTable ltab;
   TgmMachine machine;
   TimLibTime cycleStamp;

   TimLibClass      iclss;    /* Class of interrupt */
   unsigned long    equip;    /* PTIM CTIM or hardware mask */
   unsigned long    plnum;    /* Ptim line number 1..n or 0 */
   TimLibTime       onzero;   /* Time of interrupt/output */
   unsigned long    payload;  /* Payload of trigger event */
   unsigned long    missed;   /* Number of missed interrupts */
   unsigned long    qsize;    /* Remaining interrupts on queue */
   TgmMachine       mch;      /* Corresponding TgmMachine */

   tErr = TimLibInitialize(TimLibDevice_ANY);
	
   tErr = TimLibQueue(0, 0); //no timeout, queuing on
	
   machine = TgmGetMachineId("SPS");
   TgmAttach(machine,TgmTELEGRAM);
		
#define START_SCY 308

   tErr = TimLibConnect(TimLibClassCTIM, START_SCY, 0);
		
#define BEAM_OUT 310

   tErr = TimLibConnect(TimLibClassCTIM, BEAM_OUT, 0);

   while (1) {
      tErr = TimLibWait(&iclss, &equip, &plnum, NULL, &onzero, NULL, NULL, NULL,
			&payload, NULL, &missed, &qsize, &mch);

      if (equip == BEAM_OUT) {

	 grpNb = (unsigned long) TgmGetGroupNumber(machine, "NUSER");
	 if (grpNb) {
	    if (TgmGetGroupValue(machine,TgmCURRENT,0,grpNb,(Cardinal *) &nextUser) == TgmSUCCESS) {
	       if (TgmGetLineNameTable(machine,"USER",&ltab) == TgmSUCCESS) {
		  if (strncmp(ltab.Table[nextUser -1].Name,"LHC",3) == 0) {

		     printf("Send here\n");
		  }
	       }
	    }
	 }
	 continue;
      }
      printf("Start cycle\n");
   }
   exit(0);
}
