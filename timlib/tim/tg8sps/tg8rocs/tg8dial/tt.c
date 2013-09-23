#include <stdio.h>
#include <file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <smem.h>
#define VMEBASE 0xDE000000
#define tg8address 0xC00000L
#define dpramsize 0x7fc/2
char *test_init();
main(argc,argv)
int argc; char **argv;
{ unsigned short *dpramstart, *dpramend, *w, st;
  unsigned long address; char c;
/*                                                                          */
  if(argc<2) {address=tg8address; goto start;}
  if(sscanf(&argv[0][1],"%x",&address) !=1) address = tg8address;
start: dpramstart= (unsigned short *)test_init(address);
       dpramend = dpramstart + dpramsize;
       dpramstart += 2;
   printf("Read the whole DPRAM? [y/n]: ");
   c=getchar();
   if(c=='y' || c=='Y') goto lp;
   printf("\nWhich address to read (hex): ");
   scanf("%x",&st);
   w = dpramstart +st/2;
  lpa: while(1) st = *w;
  lp:  for(w=dpramstart; w!=dpramend; w++) st=*w;
       goto lp;
}
/*--------------------------------------------------------------------------*/
char *test_init(address)
unsigned long address;
{ register int i;
printf("smem_create(%s,0x%x,0x%x,%x)\n","TG8T",address|VMEBASE,0x800,
       SM_READ|SM_WRITE);
return(smem_create("TG8T",address|VMEBASE,0x800,SM_READ|SM_WRITE));
}
/*--------------------------------------------------------------------------*/

