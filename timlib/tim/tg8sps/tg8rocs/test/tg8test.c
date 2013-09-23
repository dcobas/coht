/***************************************************************************/
/* Tg8 test program. Total re-write                                        */
/* J.Lewis June 1994                                                       */
/* Complete re-write of older version.                                     */
/* Vladimir Kovaltsov for the SL Version, February, 1997		   */
/***************************************************************************/

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

#include <tg8drvr.h>
#include <tg8.h>

#include "tg8test.h"
#include "Tg8ReadFirm.c"

#define UBCD(bcd) ((bcd&0xF)+10*(bcd>>4))
#define SET_RANGE(from,to)\
   iob.Range.Row = (param_count>0? param[0]: (from));\
   iob.Range.Cnt = (param_count>1? param[1]: (to)-iob.Range.Row+1)
#define SET_RANGE1(from)\
   iob.Range.Row = (param_count>0? param[0]: (from));\
   iob.Range.Cnt = (param_count>1? param[1]: 1)

extern int errno;

#define DEFAULT_PRIORITY 10

static char *bltxt = "NOBELL";

static char *mnames[] = {"","LEP", "SPS", "CPS", "PSB", "LPI", "LHC"};
static char *clk[]    = {"1KHz","Cable","E1/IN","Ext2"};
static char *start[]  = {"Norml","Chain","ExtSt","?????"};
static char *outty[]  = {"UNDEF","OutPut","BusInt","OutInt"};
static char *statety[]= {"ENABLED","DISABLED"};
static char *groupt[] = {"","NUM","EXC","BIT"};
static int   rpls[]   = {0x14fe0000,0x24fe0000,0x34fe0000,0x44fe0000,0x54fe0000};

static int status,dev[Tg8DrvrMODULES];

/* Data returned by the ioct operations */
static Tg8IoBlock iob;

static int param_count, params[PARAMS], *param, hlpN=0;
static char cmd[512];

static int modnum = 0; /* Current module */
static int apppri = DEFAULT_PRIORITY;

/***************************************************************************/
/* Signal handler to do gracefull termination.                             */
/***************************************************************************/

static int not_macro = 1;
static int tg8 = 0;
static int semid = 0;

void Terminate(ar)
int ar; {

   if (tg8>0) close(tg8);
   if (not_macro) printf("End (%1d) tg8test.\n",ar);
   else           printf("End of MACRO\n");
   exit(ar);
};

/***************************************************************************/
/* Handle timeouts                                                         */
/***************************************************************************/

static int timeout = 0;
static void alarm_handler() {
   timeout = 1;
   printf(" ** TimeOut **\n");
}

int OpenModule(int modn) {
int i,f,dv;
  f = dev[modn-1];
  if (f>0) return f;
  dv = (modn-1)*Tg8DrvrDEVICES+1;
  for (i=0; i<Tg8DrvrDEVICES;i++,dv++) {
    sprintf(cmd,"/dev/Tg8.%d",dv);
    f = open(cmd,O_RDWR,O_NONBLOCK);
    if (f>0) {
      dev[modn-1] = f;
      return f;
    };
  };
  printf("tg8test: Can not open module: %d\n",modn);
  perror("Open:");
  return f;
}

void sigHandl(int sig) {
Tg8DrvrEvent * conn; int ln;

  if (tg8>0) { /* Read event */
    ln = read(tg8,&iob.Event,sizeof(Tg8DrvrEvent));
    if (ln<0) {
      perror("ReadEvent:");
      return;
    };
    if (!ln) printf("sigHandl: Time Out!\n");
    else {
      conn = &iob.Event;
      printf("sigHandl(%d): Declared timing Id:%d Event: %08X\n",sig,conn->Id,conn->Event.Long);
      printf("Interrupt information:\n ");
      DisplayInterrupt(&conn->Inter);
      DisplayAlarms(conn->Alarms);         /* Show the Fw & Driver alarms */
      DisplayFirmwareStatus(conn->FwStat); /* Show the firmware status */
    };
  };
}

/***************************************************************************/
/* Main                                                                    */
/***************************************************************************/

