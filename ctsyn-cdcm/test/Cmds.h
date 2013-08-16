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

int GetVersion();           /* Get Drvr/Fmwr version     */
int GetSetTmo();            /* Get/Set wait timeout      */
int GetSetQue();            /* Get/Set queue flag        */
int GetSetModule();         /* Get/List or Set modules   */
int NextModule();           /* Go to the next module     */
int PrevModule();           /* Go to the previous module */
int ResetMod();             /* Reset current module      */
int GetStatusMod();         /* Get & Clear Status        */
int GetClientConnections(); /* Get client connections    */
int WaitInterrupt();        /* Wait for Interrupts       */

int ReloadJtag();           /* Reload VHDL bit stream */
int MemIo();                /* Raw memory IO          */

int GetSetEnable();         /* Enable/Disable outputs */
int ReSync();               /* Resync PPS to external PPS */
int GetSetDelay();          /* Get/Set output delay */
int GetSetSyncPeriod();     /* Basic period length */
int GetPll();               /* Get PLL parameters */
int SetPllPhase();          /* Set the PLL phase */
int SetPllNumAverage();     /* Set numeric average */
int SetPllKP();             /* Set constant of proportionality */
int SetPllKI();             /* Set constant of integration */
int SetPllIntegrator();     /* Set the PLL integrator */
int SetPllAsPrdNs();        /* Set the PLL async oscillator period */
int GetPps();               /* Get PPS data */

int Plot();                 /* Plot PLL curves */

static void IErr();

/* Jtag backend to public code */

FILE *inp;

void setPort(short p, short val);
void readByte();
unsigned char readTDOBit();
void waitTime();
void pulseClock();

/* Commands */

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

   CmdVER,     /* Get Drvr/Fmwr version     */
   CmdTMO,     /* Get/Set wait timeout      */
   CmdQUE,     /* Get/Set queue flag        */
   CmdMOD,     /* Get/List or Set modules   */
   CmdNXM,     /* Go to the next module     */
   CmdPRM,     /* Go to the previous module */
   CmdRSM,     /* Reset current module      */
   CmdRST,     /* Get & Clear Status        */
   CmdCLS,     /* Get client connections    */
   CmdWAI,     /* Wait for Interrupts       */

   CmdEN,      /* Enable/Disable outputs */
   CmdRSYN,    /* Resync PPS to external PPS */
   CmdDELAY,   /* Get/Set output delay */
   CmdSYNP,    /* Sync period */
   CmdPLL,     /* Get PLL parameters */
   CmdPHASE,   /* Set the PLL phase */
   CmdNUM_AV,  /* Set numeric average */
   CmdKP,      /* Set constant of proportionality */
   CmdKI,      /* Set constant of integration */
   CmdINTG,    /* Set integrator */
   CmdAPN,     /* Set asynchronous period ns */
   CmdPLOT,    /* Plot PLL curve */

   CmdPPS,     /* Get PPS data */

   CmdJTAG,    /* Reload VHDL bit stream */
   CmdDM,      /* Raw memory IO          */

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

   { CmdVER,     "ver",   "Get Drvr/Fmwr version"    ,""                   ,GetVersion},
   { CmdTMO,     "tmo",   "Get/Set wait timeout"     ,"USeconds"           ,GetSetTmo},
   { CmdQUE,     "qf",    "Get/Set queue flag"       ,"1/0"                ,GetSetQue},
   { CmdMOD,     "mo",    "Get/List or Set modules"  ,"ModuleNum"          ,GetSetModule},
   { CmdNXM,     "nm",    "Go to the next module"    ,""                   ,NextModule},
   { CmdPRM,     "pm",    "Go to the previous module",""                   ,PrevModule},
   { CmdRSM,     "reset", "Reset current module"     ,""                   ,ResetMod},
   { CmdRST,     "rst",   "Get & Clear Status"       ,""                   ,GetStatusMod},
   { CmdCLS,     "cl",    "Get client connections"   ,""                   ,GetClientConnections},
   { CmdWAI,     "wi",    "Wait for Interrupts"      ,"0/Mask"             ,WaitInterrupt},

   { CmdEN,      "en",    "Enable/Disable outputs"   ,"1/0"                ,GetSetEnable},
   { CmdRSYN,    "sync",  "Resync PPS to extrn PPS"  ,""                   ,ReSync},
   { CmdDELAY,   "dly",   "Get/Set output delay"     ,"Delay"              ,GetSetDelay},
   { CmdSYNP,    "period","Get/Set sync period"      ,"Ms"                 ,GetSetSyncPeriod},
   { CmdPLL,     "pll",   "Get PLL parameters"       ,""                   ,GetPll},
   { CmdPHASE,   "phs",   "Set PLL phase"            ,"Phase"              ,SetPllPhase},
   { CmdNUM_AV,  "nav",   "Set PLL numeric average"  ,"NumAv"              ,SetPllNumAverage},
   { CmdKP,      "kp",    "Set PLL K proportion"     ,"KP"                 ,SetPllKP},
   { CmdKI,      "ki",    "Set PLL K integration"    ,"KI"                 ,SetPllKI},
   { CmdINTG,    "intg",  "Set PLL integrator value" ,"INTG"               ,SetPllIntegrator},
   { CmdAPN,     "apn",   "Set async period ns"      ,"float"              ,SetPllAsPrdNs},
   { CmdPLOT,    "plot",  "Plot PLL curves"          ,"Nav,Kp,Ki,Points"   ,Plot},
   { CmdPPS,     "pps",   "Get PPS data"             ,""                   ,GetPps},

   { CmdJTAG,    "jtag",  "Reload VHDL bit stream"   ,"Path"               ,ReloadJtag},
   { CmdDM,      "io",    "Raw memory IO"            ,"Address"            ,MemIo} };

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
  0 ,1,9,1,9,4,1,9,2,3,1,1,0,1,11,1,5,5,5,5,5,5,5,5,5,5,1,1,1,1,1,9,
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

#define True 1
#define False 0
