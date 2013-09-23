/********************************************************************/
/* Selftest program for the Tg8 modules.                            */
/* J.Lewis Oct 1994                                                 */
/* V.Kovaltsov Oct 1996                                             */
/* Requirements are done by B.Puccio, SL/CO                         */
/********************************************************************/

#define ST_DEBUG 0 /* 1=debugg the selftest program */

#include <selftestP.h>

#define BCD(b) ((b&0xF)+((b>>4)*10))

/********************************************************************/
/* Start of the bootstrap code                                      */
/********************************************************************/

Tg8LoadDpm     *dpram; /* Dpram layout used by the download procedure */
unsigned short *x;     /* Local variable that points to the DP buffer */
unsigned short *p;     /* Local variable that points to the target RAM buffer */
short           l;     /* Words counter - the size of the next downloaded record */

long *v,del;           /* Vector address and delay counter */
short i,frst;          /* Loop counter, firmware status value from the DPRAM */

extern default_isr();
#if ST_DEBUG /*debug */
asm ("
 .text
 .globl __main
__main:
  bra   Tests        /* Start the downloading ...          */
");
#else
asm ("
 .text
 .globl __main
__main:
       movew #Tg8DISABLE_INTERRUPTS,SR /* Disable interrupts             */
       movew #6,SimProtect          /* Enable Bus Monitor with 16 CLK    */
       oriw  #0x4000,SimSync        /* Set the frequency to 16 MHz       */
       movew #0x42cf,SimConf        /* freese, show cycles, IARB is high */
       movel #0xffffffff,SimCSpin   /* All pins are chip selects         */
       movel #Tg8CS0,SimCS0         /* Chip select 0 as in 332Bug        */
       movel #Tg8CS1,SimCS1         /* Chip-select 1 as in 332Bug        */
       movel #Tg8CSBOOT,SimCSboot   /* Chip-select Base Address Register Boot */
       movel #Tg8CS2,SimCS2         /* Chip-select 2 as in 332Bug        */
       movew #0,SbrConf             /* Set the Standby RAM to contain    */
       movew #0x4000,SbrBaSt        /* the system stack                  */
       movel #0x4007fc,a7           /* System stack bottom               */
       movel #Tg8CS8,SimCS8         /* Set the Chip-select 8 (DPRAM)     */
       movel #0,SimCS9              /* Clear chip selects 9 and 10       */
       movel #0,SimCS10
       movew #851,d0                /* Make a delay while the XILINXs are */
    0: movew #0xff,d1               /* loaded                             */
    2: dbf   d1,2b
       dbf   d0,0b
       movel #Tg8DPM_BASE,_dpm      /* Set the DPRAM base pointer         */
       bra   Tests                  /* Start the downloading ...          */
      ");
#endif
#if 0  /* Old codes for the history */
       cmpiw #0x3856,0x10004         /* Is Tg8_DOWNLOAD in the mail box ?  */
       beq   1f                      /* Yes, download the firmware program */
       movew #0x3844,0x10004         /* No,  set the 332BUG state          */
       movel #0x6e12a,a3             /*      and start the 332 BUG         */
       jmp   a3@
#endif

/* -----------------------------------------------------*/
/* Download code across DPRAM                           */
/* -----------------------------------------------------*/

Tests () {
asm ("
 .text
 .globl Tests
Tests:
 ");

   v = (long *) 0;
   asm volatile ("movec %0,VBR"::"r"(v)); /* Define the Vectors Base as 0 */

   /* Set the default ISR for each vector */

   v = (long *) 4;
   for (i=255; i>=0; i--)  *v++ = (long) default_isr;

   Init(); /* Install ISRs, initialize basic hardware, enable interrupts */

   /*********************/
   /* Process any tests */
   /*********************/

   DpramTest();    /* Test the DPRAM */
#if ST_DEBUG==0
   RamTest();      /* Test the RAM region */
#endif
   CamTest();      /* Test the CAM */
   MsTest();       /* Test the 1 MS clock */
   XilinxTest();   /* Test Xilinx Receiver */
   CountersTest(); /* Test counters */

   dpm->Info.TestProg = 0; /* Any tests have been completed */
   dpm = (StDpm*)dpram;
   dpm->Info = dpr.Info;

   memset((short*)tpu_int,0,sizeof(tpu_int));

   if (!dpm->Info.FaultCnt) {
     /* Set the TEST_OK bit in the hardware test register */
     sim->SimDataF1 |= SelfTestOk;
     sim->SimDataF1  = 0;
     sim->SimDataF1 |= SelfTestOk;
   };

   dpm->Info.N_of_frames = frame_int;
   dpm->Info.EventFrame = event_frame.Long;
   dpm->XilinxStatus = xlx->WClrRerr;

   dpram->Firmware_status = frst; /* Restore initial Firmware status value */

   tpu->TpuIe = 0x0;  /* Disable TPU interrupts 0-8 */
   INTOFF;            /* Disable interrupts */

   /******************************************************/
   /* Copy the resident firmware into the RAM and run it */
   /******************************************************/

   if (frst != Tg8DOWNLOAD) { /* Run f/w OR Start the 332BUG */
/*if (!dpm->Info.FaultCnt) {*/
       /* Copy & Run the resident firmware */
       l = fwob.Size;
       x = (Ushort*)fwob.Object;
       p = (Ushort*)fwob.StartAddress;
       for (; l>0; l--) *p++ = *x++;
       p = (Ushort*) fwob.StartAddress;
       asm (" jmp %0@"::"a"(p)); /* Start main program after loading */
#if 0
     } else {
       /* Run the 332 bug */
       xlx->XWsscRframe1 = 0x0;  /* Unset the CPS SSC event */
       asm volatile ("
           movew #0x3844,0x10004         /* set the 332BUG state  */
           movel #0x6e12a,a3             /* and start the 332 BUG */
           jmp   a3@");
     };
#endif
   };

   /**********************************/
   /* Start the DownLoader program   */
   /**********************************/

   /* Waiting for the driver's download request */

  del = 1500;
  while (dpram->Firmware_status != Tg8DOWNLOAD)
    if (--del <= 0 && !dpm->Info.FaultCnt) {
      sim->SimDataF1 |= (short)   OkLed;
      sim->SimDataF1 &= ~OkLed;
      sim->SimDataF1 |= (short)   OkLed;
      del= 1500;
    };

  /* Send an acknowledge */
  dpram->Firmware_status = Tg8BOOT;

  /* Read the firmware codes from the LynxOS system */
  /* ---------------------------------------------- */

  for (;;) {
    /* Waiting for the next block of codes */
    del = 1500;
    while (dpram->Loader_done_flag != Tg8DWNLD_FLAG_READY)
      if (--del == 0 && !dpm->Info.FaultCnt) {
	sim->SimDataF1 |= (short)   OkLed;
	sim->SimDataF1 &= ~OkLed;
	sim->SimDataF1 |= (short)   OkLed;
	del= 1500;
      };

    /* Read specified address and length */

    x = (unsigned short *) dpram->LdBuffer;       /* Codes are placed here */
    p = (unsigned short *) *(int*)dpram->Address; /* Take the Base address */
    l = dpram->Length;                            /* Take the block length */

    /* Start the firmware program (if block length is 0) */
    /* The base address will stay the start address      */
    if (l == 0)
      asm (" jmp %0@"::"a"(p)); /* Start main program after loading */

    /* Copy data block from the DPRAM to local RAM using word access */

    while (l>0) {
      l -= 2;
      *p++ = *x++;
    };
    /* Send an acknowledge, ready to receive the next block of codes */
    dpram->Loader_done_flag = Tg8DWNLD_FLAG_DONE;
  };
}

/********************************************************************/
/* Install Interrupt Service Routines.                              */
/* Initialize the systems integration module chip selects.          */
/* Initialize the Time Processor Unit.                              */
/********************************************************************/

void Init() {
void  ( * p)();         /* Procedure pointer */
short   * f;            /* TPU Function */
long    * v;            /* Vector pointer */
int       j, k;

   INTOFF;              /* Interrupts OFF */
   bus_int = 0;
   if (bus_int) {
     busi[0] = busi[1] = BI_RAM;
     busn= bus_int;
     bus_int = 0;
   } else busn = 0;

   /*************************************************/
   /* Set up the interrupt service routine vectors. */
   /*************************************************/

   v = (long*) (Tg8TPU_BASE_VECTOR*4); /* TPU interrupts 0-8 */
   p = Tpu0Isr; *v++ = (long) p;
   p = Tpu1Isr; *v++ = (long) p;
   p = Tpu2Isr; *v++ = (long) p;
   p = Tpu3Isr; *v++ = (long) p;
   p = Tpu4Isr; *v++ = (long) p;
   p = Tpu5Isr; *v++ = (long) p;
   p = Tpu6Isr; *v++ = (long) p;
   p = Tpu7Isr; *v++ = (long) p;
   p = Tpu8Isr; *v   = (long) p;

   v = (long*) Tg8ABT_VECTOR; p = AbortIsr; *v = (long) p; /* Abort button */
   v = (long*) Tg8XLX_VECTOR; p = XrIsr;    *v = (long) p; /* Incoming timing event */
   v = (long*) Tg8DSC_VECTOR; p = DscIsr;   *v = (long) p; /* DSC -> Tg8 , not used */

   /* Bus error, address error, privilage violation, spurious interrupts */

   v = (long*) McpVECTOR_BUS_ERROR;          p = BusErrorIsr; *v = (long) p;
   v = (long*) McpVECTOR_ADDRESS_ERROR;      p = AddressErrorIsr; *v = (long) p;
   v = (long*) McpVECTOR_PRIVILEGE_VIOLATION;p = PrivViolationIsr; *v = (long) p;
   v = (long*) McpVECTOR_SPURIOUS_INTERRUPT; p = SpuriousIsr; *v = (long) p;

   /********************************************************/
   /* Define base address pointers to Tg8 hardware modules */
   /********************************************************/

   sim = (McpSim *) McpSIM_BASE;        /* System Integration Module */
   tpu = (McpTpu *) McpTPU_BASE;        /* Time Processor Unit */
   dpm = (StDpm  *) Tg8DPM_BASE;        /* Dual Port RAM */
   xlx = (Tg8Xlx *) Tg8XLX_BASE;        /* XiLinX gate arrays */
   cam = (short  *) Tg8CAM_BASE;        /* Content Addressed Memory */

   /***************************************/
   /* Set up the chip selects for the SIM */
   /***************************************/

   sim->SimCS3 = Tg8CS3; /* Selects the Xilinx memory map for X1 X2 and XR. */
   sim->SimCS4 = Tg8CS4; /* Is the Abort button interrupt autovector. */
   sim->SimCS5 = Tg8CS5; /* Is the XILINX frame interrupt autovector. */
   sim->SimCS6 = Tg8CS6; /* Is Write the CAM with an option control bit. */
   sim->SimCS7 = Tg8CS7; /* Is Read the CAM */
   sim->SimCS8 = Tg8CS8; /* Selects the DPRAM */

   sim->SimProtect = Tg8ENABLE_BUSMONITOR;

   /********************************************/
   /* Initialize the Time Processor Unit (TPU) */
   /********************************************/

   tpu->TpuConf = 7; /* IMB interrupt arbitration ID number */

   /* Set up the interrupt configuration register */

   tpu->TpuIc = Tg8TPU_BASE_VECTOR | (Tg8TPU_INT_LEVEL<<8);

   /* Set 9 TPU channels up to Input capture/ Input transition counter */
   /* by loading the predefined time function labled "$A" into the */
   /* Channel Function Selection registers. */

   tpu->TpuCfs1 = 0xa;     /* TPU channel 8 function select "A" = ITC */
   tpu->TpuCfs2 = 0xaaaa;  /* Channels 4 to 7 */
   tpu->TpuCfs3 = 0xaaaa;  /* Channels 0 to 3 */

   /* Initialize the ITC functions for channels 0 to 8 by setting */
   /* each of the two bit Host Sequence Registers to the value 1. */
   /* The value 1 is defined as "Initialize" for function "$A".   */

   tpu->TpuHs0  = 1;
   tpu->TpuHs1  = 0x5555;
   tpu->TpuIe   = 0;       /* Disable TPU interrupts */

   /* We want to initialize the Interrupt status Register to zero.  */
   /* But tpu->TpuIs = 0; will not work because the gcc compiler    */
   /* generates a CLR instruction in this case, which would provoke */
   /* a bus error because CLR contains a READ cycle !! */

   asm(" movew #0,TpuIs");

   /* Set TPU ITC function params for channels 0 - 8 */
   /* First set up the addresses of the 16 tpu parameter blocks. */

   k   = McpTPU_ACT;                /* TPU Parameter RAM map */
   for(j=0; j<16; j++) {
      tpu_parameters[j] = (short*) k;
      k += 16;
   };

   for(j=0; j<9; j++) {
      f = tpu_parameters[j];
      *f++ = 0x7; /* Channel control: PSC=DontCare PAC=RisingEdge TBS=Input */
      *f++ = 0xe; /* Start link:      LastChannel IE No link blocks */
      *f++ = 0x1; /* Link count:      1 Must be greater than zero !! */
      *f++ = 0x0; /* Bank address:    I will do my own incrementing */
   };

   /* Initialize each channel again by setting 0x01 in each in the */
   /* Host Sequence Registers. */

   tpu->TpuHsr0 = 1;
   tpu->TpuHsr1 = 0x5555;

   /* Channel priority registers */

   tpu->TpuCp0 = 2;         /* Set the high priority for channel 0 and */
   tpu->TpuCp1 = 0xaaab;    /* middle priority for channels 1-8        */

   if (bus_int) {
     busi[0] = busi[1] = BI_INIT;
     busn= bus_int;
     bus_int = 0;
   };

   /************************/
   /* Initialize the DPRAM */
   /************************/

   dpram = (Tg8LoadDpm*) dpm;
   frst = dpram->Firmware_status; /* Save the Firmware status value */

   memset((short*)&dpm->Info,0,sizeof(dpm->Info));
   memset((short*)&dpr,0,sizeof(dpr));

   /* Signal the down loader that we are running OK */

   dpm->Head.DoneFlag    = MbxOP_NONE;
   dpm->Head.IoDirection = MbxOP_NONE;
   dpm->Head.FirmwareStatus = MbxCC_FIRMWARE_RUNNING;

   if (bus_int) {
     busi[0] = busi[1] = BI_DPRAM;
     busn= bus_int;
     bus_int = 0;
   };

   /**************************/
   /* Set up Port F IO pins  */
   /**************************/

   sim->SimPinF   = 0xfff1;
   sim->SimDirF   = 0xe;

   /******************/
   /* CAM setting up */
   /******************/

   CamInitialize();

   if (bus_int) {
     busi[0] = busi[1] = BI_CAM;
     busn= bus_int;
     bus_int = 0;
   };

   /***********************/
   /* XILINX initializing */
   /***********************/

   k = xlx->WClrRerr;        /* Read the reciever error */
   xlx->XWsscRframe1 = 0x34; /* Set the CPS SSC (Start Super Cycle) event header */

   if (bus_int) {
     busi[0] = busi[1] = BI_XILINX;
     busn= bus_int;
     bus_int = 0;
   };

   /* Debug routine for setting break points in this firmware */
   debug_p = (long *) &(dpm->Head.ExceptionPC);

   /* Clear the incoming frames counter */
   frame_int = 0;

   dpm->Info.Ram.Err.BusInt = (busi[0]==busi[1]? busi[0]: BI_RAM);
   dpm->Info.N_of_bus = busn;

   tpu->TpuIe = 0x1ff;      /* Enable TPU interrupts 0-8 */
   INTON;                   /* Interrupts on, watchdog disabled yet */
}

/********************************************************************/
/* Initialize the CAM                                               */
/********************************************************************/

void CamInitialize() {

   CamSetCommandMode;
   CamClear;                            /* Clear all */

   /* Write zeros to CAM */

   asm(" moveal _cam,a0
	 movew  #CamWRITE,d0
	 movel  #255,d1
      0: oriw   #CamDATA_MODE,SimDataF1  /* Set data mode */
	 movew  #0,a0@                   /* Clear the comparand register */
	 movew  #0,a0@                   /* words 0 1 2. */
	 movew  #0,a0@
         andiw  #CamCOMMAND_MODE,SimDataF1 /* Set command mode */
	 movew  d0,a0@                   /* Do the COPY command */
	 addiw  #1,d0                    /* Next CAM array address */
	 dbf    d1,0b                    /* Loop in CAM */
	 movew  #0,a0@ ");               /* Clear ALL: skip and mark empty */
}

/************************************************/
/* Tracing using a simple break point and value */
/************************************************/

Debug(n) long n; { while (*debug_p) WAIT; *debug_p = n; }

/*****************/
/* Blow up a led */
/*****************/

ShowLed(int del) {
  while (--del) {
    sim->SimDataF1 |= OkLed;
    sim->SimDataF1 &= ~OkLed;
    sim->SimDataF1 |= OkLed;
  };
}

/***********************************************/
/* Fill the memory block using the word access */
/***********************************************/

void memset(short *m,short v,unsigned int len) {
  len >>= 1;
  while (len--) *m++ = v;
}

static void bcopy(short *s,short *d,int bc) {
register int n;
  for (n=(bc>>1); n>0; n--) *d++ = *s++;
  if (bc&1) *(char*)d = *(char*)s;
}

/********************************************************************/
/* Add here any others source files                                 */
/********************************************************************/

#include "self_isr.c"
#include "self_dpram.c"
#include "self_ram.c"
#include "self_cam.c"
#include "self_ms.c"
#include "self_xilinx.c"
#include "self_counter.c"
#include "self_rs232.c"

/********************************************************************/
/* Recognizable text for memory dump etc.                           */
/********************************************************************/

char *header = " Tg8 Firmware bootstrap and selftest: ";
char *versin = __DATE__;

/* eof selftest.c */