main(argc,argv)
int argc; char **argv; {

FILE                     *fp, *lfp;
char                     str[512];
char                     s[512], strsv[512], mach_name[6], *cp, *ep;
int                      i, j, k, trig_time, ch, ln, an, pn, la, a, m;
int                      fc, lc;
int                     *memp;
int			 blp, trp, cc;
Word                     cw;
Command                  command;
int		        *cnid;
Tg8User                   usr;
Tg8Machine                mach;
Tg8PpmLine               *ppml;
Tg8Gate                  *g;
Tg8Object                *obj;
Tg8PpmUser               *ppm;
Tg8User                  *act;
Tg8DrvrEvent             *conn;
Tg8FirmwareObject        *fwob;

   for (i=1; i<argc; i++) {
     if (!strcmp(argv[i],"macro")) not_macro = 0;
     else
     if (!strcmp(argv[i],"module")) modnum= atoi(argv[++i]);
   };

   bzero((char*)dev,sizeof(dev));

   /* Open the Tg8 minor device */
   if (modnum)
     tg8 = OpenModule(modnum);
   else
     for (modnum=1; modnum<=Tg8DrvrMODULES;modnum++) {
       tg8 = OpenModule(modnum);
       if (tg8>0) break;
     };
   if (tg8 <= 0) {
     printf("WARNING: Can't open Tg8 device driver\n");
     Terminate(-1);
   };
  
   if (not_macro) {
      printf("Tg8 test. Version: %s\n\n",__DATE__);
      printf("Tg8 \" \" <command> Passes command on to LynxOS\n");
      printf("Tg8 \".\"           Repeat the last command\n");
      printf("Tg8 \"h\"           HELP lists ALL commands and parameters\n\n");
   };

   hlptxts[0] = hlptxt1;
   hlptxts[1] = hlptxt2;
   hlptxts[2] = hlptxt3;
   hlptxts[3] = hlptxt4;
   hlptxts[4] = hlptxt5;
   hlptxts[5] = hlptxt6;

   for (;;) {
      if (not_macro) printf("tg8[%d]: ",modnum);
      gets(s);
      /** if (*s == '\0') continue; **/
      if (!*s || *s == '.') {
	 strcpy(s,strsv);
	 printf("Repeat command: \"%s\"\n",s);
      } else
	 strcpy(strsv,s);

      /* Get the command number from the input string */

      for (command = FIRST_COMMAND; command < LAST_COMMAND; command++)
	 if (strncmp(s, commands[command], cc=strlen(commands[command]))
	 == 0) break;

      /* Get any parameters supplied */

      param_count = 0;
      bzero((char*)params,sizeof(params));
      cp = &s[cc];
      while (param_count < PARAMS) {
	 params[param_count] = strtoul(cp,&ep,0);
	 if (cp == ep) break;
	 param_count++;
	 cp = ep;
      };
      param = params;

      /* Go do the command */

      switch (command) {
	 case FIRST_COMMAND:
	 case H:	/* Help */
	    printf("%s",hlptxts[hlpN]); hlpN = (hlpN+1)%HELPS;
	 break;

	 case COM:      /* Comment => Do nothing */
	 break;

	 case SPACE:	/* Execute a command */
	    printf("Execute: %s\n",&s[1]);
	    system(&s[1]);
	 break;

	 case Q:	/* Quit the program */
	    Terminate(0);
	 break;

	 /* >>>>>>>>>>>>>> Module related functions <<<<<<<<<<<<<< */

	 case DON:
	    ioctl(tg8,Tg8DrvrDEBUG_ON,NULL);
	 break;

	 case DOF:
	    ioctl(tg8,Tg8DrvrDEBUG_OFF,NULL);
	 break;

	 case TG:	/* Select a module */
	    if (param_count > 0) {
	      if (*param<=0 || *param>Tg8DrvrMODULES) {
		printf("Bad module number\n");
		break;
	      };
	      /* Install the new module */
	      if (param_count >= 3) { /* tg8 <mod> <addr> <vect> <lev> <switch> <ssc> */
		iob.InsModule.Index = param[0]-1;
		iob.InsModule.Address.VMEAddress = (void*) param[1];
		iob.InsModule.Address.InterruptVector = param[2];
		iob.InsModule.Address.InterruptLevel = param[3];
		iob.InsModule.Address.SwitchSettings = param[4];
		iob.InsModule.Address.SscHeader = param[5];
		if (ioctl(tg8,Tg8DrvrINSTALL_MODULE,&iob)<0) {
		  printf("Cann't install the module: %d\n",param[0]);
		  perror("InstallModule:");
		};
		break;
	      };

	      if (modnum != *param) {
		i = OpenModule(*param);
		if (i>0) {
		  tg8 = i;
		  modnum = *param;
		  printf("Module: %u SELECTED\n",modnum);
		} else break; /* No such module */
	      };
	    } else
	      printf("Current Module: %u\n",modnum);
	    if (ioctl(tg8,Tg8DrvrGET_RAW_STATUS,&iob)<0) {
	       printf("Can not get the module status\n");
	       perror("RawStatus:");
	    };
	    DisplayMmResult(RHST);
	 break;

	 case SSC:	/* Set the SSC header code*/
	    if (param_count > 0) {
	      iob.SscHeader = *param;
	      if (ioctl(tg8,Tg8DrvrSET_SSC_HEADER,&iob) < 0) {
		printf("Can not set the SSC Header\n");
		perror("SetSSC:");
	      };
	    };
	 break;

         case ONC: /* Setup the close <mode> <mask> */
	   iob.CloseMode.Mode = param[0];
	   iob.CloseMode.SimPulseMask = (param_count>1? param[1]: 0);
	   if (ioctl(tg8,Tg8DrvrON_CLOSE,&iob) < 0) {
	     printf("Can not setup the device close mode\n");
	     perror("OnClose:");
	   };
	 break;

         case SIM: /* Simulate pulses */
	   iob.SimPulseMask = param[0];
	   if (ioctl(tg8,Tg8DrvrSIMULATE_PULSE,&iob) < 0) {
	     printf("Can not simulate pulses. Module isn't disabled\n");
	   };
	 break;

	 case TR:	/* Trace the firmware */
	    ioctl(tg8,Tg8DrvrTRACE_FIRMWARE,&iob);
	    DisplayMmResult(command);
	    break;

	 case RDT:	/* Read date and time */
	    ioctl(tg8,Tg8DrvrDATE_TIME,&iob);
	    DisplayMmResult(RDT);
	 break;

	 case RSC:	/* Read the super cycle info */
	    ioctl(tg8,Tg8DrvrSC_INFO,&iob);
	    printf(" The Super Cycle number: %d\n",iob.ScInfo.ScNumber);
	    printf(" The Super Cycle length: %d\n",iob.ScInfo.ScLength);
	    printf(" The curr. S-Cycle time: %d\n",iob.ScInfo.ScTime);
	    printf(" The S-Cycles counter  : %d\n",iob.ScInfo.ScCounter);
	 break;

	 case RST:	/* Read a module status */
	    ioctl(tg8,Tg8DrvrGET_STATUS,&iob);
	    DisplayDriverStatus();
	 break;

	 case RHST:	/* Read the hardware statuses */
	    ioctl(tg8,Tg8DrvrGET_RAW_STATUS,&iob);
	    if (ioctl(tg8,Tg8DrvrGET_RAW_STATUS,&iob)<0) {
	       printf("Can not get the module status\n");
	       perror("RawStatus:");
	    };
	    DisplayMmResult(RHST);
	 break;

	 case RAD:	/* Read the hardware configuration */
	   if (ioctl(tg8,Tg8DrvrGET_CONFIGURATION,&iob)<0) {
	     perror("GetCongiguration:");
	     break;
	   };
	   for (i=0; i<Tg8DrvrMODULES; i++) {
	     if ((k=iob.GetConfig.DevMasks[i]) != -1) {
	       Tg8ModuleAddress *moad = &iob.GetConfig.Addresses[i];
	       printf("Module #%d: ",i+1);
	       printf("Used devices: %X\n",k);
	       printf(" VME Addr:%x Vect:%x Level:%x Switch:%x SSC:%X\n",
		      moad->VMEAddress,
		      moad->InterruptVector,
		      moad->InterruptLevel,
		      moad->SwitchSettings,
		      moad->SscHeader);
	     };
	   };
	 break;

	 case DPRAM:	/* Read the dpram content */
	    ioctl(tg8,Tg8DrvrGET_DPRAM,&iob);
	    DisplayMmResult(DPRAM);
	 break;

	 case DPT:	/* Test the dpram */
	    ioctl(tg8,Tg8DrvrTEST_DPRAM,&iob);
	    if (iob.TestDpram.Status == 0)
	      printf(" DPRAM Ok\n");
	    else {
	      printf(" Result status: %X\n",iob.TestDpram.Status);
	      printf(" Fault address: %X\n",iob.TestDpram.Addr);
	      printf(" Written      : %X\n",iob.TestDpram.Val);
	      printf(" Read back    : %X\n",iob.TestDpram.Back);
	    };
	    break;

	 case VRS:	/* Read the driver and firmware versions */
	    ioctl(tg8,Tg8DrvrGET_DRI_VERSION,iob.Ver);
	    printf("Driver version  : %s\n",iob.Ver);
	    ioctl(tg8,Tg8DrvrGET_FIRMWARE_VERSION,iob.Ver);
	    printf("Firmware version: %s\n",iob.Ver);
	 break;

	 case AGAIN:	/* Install the new firmware object */
	    if (strlen(s) > strlen(commands[command])) {
	      cp = &(s[strlen(commands[command])]);
	      while ((*cp == ' ') || (*cp == ',')) cp++;
	    } else
	      cp = "../firmware/tg8.exe";
	    fwob = &iob.FirmwareObject;
	    if (*cp == '-') { /* Test the EPROM's boot reply */
	      fwob->StartAddress = 0x4020;
	      fwob->Size = 0;
	    } else {
	      printf("Read firmware from: %s\n",cp);
	      if (Tg8ReadFirmware(cp,fwob) != Tg8ERR_OK) {
		printf("Can't read firmware object: %s\n",cp);
		perror("Read error:");
		break;
	      };
	    };
	    InstallTheNewFirmware(fwob,1/*Verbose mode*/);
	 break;

	 case AREL:	/* Reload the actions table */
	   if (ioctl(tg8,Tg8DrvrRELOAD_ACTIONS,NULL) < 0) {
	       printf("Can't reload actions module: %u\n",modnum);
	       perror("ReloadAction:");
	    };
	 break;

	 case EN:	/* Enable module */
	    while (*cp==' ' || *cp=='\t') cp++;
	    if (ioctl(tg8,(*cp? Tg8DrvrENABLE_SYNC: Tg8DrvrENABLE_MODULE),NULL) < 0) {
	      printf("Can't enable module\n");
	      perror("Enable:");
	      break;
	    };
	 break;

	 case DS:	/* Disable module */
	   while (*cp==' ' || *cp=='\t') cp++;
	   if (ioctl(tg8,(*cp? Tg8DrvrDISABLE_SYNC: Tg8DrvrDISABLE_MODULE),NULL) < 0) {
	       printf("Can't disable module\n");
	       perror("Disable:");
	    };
	 break;

	 case RES:	/* Reset module */
	   if (ioctl(tg8,Tg8DrvrRESET_MODULE,NULL) < 0) {
	       printf("Can't reset modules\n");
	       perror("Reset:");
	    };
	 break;

	 case REC:	/* Read the recording table */
	    SET_RANGE(1,255);
	    if (ioctl(tg8,Tg8DrvrRECORDING_TABLE,&iob) < 0) {
	       printf("Can't read recording table\n");
	       perror("ReadRec:");
	       break;
	    };
	    DisplayMmResult(command);
	 break;

	 case INT:	/* Read the interrupt table */
	    SET_RANGE(1,Tg8CHANNELS+Tg8IMM_INTS);
	    params[0] = iob.Range.Row;
	    params[1] = iob.Range.Cnt;
	    if (ioctl(tg8,Tg8DrvrINTERRUPT_TABLE,&iob) < 0) {
	       printf("Can't read the interrupts table\n");
	       perror("ReadInt:");
	       break;
	    };
	    DisplayMmResult(command);
	 break;

	 case AUX:	/* Read the auxiliary table */
	    if (ioctl(tg8,Tg8DrvrAUX_TABLE,&iob) < 0) {
	       printf("Can't read the auxiliary table\n");
	       perror("ReadAux:");
	       break;
	    };
	    DisplayMmResult(command);
	 break;

	 case EVH:	/* Read the events history */
	    iob.HistTable.Cnt = (param_count? param[0]: 50);
	    if (param_count<2) params[1] = 0xFFFFFFFF;
	    if (ioctl(tg8,Tg8DrvrHISTORY_TABLE,&iob) < 0) {
	       printf("Can't read the event histories\n");
	       perror("ReadHistory:");
	       break;
	    };
	    DisplayMmResult(command);
	 break;

	 case CLK:	/* Read the clock table */
	    iob.ClockTable.Cnt = (param_count? param[0]: 16);
	    if (ioctl(tg8,Tg8DrvrCLOCK_TABLE,&iob) < 0) {
	       printf("Can't read the clock table\n");
	       perror("ReadClockTable:");
	       break;
	    };
	    DisplayMmResult(command);
	 break;

	 /* >>>>>>>>>> Timing action related functions <<<<<<<<<<<< */

         case PAR: /* Change object's parameters: PAR iden line sel val1 val2 */
	   iob.ObjectParam.Id    = param[0];
	   iob.ObjectParam.Line  = param[1];
	   iob.ObjectParam.Sel   = param[2];
	   iob.ObjectParam.Value = param[3];
	   iob.ObjectParam.Aux   = (param_count<5? 0: param[4]);
	   if (ioctl(tg8,Tg8DrvrOBJECT_PARAM,&iob)) {
	     printf("Can't set parameters\n");
	     perror("ObjectParam:");
	   };
         break;

         case OBJ: /* Set Object: line, trig,cw,delay, machine,Gn,Gt,Gv,dim */
	   if (!param_count) { /* Get the list of all objects */
	     iob.ObjectsList.Length = 256;
	     if (ioctl(tg8,Tg8DrvrOBJECTS_LIST,&iob)) {
	       printf("Can't get objects list\n");
	       perror("ObjectsList:");
	       break;
	     };
	     printf(" Known objects: %d\n",iob.ObjectsList.Length);
	     for (i=0; i<iob.ObjectsList.Length; i++) {
	       an = iob.ObjectsList.Id[i];
	       printf(" %d\n",an);
	     };
	     break;
	   };
	   
	   if (param[0]<0) { /* Delete an object */
	     iob.ObjectId = -param[0];
	     if (ioctl(tg8,Tg8DrvrREMOVE_OBJECT,&iob)) {
	       printf("Can't remove timing object\n");
	       perror("RemoveObject:");
	     };
	     break;
	   };
	   
	   if (param_count==1) { /* Display the object's details */
	     obj= &iob.ObjectDescr.Object;
	     obj->Id = param[0];
	     if (ioctl(tg8,Tg8DrvrGET_OBJECT,&iob)) {
	       printf("Can't get object\n");
	       perror("GetObj:");
	       break;
	     };
	     if (obj->Id == -1) {
	       printf(" No such object\n");
	       break;
	     };
	     ppm= &obj->Timing;
	     printf("Id:%d At:%d Dim:%d Trig:%08X Machine:%s Gn:%02d Gt:%s\n",
		    obj->Id,iob.ObjectDescr.Act,iob.ObjectDescr.Dim,
		    ppm->Trigger.Long,
		    mnames[ppm->Machine],ppm->GroupNum,
		    groupt[ppm->GroupType]);
	     k = ppm->Dim;
	     for (i=0;i<k;i++)
	       printf("%d/ CW:%04X Delay:%05d Gv:0x%X\n",1+i,
		      ppm->LineCw[i],ppm->LineDelay[i],ppm->LineGv[i]);
	     break;
	   };
	   /* Define object */
	   obj = &iob.Object;
	   obj->Id = param[0];
	   ppm = &obj->Timing;
	   ppm->Trigger.Long = param[1];
	   ppm->Machine  = param[4];
	   ppm->GroupNum = param[5];
	   ppm->GroupType= param[6];
	   ppm->Dim = k = param[8];
	   if (k<=0 || k>Tg8PPM_LINES) {
	     printf(" Illegal dimension: %d\n",k);
	     break;
	   };
	   for (i=0; i<k; i++) {
	     ppm->LineCw[i] = param[2];
	     ppm->LineDelay[i]= param[3]+(i*2);
	     ppm->LineGv[i] = param[7]+i;
	   };
	   if (ioctl(tg8,Tg8DrvrSET_OBJECT,&iob)) {
	     printf("Can't set object\n");
	     perror("SetObj:");
	   };
	 break;

         case ML:       /* Set line: trig,cw,delay */
	   if (param_count==3) {
	     act = iob.UserTable.Table;
	     act->uEvent.Long = param[0];
	     act->uControl = param[1];
	     act->uDelay = param[2];
	     if (write(tg8,act,sizeof(Tg8User)) != sizeof(Tg8User)) {
	       printf("Can't write the user table\n");
	       break;
	     };
	   };
	 break;

	 case MT:	/* Modify trigger */
	 case MO:	/* Modify output channel */
	 case MS:	/* Modify start type */
	 case MI:	/* Modify interrupt type */
	 case MC:	/* Modify clock type */
	 case MD:	/* Modify delay */
	   /* Read action */
	   if (param_count<2) break;
	   an = param[0];
	   if (an<0 || an>=Tg8ACTIONS) break;
	   iob.Range.Row = an;
	   iob.Range.Cnt = 1;
	   if (ioctl(tg8,Tg8DrvrUSER_TABLE,&iob) < 0) {
	     printf("Can't read the user table\n");
	     perror("ReadUserTable:");
	     break;
	   };
	   act = iob.UserTable.Table;
	   cw = act->uControl;
	   switch (command) {
	   case MT:	/* Modify trigger */
	     act->uEvent.Long = param[1];
	     break;
	   case MO:	/* Modify output channel */
	     Tg8CW_CNT_Clr(cw); Tg8CW_CNT_Set(cw,param[1]); break;
	   case MS:	/* Modify start type */
	     Tg8CW_START_Clr(cw); Tg8CW_START_Set(cw,param[1]); break;
	   case MI:	/* Modify interrupt type */
	     Tg8CW_INT_Clr(cw); Tg8CW_INT_Set(cw,param[1]); break;
	   case MC:	/* Modify clock type */
	     Tg8CW_CLOCK_Clr(cw); Tg8CW_CLOCK_Set(cw,param[1]); break;
	   case MD:	/* Modify delay */
	     act->uDelay = param[1];
	     break;
	   };
	   act->uControl = cw;
	   if (ioctl(tg8,Tg8DrvrCLEAR_ACTION,&iob) < 0) {
	     printf("Can't clear the line:%d\n",an);
	     perror("ClearAction:");
	     break;
	   };
	   if (write(tg8,act,sizeof(Tg8User)) != sizeof(Tg8User)) {
	     printf("Can't write the user table at line:%d\n",an);
	     perror("write:");
	     break;
	   };
	 break;

	 case DM:	/* Disable a timing action */
	 case EM:	/* Enable a timing action */
	   SET_RANGE1(1);
	   iob.ActState.State = (command==EM? Tg8ENABLED: Tg8DISABLED);
	   if (ioctl(tg8,Tg8DrvrSET_ACTION_STATE,&iob) < 0) {
	     printf("Can't set action's state\n");
	     perror("SetState:");
	     break;
	   };
	 break;

	 case L:	/* List of actions */
	    cp = &(s[strlen(commands[(int) command])]);
	    while (*cp == ' ') cp++;

	    an=la=1; ch = 0;

	    if ((*cp == 'a') || (*cp == 'A')) {
	       cp++;
	       an = strtoul(cp,&ep,0);
	       if (cp==ep) {
		 an = 1;
		 la = 255;
	       } else {
		 if (an<=0) break;
		 cp = ep;
		 pn = strtoul(cp,&ep,0);
		 if (cp!=ep) {
		   la = pn;
		   if (la < an) la = an;
		 } else la = an;
	       };
	    } else if ((*cp == 'c') || (*cp == 'C')) {
	       cp++;
	       ch = strtoul(cp,&ep,0);
	       if (cp==ep) ch=1;
	       if (ch < 1 || ch > Tg8CHANNELS) {
		  printf("Bad channel: %u\n",ch);
		  break;
	       };
	       cp = ep;
	       an = 1;
	       la = 255;
	    } else {
	       printf("??\n");
	       break;
	    };
	    if (an <= 0 || an > 255) {
	       printf("Bad action: %u\n",an);
	       break;
	    };
	    if (la>255) la= 255;

	    iob.Range.Row = an;
	    iob.Range.Cnt = la-an+1;
	    if (ioctl(tg8,Tg8DrvrUSER_TABLE,&iob) < 0) {
	      printf("Can't read actions: %u-%u\n",an,la);
	      perror("ReadAction:");
	      break;
	    };

	    for (a=an,act=iob.UserTable.Table; a<=la; a++,act++) {
	       cw = act->uControl;
	       ln = Tg8CW_CNT_Get(cw);
	       if (ln==0) ln=Tg8CHANNELS; /* Get the chennel number 1-8 */
	       if (ch==0 || ln==ch)
		  printf("%03u/ %08x %s/%03u Ch:%1u/%s %s %s\n",a,
			 act->uEvent.Long,
			 clk[Tg8CW_CLOCK_Get(cw)],
			 act->uDelay,
			 ln,
			 start[Tg8CW_START_Get(cw)],
			 outty[Tg8CW_INT_Get(cw)],
			 statety[Tg8CW_STATE_Get(cw)]
			 );
	    };
	 break;

	 case CLR:	/* Clear a part of the user table */
	   SET_RANGE(1,255);
	   if (ioctl(tg8,Tg8DrvrCLEAR_ACTION,&iob) < 0)
	     printf("Can't clear the user table\n");
	   break;

	 case UT:	/* Download the user table */
	   while (*cp==' ' || *cp=='\t') cp++;
	   fp = fopen((*cp? cp: (cp="../tests/user_table.txt")),"r");
	   if (!fp) {printf("Cannt open a file:%s\n",cp); break;}
	   while(cmd == fgets(cmd,sizeof(cmd),fp)) {
	     printf("Action: %s",cmd);
	     /* event controlWord delay */
	     cp = cmd;
	     usr.uEvent.Long = strtoul(cp,&ep,0);
	     if (cp == ep) {
syn:           printf(" Syntax error in line: %s\n",cmd);
               break; /* Syntax err. */
	     };
	     cp = ep;
	     usr.uControl = strtoul(cp,&ep,0);
	     if (cp == ep) goto syn; /* Syntax err. */
	     cp = ep;
	     usr.uDelay = strtoul(cp,&ep,0);
	     if (cp == ep) goto syn; /* Syntax err. */
	     if (write(tg8,&usr,sizeof(usr))<0) {
	       printf("Cannt write the action: %s\n",cmd);
	       perror("Write:");
	       break;
	     };
	   };
	   fclose(fp);
	 break;

	 case DEC:	/* Display actions for the given channel */
	    if (param_count<3) params[2] = 0;
	    else if (params[2] == Tg8CHANNELS) params[2] = 0;
	    /*+++*/
	 case DAL:	/* Display All */
	 case DEF:	/* Display DEFined only */
	    SET_RANGE(1,255);
	    if (ioctl(tg8,Tg8DrvrUSER_TABLE,&iob) < 0) {
	      printf("Can't read the user table\n");
	      perror("ReadUserTable:");
	      break;
	    };
	    for (an=iob.Range.Row,act=iob.UserTable.Table;
		 iob.Range.Cnt-- >0; an++,act++) {
	       if (command == DEF && !act->uEvent.Long) continue;
	       if (command == DEC && Tg8CW_CNT_Get(act->uControl) != params[2]) continue;
	       printf("%3d/ Frame:%08X  CW:%04X  Delay:%d\n",an,
		      act->uEvent.Long,act->uControl,act->uDelay);
	    };
	 break;

	 case DPPM:	/* Display PPM timing units */
	    SET_RANGE(1,255);
	    if (ioctl(tg8,Tg8DrvrGET_PPM_LINE,&iob) < 0) {
	      printf("Can't read the PPM info\n");
	      perror("GetPpmLine:");
	      break;
	    };
	    for (an=iob.Range.Row,ppml=iob.PpmLineTable.Table;
		 iob.Range.Cnt-- >0; an++,ppml++) {
	       if (!ppml->Action.uEvent.Long) continue;
	       act = &ppml->Action;
	       g = &ppml->Gate;
	       if (!g->Machine) continue;
	       printf("%3d/ Trig:%08X CW:%04X Delay:%05d Machine:%s Gn:%02d Gv:%02d Gt:%s\n",an,
		      act->uEvent.Long,act->uControl,act->uDelay,
                      mnames[g->Machine>>4],g->GroupNum,g->GroupVal,
                      groupt[g->Machine&0xF]);
	    };
	 break;

         case CON:  /* Connect to the timing member */
	   iob.ObjectConnect.Id = param[0];
	   iob.ObjectConnect.Mask = param[1];
	   if (ioctl(tg8,Tg8DrvrCONNECT,&iob)<0) {
	     printf("Cannt connect to object\n");
	     perror("Connect:");
	     break;
	   };
	   printf("Matched mask: %X\n",iob.ObjectConnect.Mask);
	   if (!iob.ObjectConnect.Mask) break;
	   param[1] = param[2]; /* cnt */
	   param[2] = 1; /* step */
#if 0
	   goto w_ev;
#endif
         case WAIT: /* Wait for any event */
	   if (param_count<1) params[0] = 0xFFFFFFFF;
	   if (param_count<2) params[1] = 1;
	   if (param_count<3) params[2] = 1;

	   /* Set up the filter of events */
	   iob.Filter.Event.Long = params[0];
	   if (ioctl(tg8,Tg8DrvrFILTER_EVENT,&iob)<0) {
	     printf("Cannt set up the event's filter\n");
	     perror("Filter:");
	     break;
	   };
	   printf("Matches: %d\n",iob.Filter.Matches);
      w_ev:
	   conn = &iob.Event;
	   for (i=j=k=0; i<params[1];) {
	     /* Wait for any event */
	     ln = read(tg8,&iob.Event,sizeof(Tg8DrvrEvent));
	     if (ln<0) {
	       perror("ReadEvent:");
	       break;
	     };
	     if (!ln) {
	       printf("User table is empty!\n");
	       break;
	     };

	     /* There is an event. Display its details. */
	     /*** if (!Equ(params[0],conn->Inter.iEvent.Long)) continue; ***/

	     i++; j++;
	     if (++k < params[2]) continue;
	     k = 0;
	     printf("Declared timing Id:%d Event: %08X\n",conn->Id,conn->Event.Long);
	     printf("Interrupt information:\n ");
	     DisplayInterrupt(&conn->Inter);
	     DisplayAlarms(conn->Alarms);         /* Show the Fw & Driver alarms */
	     DisplayFirmwareStatus(conn->FwStat); /* Show the firmware status */
	   };
         break;

         case TEL: /* Read telegram */
	   mach = (param_count? params[0]: Tg8CPS);
	   k = (param_count>1? params[1]: 1); /* repeate command n times */
	   if (mach<0 || mach>Tg8LPI) break;

	   /* Wait for the specified event */
	   while (k-->0) {
	     conn = &iob.Event;
	     iob.Event.Event.Long = rpls[mach-1];
	     if (ioctl(tg8,Tg8DrvrWAIT_EVENT,&iob.Event)<0) {
	       printf("Cannt wait for the RPLS event\n");
	       perror("Wait:");
	       break;
	     };
	     if (!conn->Event.Long) {
	       printf("Time OUT!\n");
	       break;
	     };
	     
	     /* There is the RPLS event. Display its details. */
	     
	     printf("Declared timing Id:%d Event: %08X\n",conn->Id,conn->Event.Long);
	     printf("Interrupt information:\n ");
	     DisplayInterrupt(&conn->Inter);
	     DisplayAlarms(conn->Alarms);         /* Show the Fw & Driver alarms */
	     DisplayFirmwareStatus(conn->FwStat); /* Show the firmware status */

	     /* Read machine's telegram */

	     iob.Telegram.Machine = mach;
	     if (ioctl(tg8,Tg8DrvrTELEGRAM,&iob)<0) {
	       printf("Cannt read telegram\n");
	       perror("ReadTelegram:");
	       break;
	     };
	     DisplayTelegram(iob.Telegram.Machine,iob.Telegram.Data);
	   };
	 break;

	 /* >>>>>>>>>>>> Macros <<<<<<<<<<<<< */

	 case BMC:
	    if (param_count < 1) {
	       printf("No macro number specified\n");
	       break;
	    };
	    sprintf(cmd,"e macro_%u",*params);
	    printf("%s\n",cmd);
	    system(cmd);
	 break;

	 case XMC:
	    if (param_count < 1) {
	       printf("No macro number specified\n");
	       break;
	    };
	    sprintf(cmd,"tg8test module %u macro < macro_%u",modnum,*params);
	    printf("%s\n",cmd);
	    system(cmd);
	 break;

	 case SMC:
	    sprintf(cmd,"ls macro_* > /tmp/macros");
	    system(cmd);
	    fp = fopen("/tmp/macros","r");
	    if (fp == NULL) {
	       perror("Cant open /tmp/macros");
	       break;
	    };
	    while (fgets(cmd,128,fp)) {
	       if (strncmp(cmd,"macro_",6) != 0) continue;
	       cmd[strlen(cmd) -1] = 0;
	       sprintf(str,"%s",cmd);
	       if ((lfp = fopen(str,"r")) == NULL) continue;
	       if (fgets(s,128,lfp)) {
		  printf("%s:\t%s",cmd,s);
	       };
	       fclose(lfp);
	    };
	    fclose(fp);
	 break;

	 default:
	    printf("??\n");
      };
   };
}

