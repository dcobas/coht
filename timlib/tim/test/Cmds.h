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
int GetVersion();

int WaitInterrupt();
int GetSetQue();
int GetSetTmo();
int GetSetPtim();
int SimulateInterrupt();
int GetSetRemote();
int GetUtc();
int WaitTelegram();
int GetTelegram();
int LineNames();
int GetSetModule();
int GetStatus();
int GetSetPll();
int NextModule();
int GetSetCtim();
int SwDeb();
int CtimRead();
int DoIo();
int SetRemoteCmd();
int GetSetCounter();
int NextCounter();
int Video();
int Config();
int LaunchHwTest();
int LaunchLookat();
int LaunchClock();
int ParseCycleString();
int ShowUserMat();
int ConnectCTime();

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
   CmdCST,   /* Convert cycle string */

   CmdWI,    /* Wait for an interrupt */
   CmdCC,    /* Connect to a C time */
   CmdQF,    /* Get set the queue flag */
   CmdTMO,   /* Get set timeout */
   CmdPTM,   /* Edit PTIM equipments */
   CmdSI,    /* Simulate an interrupt */
   CmdDOIO,  /* Use CTR as an IO register */
   CmdREM,   /* Remote control a counter */
   CmdRCM,   /* Send a remote command */
   CmdCNF,   /* Edit counter configuration */
   CmdUTC,   /* Get UTC time */
   CmdWTGM,  /* Wait telegram */
   CmdGTGM,  /* Get telegram */
   CmdLNAM,  /* Line names */
   CmdUMAT,  /* User Matrix */
   CmdMOD,   /* Show modules */
   CmdNM,    /* Next module */
   CmdCNT,   /* Get or set current counter */
   CmdNC,    /* Next counter */
   CmdRST,   /* Read status */
   CmdPLL,   /* Get set PLL locking method */
   CmdCTM,   /* Edit CTIM objects */
   CmdDEB,   /* Get set debug level */
   CmdCLD,   /* Load CTIM definitions */
   CmdVID,   /* Launch Tgm Video */
   CmdHWT,   /* Launch hardware test program */
   CmdLKM,   /* Launch lookat a timing */
   CmdCLK,   /* Launch clock */

   CmdCMDS } CmdId;

typedef struct {
   CmdId  Id;
   char  *Name;
   char  *Help;
   char  *Optns;
   int  (*Proc)(); } Cmd;

static Cmd cmds[CmdCMDS] = {

   { CmdNOCM,   "???",    "Illegal command"          ,""                   ,Illegal },

   { CmdQUIT,    "q" ,    "Quit test program"        ,""                   ,Quit    },
   { CmdHELP,    "h" ,    "Help on commands"         ,""                   ,Help    },
   { CmdNEWS,    "news",  "Show GMT test news"       ,""                   ,News    },
   { CmdHIST,    "his",   "History"                  ,""                   ,History },
   { CmdSHELL,   "sh",    "Shell command"            ,"UnixCmd"            ,Shell   },
   { CmdSLEEP,   "s" ,    "Sleep seconds"            ,"Seconds"            ,Sleep   },
   { CmdPAUSE,   "z" ,    "Pause keyboard"           ,""                   ,Pause   },
   { CmdATOMS,   "a" ,    "Atom list commands"       ,""                   ,Atoms   },

   { CmdCE,      "ce",    "Change text editor"       ,""                   ,ChangeEditor      },
   { CmdCD,      "cd",    "Change configuration"     ,"Path"               ,ChangeDirectory   },
   { CmdVER,     "ver",   "Get TimLib version"       ,""                   ,GetVersion        },
   { CmdCST,     "cstr",  "Convert cycle string"     ,"mch.grp.val"        ,ParseCycleString  },

   { CmdWI,      "wi",    "Wait for an interrupt"    ,"?|P<n>|C[M]<n>|Msk<t>" ,WaitInterrupt  },
   { CmdCC,      "cc",    "Connect to a C-event time","payload"            ,ConnectCTime      },
   { CmdQF,      "qf",    "Get set the queue flag"   ,"1/0"                ,GetSetQue         },
   { CmdTMO,     "tmo",   "Get set timeout"          ,"Timeout"            ,GetSetTmo         },
   { CmdPTM,     "ptm",   "Edit PTIM equipments"     ,"?|PtimId"           ,GetSetPtim        },
   { CmdSI,      "si",    "Simulate an interrupt"    ,"?|T<n>P<n>|C<n>[pld]|Msk",SimulateInterrupt },
   { CmdDOIO,    "io",    "Use CTR as an IO reg"     ,"Lemos,Mask"         ,DoIo              },
   { CmdREM,     "rem",   "Remote control a counter" ,"?|[<Flg>]"          ,GetSetRemote      },
   { CmdRCM,     "rcm",   "Send a remote command"    ,"?|<Cmd>"            ,SetRemoteCmd      },
   { CmdCNF,     "cnf",   "Configure remote counter" ,""                   ,Config            },
   { CmdUTC,     "utc",   "Get UTC (Real/Adjusted)"  ,"R|A"                ,GetUtc            },
   { CmdWTGM,    "wtgm",  "Wait telegram"            ,"?|[<Mch>|<n>]"      ,WaitTelegram      },
   { CmdGTGM,    "gtgm",  "Get telegram"             ,"?|[<Mch>|<n>]"      ,GetTelegram       },
   { CmdLNAM,    "stgm",  "Show telegram layout"     ,"?|[<Mch>|<n>]"      ,LineNames         },
   { CmdUMAT,    "umat",  "Show User Matrix"         ,"?|[<Mch>|<n>]"      ,ShowUserMat       },
   { CmdMOD,     "mo",    "Get set module"           ,"[<Module>]"         ,GetSetModule      },
   { CmdNM,      "nm",    "Next Module"              ,""                   ,NextModule        },
   { CmdCNT,     "cnt",   "Get set Counter"          ,"[<Counter>]"        ,GetSetCounter     },
   { CmdNC,      "nc",    "Next Counter"             ,""                   ,NextCounter       },
   { CmdRST,     "rst",   "Read module Status"       ,""                   ,GetStatus         },
   { CmdPLL,     "upll",  "UTC PLL lock Brutal/Slow" ,"?|0|1"              ,GetSetPll         },

   { CmdCTM,     "ctm",   "Edit CTIM objects"        ,"?|CtimId"           ,GetSetCtim           },
   { CmdDEB,     "deb",   "Get set debug level"      ,"Level"              ,SwDeb             },
   { CmdCLD,     "ctmr",  "Load CTIM definitions"    ,""                   ,CtimRead          },
   { CmdVID,     "vid",   "Launch telegram video"    ,"?|Machine"          ,Video             },
   { CmdHWT,     "hwt",   "Launch hw-test program"   ,""                   ,LaunchHwTest      },
   { CmdLKM,     "lkm",   "Launch look at timing"    ,"P|C|H <eqp>"        ,LaunchLookat      },
   { CmdCLK,     "clk",   "Launch timing clock"      ,"[<Module>]"         ,LaunchClock       } };

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
