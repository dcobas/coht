/**************************************************************************/
/* Command line stuff                                                     */
/**************************************************************************/

int Illegal();              /* llegal command */

int Quit();                 /* Quit test program  */
int Help();                 /* Help on commands   */
int News();                 /* Show GMT test news */
int History();              /* History            */
int Shell();                /* Shell command      */
int Sleep();                /* Sleep seconds      */
int Pause();                /* Pause keyboard     */
int Atoms();                /* Atom list commands */

int ChangeEditor();
int ChangeDirectory();
int SwDeb();
int MemIo();
int SinWave();
int PutFunc();
int PllConfig();
int LoadFpga();
int GetAqn();
int GetVersion();
int GetTemperature();
int ReadStatus();
int SetControl();
int SetArmed();

int NextFunc();
int PrevFunc();
int SetFunc();
int SetAqnFuncs();
int Plot();
int IncFreq();
int SetParams();
int TimeWindow();
int Scan();
int Calc();
int Macro();
int LowPass();
int Batch();

static void IErr();

/* Jtag backend to public code */

FILE *inp;

typedef enum {

   CmdNOCM,    /* llegal command */

   CmdQUIT,    /* Quit test program  */
   CmdHELP,    /* Help on commands   */
   CmdNEWS,    /* Show GMT test news */
   CmdHIST,    /* History            */
   CmdSHELL,   /* Shell command      */
   CmdSLEEP,   /* Sleep seconds      */
   CmdPAUSE,   /* Pause keyboard     */
   CmdATOMS,   /* Atom list commands */
   CmdMACRO,   /* Execute a macro    */
   CmdBATCH,   /* Control batch mode */

   CmdCHNGE,   /* Change editor to next program */
   CmdCD,      /* Change working directory */
   CmdDEBUG,   /* Get Set debug mode */
   CmdVER,     /* Get version */
   CmdTMP,     /* Get temperature */
   CmdRST,     /* Read status */
   CmdCTRL,    /* Set control */
   CmdARM,     /* Arm using ctrl + 0x51A002 */

   CmdMEMIO,   /* Raw memory IO */

   CmdINCF,    /* Increment frequency */
   CmdPRMS,    /* Sin wave parameters */
   CmdSIN,     /* Generate function */
   CmdLDF,     /* Load function */
   CmdAQN,     /* Acquire signal from sram */
   CmdPLAY,    /* Play configuration file */

   CmdSAF,     /* Set acquisition function number */
   CmdSAFC,    /* Set acquisition function count */
   CmdNAF,     /* Next acquisition function */
   CmdPAF,     /* Previous acquisition function */
   CmdPLOT,    /* GNU plot acquisition function */
   CmdTWIN,    /* Time window */
   CmdSCAN,    /* Scan acquisition file */
   CmdCALC,    /* Calculate Zeta and Phase angle */
   CmdLOWP,    /* Low pass filter */

   CmdLFPGA,   /* Load FPGA */

   CmdCMDS } CmdId;

typedef struct {
   CmdId  Id;
   char  *Name;
   char  *Help;
   char  *Optns;
   int  (*Proc)(); } Cmd;