/************************************/
/* Display the driver's status code */
/************************************/

DisplayDriverStatus() {
int status,alarms;
  status = iob.Status.Status;
  alarms = iob.Status.Alarms;
  printf("Driver's Status:%X\n",status);
  if (status & Tg8DrvrMODULE_ENABLED)
    printf("  Module enabled\n");
  else
    printf("  Module disabled, no interrupts or timings output\n");
  if (status & Tg8DrvrMODULE_FIRMWARE_LOADED)
    printf("  Firmware loaded\n");
  else
    printf("  Firmware not loaded\n");
  if (status & Tg8DrvrMODULE_RUNNING)
    printf("  Firmware running\n");
  else
    printf("  Firmware not running, pending DOWLOAD\n");
  if (status & Tg8DrvrMODULE_HARDWARE_ERROR)
    printf("  Hardware error, see hardware status\n");
  if (status & Tg8DrvrMODULE_FIRMWARE_ERROR)
    printf("  Firmware error, see cprintf output on console\n");
  if (status & Tg8DrvrMODULE_SWITCH_SET)
    printf("  JUMPER is set: X1 CLOCK\n");
  else
    printf("  JUMPER is set: INTERNAL 10MHz CLOCK\n");
  /**
    if (status & Tg8DrvrMODULE_USER_TABLE_VALID)
    printf("  Action table loaded and valid\n");
    else
    printf("  Action table NOT valid\n");
  **/
  if (status & Tg8DrvrMODULE_TIMED_OUT)
    printf("  DPRAM Time out, see hardware status\n");
  if (status & Tg8DrvrDRIVER_DEBUG_ON)
    printf("  Driver debug ON, system runs very slowly\n");
  DisplayAlarms(alarms);
}

