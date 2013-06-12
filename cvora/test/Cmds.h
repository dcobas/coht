/**************************************************************************/
/* Command line stuff                                                     */
/**************************************************************************/

#ifndef CMDS_H
#define CMDS_H

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
int Batch();

int EditDrvName();
int EditLun();
int EditDmaFlag();
int EditWindowNumber();
int EditOffset();
int OpenLun();

int EditDebug();
int GetVersion();
int EditTimeout();
int WaitLun();
int DisplayDevicePars();
int RawRead();
int RawWrite();
int EditMem();
int DoInterrupt();
int EditRegs();

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

   CmdCHNGE,   /* Change editor to next program */
   CmdCD,      /* Change working directory */
   CmdBATCH,   /* Set batch mode on off */

   CmdDRVNM,   /* Edit driver name */
   CmdLUN,     /* Edit Logical Unit Number */
   CmdDMAFL,   /* Edit raw DMA memory IO */
   CmdWIN,     /* Edit window number */
   CmdOFFSET,  /* Edit block offset */
   CmdOPEN,    /* Open LUN */

   CmdDEBUG,   /* Edit LUN debug level */
   CmdGETVER,  /* Get version */
   CmdTMOUT,   /* Edit timeout */
   CmdDOINT,   /* Do an interrupt now */
   CmdWAIT,    /* Wait interrupt */
   CmdDEVP,    /* Display device parameters */
   CmdRMEM,    /* Read from device to memory */
   CmdWMEM,    /* Write from memory to device */
   CmdEMEM,    /* Edit memory */
   CmdEREG,    /* Edit registers */

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

   { CmdCHNGE,   "ce",    "Change text editor used"  ,""                   ,ChangeEditor },
   { CmdCD,      "cd",    "Change working directory" ,""                   ,ChangeDirectory },
   { CmdBATCH,   "bat",   "Set batch mode on off"    ,"1|0"                ,Batch   },

   { CmdDRVNM,   "drvnm", "Edit driver name"         ,"Name"               ,EditDrvName       },
   { CmdLUN,     "lun",   "Edit Logical Unit Number" ,"lun"                ,EditLun           },
   { CmdDMAFL,   "dma",   "Edit DMA flags"           ,"dma,swap"           ,EditDmaFlag       },
   { CmdWIN,     "win",   "Edit window number"       ,"window"             ,EditWindowNumber  },
   { CmdOFFSET,  "offs",  "Edit block offset reg"    ,"regcount"           ,EditOffset        },
   { CmdOPEN,    "opn",   "Open LUN"                 ,""                   ,OpenLun           },

   { CmdDEBUG,   "deb",   "Edit LUN debug level"     ,"level"              ,EditDebug         },
   { CmdGETVER,  "ver",   "Get version"              ,""                   ,GetVersion        },
   { CmdTMOUT,   "tmo",   "Edit timeout"             ,"jiffels"            ,EditTimeout       },
   { CmdDOINT,   "doi",   "Make an interrupt now"    ,"mask"               ,DoInterrupt       },
   { CmdWAIT,    "wi",    "Wait interrupt"           ,""                   ,WaitLun           },
   { CmdDEVP,    "dis",   "Display device params"    ,""                   ,DisplayDevicePars },
   { CmdRMEM,    "rmem",  "Read from dev to memory"  ,"start,count"        ,RawRead           },
   { CmdWMEM,    "wmem",  "Write from memory to dev" ,"start,count"        ,RawWrite          },
   { CmdEMEM,    "emem",  "Alloc/Set/Edit memory"    ,"start/count,value"  ,EditMem           },
   { CmdEREG,    "ereg",  "Edit registers"           ,"reg|name,val"       ,EditRegs          }

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

#endif