static Cmd cmds[CmdCMDS] = {

   { CmdNOCM,   "???",    "Illegal command"          ,""                   ,Illegal },

   { CmdQUIT,    "q" ,    "Quit test program"        ,""                   ,Quit  },
   { CmdHELP,    "h" ,    "Help on commands"         ,""                   ,Help  },
   { CmdNEWS,    "news",  "Show GMT test news"       ,""                   ,News  },
   { CmdHIST,    "his",   "History"                  ,""                   ,History},
   { CmdSHELL,   "sh",    "Shell command"            ,"UnixCmd"            ,Shell },
   { CmdSLEEP,   "s" ,    "Sleep seconds"            ,"Seconds"            ,Sleep },
   { CmdPAUSE,   "z" ,    "Pause keyboard"           ,""                   ,Pause },
   { CmdATOMS,   "a" ,    "Atom list commands"       ,""                   ,Atoms },
   { CmdMACRO,   "do",    "Do command macro file"    ,"path"               ,Macro },
   { CmdBATCH,   "batch", "Batch mode confirmation"  ,"0|1"                ,Batch },

   { CmdCHNGE,   "ce",    "Change text editor used"  ,""                   ,ChangeEditor },
   { CmdCD,      "cd",    "Change working directory" ,""                   ,ChangeDirectory },
   { CmdDEBUG,   "deb",   "Get/Set driver debug mode","1|2|4/0"            ,SwDeb },
   { CmdVER,     "ver",   "Get version"              ,""                   ,GetVersion },
   { CmdTMP,     "tmp",   "Get temperature"          ,""                   ,GetTemperature },
   { CmdRST,     "rst",   "Read status"              ,""                   ,ReadStatus },
   { CmdCTRL,    "ctrl",  "Set(+)/Clear(-) control"  ,"+,-,ctrl|?"         ,SetControl },
   { CmdARM,     "arm",   "Arm using ctrl +0x51A002" ,""                   ,SetArmed   },

   { CmdMEMIO,   "rio",   "Raw memory IO"            ,"Address"            ,MemIo },
   { CmdINCF,    "incf",  "Increment frequency"      ,"Increment"          ,IncFreq },
   { CmdPRMS,    "prms",  "Set Sin wave parameters"  ,"?|fq,am,rt,tt,ft"   ,SetParams },
   { CmdSIN,     "sin",   "Generate function file"   ,"path"               ,SinWave },
   { CmdLDF,     "ldf",   "Load sin function file"   ,"path"               ,PutFunc },
   { CmdAQN,     "aqn",   "Acquire from sram to file","path"               ,GetAqn  },
   { CmdPLAY,    "play",  "Play a config file"       ,"path"               ,PllConfig },

   { CmdSAF,     "saf",   "Set aqn function number"  ,"fn"                 ,SetFunc },
   { CmdSAFC,    "safc",  "Set aqn function count"   ,"fc"                 ,SetAqnFuncs },
   { CmdNAF,     "naf",   "Next aqn function"        ,""                   ,NextFunc },
   { CmdPAF,     "paf",   "Previous aqn function"    ,""                   ,PrevFunc },
   { CmdPLOT,    "plot",  "GNU plot aqn function"    ,"path"               ,Plot },
   { CmdTWIN,    "twin",  "Set sampling time window" ,"low,high"           ,TimeWindow },
   { CmdSCAN,    "scan",  "Scan acquisitions"        ,""                   ,Scan },
   { CmdCALC,    "calc",  "Calculate Zeta, Phase"    ,"1,3,5"              ,Calc },
   { CmdLOWP,    "lowp",  "Apply low pass filter"    ,""                   ,LowPass },

   { CmdLFPGA,   "lfpga", "Load FPGA bit stream file","path"               ,LoadFpga }
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
   { OprNOOP, "?"  ,"???     Not an operator"       },
   { OprNE,   "#"  ,"Test:   Not equal"             },
   { OprEQ,   "="  ,"Test:   Equal"                 },
   { OprGT,   ">"  ,"Test:   Greater than"          },
   { OprGE,   ">=" ,"Test:   Greater than or equal" },
   { OprLT,   "<"  ,"Test:   Less than"             },
   { OprLE,   "<=" ,"Test:   Less than or equal"    },
   { OprAS,   ":=" ,"Assign: Becomes equal"         },
   { OprPL,   "+"  ,"Arith:  Add"                   },
   { OprMI,   "-"  ,"Arith:  Subtract"              },
   { OprTI,   "*"  ,"Arith:  Multiply"              },
   { OprDI,   "/"  ,"Arith:  Divide"                },
   { OprAND,  "&"  ,"Bits:   AND"                   },
   { OprOR,   "!"  ,"Bits:   OR"                    },
   { OprXOR,  "!!" ,"Bits:   XOR"                   },
   { OprNOT,  "##" ,"Bits:   Ones Compliment"       },
   { OprNEG,  "#-" ,"Arith:  Twos compliment"       },
   { OprLSH,  "<<" ,"Bits:   Left shift"            },
   { OprRSH,  ">>" ,"Bits:   Right shift"           },
   { OprINC,  "++" ,"Arith:  Increment"             },
   { OprDECR, "--" ,"Arith:  Decrement"             },
   { OprPOP,  ";"  ,"Stack:  POP"                   },
   { OprSTM,  "->" ,"Stack:  PUSH"                  } };

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

#ifndef True
#define True 1
#define False 0
#endif