/************************************/
/* Display a module's status code   */
/************************************/

DisplayHardwareStatus(int status) {
int i,j;
  printf("HardwareStatus: %X\n",status);
  if (status & Tg8HS_SELF_TEST_ERROR)
    printf(" Self-test is not OK (NOT FATAL)\n");
  else
    printf(" Self-test is OK\n");
  if (status & Tg8HS_XILINX_X1_LOADED)
    printf(" Xilinx X1 (Counters 1-4) loaded OK\n");
  else
    printf(" Xilinx X1 (Counters 1-4) not loaded - FATAL!\n");
  if (status & Tg8HS_XILINX_X2_LOADED)
    printf(" Xilinx X2 (Counters 5-8) loaded OK\n");
  else
    printf(" Xilinx X2 (Counters 5-8) not loaded - FATAL!\n");
  if (status & Tg8HS_XILINX_XR_LOADED)
    printf(" Xilix XR (Frame Reciever) loaded OK\n");
  else
    printf(" Xilinx XR (Frame Reciever) not loaded - FATAL!\n");
  if (status & Tg8HS_PENDING_DOWNLOAD)
    printf(" Tg8 is in \"PENDING\" state\n");
  else
    printf(" Tg8 is not in \"PENDING firmware DOWNLOAD\" state\n");
  if (status & Tg8HS_EXTERNAL_CLOCK_JUMPER)
    printf(" Jumper set X1 (external clock 1)\n");
  else
    printf(" Jumper set INTERNAL (internal 10MHz clock)\n");
  if (status & Tg8HS_RECEIVER_ENABLED)
    printf(" XR Interrupts to MC68332 (timing frames) ENABLED\n");
  else
    printf(" XR Interrupts to MC68332 (timing frames) DISABLED\n");
  if (!(status & Tg8INTERRUPT_MASK))
    printf(" No VME interrupts pending\n");
  else {
    if (status & Tg8HS_DPRAM_INTERRUPT)
      printf(" DPRAM interrupt pending\n");
    for (i=0,j=1; i<8; i++,j<<=1) {
      if (status & j)
	printf(" Channel %u interrupt pending\n", i+1);
    };
  };
}

