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
int List();                 /* List files         */
int Clean();                /* Clean files        */

int ChangeEditor();
int ChangeDirectory();
int GetVersion();
int RemGetVersion();

int SaveDscList();
int LoadDscList();
int ClearDsc();
int ClearAllDscs();

int GetSetDsc();
int NextDsc();
int GetStatus();
int KillSurvey();
int PingSurvey();
int StartSurvey();
int ResetSurvey();
int GetCcv();
int GetAqn();
int GetLsp();
int RemGetStatus();
int RemGetCcv();
int RemGetAqn();
int RemGetLsp();
int GetSetTxt();
int SaveAqn();
int RestAqn();
int SaveCcv();
int RestCcv();
int CompareAqn();
int CompareCcv();
int GetSetSampling();
int GetHelp();
int GetSetDebug();

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
   CmdLS,    /* List files */
   CmdRM,    /* Remove files */
   CmdVER,   /* Get all versions */
   CmdRVER,  /* Remote get all versions */

   CmdDSC,   /* Select target DSC */
   CmdND,    /* Next DSC */
   CmdSDSCL, /* Save list of DSCs to file */
   CmdLDSCL, /* Load list of DSCs from file */
   CmdXDSC,  /* Delete current DSC from list */
   CmdXDSCL, /* Delete DSC list */

   CmdRST,   /* Read target status */
   CmdKIL,   /* Kill survey daemons */
   CmdPNG,   /* Ping daemon on DSC */
   CmdSTD,   /* Start daemon on DSC */
   CmdXEH,   /* Delete/Clear Error history */
   CmdCCV,   /* Read target control values */
   CmdAQN,   /* Read target acquisition values */
   CmdLSP,   /* Read target PTIM list */
   CmdTXT,   /* Set target text mode */
   CmdSMP,   /* Get/Set AQN sampling */
   CmdRH,    /* Get remote help, ping */
   CmdRD,    /* Get/Set remote debug printing */

   CmdRRST,  /* Remote read target status */
   CmdRCCV,  /* Remote read target control values */
   CmdRAQN,  /* Remote read target acquisition values */
   CmdRLSP,  /* Remote read target PTIM list */

   CmdSAQN,  /* Save aqn in reference */
   CmdOAQN,  /* Restore aqn from reference */
   CmdSCCV,  /* Save ccv in reference */
   CmdOCCV,  /* Restore ccv from reference */

   CmdCAQN,  /* Compare aqn with reference */
   CmdCCCV,  /* Compare ccv with reference */

   CmdCMDS } CmdId;

typedef struct {
   CmdId  Id;
   char  *Name;
   char  *Help;
   char  *Optns;
   int  (*Proc)(); } Cmd;

static Cmd cmds[CmdCMDS] = {

   { CmdNOCM,   "???",    "Illegal command"            ,""        ,Illegal },

   { CmdQUIT,    "q" ,    "Quit test program"          ,""        ,Quit    },
   { CmdHELP,    "h" ,    "Help on commands"           ,""        ,Help    },
   { CmdNEWS,    "news",  "Show GMT test news"         ,""        ,News    },
   { CmdHIST,    "his",   "History"                    ,""        ,History },
   { CmdSHELL,   "sh",    "Shell command"              ,"UnixCmd" ,Shell   },
   { CmdSLEEP,   "s" ,    "Sleep seconds"              ,"Seconds" ,Sleep   },
   { CmdPAUSE,   "z" ,    "Pause keyboard"             ,""        ,Pause   },
   { CmdATOMS,   "a" ,    "Atom list commands"         ,""        ,Atoms   },

   { CmdCE,      "ce",    "Change text editor"         ,""        ,ChangeEditor    },
   { CmdCD,      "cd",    "Change configuration"       ,"Path"    ,ChangeDirectory },
   { CmdLS,      "ls",    "List files in /tmp"         ,""        ,List            },
   { CmdRM,      "clean", "Clean files from /tmp"      ,""        ,Clean           },
   { CmdVER,     "ver",   "Get all versions"           ,""        ,GetVersion      },
   { CmdRVER,    "rver",  "Remote get all versions"    ,""        ,RemGetVersion   },

   { CmdDSC,     "dsc",   "Select target DSC"          ,"Name"    ,GetSetDsc    },
   { CmdND,      "nd",    "Next DSC"                   ,""        ,NextDsc      },
   { CmdSDSCL,   "sdscl", "Save DSC list to file"      ,"Path"    ,SaveDscList  },
   { CmdLDSCL,   "ldscl", "Load DSC list from file"    ,"Path"    ,LoadDscList  },
   { CmdXDSC,    "xdsc",  "Delete DSC from list"       ,""        ,ClearDsc     },
   { CmdXDSCL,   "xdscl", "Delete DSC list"            ,""        ,ClearAllDscs },

   { CmdRST,     "rst",   "Read target status"         ,""        ,GetStatus   },
   { CmdKIL,     "kill",  "Kill survey daemons on DSCs",""        ,KillSurvey  },
   { CmdPNG,     "ping",  "Ping survey daemon on DSC"  ,""        ,PingSurvey  },
   { CmdSTD,     "start", "Start survey daemon on DSC" ,""        ,StartSurvey },
   { CmdXEH,     "xeh",   "Delete/Clear Error history" ,""        ,ResetSurvey },

   { CmdCCV,     "ccv",   "Read target control"        ,"Mask"    ,GetCcv     },
   { CmdAQN,     "aqn",   "Read target acquisition"    ,""        ,GetAqn     },
   { CmdLSP,     "lsp",   "Read target PTIM list"      ,""        ,GetLsp     },
   { CmdTXT,     "txt",   "Set target text mode"       ,"0|1"     ,GetSetTxt  },
   { CmdSMP,     "smp",   "Get/Set AQN sampling"       ,"Mask"    ,GetSetSampling },
   { CmdRH,      "rh",    "Get remote help, ping DSCs" ,""        ,GetHelp        },
   { CmdRD,      "deb",   "Get/Set remote debug print" ,"0|1"     ,GetSetDebug    },

   { CmdRRST,    "rrst",  "Remote read status"         ,""        ,RemGetStatus },
   { CmdRCCV,    "rccv",  "Remote read ccvs"           ,""        ,RemGetCcv    },
   { CmdRAQN,    "raqn",  "Remote read aqns"           ,""        ,RemGetAqn    },
   { CmdRLSP,    "rlsp",  "Remote read ptms"           ,""        ,RemGetLsp    },

   { CmdSAQN,    "saqn",  "Save aqn in reference"        ,""        ,SaveAqn    },
   { CmdOAQN,    "oaqn",  "Overwrite aqn from reference" ,""        ,RestAqn    },
   { CmdSCCV,    "sccv",  "Save ccv in reference"        ,""        ,SaveCcv    },
   { CmdOCCV,    "occv",  "Overwrite ccv from reference" ,""        ,RestCcv    },

   { CmdCAQN,    "caqn",  "Compare aqn with reference" ,""        ,CompareAqn   },
   { CmdCCCV,    "cccv",  "Compare ccv with reference" ,""        ,CompareCcv } };

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
