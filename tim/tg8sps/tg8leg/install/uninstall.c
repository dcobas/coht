/***************************************************************************/
/* Tg8 installation program.                                              */
/* Mon 30th May 1994                                                       */
/* Version 3                                                               */
/* Julian Lewis                                                            */
/* Exploit cpp instead of using yacc                                       */
/* VMTG: Kovaltsov V.                                                      */
/***************************************************************************/

#include <stdio.h>
#include <io.h>
#include <file.h>
#include <a.out.h>
#include <errno.h>

typedef unsigned char byte;

#include <tg8drvrP.h>
#include <tg8install.h>

main(int argc,char **argv) {
int i,minor, verbose=1, driver_id,major;
char symb[32],*progname;
FILE *logf=NULL;

   progname= argv[0];

   if (argc == 1) {
     printf("Usage: %s [id driver] [U]\n",progname);
     exit(0);
   };

   if (argc >= 3) {
     major    = atoi(argv[1]);
     driver_id= atoi(argv[2]);
   } else {

     logf = fopen(TG8_LOG_FILE,"r");
     if (!logf) {
       printf("%s: No Log file!\n",progname);
       exit(1);
     };

     fread(&driver_id,sizeof(int),1,logf);
     fread(&major    ,sizeof(int),1,logf);
     fclose(logf);
   };

   if (verbose)
     printf("%s: Driver id=%d, Major=%d, Nodes:%d\n",
	    progname,driver_id,major,Tg8DrvrDEVICES);

   umask(0);
   for (minor=1; minor<=Tg8DrvrDEVICES; minor++) {
      sprintf(symb,"/dev/Tg8.%1d",minor);
      if (verbose)
	 printf("%s: Deleting minor device node: %s\n",progname,symb);
      if (unlink (symb) < 0) {
	perror(" Unlink failed: ");
	if (argc == 4 && argv[3][0]=='U') break;
	printf("FATAL exit!\n");
	exit(1);
      };
   };

   if (verbose)
      printf("%s: Driver identification: %d\n",progname,driver_id);

   if (cdv_uninstall(major) < 0) {
      printf("Error calling cdv_uninstall\n");
      perror(progname);
      exit(1);
   };

   if (verbose ) printf("\nUninstalling driver No[%d]...", driver_id);

   if (dr_uninstall(driver_id) == -1) {
	perror("  Error from dr_uninstall");
	exit(errno);
   };

   if (logf) unlink(TG8_LOG_FILE);

   if (verbose ) printf(" Done\n");
}

/* eof */