/************************************/
/* Display the Xilinx's status code */
/************************************/

DisplayXilinxStatus(int status) {
  printf("Xilinx XR Status: %X\n",status);
  if (!status)
    printf(" No XILINX XR (incomming timing frame) errors\n");
  else {
    if (status & XrDATA_OVERFLOW)
      printf(" XILINX XR Data overflow error\n");
    if (status & XrPARITY_ERROR)
      printf(" XILINX XR Parity error\n");
    if (status & XrEND_SEQUENCE)
      printf(" XILINX XR End Sequence error\n");
    if (status & XrMID_BIT)
      printf(" XILINX XR Mid Bit Transmission error\n");
    if (status & XrSTART_SEQUENCE)
      printf(" XILINX XR Start Sequence error\n");
    if (status & XrDISABLED)
      printf(" XILINX XR Receiver disabled\n");
    if (status & XrMS_MISSING)
      printf(" XILINX XR MS event missing\n");
    if (status & XrMS_WATCH_DOG)
      printf(" XILINX XR MS Watch dog error\n");
  };
}

/************************************/
/* Display the firmware status code */
/************************************/

DisplayFirmwareStatus(int status) {
  printf("FirmwareStatus : %X\n",status);
  if (!status)                   printf(" Firmware is OK\n");
  if (status & Tg8FS_RUNNING)    printf(" The firmware program running normally\n");
  if (status & Tg8FS_ALARMS)     printf(" Errors (alarms) were occured\n");
}

