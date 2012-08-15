/**************************************************************************/
/* Command line stuff                                                     */
/**************************************************************************/

#define True 1
#define False 0

int Illegal();              /* llegal command */

int Quit();                 /* Quit test program  */
int Help();                 /* Help on commands   */
int History();              /* History            */
int Shell();                /* Shell command      */
int Sleep();                /* Sleep seconds      */
int Pause();                /* Pause keyboard     */
int Atoms();                /* Atom list commands */

int GetSetDebug();

int ChangeEditor();
int GetVersion();

int Levels();
int GetTemp();
int SetOnOff();
int Mute();
int SetAmpAddr();
int GetMeanTemperature();
int GetTurnOnCount();
int GetEventHistory();
int GetVoltageClockTempStat();
int GetLastEvent();
int GetWorkingTime();
int GetProtectCount();
int DeviceOnOff();
int ResetModule();
int SetFakeTemp();
int SetOutAtten();
int SetOutAttenVariable();
int GetWave();
int SetInputRoute();
int SetBrownOut();
int SetHiMains();
int SetLoMains();
int GetDspParams();
int EditDspParams();
int SetDspParams();
int WriteParamsFile();
int ReadParamsFile();
int DisplayDspParams();
int CompareParamsFile();

int GetStatus();
int ShowStatus();
int WriteStatusFile();
int ReadStatusFile();
int CompareStatusFile();

typedef enum {

   CmdNOCM,    /* llegal command */

   CmdQUIT,    /* Quit test program  */
   CmdHELP,    /* Help on commands   */
   CmdHIST,    /* History            */
   CmdSHELL,   /* Shell command      */
   CmdSLEEP,   /* Sleep seconds      */
   CmdPAUSE,   /* Pause keyboard     */
   CmdATOMS,   /* Atom list commands */
   CmdDEBUG,   /* Get/Set debug level */
   CmdCHNGE,   /* Change editor to next program */
   CmdAMPADDR, /* Set amplifier address */

   CmdVER,     /* Get versions */
   CmdILEV,    /* Get input levels */
   CmdRST,     /* Read status */
   CmdTEMP,    /* Get temp */
   CmdMTEMP,   /* Get mean temperature */
   CmdONOFF,   /* Switch On or Off */
   CmdMUTE,    /* Mute channels */
   CmdEVH,     /* Get event history */
   CmdLEV,     /* Get Last event */
   CmdTURNON,  /* Get turn on count */
   CmdVCLTS,   /* Voltage, Clock, Temp and LED status */
   CmdWKTM,    /* Get device working time */
   CmdPCNT,    /* Get amplifier protection count */
   CmdDEVONOF, /* Turns a device on or off */
   CmdRESETMOD,/* Reset an amplifier module */
   CmdSETFKTM, /* Set fake temperature */
   CmdSETOATEN,/* Set output attenuation */
   CmdSETVATEN,/* Set output attenuation variable chanel count */
   CmdGETWAVE, /* Get wave samples for plotting */
   CmdSETIROT, /* Set input routing */
   CmdSETBROWN,/* Enable/Disable brownout detection */
   CmdSETHI,   /* Set high mains calibration value */
   CmdSETLO,   /* Set low mains calibration value */
   CmdGETDSP,  /* Get a DSP parameter block */
   CmdSETDSP,  /* Set a DSP parameter block */
   CmdEDITDSP, /* Edit live DSP parameters */
   CmdWFDSP,   /* Write DSP binary data to file */
   CmdRFDSP,   /* Read DSP binary data from file */
   CmdDDSPMEM, /* Display DSP settings in memory */
   CmdCMPDSP,  /* Compare DSP settings with file */
   CmdDSTAT,   /* Display Status */
   CmdWFSTAT,  /* Write status to file */
   CmdRFSTAT,  /* Read file status */
   CmdCMPSTAT, /* Compare status with file */

   CmdCMDS } CmdId;

typedef struct {
   CmdId  Id;
   char  *Name;
   char  *Help;
   char  *Optns;
   int  (*Proc)(); } Cmd;

