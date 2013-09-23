/*------------------------------------------------------------------*/
/* RS-232 support                                                   */
/*------------------------------------------------------------------*/

/******************/
/* Print a string */
/******************/

PrintStr(char *s) {
char v;
  while (chr= *s++) {
    v= chr;
    asm volatile ("
          moveb  _chr,a7@-
          trap   #15
          .word  0x20 ");
    if (v=='\r') break;
  };
}

/************************/
/* Print a character    */
/************************/

PrintChar(char ch) {
  chr= ch;
  asm volatile ("
          moveb  _chr,a7@-
          trap   #15
          .word  0x20 ");
}

/************************/
/* Print a memory block */
/************************/

PrintBuf(char *s,int len) {
  while (len>0) {
    chr= *s++;
    len--;
    asm volatile ("
          moveb  _chr,a7@-
          trap   #15
          .word  0x20 ");
  };
}

PrintNum(char *txt,int num) {
  if (txt) PrintStr(txt);
  if (num<0) {PrintChar('-'); PrintNum(0,-num);}
  else
  if (num<10) PrintChar('0'+num);
  else {PrintNum(0,num/10); PrintChar('0'+(num%10));}
}

PrintHex(char *txt,unsigned int num) {
  if (txt) PrintStr(txt);
  if (num < 10) PrintChar('0'+num);
  else
  if (num < 16) PrintChar('A'+num-10);
  else {PrintHex(0,num>>4); PrintHex(0,num&0xF);}
}

ReturnToDebug() {
     asm volatile ("
           trap   #15
          .word  0x63 "); /* Return to the CPU32BUG */
}

#define NewLine PrintStr("\n\r")

/* Allocate all Text messages */

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
,  "XILINX XR Read Frame 1"
,  "XILINX XR Read Frame 2"
,  "XILINX XR Read the Receiver Error"
,  "XILINX XR Reset the Receiver Error"
,  "XILINX XR Set the SSC code"
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
  if (pass &T_RAM)      PrintStr(" RAM");
  if (pass &T_DPRAM)    PrintStr(" DPRAM");
  if (pass &T_CAM)      PrintStr(" CAM");
  if (pass &T_XILINX)   PrintStr(" XILINX");
  if (pass &T_MS)       PrintStr(" MS");
  if (pass &T_COUNTERS) PrintStr(" COUNTERS");
  NewLine;
}

static StFault *f; static int pass,fail,dir,err;

/*------------------------------------------------------------------*/
/* Display the content of the DPRAM describing the selftest results */
/*------------------------------------------------------------------*/

DisplayDpram() {

   /********************************************************/
   /* Define base address pointers to Tg8 hardware modules */

   sim = (McpSim *) McpSIM_BASE;        /* System Integration Module */
   tpu = (McpTpu *) McpTPU_BASE;        /* Time Processor Unit */
   /*if (dpm != (StDpm  *) Tg8DPM_BASE) // Dual Port RAM */
     dpm = &dpr;
   xlx = (Tg8Xlx *) Tg8XLX_BASE;        /* XiLinX gate arrays */
   cam = (short  *) Tg8CAM_BASE;        /* Content Addressed Memory */

   /***************************************/
   /* Set up the chip selects for the SIM */

   sim->SimCS3 = Tg8CS3; /* Selects the Xilinx memory map for X1 X2 and XR. */
   sim->SimCS4 = Tg8CS4; /* Is the Abort button interrupt autovector. */
   sim->SimCS5 = Tg8CS5; /* Is the XILINX frame interrupt autovector. */
   sim->SimCS6 = Tg8CS6; /* Is Write the CAM with an option control bit. */
   sim->SimCS7 = Tg8CS7; /* Is Read the CAM */
   sim->SimCS8 = Tg8CS8; /* Selects the DPRAM */

   tpu->TpuIe = 0;       /* Disable all TPU interrupts */
   sim->SimProtect = 0;  /* Disable the Watch Dog */

  if (pass=dpm->Info.TestProg) {
    PrintHex("The test in progress           : " ,pass);
    WhatTests(pass);
  };
  PrintHex("The bitmask of the passed tests: " ,pass=dpm->Info.TestPassed);
  WhatTests(pass);
  PrintHex("The bitmask of failed test     : " ,fail=dpm->Info.FaultType);
  WhatTests(fail);
  PrintNum("The number of failed tests     : ",dpm->Info.FaultCnt);
  NewLine;

  PrintNum("The number of the BUS interrpts: ",dpm->Info.N_of_bus);
  NewLine;
  PrintNum("Number of Event Frame interrpts: ",dpm->Info.N_of_frames);
  NewLine;
  PrintHex("Event Frame code               : ", dpm->Info.EventFrame);
  NewLine;
/**
  PrintNum("Number of MPC Mbx operations   : ",dpm->Info.MbxInt);
  NewLine;
  PrintNum("Number of DSC Mbx interrpts    : ",dpm->Info.DscInt[0]);
  for (i=1;i<=8;i++)
    PrintNum("Number of DSC Counter_%d interrp: %d.\n",i,dpm->Info.DscInt[i]);
  NewLine;
**/

  PrintStr("*********** DPRAM ****************");
  if (dpm->Info.FaultType & T_DPRAM) {
    PrintStr(" THERE ARE ERRORS!\n\r");
    PrintHex("Tested DPRAM region: ",dpm->Info.Dpram.Mem.Bot);
    PrintHex(" - ",dpm->Info.Dpram.Mem.Top);
    NewLine;
    PrintNum("Phase code        : ",dpm->Info.Dpram.Err.Code);
    NewLine;
    PrintNum("BUS interrupts    : ",dpm->Info.Dpram.Err.N_of_bus);
    NewLine;
    PrintStr("    conditions    : "); PrintStr(b_int[dpm->Info.Ram.Err.BusInt]);
    NewLine;
    PrintHex("Location address  : ", dpm->Info.Dpram.Err.At);
    NewLine;
    dir = dpm->Info.Dpram.Err.Dir;
    PrintStr("Direction & Access: ");
    PrintStr(t_dir[dir&0xFF]); PrintStr(t_acc[dir>>8]);
    NewLine;
    PrintHex("Data expected     : ", dpm->Info.Dpram.Err.Templ);
    NewLine;
    PrintHex("Data was read from: ", dpm->Info.Dpram.Err.Actual);
    NewLine;
  } else
    PrintStr(" OK!\n\r");

  PrintStr("*********** RAM ******************");
  if (dpm->Info.FaultType & T_RAM) {
    PrintStr(" THERE ARE ERRORS!\n\r");
    PrintHex("Tested RAM region : ", dpm->Info.Ram.Mem.Bot);
    PrintHex(" - ",RAM_TOP);
    NewLine;
    PrintNum("Phase code        : ",dpm->Info.Ram.Err.Code);
    NewLine;
    PrintNum("BUS interrupts    : ",dpm->Info.Ram.Err.N_of_bus);
    NewLine;
    PrintStr("    conditions    : "); PrintStr(b_int[dpm->Info.Ram.Err.BusInt]);
    NewLine;
    PrintHex("Location address  : ", dpm->Info.Ram.Err.At);
    NewLine;
    dir = dpm->Info.Ram.Err.Dir;
    PrintStr("Direction & Access: "); PrintStr(t_dir[dir&0xFF]);
                                      PrintStr(t_acc[dir>>8]);
    NewLine;
    PrintHex("Data expected     : ", dpm->Info.Ram.Err.Templ);
    NewLine;
    PrintHex("Data was read from: ", dpm->Info.Ram.Err.Actual);
    NewLine;
  } else
    PrintStr(" OK!\n\r");

  PrintStr("*********** CAM ******************");
  if (dpm->Info.FaultType & T_CAM) {
    PrintStr(" THERE ARE ERRORS!\n\r");
    PrintHex("Tested CAM region : ",dpm->Info.Cam.Mem.Bot);
    PrintHex(" - ",dpm->Info.Cam.Mem.Top);
    NewLine;
    PrintNum("Error code        : ",dpm->Info.Cam.Err.Code);
    NewLine;
    PrintNum("BUS interrupts    : ",dpm->Info.Cam.Err.N_of_bus);
    NewLine;
    PrintStr("    conditions    : "); PrintStr(b_int[dpm->Info.Ram.Err.BusInt]);
    NewLine;
    PrintHex("Location address  : ", dpm->Info.Cam.Err.At);
    NewLine;
    dir = dpm->Info.Cam.Err.Dir;
    PrintStr("Direction & Access: "); PrintStr(t_dir[dir&0xFF]);
    PrintStr(t_cam[dir>>8]);
    NewLine;
    PrintHex("Data expected     : ", dpm->Info.Cam.Err.Templ);
    NewLine;
    PrintHex("Data was read from: ", dpm->Info.Cam.Err.Actual);
    NewLine;
    PrintNum("CAM match address : ",dpm->Info.Cam.Match.At);
    NewLine;
    PrintNum("Number of matches : ",dpm->Info.Cam.Match.Cnt);
    NewLine;
  } else
    PrintStr(" OK!\n\r");

  PrintStr("*********** XILINX XR ************");
  err = dpm->Info.Xilinx.Rcv & 0xFF;
  if (err || (dpm->Info.FaultType & T_XILINX)) {
    if (fail & T_XILINX) PrintStr(" THERE ARE ERRORS!");
    NewLine;
    PrintNum("Extra code        : ",dpm->Info.Xilinx.Err.Code);
    NewLine;
    PrintNum("BUS interrupts    : ",dpm->Info.Xilinx.Err.N_of_bus);
    NewLine;
    PrintStr("    conditions    : "); PrintStr(b_int[dpm->Info.Ram.Err.BusInt]);
    NewLine;
    PrintHex("Last XILINX error : " ,err);
    NewLine;
    if (err & XrDATA_OVERFLOW)  PrintStr("XILINX XR Data overflow error\n\r");
    if (err & XrPARITY_ERROR)   PrintStr("XILINX XR Parity error\n\r");
    if (err & XrEND_SEQUENCE)   PrintStr("XILINX XR End Sequence error\n\r");
    if (err & XrMID_BIT)        PrintStr("XILINX XR Mid bit transission error\n\r");
    if (err & XrSTART_SEQUENCE) PrintStr("XILINX XR Start Sequence error\n\r");
    if (err & XrDISABLED)       PrintStr("XILINX XR Receiver disabled\n\r");
    if (err & XrMS_MISSING)     PrintStr("WARNING: XILINX XR MS event Missing\n\r");
    if (err & XrMS_WATCH_DOG)   PrintStr("XILINX XR MS Watch dog error\n\r");
  } else
    PrintStr(" OK!\n\r");

  PrintStr("*** TPU Channel #0 : MS clock ****");
  if (dpm->Info.FaultType & T_MS) {
    PrintStr(" NO CLOCK!\n\r");
  } else
    PrintStr(" OK!\n\r");

  PrintStr("*********** COUNTERs *************");
  if (dpm->Info.FaultType & T_COUNTERS) {
    PrintStr(" THERE ARE ERRORS!\n\r");
    for (i=0,f= dpm->Info.Counter.Err; i<StCOUNTERS; i++,f++) {

      err = f->Code;
      PrintNum("COUNTER: ",i+1); PrintStr(err? " BAD":" OK");
      NewLine;
      if (!err) continue;  /* A counter is OK, lets check the next one */

      /* Display the counter test results */

      PrintNum("Error code        : ",err); PrintChar(' ');
      PrintStr(err<N_OF_CONT_ER? co_err[err]:"BUS interrupt");
      NewLine;
      PrintNum("BUS interrupts    : ",f->N_of_bus);
      NewLine;
      PrintStr("    conditions    : "); PrintStr(b_int[f->BusInt]);
      NewLine;
      PrintHex("Xilinx Error      : ",f->Dir);
      NewLine;
      PrintHex("Delay programmed  : ", f->Templ);
      NewLine;
      PrintHex("Actual delay      : ", f->Actual);
      NewLine;
    };
    PrintHex("Bad Counters Mask : " ,dpm->Info.Counter.Bad);
    NewLine;
  } else
    PrintStr(" OK!\n\r");

  ReturnToDebug();
}

/* eof */
