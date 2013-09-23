/***************************************************************************/
/* Tg8 installation program.                                               */
/* Vladimir Kovaltsov for ROCS Project, February, 1997			   */
/***************************************************************************/

#include <stdio.h>
#include <io.h>
#include <file.h>
#include <errno.h>
#include <string.h>
#include <tg8drvrP.h>
#include "tg8install.h"

extern int errno;

static Tg8DrvrInfoTable info_table;    /* Info table */

/***************************************************************************/
/* Main                                                                    */
/***************************************************************************/

int main(argc,argv)
int argc;
char **argv; {
FILE *fp;
char * progname   = "tg8install";
char * info_file  = "./tg8drvr.info";
char * def_file  = "./tg8drvr.def", str[128],*ep;
int  verbose    = 0;
int  fd;
int  i, j, k;

   /* If no args supplied give help text and exit. */

   if (argc == 1) {
      printf("Usage: tg8install options are:\n");
      printf("       -verbose       :: Detailed installation print out.\n");
      exit(0);
   };

   for (i=1; i<argc; i++) {
      if (strcmp(argv[i],"-verbose") == 0) verbose   = 1;
   };

   /***********************************/
   /* Build the info table in memory. */
   /***********************************/

   bzero((char*)&info_table,sizeof(Tg8DrvrInfoTable));   /* Clean table */

   /******************************************************/
   fp = fopen(def_file,"r");
   if (!fp) {
     printf("Cann't open file:%s\n",def_file);
     perror("Open");
     exit(-1);
   };
   if (str == fgets(str,sizeof(str),fp)) {
     info_table.Address  = strtoul(str,&ep,0);
     if (ep == str) {
       printf("Address value expected: %s\n",str);
       exit(-1);
     };
   } else {perror("fgets"); exit(-1);}
   if (str == fgets(str,sizeof(str),fp)) {
     info_table.Increment  = strtoul(str,&ep,0);
     if (ep == str) {
       printf("Increment value expected: %s\n",str);
       exit(-1);
     };
   } else {perror("fgets"); exit(-1);}
   if (str == fgets(str,sizeof(str),fp)) {
     info_table.Vector  = strtoul(str,&ep,0);
     if (ep == str) {
       printf("Vector value expected: %s\n",str);
       exit(-1);
     };
   } else {perror("fgets"); exit(-1);}
   if (str == fgets(str,sizeof(str),fp)) {
     info_table.Level  = strtoul(str,&ep,0);
     if (ep == str) {
       printf("Level value expected: %s\n",str);
       exit(-1);
     };
   } else {perror("fgets"); exit(-1);}
   if (str == fgets(str,sizeof(str),fp)) {
     info_table.SscHeader  = strtoul(str,&ep,0);
     if (ep == str) {
       printf("SscHeader value expected: %s\n",str);
       exit(-1);
     };
   } else {perror("fgets"); exit(-1);}
   fclose(fp);

   if (verbose) {
     printf("Driver definition data :\n");
     printf("      Address: 0x%x\n", info_table.Address);
     printf("      Increm.: 0x%x\n", info_table.Increment);
     printf("      Vector : 0x%x\n", info_table.Vector);
     printf("      Level  : %d\n",   info_table.Level);
     printf("      SSC hdr: 0x%X\n", info_table.SscHeader);
     printf("\n");
   };

   /* Open the info file and write the table to it. */

   if ((fd = open(info_file,(O_WRONLY | O_CREAT),0644)) < 0) {
      printf("Can't create Tg8 driver information file: %s\n",info_file);
      perror(progname);
      exit(1);
   };
   if (write(fd,&info_table,sizeof(Tg8DrvrInfoTable)) != sizeof(Tg8DrvrInfoTable)) {
      printf("Can't write to Tg8 driver information file: %s\n",info_file);
      perror(progname);
      exit(1);
   };
   if (verbose) printf("Info size:%d\n",sizeof(Tg8DrvrInfoTable));
   close(fd);

#if _MVME_167_
{int driver_id,major,minor;
 FILE * logf;
 char * object = "../driver/tg8drvr.o.167";
 char * extra  = "Tg8";
 char symb[32];
 
  if (verbose)
      printf("Begin install of driver object: %s\n",object);

   /* Install the driver object code in the system and call its install */
   /* procedure to initialize memory and hardware. */

   if ((driver_id = dr_install(object,CHARDRIVER)) < 0) {
      printf("Error calling dr_install with: %s\n", object);
      perror(progname);
      exit(1);
   };
   if (verbose)
      printf("%s: Driver identification: %d\n",progname,driver_id);

   if ((major = cdv_install(info_file, driver_id, extra)) < 0) {
      printf("Error calling cdv_install with: %s\n",info_file);
      perror(progname);
      exit(1);
   };
   if (verbose) printf("%s: Major device number: %d\n",progname,major);

   logf = fopen("/tmp/tg8install.log","w");

   /* Create the device nodes. */

   umask(0);
   for (minor=1; minor <= 4; minor++) {
      sprintf(symb,"/dev/Tg8.%1d",minor);
      if (verbose)
	 printf("%s: Creating minor device node: %s\n",progname,symb);
      unlink (symb);
      if (mknod(symb,(S_IFCHR | 0666),((major << 8) | minor)) < 0) {
	 printf("Error making node: %s\n",symb);
	 perror(progname);
	 exit(1);
      };
   };

   if (logf) {
     fwrite(&driver_id,sizeof(int),1,logf);
     fwrite(&major    ,sizeof(int),1,logf);
     fclose(logf);
   } else
     printf("Can not open the log file: %s\n",TG8_LOG_FILE);

}
#endif

   exit(0);
}

/* eof tg8instl.c */
