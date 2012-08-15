/**************************************************************************/
/* Open the Acdx driver                                                    */
/**************************************************************************/

#include <drm.h>
#include <acdxdrvr.h>

/* ======================== */

int AcdxOpen() {

char fnm[32];
int  i, fn;

   for (i = 1; i <= AcdxDrvrCLIENT_CONTEXTS; i++) {
      sprintf(fnm,"/dev/acdx.%1d",i);
      if ((fn = open(fnm,O_RDWR,0)) > 0) return(fn);
   };
   return(0);
}
