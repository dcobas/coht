#include <vd80hard.h>

typedef struct {   /* mcon->Registers */

   uint32_t GCR1;
   uint32_t GCR2;
   uint32_t GSR;


 } GeneralRegs;


typedef struct {   /* mcon->UserData */

   char revis_id[VD80_CR_REV_ID_LEN +1]; /* The revis_id and terminator byte */

 } UserData;

#define VD80_KLUDGE_DELAY 100000	/* fixing the delay in vd80 module */