static Cmd cmds[CmdCMDS] = {

   { CmdNOCM,   "???",    "Illegal command"               ,""             ,Illegal },

   { CmdQUIT,    "q" ,    "Quit test program"             ,""             ,Quit  },
   { CmdHELP,    "h",     "Help on commands"              ,""             ,Help  },
   { CmdHIST,    "his",   "History"                       ,""             ,History},
   { CmdSHELL,   "sh",    "Shell command"                 ,"UnixCmd"      ,Shell },
   { CmdSLEEP,   "sleep", "Sleep seconds"                 ,"Seconds"      ,Sleep },
   { CmdPAUSE,   "pause", "Pause keyboard"                ,""             ,Pause },
   { CmdATOMS,   "alist", "Atom list commands"            ,""             ,Atoms },
   { CmdDEBUG,   "deb",   "Get/Set debug level"           ,"0|1|2"        ,GetSetDebug  },
   { CmdCHNGE,   "ce",    "Change text editor used"       ,""             ,ChangeEditor },
   { CmdAMPADDR, "mod",   "Select amplifier"              ,"1|2"          ,SetAmpAddr   },

   { CmdVER,     "I",     "Get versions"                  ,""             ,GetVersion   },
   { CmdILEV,    "L",     "Get levels"                    ,""             ,Levels   },
   { CmdRST,     "S",     "Read status"                   ,""             ,GetStatus    },
   { CmdTEMP,    "St",    "Get temperatures"              ,""             ,GetTemp      },
   { CmdMTEMP,   "B",     "Get mean temperatures"         ,"?|period"     ,GetMeanTemperature },
   { CmdONOFF,   "c",     "Switch On or Off"              ,"0|1"          ,SetOnOff     },
   { CmdMUTE,    "m",     "Mute channels"                 ,"0|1"          ,Mute         },
   { CmdEVH,     "E",     "Get Event history"             ,"?|count,Id"   ,GetEventHistory },
   { CmdLEV,     "U",     "Get last event"                ,"?"            ,GetLastEvent },
   { CmdTURNON,  "A",     "Get power on counter"          ,"?"            ,GetTurnOnCount },
   { CmdVCLTS,   "T",     "Volt/Clock/Temp/LED stat"      ,"?"            ,GetVoltageClockTempStat },
   { CmdWKTM,    "N",     "Get device working time"       ,"?"            ,GetWorkingTime },
   { CmdPCNT,    "P",     "Get protection count"          ,"?"            ,GetProtectCount },
   { CmdDEVONOF, "p",     "Turns a device on or off"      ,"?|1|0"        ,DeviceOnOff },
   { CmdRESETMOD,"r",     "Reset an amplifier module"     ,"?|1|2|3"      ,ResetModule },
   { CmdSETFKTM, "t",     "Set fake temperature"          ,"?|Temp"       ,SetFakeTemp },
   { CmdSETOATEN,"v",     "Set out attenuation"           ,"?|Chn,Aten"   ,SetOutAtten },
   { CmdSETVATEN,"w",     "Set out atten var chan"        ,"?|<AtenList>" ,SetOutAttenVariable },
   { CmdGETWAVE, "D",     "Get wave samples for plot"     ,"?|Fr,In,Ot,Cm",GetWave },
   { CmdSETIROT, "Z",     "Set input routing"             ,"?|Route"      ,SetInputRoute },
   { CmdSETBROWN,"b",     "Enb/Dis brownout detect"       ,"?|0|1"        ,SetBrownOut },
   { CmdSETHI,   "hh",    "Set high mains calib"          ,"?|Volts"      ,SetHiMains },
   { CmdSETLO,   "o",     "Set low mains calib"           ,"?|Volts"      ,SetLoMains },
   { CmdGETDSP,  "dC",    "Get a DSP parameter block"     ,"?|Block"      ,GetDspParams },
   { CmdSETDSP,  "dj",    "Set a DSP parameter block"     ,"?|Block"      ,SetDspParams },
   { CmdEDITDSP, "ded",   "Edit live DSP parameters"      ,"?|Address"    ,EditDspParams },
   { CmdWFDSP,   "wfd",   "Write DSP binary data to file" ,"?|Block,File" ,WriteParamsFile },
   { CmdRFDSP,   "rfd",   "Read DSP binary data from file","?|Block,File" ,ReadParamsFile },
   { CmdDDSPMEM, "dsp",   "Display DSP settings in memory","?|Block"      ,DisplayDspParams },
   { CmdCMPDSP,  "cfd",   "Compare DSP settings with file","?|Block,File" ,CompareParamsFile },
   { CmdDSTAT,   "dst",   "Display Status"                ,"?|File"       ,ShowStatus },
   { CmdWFSTAT,  "wfs",   "Write status to file"          ,"?|File"       ,WriteStatusFile },
   { CmdRFSTAT,  "rfs",   "Read status from file"         ,"?|File"       ,ReadStatusFile },
   { CmdCMPSTAT, "cfs",   "Compare status to file"        ,"?|File"       ,CompareStatusFile }

 };

