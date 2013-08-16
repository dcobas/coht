/*******************************************************/
/* file: ports.c                                       */
/* asynract:  This file contains the routines to       */
/*            output values on the JTAG ports, to read */
/*            the TDO bit, and to read a byte of data  */
/*            from the prom                            */
/*                                                     */
/*******************************************************/
#include "ports.h"

#define SynDrvrJTAG_TDO     0x01   /* Data Out */
#define SynDrvrJTAG_TDO_BAR 0x10

#define SynDrvrJTAG_TDI     0x02   /* Data In */
#define SynDrvrJTAG_TDI_BAR 0x20

#define SynDrvrJTAG_TMS     0x04   /* Mode Select */
#define SynDrvrJTAG_TMS_BAR 0x40

#define SynDrvrJTAG_TCK     0x08   /* Clock */
#define SynDrvrJTAG_TCK_BAR 0x80

/*******************************************************/
/* Write one bit to selected Jtag bit                  */
/* p is the jtag port TMS, TDI or TCK                  */
/* val contains the bit value one or zero              */

static unsigned long jtag = 0x0F; /* Keep all jtag bits   */

void setPort(short p,short val) {
   if (val) {
      if      (p == TMS) { jtag |=  SynDrvrJTAG_TMS;
			   jtag &= ~SynDrvrJTAG_TMS_BAR; }
      else if (p == TDI) { jtag |=  SynDrvrJTAG_TDI;
			   jtag &= ~SynDrvrJTAG_TDI_BAR; }
      else if (p == TCK) { jtag |=  SynDrvrJTAG_TCK;
			   jtag &= ~SynDrvrJTAG_TCK_BAR; }
      else return;
   } else {
      if      (p == TMS) { jtag |=  SynDrvrJTAG_TMS_BAR;
			   jtag &= ~SynDrvrJTAG_TMS; }
      else if (p == TDI) { jtag |=  SynDrvrJTAG_TDI_BAR;
			   jtag &= ~SynDrvrJTAG_TDI; }
      else if (p == TCK) { jtag |=  SynDrvrJTAG_TCK_BAR;
			   jtag &= ~SynDrvrJTAG_TCK; }
      else return;
   }
   if (ioctl(syn,SynDrvrJTAG_WRITE_BYTE,&jtag) < 0)
      IErr("JTAG_WRITE_BYTE",NULL);
}

/*******************************************************/
/* read in a byte of data from the input stream        */

void readByte(unsigned char *data) {

    *data = (unsigned char) fgetc(inp);
}

/*******************************************************/
/* read the TDO bit from port                          */

unsigned char readTDOBit() {
char rback;

   if (ioctl(syn,SynDrvrJTAG_READ_BYTE,&rback) < 0)
      IErr("JTAG_READ_BYTE",NULL);
   if (rback & SynDrvrJTAG_TDO)
      return (unsigned char) 1;
   return (unsigned char) 0;
}

/*****************************************************************************/
/* Wait at least the specified number of microsec.                           */

void waitTime(long microsec) {
   usleep(microsec);
}

/*******************************************************/
/* Pulse the TCK clock                                 */

void pulseClock() {

    setPort(TCK,0);  /* set the TCK port to low  */
    setPort(TCK,1);  /* set the TCK port to high */
}
