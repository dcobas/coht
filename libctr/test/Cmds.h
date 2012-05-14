/**************************************************************************/
/* Command line stuff                                                     */
/**************************************************************************/

int Illegal();              /* llegal command */

int Quit();                 /* Quit test program  */
int Help();                 /* Help on commands   */
int get_news();             /* Show GMT test news */
int History();              /* History            */
int Shell();                /* Shell command      */
int Sleep();                /* Sleep seconds      */
int Pause();                /* Pause keyboard     */
int Atoms();                /* Atom list commands */

int change_text_editor();
int change_working_directory();
int GetVersion();

int WaitInterrupt();
int GetSetQue();
int GetSetTmo();
int GetSetPtim();
int SimulateInterrupt();
int GetSetRemote();
int GetUtc();
int GetSetModule();
int GetStatus();
int NextModule();
int GetSetCtim();
int SwDeb();
int CtimRead();
int SetRemoteCmd();
int GetSetCounter();
int NextCounter();
int Config();
int ConnectCTime();
int GetSetEnable();
int GetTelegram();
int GetSetInputDelay();
int GetSetCableId();
int GetSetPllLockMethod();
int GetSetP2Byte();
int MemTest();
int ListClients();

/* Commands */

typedef enum {

   CmdNOCM,  /* llegal command */

   CmdQUIT,  /* Quit test program  */
   CmdHELP,  /* Help on commands   */
   CmdNEWS,  /* Show GMT test news */
   CmdHIST,  /* History            */
   CmdSHELL, /* Shell command      */
   CmdSLEEP, /* Sleep seconds      */
   CmdPAUSE, /* Pause keyboard     */
   CmdATOMS, /* Atom list commands */

   CmdCE,    /* Change text editor   */
   CmdCD,    /* Change configuration */
   CmdVER,   /* Get version */

   CmdWI,    /* Wait for an interrupt */
   CmdCL,    /* Clients list */
   CmdCC,    /* Connect to a C time */
   CmdQF,    /* Get set the queue flag */
   CmdTMO,   /* Get set timeout */
   CmdPTM,   /* Edit PTIM equipments */
   CmdSI,    /* Simulate an interrupt */
   CmdREM,   /* Remote control a counter */
   CmdRCM,   /* Send a remote command */
   CmdCNF,   /* Edit counter configuration */
   CmdUTC,   /* Get UTC time */
   CmdTGM,   /* Get telegram */
   CmdIND,   /* Edit input delay */
   CmdCID,   /* Edit cable ID */
   CmdPLK,   /* Edit PLL lock method */
   CmdP2B,   /* Edit P2 VME byte */
   CmdMTS,   /* Memory test */
   CmdMOD,   /* Show modules */
   CmdNM,    /* Next module */
   CmdCNT,   /* Get or set current counter */
   CmdNC,    /* Next counter */
   CmdRST,   /* Read status */
   CmdENB,   /* Get/Set enable */
   CmdCTM,   /* Edit CTIM objects */
   CmdDEB,   /* Get set debug level */

   CmdCMDS } CmdId;

typedef struct {
   CmdId  Id;
   char  *Name;
   char  *Help;
   char  *Optns;
   int  (*Proc)(); } Cmd;

static Cmd cmds[CmdCMDS] = {

   { CmdNOCM,   "???",    "Illegal command"          ,""                   ,Illegal },

   { CmdQUIT,    "q" ,    "Quit test program"        ,""                   ,Quit     },
   { CmdHELP,    "h" ,    "Help on commands"         ,""                   ,Help     },
   { CmdNEWS,    "news",  "Show GMT test news"       ,""                   ,get_news },
   { CmdHIST,    "his",   "History"                  ,""                   ,History  },
   { CmdSHELL,   "sh",    "Shell command"            ,"UnixCmd"            ,Shell    },
   { CmdSLEEP,   "s" ,    "Sleep seconds"            ,"Seconds"            ,Sleep    },
   { CmdPAUSE,   "z" ,    "Pause keyboard"           ,""                   ,Pause    },
   { CmdATOMS,   "a" ,    "Atom list commands"       ,""                   ,Atoms    },

   { CmdCE,      "ce",    "Change text editor"       ,""                   ,change_text_editor       },
   { CmdCD,      "cd",    "Change configuration"     ,"Path"               ,change_working_directory },
   { CmdVER,     "ver",   "Get TimLib version"       ,""                   ,GetVersion         },

   { CmdWI,      "wi",    "Wait for an interrupt"    ,"?|P<n>|C<n><n>|Msk<t>" ,WaitInterrupt  },
   { CmdCL,      "cl",    "Clients list connections" ,""                   ,ListClients       },
   { CmdCC,      "cc",    "Connect to a C-event time","payload"            ,ConnectCTime      },
   { CmdQF,      "qf",    "Get set the queue flag"   ,"1/0"                ,GetSetQue         },
   { CmdTMO,     "tmo",   "Get set timeout"          ,"Timeout"            ,GetSetTmo         },
   { CmdPTM,     "ltm",   "Edit LTIM equipments"     ,"?|LtimId"           ,GetSetPtim        },
   { CmdSI,      "si",    "Simulate an interrupt"    ,"?|T<n>P<n>|C<n>[pld]|Msk",SimulateInterrupt },
   { CmdREM,     "rem",   "Remote control a counter" ,"?|[<Flg>]"          ,GetSetRemote      },
   { CmdRCM,     "rcm",   "Send a remote command"    ,"?|<Cmd>"            ,SetRemoteCmd      },
   { CmdCNF,     "cnf",   "Configure remote counter" ,""                   ,Config            },
   { CmdUTC,     "utc",   "Get UTC (Real/Adjusted)"  ,"R|A"                ,GetUtc            },
   { CmdTGM,     "tgm",   "Get a telegram"           ,"num"                ,GetTelegram       },
   { CmdIND,     "ind",   "Get/Set input delay"      ,"inputdelay"         ,GetSetInputDelay  },
   { CmdCID,     "cid",   "Get/Set cable ID"         ,"cableid"            ,GetSetCableId     },
   { CmdPLK,     "plk",   "Get/Set PLL lock method"  ,"0=Brutal, 1=Slow"   ,GetSetPllLockMethod },
   { CmdP2B,     "oby",   "Get/Set P2 VME byte"      ,"Byte[0..8]"         ,GetSetP2Byte      },
   { CmdMTS,     "mem",   "Mem test (disable mod)"   ,""                   ,MemTest           },
   { CmdMOD,     "mo",    "Get set module"           ,"[<Module>]"         ,GetSetModule      },
   { CmdNM,      "nm",    "Next Module"              ,""                   ,NextModule        },
   { CmdCNT,     "ch",    "Get/Set Counter"          ,"[<Counter>]"        ,GetSetCounter     },
   { CmdNC,      "nc",    "Next Counter"             ,""                   ,NextCounter       },
   { CmdRST,     "rst",   "Read module Status"       ,""                   ,GetStatus         },
   { CmdENB,     "enb",   "Get/Set enable"           ,"flag"               ,GetSetEnable      },

   { CmdCTM,     "ctm",   "Edit CTIM objects"        ,"?|CtimId"           ,GetSetCtim        },
   { CmdDEB,     "deb",   "Get set debug level"      ,"Level"              ,SwDeb             },
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
