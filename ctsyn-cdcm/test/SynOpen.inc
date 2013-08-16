/**************************************************************************/
/* Open the Syn driver                                                    */
/**************************************************************************/

#include <syndrvr.h>

int SynOpen() {

char fnm[32];
int  i, fn;

   for (i = 1; i <= SynDrvrCLIENT_CONTEXTS; i++) {
      sprintf(fnm,"/dev/syn.%1d",i);
      if ((fn = open(fnm,O_RDWR,0)) > 0) return(fn);
   };
   return(0);
}