/************************************/
/* Display alarm codes              */
/************************************/

DisplayAlarms(int status) {
  printf("Alarms: %X\n",status);
  /* Firmware bits */
  if (status == Tg8ALARM_OK)        printf(" No alarms where generated\n");
  if (status & Tg8ALARM_ISR)        printf(" There where nested MS interrupts\n");
  if (status & Tg8ALARM_LOST_IMM)   printf(" The immediate action's interrupt was lost\n");
  if (status & Tg8ALARM_LOST_OUT)   printf(" The counter pulse was lost\n");
  if (status & Tg8ALARM_MANY_PROC)  printf(" The UT processes queue is full\n");
  if (status & Tg8ALARM_QUEUED_PROC)printf(" The UT process was queued\n");
  if (status & Tg8ALARM_DIFF_EVN)   printf(" The number of events for 2 seq. SC are different\n");
  if (status & Tg8ALARM_MOVED_PROC) printf(" The UT process is not completed before the next ms\n");
  if (status & Tg8ALARM_MOVED_IMM)  printf(" The immediate interrupts where moved\n");
  if (status & Tg8ALARM_ACT_DISBL)  printf(" The action was disabled, but till is in the CAM\n");
  if (status & Tg8ALARM_IMMQ_OVF)   printf(" The immediate actions queue is overflowed\n");
  if (status & Tg8ALARM_MBX_BUSY)   printf(" The mailbox busy\n");
  /* Drivers bits */
  if (status & Tg8ALARM_UNCOM)      printf(" Spurious VME BUS interrupt\n");
  if (status & Tg8ALARM_BAD_SWITCH) printf(" The switch is in bad position\n");
  if (status & Tg8ALARM_INT_LOST)   printf(" Interrupts where lost - the driver is too busy\n");
}

/************************************/
/* Display firmware error codes     */
/************************************/

static char *fw_err[] = {
  "No errors",
  "Illegal Mbx operation",
  "Illegal argument supplied",
  "Mbx timed out",
  "Too big array's dimension",
  "Bad counter (==0xFFFF)",
  "Bad clock type",
  "Bad start mode",
  "Operation was rejected by the driver (sys error)",
  "Bad firmware object format",
  "There is no acknowledge from boot program",
  "The firmware is not running",
  "No such the firmware executable file",
  "The previous request was not served",
  "The VME BUS Error detected",
  "The Tg8 Firmware Exception detected",
  "Bad Interrupt vector position",
  "Bad Switch position",
  "Bad Trigger code (its header is 0xFF)",
  "??? UNKNOWN ERROR!"
};

DisplayFwError(int err) {
  printf("CompletionCode : %d\n",err);
  if (err<0) err= -err;
  printf(" [%d] %s\n",err,fw_err[err>-Tg8ERR_BAD_SWITCH? (err+1):err]);
}

/************************************/
/* Display the interrupts table     */
/************************************/

DisplayInterrupt(Tg8Interrupt *ip) {
  printf("Act:%03d Fr:%08X SC:%06d Occ:%05d Out:%05d Rcv:%02X Owr:%03d Sem:%d\n",
	 ip->iExt.iAct,
	 ip->iEvent.Long,
	 ip->iSc,
	 ip->iOcc,
	 ip->iOut,
	 ip->iExt.iRcvErr,
	 ip->iExt.iOvwrAct,
	 ip->iExt.iSem);
}

DisplayInterruptTable(Tg8InterruptTable *t,int from,int cnt) {
Tg8Interrupt *ip; int i;
  if (--from<Tg8CHANNELS) {
    printf(">>> Counter Interrupts Table <<<\n");
    for (i=from,ip=&t->CntInter[from]; i<Tg8CHANNELS && cnt>0; i++,cnt--,from++,ip++) {
      printf("%d/ ",i+1);
      DisplayInterrupt(ip);
    };
  };
  if (cnt) {
    from -= Tg8IMM_INTS;
    printf(">>> Immediate Interrupts Table <<<\n");
    for (i=from,ip=&t->ImmInter[from]; i<Tg8IMM_INTS && cnt>0; i++,cnt--,ip++) {
      printf("%d/ ",i+1);
      DisplayInterrupt(ip);
    };
  };
}

/************************************/
/* Display the auxiliary table      */
/************************************/

DisplayAuxTable(Tg8Aux *a) {
Tg8DateTime *tp = &a->aDt; int i;

  printf(" >>> Auxiliary Table <<<\n");
  printf("Last Frame     : %08X\n",a->aEvent.Long);
  printf("Last MS Frame  : %08X\n",a->aMsEvent.Long);
  printf("SC Length      : %d\n",a->aScLen);
  printf("SC Time        : %d\n",a->aScTime);
  printf("SC Number      : %d\n",a->aSc);
  DisplayAlarms(a->aAlarms);
  printf("InterruptSource: %X\n",a->aIntSrc);
  printf("Firmware TRACE : %X\n",a->aTrace);
  printf("BUS Interrupts : %d\n",a->aNumOfBus);
  printf("Spurious Ints  : %d\n",a->aNumOfSpur);
  printf("FirmwareCommand: %d\n",a->aMbox);
  DisplayFwError(a->aCoco);
  DisplayFirmwareStatus(a->aFwStat);
  printf("Module SSC Hdr : %02X\n",a->aSscHeader);
  printf("Number of SSC  : %d\n",a->aNumOfSc);
  printf("Numb of frames : %d\n",a->aNumOfEv);
  printf("Frames in prevs: %d\n",a->aPrNumOfEv);
  printf("User Table leng: %d\n",a->aNumOfAct);
  printf("Frames in Queue: %d\n",a->aQueueSize);
  printf("Maximal Queue  : %d\n",a->aMaxQueueSize);
  printf("Moved Frames   : %d\n",a->aMovedFrames);
  printf("      Time     : %d\n",a->aMovedScTime);
  printf("      S-Cycle  : %d\n",a->aMovedSc);
  printf("Processd Frames: %d\n",a->aProcFrames);
  printf("IncFrames Queue:\n");
  for (i=0;i<8;i++)
  printf(" Frame #%d     : %08X\n",i+1,a->aQueue[i].Long);
  DisplayXilinxStatus(tp->aRcvErr);
  DisplayDateTime(tp);
  printf("Firmwre Version: %s\n",a->aFwVer);
  printf("R/W Semaphore  : %d  %s\n",a->aSem,(a->aSem? "DATA CHANGED":"OK"));
}

DisplayDateTime(Tg8DateTime *tp) {
  printf("Current Time   : %u-%u-%u %u:%u:%u.%u #%d\n",
	 UBCD(tp->aYear),UBCD(tp->aMonth),UBCD(tp->aDay),
	 UBCD(tp->aHour),UBCD(tp->aMinute),UBCD(tp->aSecond),
	 tp->aMilliSecond,tp->aMsDrift);
}

DisplayTelegram(int machine,Word *tel) {
int i,j;
  printf("Machine:%s. Telegram:\n",mnames[machine]);
  for (i=0; i<Tg8GROUPS;) {
    for (j=0; j<6; j++,i++)
      printf(" %2d/%04X",i+1,tel[i]);
    printf("\n");
  };
}

/*************************************************/
/* Display results of any multi-module operation */
/*************************************************/

