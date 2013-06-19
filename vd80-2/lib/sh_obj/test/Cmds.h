/**************************************************************************/
/* Command line stuff                                                     */
/**************************************************************************/

int Illegal();

int Quit();
int Help();
int News();
int History();
int Shell();
int Sleep();
int Pause();
int Atoms();

int get_set_dev();
int get_version();
int get_set_debug();
int reset();
int get_adc_value();
int get_state();
int get_set_clk();
int get_set_clk_div();
int get_set_trg();
int get_set_analog_trg();
int get_set_config_trg();
int get_set_buffer_size();
int strt_acquisition();
int trig_acquisition();
int stop_acquisition();
int connect();
int get_set_timeout();
int wait_interrupt();
int read_file();
int write_file();
int plot_samp();
int read_samp();
int print_samp();
int ms_sleep();
int raw_io();
int swap_buf();

/* Commands */

typedef enum {

   CmdNOCM,

   CmdQUIT,
   CmdHELP,
   CmdNEWS,
   CmdHIST,
   CmdSHELL,
   CmdSLEEP,
   CmdPAUSE,
   CmdATOMS,

   CmdLUN,   /** Choose logical unit */
   CmdVER,   /** Versions */
   CmdDEB,   /** Get/Set debug level */
   CmdRIO,   /** Raw IO */
   CmdRSET,  /** Reset */
   CmdADC,   /** ADC value */
   CmdSTATE, /** State */
   CmdSCLK,  /** Configure clock */
   CmdCLKDV, /** Clock divide */
   CmdSTRG,  /** Configure trigger */
   CmdSATRG, /** Analogue trigger */
   CmdSPTRG, /** Trigger pre configuration */
   CmdBSZE,  /** Allocate buffer */
   CmdSTR,   /** Start acquisition */
   CmdTRG,   /** Trigger acquisition */
   CmdSTP,   /** Stop acquisition */
   CmdCON,   /** Connect to interrupt */
   CmdTMO,   /** Timeout */
   CmdWI,    /** Wait for an interrupt */
   CmdRF,    /** Read file */
   CmdWF,    /** Write file */
   CmdPLT,   /** Plot file */
   CmdRSMP,  /** Read samples buf to mem */
   CmdPSMP,  /** Print samples buf */
   CmdSWP,   /** Swap words/bytes */
   CmdMS,    /** Milliseconds sleep */

   CmdCMDS } CmdId;

typedef struct {
   CmdId  Id;
   char  *Name;
   char  *Help;
   char  *Optns;
   int  (*Proc)();; } Cmd;

static Cmd cmds[CmdCMDS] = {

   { CmdNOCM,   "???",   "Illegal command"          ,""                   ,Illegal },

   { CmdQUIT,   "q" ,    "Quit test program"        ,""                   ,Quit  },
   { CmdHELP,   "h" ,    "Help on commands"         ,""                   ,Help  },
   { CmdNEWS,   "news",  "Show BST test news"       ,""                   ,News  },
   { CmdHIST,   "his",   "History"                  ,""                   ,History},
   { CmdSHELL,  "sh",    "Shell command"            ,"UnixCmd"            ,Shell },
   { CmdSLEEP,  "s" ,    "Sleep seconds"            ,"Seconds"            ,Sleep },
   { CmdPAUSE,  "z" ,    "Pause keyboard"           ,""                   ,Pause },
   { CmdATOMS,  "a" ,    "Atom list commands"       ,""                   ,Atoms },

   { CmdLUN,    "dev",   "Choose a device"          ,"dev"                ,get_set_dev },
   { CmdVER,    "ver",   "Versions"                 ,""                   ,get_version },
   { CmdDEB,    "deb",   "Get/Set debug level"      ,"lvl 0|1|2"          ,get_set_debug },
   { CmdRIO,    "rio",   "Raw IO 32-bit reg edit"   ,"regn"               ,raw_io },
   { CmdRSET,   "rset",  "Reset"                    ,""                   ,reset },
   { CmdADC,    "adc",   "ADC value"                ,"channel 1..16"      ,get_adc_value },
   { CmdSTATE,  "ste",   "State"                    ,""                   ,get_state },
   { CmdSCLK,   "cclk",  "Configure clock"          ,"src,edge,trmn"      ,get_set_clk },
   { CmdCLKDV,  "clkd",  "Clock divide"             ,"divider"            ,get_set_clk_div },
   { CmdSTRG,   "ctrg",  "Configure trigger"        ,"src,edge,trmn"      ,get_set_trg },
   { CmdSATRG,  "atrg",  "Analogue trigger"         ,"control,level"      ,get_set_analog_trg },
   { CmdSPTRG,  "ptrg",  "Pre trigger configuration","delay,min-pre"      ,get_set_config_trg },
   { CmdBSZE,   "buf",   "Allocate buffer"          ,"size,post"          ,get_set_buffer_size },
   { CmdSTR,    "str",   "Start acquisition"        ,""                   ,strt_acquisition },
   { CmdTRG,    "trg",   "Trigger acquisition"      ,""                   ,trig_acquisition },
   { CmdSTP,    "stp",   "Stop acquisition"         ,""                   ,stop_acquisition },
   { CmdCON,    "con",   "Connect to interrupt"     ,"1=trig,2=acqn"      ,connect },
   { CmdTMO,    "tmo",   "Timeout"                  ,"milliseconds"       ,get_set_timeout },
   { CmdWI,     "wi",    "Wait for an interrupt"    ,"mask"               ,wait_interrupt },
   { CmdRF,     "rf",    "Read file"                ,"channel 1..16"      ,read_file },
   { CmdWF,     "wf",    "Write file"               ,"channel 1..16"      ,write_file },
   { CmdPLT,    "plt",   "Plot file"                ,"channel 1..16"      ,plot_samp },
   { CmdRSMP,   "rbf",   "Read samples buf to mem"  ,""                   ,read_samp },
   { CmdPSMP,   "pbf",   "Print samples buf"        ,"start,count"        ,print_samp },
   { CmdSWP,    "swp",   "Swap words/bytes"         ,"iflag,wflag,bflag"  ,swap_buf },
   { CmdMS,     "ms",    "Milliseconds sleep"       ,"milliseconds"       ,ms_sleep }
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

#define True 1
#define False 0
