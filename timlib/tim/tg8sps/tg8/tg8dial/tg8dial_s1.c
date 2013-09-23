/***************************************************************************/
/* For Tg8 Dialog program.                                                 */
/* Sub-prog of tg8dial.c:                                                  */
/* _ Display results                                                       */
/* _ Display Hardware status                                               */
/* _ Display the Xilinx's status                                           */
/* _ Display the driver's status code                                      */
/* _ Display the Self-Test result                                          */
/*  Original Vers. 05/08/97  ( Bruno )                                     */
/***************************************************************************/

#include <stdio.h>
#include <file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <a.out.h>
#include <smem.h>

#include <tg8drvr.h>
#include <tg8.h>

static Tg8IoBlock iob;          /* Data returned by all ioctl operations */

typedef enum {RHST,DPRAM} Command;

/****************************************************************/
/* Display results of any multi-module operation by Vova Feb.97 */
/****************************************************************/

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
	    
	    /* DisplayAlarms(sb->Am); */
	    /* printf("CloseMode: %d %X\n",iob.RawStatus.Cm.Mode,
		   iob.RawStatus.Cm.SimPulseMask);
	    printf("Errors : %d\n",iob.RawStatus.ErrCnt);
	    printf(" Last   : %d\n",iob.RawStatus.ErrCode);
	    printf(" Message: %s\n",iob.RawStatus.ErrMsg); */
	break;
	
	case DPRAM:

	    printf("Except Vector  : %X\n",dp->ExceptionVector);
	    printf("Exception PC   : %X\n",dp->ExceptionPC);

	    /* DisplayInterruptTable(&dp->It,0,Tg8CHANNELS+Tg8IMM_INTS); */
	    /* DisplayAuxTable(&dp->At); */

	    dpl = (StDpm *)dp;
	    a = dpl->Head.FirmwareStatus;
	    printf("Firmware status: %04X (%s)\n",a,
		   (a==Tg8BOOT? "BOOT":
		    a==Tg8DOWNLOAD? "DOWNLOAD":
		    a==Tg8DOWNLOAD_PENDING? "D/L PENDING":
		    a==Tg8332Bug? "332BUG":
		    a==0? "FIRMWARE RUNNING": "???"));
	    if (a) DisplaySelfTestResult(&dpl->Info);
	break;
	default: ;
	}
}
	
/****************************************************************/
/* Display a module's status code  / Vova Feb.97,  Mod.by Bruno */
/****************************************************************/

DisplayHardwareStatus(int status) {
int i,j;
  printf("Status Register value = 0x%04X\n",status);

  if (status & 0x8000)
  	printf(" Fatal Error: Self-test is not OK \n");
  else
    	printf(" OK: Self-test fullfilled without error\n");
    
  if (status & 0x2000)
    	printf(" OK: Xilinx X1 (Counters#1-4) well loaded \n");
  else
    	printf(" Fatal Error: Xilinx X1 (Counters#1-4) not loaded!\n");
    
  if (status & 0x1000)
    	printf(" OK: Xilinx X2 (Counters#5-8) well loaded \n");
  else
    	printf(" Fatal Error: Xilinx X2 (Counters#5-8) not loaded!\n");
    
  if (status & 0x0800)
    	printf(" OK: Xilinx XR (Frame Recvr.) well loaded \n");
  else
    	printf(" Fatal Error: Xilinx XR (Frame Recvr.) not loaded!\n");
    
  if (status & 0x0400)
    	printf(" Warning: Tg8 is in a \"Disable/Enable PENDING\" state!\n");
  else
    	printf(" OK: Tg8 is not in a \"Disable/Enable PENDING\" state\n");

  if (status & 0x200)
    	printf(" OK: the module is ENABLED ( i.e. receiving Timing Frames) \n");
  else
    	printf(" Warning: the module is DISABLED \n");
     
  if (!(status & Tg8INTERRUPT_MASK))
    	printf(" OK: No VME interrupts pending\n");
  else {
    	if (status & Tg8HS_DPRAM_INTERRUPT)
      		printf(" Error: DPRAM interrupt is pending\n");
    		for (i=0,j=1; i<8; i++,j<<=1) {
      			if (status & j)
			printf(" Error: Counter#%u interrupt is pending\n", i+1);
    		};
  };
    
  if (status & 0x4000)
    	printf(" Third clock selection: Jumper J2 set to External clock#1\n\n");
  else
    	printf(" Third clock selection: Jumper J2 set to Internal 10MHz\n\n"); 
  
}

/*****************************************************************/
/* Display the Xilinx's status code / Vova Feb.97,  Mod.by Bruno */
/*****************************************************************/

DisplayXilinxStatus(int status) {
  printf("Receiver Error Register value = 0x%04X\n",status);
  if (!status)
  	printf(" OK: No errors detected\n");
  else {
    	if (status & 0x01) printf(" XR Data overflow error\n");
    	if (status & 0x02) printf(" XR Parity error\n");
    	if (status & 0x04) printf(" XR End Sequence error\n");
    	if (status & 0x08) printf(" XR Mid Bit Trans. error\n");
    	if (status & 0x10) printf(" XR Start Sequence error\n");
    	if (status & 0x20) printf(" XR Receiver disabled!\n");
    	if (status & 0x40) printf(" XR MS event missing\n");
    	if (status & 0x80) printf(" XR MS Watch dog error\n");
  };  
}

/**************************************************/
/* Display the firmware status code by Vova Feb.97*/
/**************************************************/

DisplayFirmwareStatus(int status) {
  printf("\nFirmware Status = 0x%04X  ",status);
  if (!status)                   printf("( Firmware is OK )");
  if (status & Tg8FS_RUNNING)    printf("( Firmware is running normally )");
  if (status & Tg8FS_ALARMS)     printf("( Errors (alarms) were occured )");
}

/*************************************************************/
/* Display the driver's status code by Vova Feb.97           */
/*************************************************************/

DisplayDriverStatus(int status) {
  printf("\nDriver's Status = 0x%04X\n",status);
  if (status & Tg8DrvrMODULE_ENABLED)
    printf("  OK: Module enabled\n");
  else
    printf("  WARNING: Module disabled!\n");
  if (status & Tg8DrvrMODULE_FIRMWARE_LOADED)
    printf("  WARNING: Firmware overwritten!\n");
  else
    printf("  OK: Internal Firmware not re-loaded\n");
  if (status & Tg8DrvrMODULE_RUNNING)
    printf("  OK: Firmware is running\n");
  else
    printf("  ERROR: Firmware not running, pending DOWLOAD\n");
  if (status & Tg8DrvrMODULE_HARDWARE_ERROR)
    printf("  Hardware error, see hardware status\n");
  if (status & Tg8DrvrMODULE_FIRMWARE_ERROR)
    printf("  Firmware error, see cprintf output on console\n");
  if (status & Tg8DrvrMODULE_TIMED_OUT)
    printf("  DPRAM Time out, see hardware status\n");
  if (status & Tg8DrvrDRIVER_DEBUG_ON)
    printf("  Driver debug ON, system runs very slowly\n");
}

/*************************************************************/
/* Display the Self-Test result     by Vova Feb.97           */
/*************************************************************/

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
  printf("\n");
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
    printf("\n");
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

WhatTests(int pass) {
  if (pass &T_RAM) printf(" RAM");
  if (pass &T_DPRAM) printf(" DPRAM");
  if (pass &T_CAM) printf(" CAM");
  if (pass &T_XILINX) printf(" XILINX");
  if (pass &T_MS) printf(" MS");
  if (pass &T_COUNTERS) printf(" COUNTERS");
  printf("\n");
}
/*************************************************************/