DisplayMmResult(int op) {
Tg8StatusBlock	*sb = &iob.RawStatus.Sb;
Tg8Dpram      	*dp = &iob.Dpram;
StDpm           *dpl;
Tg8DateTime	*tp = &iob.DateTime;
int		trp = iob.Trace;
Tg8History   *ah;
Tg8Clock     *c;
Tg8Recording *r;
int i,a; char cmd[32];
	switch (op) {
	case RHST:
	    DisplayHardwareStatus(sb->Hw);
	    if (sb->Hw & Tg8HS_SELF_TEST_ERROR)
	      DisplaySelfTestResult(&iob.RawStatus.Res);

	    DisplayXilinxStatus(sb->Dt.aRcvErr);
	    DisplayFirmwareStatus(sb->Fw);
	    DisplayAlarms(sb->Am);
	    printf("CloseMode: %d %X\n",iob.RawStatus.Cm.Mode,
		   iob.RawStatus.Cm.SimPulseMask);
	    printf("Errors : %d\n",iob.RawStatus.ErrCnt);
	    printf(" Last   : %d\n",iob.RawStatus.ErrCode);
	    printf(" Message: %s\n",iob.RawStatus.ErrMsg);
	break;
	case DPRAM:
#if 0
	    printf("Except Vector  : %X\n",dp->ExceptionVector);
	    printf("Exception PC   : %X\n",dp->ExceptionPC);
	    DisplayHardwareStatus(dp->Hw);
	    DisplayInterruptTable(&dp->It,0,Tg8CHANNELS+Tg8IMM_INTS);
	    DisplayAuxTable(&dp->At);
	    /* DisplayTelegram(Tg8LEP,dp->At.aTelegLEP);
	    DisplayTelegram(Tg8SPS,dp->At.aTelegSPS); */
	    DisplayTelegram(Tg8CPS,dp->At.aTelegCPS);
	    DisplayTelegram(Tg8PSB,dp->At.aTelegPSB);
	    DisplayTelegram(Tg8LPI,dp->At.aTelegLPI);
#else
	    dpl = (StDpm *)dp;
	    a = dpl->Head.FirmwareStatus;
	    printf("Firmware status: %X (%s)\n",a,
		   (a==Tg8BOOT? "BOOT":
		    a==Tg8DOWNLOAD? "DOWNLOAD":
		    a==Tg8DOWNLOAD_PENDING? "D/L PENDING":
		    a==Tg8332Bug? "332BUG":
		    a==0? "FIRMWARE RUNNING": "???"));
	    if (a) DisplaySelfTestResult(&dpl->Info);
#endif
	break;
	case RDT:
	    DisplayDateTime(tp);
	break;

        case INT:
	    DisplayInterruptTable(&iob.IntTable,params[0],params[1]);
	break;

        case AUX:
	    DisplayAuxTable(&iob.AuxTable);
	break;

	case REC:
	    for (i=0,r=iob.RecTable.Table; i<iob.Range.Cnt; i++,r++) {
               printf("%3d/  Sc:%06d Occ:%05d Out:%05d TrNum:%06d Ovwr:%3d ByAct:%03d\n",
		      iob.Range.Row+i,
		      r->rSc,
		      r->rOcc,
		      r->rOut,
		      r->rNumOfTrig,
		      r->rOvwrCnt,
		      r->rOvwrAct);
            };
	break;
	case EVH:
	    for (i=0,ah=iob.HistTable.Table+iob.HistTable.Cnt-1; i<iob.HistTable.Cnt; i++,ah--) {
	       if (params[1]==0xFFFFFFFF || Equ(params[1],ah->hEvent.Long))
		 printf("%3d/  Ev:%08X Sc:%6d Occ:%5d Rcv:%02X Time:%02d.%02d.%02d\n",
		      i+1,
		      ah->hEvent.Long,
		      ah->hSc,
		      ah->hOcc,
		      ah->hRcvErr,
		      UBCD(ah->hHour),
		      UBCD(ah->hMinute),
		      UBCD(ah->hSecond));
            };
	break;
	case CLK:
	    for (i=0,c=iob.ClockTable.Table; i<iob.ClockTable.Cnt; i++,c++) {
               printf("MsEv:%08X NumMs:%5d Sc:%06d Occ:%05d Rcv:%02X Time:%02d.%02d.%02d\n",
		      c->cMsEvent,
		      c->cNumOfMs,
		      c->cSc,
		      c->cOcc,
		      c->cRcvErr,
		      UBCD(c->cHour),
		      UBCD(c->cMinute),
		      UBCD(c->cSecond));
            };
	break;
	case TR:
	    printf("Trace: %x (%d.)\n",trp,trp);
	break;
	default: ;
	};
}

int Equ(int tem,int ev) {
Byte *e,*t;
  if (tem == ev) return 1;
  e= (Byte*)&ev;
  t= (Byte*)&tem;
  if (t[0]!=0xFF && t[0]!=e[0]) return 0;
  if (t[1]!=0xFF && t[1]!=e[1]) return 0;
  if (t[2]!=0xFF && t[2]!=e[2]) return 0;
  if (t[3]!=0xFF && t[3]!=e[3]) return 0;
  return 1;
}

/*****************************************************/
/* Initialize all things related to the new firmware */
/*****************************************************/

int InstallTheNewFirmware(Tg8FirmwareObject *fwob,int verbose) {
int status;
char *progname = "tg8test";

      /* Reload the Firmware */

      if (verbose) printf("%s: Reload firmware module: %1d\n",progname,modnum);
      if (ioctl(tg8,Tg8DrvrRELOAD_FIRMWARE,fwob) < 0) {
	 if (!fwob->Size) {  /* Test the boot reply */
	    ioctl(tg8,Tg8DrvrGET_DPRAM,&iob);
	    DisplayMmResult(DPRAM);
	    return;
	 };
	 printf("%s: Can't RELOAD FIRMWARE: Module %1d\n",progname,modnum);
	 perror(progname);
	 if (ioctl(tg8,Tg8DrvrGET_RAW_STATUS,&iob)<0) {
	   printf("Can not get the module status\n");
	   perror("RawStatus:");
	 };
	 DisplayMmResult(RHST);
	 return -1;
      };

      if (verbose) printf("%s: Checking status module: %1d\n",progname,modnum);
      ioctl(tg8,Tg8DrvrGET_RAW_STATUS,&iob);
      status = iob.RawStatus.Sb.Hw;
      if (verbose)
	 printf("%s: Module %1d raw hardware status: %4x\n",
		progname,modnum, status);
      if (status & 0x1FF) {
	 printf("%s: Module %1d error: Interrupt(s) jammed ON\n",progname,modnum);
	 return -1;
      };

      /* Enable event reception XILINX */

      if (verbose) printf("%s: Enable module: %1d\n",progname,modnum);
      if (ioctl(tg8,Tg8DrvrENABLE_MODULE,NULL) < 0) {
	 printf("%s: Can't ENABLE MODULE: Module %1d\n",progname,modnum);
	 perror(progname);
      };

      /* Set driver debug mode */

      ioctl(tg8,Tg8DrvrDEBUG_OFF,NULL);
}

/*======================================================================*/
#define NewLine printf("\n")
static char *t_dir[] = {"Forward ","Backward "};
static char *t_acc[] = {"Byte","Word","Int32"};
static char *t_cam[] = {"","tem,~tem,tem^5A5A",
			   "~tem,tem^5A5A,tem",
			   "~tem,tem,tem^5A5A",
		           "tem^5A5A,tem,~tem",
			   "tem,tem^5A5A,~tem" };
static char *b_int[]={""
   ""
,  "MCP hardware initializing"
,  "Writing onto the RAM location 'bus_int'"
,  "The RAM test"
,  "The DPRAM access"
,  "The DPRAM test"
,  "The CAM access"
,  "The CAM test"
,  "The XILINX access (read err & set SSC code)"
,  "The XILINX test"
,  "Read Frame 1"
,  "Read Frame 2"
,  "Read the Receiver Error"
,  "Reset the Receiver Error"
,  "Set the SSC code"
,  "The COUNTER test"
,  "Read/Write the Delay register"
,  "Write the Configuration register"
};
static char *co_err[] = {"",
			 "The counting is too fast",
			 "The counting is blocked -- no interrupts from it",
			 "The Counter interrupt missed on the DSC site"
};

