/* ============================================================================ */
/* LynxOs Memory allocation routines                                            */
/* ============================================================================ */

#ifdef __linux__
#ifndef SYSBRK
#define SYSBRK

#include <linux/vmalloc.h>

/* ============================================================================ */
/* Emulate LynxOs call sysbrk                                                   */

char *sysbrk(unsigned long size) {
char *cp;
int i;

   cp = vmalloc(size);
   if (cp) {
      for (i=0; i<size; i++) cp[i] = 0;
      return cp;
   }
   return NULL;
}

/* ============================================================================ */
/* Emulate LynxOs call sysfree                                                  */

void sysfree(char *cp, unsigned long size) {

   vfree(cp);
}

#endif
#endif