typedef enum {

   OprNOOP,

   OprNE,  OprEQ,  OprGT,  OprGE,  OprLT , OprLE,   OprAS,
   OprPL,  OprMI,  OprTI,  OprDI,  OprAND, OprOR,   OprXOR,
   OprNOT, OprNEG, OprLSH, OprRSH, OprINC, OprDECR, OprPOP,
   OprSTM,

   OprOPRS } OprId;

typedef struct {
   OprId  Id;
   char  *Name;
   char  *Help; } Opr;

static Opr oprs[OprOPRS] = {
   { OprNOOP, "?"  ,"Not an operator"       },
   { OprNE,   "#"  ,"Not equal"             },
   { OprEQ,   "="  ,"Equal"                 },
   { OprGT,   ">"  ,"Greater than"          },
   { OprGE,   ">=" ,"Greater than or equal" },
   { OprLT,   "<"  ,"Less than"             },
   { OprLE,   "<=" ,"Less than or equal"    },
   { OprAS,   ":=" ,"Becomes equal"         },
   { OprPL,   "+"  ,"Add"                   },
   { OprMI,   "-"  ,"Subtract"              },
   { OprTI,   "*"  ,"Multiply"              },
   { OprDI,   "/"  ,"Divide"                },
   { OprAND,  "&"  ,"AND"                   },
   { OprOR,   "!"  ,"OR"                    },
   { OprXOR,  "!!" ,"XOR"                   },
   { OprNOT,  "##" ,"Ones Compliment"       },
   { OprNEG,  "#-" ,"Twos compliment"       },
   { OprLSH,  "<<" ,"Left shift"            },
   { OprRSH,  ">>" ,"Right shift"           },
   { OprINC,  "++" ,"Increment"             },
   { OprDECR, "--" ,"Decrement"             },
   { OprPOP,  ";"  ,"POP"                   },
   { OprSTM,  "->" ,"PUSH"                  } };

static char atomhash[256] = {
  10,9,9,9,9,9,9,9,9,0,0,9,9,0,9 ,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  0 ,1,9,1,9,4,1,9,2,3,1,1,0,1,11,1,5,5,5,5,5,5,5,5,5,5,1,1,1,1,1,1,
  10,6,6,6,6,6,6,6,6,6,6,6,6,6,6 ,6,6,6,6,6,6,6,6,6,6,6,6,7,9,8,9,6,
  9 ,6,6,6,6,6,6,6,6,6,6,6,6,6,6 ,6,6,6,6,6,6,6,6,6,6,6,6,9,9,9,9,9,
  9 ,9,9,9,9,9,9,9,9,9,9,9,9,9,9 ,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9 ,9,9,9,9,9,9,9,9,9,9,9,9,9,9 ,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9 ,9,9,9,9,9,9,9,9,9,9,9,9,9,9 ,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9 ,9,9,9,9,9,9,9,9,9,9,9,9,9,9 ,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9 };

typedef enum {
   Seperator=0,Operator=1,Open=2,Close=3,Comment=4,Numeric=5,Alpha=6,
   Open_index=7,Close_index=8,Illegal_char=9,Terminator=10,Bit=11,
 } AtomType;

#define MAX_ARG_LENGTH  32
#define MAX_ARG_COUNT   16
#define MAX_ARG_HISTORY 16

typedef struct {
   int      Pos;
   int      Number;
   AtomType Type;
   char     Text[MAX_ARG_LENGTH];
   CmdId    CId;
   OprId    OId;
} ArgVal;

static int pcnt = 0;
static ArgVal val_bufs[MAX_ARG_HISTORY][MAX_ARG_COUNT];
static ArgVal *vals = val_bufs[0];

#if 0
#define True 1
#define False 0
#endif