WhatTests(int pass) {
  if (pass &T_RAM) printf(" RAM");
  if (pass &T_DPRAM) printf(" DPRAM");
  if (pass &T_CAM) printf(" CAM");
  if (pass &T_XILINX) printf(" XILINX");
  if (pass &T_MS) printf(" MS");
  if (pass &T_COUNTERS) printf(" COUNTERS");
  NewLine;
}

DisplaySelfTestResult(StDpmInfo *info) {
StFault *f; int pass,fail,dir,i,err;

  pass = info->TestPassed;
  fail = info->FaultType;
  if (!pass && !fail) {
    printf(" <<< THERE IS NO THE SELFTEST PROCEDURE IN THE EPROM !!! >>>\n");
    return;
  };

  if (err=info->TestProg) {
    printf("The test in progress           : %X :" ,err);
    WhatTests(pass);
  };
  printf("The bitmask of the passed tests: %X :" ,pass);
  WhatTests(pass);
  printf("The bitmask of failed test     : %X :" ,fail);
  WhatTests(fail);
  printf("The number of failed tests     : %d.\n",info->FaultCnt);
  printf("The number of the BUS interrpts: %d.\n",info->N_of_bus);
#if 0
  printf("The number of the CPS cycles   : %d.\n",info->CycleNum);
  printf("The last CPS cycle duration    : %d.\n",info->CycleDur);
  printf("C-Train                        : %d.\n",info->CTrain);
  printf("Time: %u-%u-%u %u:%u:%u.%u\n",
		   info->Date.Year,
		   info->Date.Month,
		   info->Date.Day,
		   info->Date.Hour,
		   info->Date.Minute,
		   info->Date.Second,
		   info->Date.Ms);
  NewLine;
  printf("Number of Event Frame interrpts: %d.\n",info->N_of_frames);
  printf("Event Frame code               : %X\n", info->EventFrame);
  printf("Number of MPC Mbx operations   : %d.\n",info->MbxInt);
  printf("Number of DSC Mbx interrpts    : %d.\n",info->DscInt[0]);
#endif
  for (i=1;i<=8;i++)
    if (info->DscInt[i])
      printf("Number of DSC Counter_%d interrp: %d.\n",i,info->DscInt[i]);

  printf("********* DPRAM status ***********");
  if (info->FaultType & T_DPRAM) {
    printf(" THERE ARE ERRORS!\n");
    printf("Tested DPRAM region: %X - %X\n",
	 info->Dpram.Mem.Bot,info->Dpram.Mem.Top);
    printf("Phase code        : %d.\n",info->Dpram.Err.Code);
    printf("BUS interrupts    : %d.\n",info->Dpram.Err.N_of_bus);
    printf("    conditions    : %s\n", b_int[info->Ram.Err.BusInt]);
    printf("Location address  : %X\n", info->Dpram.Err.At);
    dir = info->Dpram.Err.Dir;
    printf("Direction & Access: %s %s\n",t_dir[dir&0xFF],t_acc[dir>>8]);
    printf("Data expected     : %X\n", info->Dpram.Err.Templ);
    printf("Data was read from: %X\n", info->Dpram.Err.Actual);
  } else
    printf((pass&T_DPRAM)? " OK!\n":" ---\n");

  printf("*********** RAM status ***********");
  if (info->FaultType & T_RAM) {
    printf(" THERE ARE ERRORS!\n");
    printf("Tested RAM region : %X - %X\n",
	   info->Ram.Mem.Bot,info->Ram.Mem.Top);
    printf("Phase code        : %d.\n",info->Ram.Err.Code);
    printf("BUS interrupts    : %d.\n",info->Ram.Err.N_of_bus);
    printf("    conditions    : %s\n", b_int[info->Ram.Err.BusInt]);
    printf("Location address  : %X\n", info->Ram.Err.At);
    dir = info->Ram.Err.Dir;
    printf("Direction & Access: %s %s\n",t_dir[dir&0xFF],t_acc[dir>>8]);
    printf("Data expected     : %X\n", info->Ram.Err.Templ);
    printf("Data was read from: %X\n", info->Ram.Err.Actual);
  } else
    printf((pass&T_RAM)? " OK!\n":" ---\n");

  printf("*********** CAM status ***********");
  if (info->FaultType & T_CAM) {
    printf(" THERE ARE ERRORS!\n");
    printf("Tested CAM region : %X - %X  ",
	   info->Cam.Mem.Bot,info->Cam.Mem.Top);
    NewLine;
    printf("Error code        : %d.\n",info->Cam.Err.Code);
    printf("BUS interrupts    : %d.\n",info->Cam.Err.N_of_bus);
    printf("    conditions    : %s\n", b_int[info->Ram.Err.BusInt]);
    printf("Location address  : %X\n", info->Cam.Err.At);
    dir = info->Cam.Err.Dir;
    printf("Direction & Access: %s %s\n",t_dir[dir&0xFF],t_cam[dir>>8]);
    printf("Data expected     : %X\n", info->Cam.Err.Templ);
    printf("Data was read from: %X\n", info->Cam.Err.Actual);
    printf("CAM match address : %d.\n",info->Cam.Match.At);
    printf("Number of matches : %d.\n",info->Cam.Match.Cnt);
  } else
    printf((pass&T_CAM)? " OK!\n":" ---\n");

  printf("*********** XILINX status ********");
  err = info->Xilinx.Rcv & 0xFF;
  if (err || (info->FaultType & T_XILINX)) {
    if (info->FaultType & T_XILINX) printf(" THERE ARE ERRORS!");
    printf("\nExtra code        : %d.\n",info->Xilinx.Err.Code);
    printf("BUS interrupts    : %d.\n",info->Xilinx.Err.N_of_bus);
    printf("    conditions    : %s\n", b_int[info->Ram.Err.BusInt]);
    printf("Last XILINX error : %X :\n" ,err);
    if (err & XrDATA_OVERFLOW)  printf("  XILINX XR Data overflow error\n\r");
    if (err & XrPARITY_ERROR)   printf("  XILINX XR Parity error\n\r");
    if (err & XrEND_SEQUENCE)   printf("  XILINX XR End Sequence error\n\r");
    if (err & XrMID_BIT)        printf("  XILINX XR Mid bit transission error\n\r");
    if (err & XrSTART_SEQUENCE) printf("  XILINX XR Start Sequence error\n\r");
    if (err & XrDISABLED)       printf("  XILINX XR Receiver disabled\n\r");
    if (err & XrMS_WATCH_DOG)   printf("  XILINX XR MS Watch dog error\n\r");
    if (err & XrMS_MISSING)     printf("WARNING: XILINX XR MS event Missing\n\r");
  } else
    printf((pass&T_XILINX)? " OK!\n":" ---\n");

  printf("*** TPU Channel #0 : MS clock ****");
  if (info->FaultType & T_MS) {
    printf(" THERE ARE ERRORS!\n");
  } else
    printf((pass&T_MS)? " OK!\n":" ---\n");

  printf("********* COUNTERs status ********");
  if (info->FaultType & T_COUNTERS) {
    printf(" THERE ARE ERRORS!\n");
    for (i=0,f= info->Counter.Err; i<StCOUNTERS; i++,f++) {
      err = f->Code;
      printf("COUNTER: %d  %s\n",i+1,(err? "BAD":"OK"));
      if (!err) continue;

      printf("Error code        : %d. %s\n",err,
	    (err<N_OF_CONT_ER? co_err[err]:"BUS interrupt"));
      if (f->N_of_bus) {
	printf("BUS interrupts    : %d.\n",f->N_of_bus);
	printf("    conditions    : %s\n", b_int[f->BusInt]);
      };
      if (f->Dir) printf("Xilinx Error      : %X\n",f->Dir);
      printf("Delay programmed  : %X\n", f->Templ);
      if (f->Actual)
	printf("Actual delay      : %X\n", f->Actual);
    };
    printf("Bad Counters Mask : %X\n" ,info->Counter.Bad);
  } else
    printf((pass&T_COUNTERS)? " OK!\n":" ---\n");
}

/* eof tg8test.c */
