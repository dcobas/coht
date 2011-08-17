/*
****************************************************************************
* tg8dpr_dump.c based on file /usr/src/ces/examples/vpeek.c written by CES
*               read a long word from given VME address (based on 'vlib') 
*                                 
* 970922: Adapted for TG8 test purpose by Bruno ( Thanks to abl )
* 
****************************************************************************
*/
#include	<stdio.h>
#include	<strings.h>

#define	AD	(0x0c00000) /* Tg8 Base Address */
#define	AM	(0x39)      /* Tg8 Address Mod. */
#define DPR_SIZE 0x800      /* Tg8 DPRam Byte Size */

main()
{
	long i,ad,am,ct,dt,mem;
	volatile short *q;
	volatile long *p;
	am = AM; ad=AD;
	
	for(i=0;i<=DPR_SIZE;i=i+2) {
		q = (short *)vme_map(ad+i,4,am);
		if (q == (short *)0){ fprintf(stderr,"unable to map VME address\n"); exit(0);}
		dt = *q;

		if ( ( i % 16 ) == 0 ) { printf("\nAD=0x%06X data:",ad+i); }
		printf(" %04x",dt);
		vme_rel(q,4);
	}
	printf("\n\n");
	printf("Date & Time at 0xC00178,A,C...\n");
}
		
