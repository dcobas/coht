/* =============================================== */
/* Thu 18th June 2009 Julian Lewis BE/CO/HT        */
/* Describe PowerSoft K-series amplifier protocol  */
/* =============================================== */

typedef enum {
   CmdGet,   /* We expect a reply from device conerning the Query */
   CmdSet,   /* We want to change the internal state of the amplifier */
   CmdRly    /* Used to control internal boards */
 } CmdType;

typedef enum {
   PktNone     = 0,
   PktGeneric  = 1, // GenericPacket
   PktReply    = 2, // ReplyPacket
 } PktType;

typedef struct {
   char Head;        /* STX (0x02) always */
   char DeviceId[2]; /* 00..99 */
   char CmdId;       /* Cmd ID is printable ascii code */
   char ModleId[2];  /* Device modle ID (Replies only) */
   char *Payload;    /* Variable length field */
   char Checksum;    /* Checksum */
   char Tail;        /* Always ETX (0x3) */
 } GenericPacket;

typedef enum {
   ACK  = 0x06, /* Successful set */
   NACK = 0x15, /* Unsuccessful set */
   INV  = 0x04, /* Invalid parameter */
   OOR  = 0x05, /* Parameter out of range */
 } Result;

typedef struct {
   char OutAten1    [2]; /* 0  */
   char OutAten2    [2]; /* 2  */
   char Mute        [2]; /* 4  */
   char Temp12      [2]; /* 6  */
   char Protect     [2]; /* 8  */
   char Ready       [2]; /* 10 */
   char SignalPres  [2]; /* 12 */
   char ProtectCount[2]; /* 14 */
   char Impedance1  [4]; /* 16 */
   char Impedance2  [4]; /* 20 */
   char Gain1       [2]; /* 24 */
   char Gain2       [2]; /* 26 */
   char OutVoltage1 [2]; /* 28 */
   char OutVoltage2 [2]; /* 30 */
   char MaxMainsCur [2]; /* 32 */
   char Limiter     [2]; /* 34 */
   char ModCounter  [4]; /* 36 */
   char Boards      [2]; /* 40 */
   char InputRout   [2]; /* 42 */
   char Idletime    [4]; /* 44 */
   char DspModCount [4]; /* 48 */
   char DspCh1Crc   [4]; /* 52 */
   char DspCh2Crc   [4]; /* 56 */
   char DspComCrc   [4]; /* 60 */
 } AmpStatus;

/* ===================================================== */

typedef struct {
   char        Id;      /* Cmd Id */
   CmdType     Type;    /* Cmd Type */
   int         CpSize;  /* Cmd payload size -ve=>Variable */
   int         RpSize;  /* Reply size 0=>Variable size */
   char       *Comment; /* Description of command */
 } CmdDesc;

#define COMMANDS 24

CmdDesc cmdtb[COMMANDS] = {

//   ID   Type    CpSize RpSize  Comment

   { 'A', CmdGet, 0,     6,      "Read device turn on counter" },
   { 'B', CmdGet, 2,     2,      "Mean temperature compute period" }, /* See p33 to interpret result */
   { 'E', CmdGet, 4,     14,     "Read specific event" },
   { 'I', CmdGet, 0,     0,     "Read information" },
   { 'L', CmdGet, 0,     8,      "Read device level meters" },
   { 'N', CmdGet, 0,     6,      "Work Time in minutes since turn on" },
   { 'P', CmdGet, 0,     6,      "Protection state count" },
   { 'S', CmdGet, 0,     64,     "Read status" },
   { 'T', CmdGet, 0,     10,     "Reads device voltage, clock, temp, and led status" },
   { 'U', CmdGet, 0,     14,     "Read last recorded event" },
   { 'c', CmdSet, 2,     2,      "Turn On/Off a channel" },
   { 'm', CmdSet, 2,     2,      "Mute On/Off a channel" },
   { 'p', CmdSet, 1,     2,      "Turn On/Off power" },
   { 'r', CmdSet, 1,     2,      "Reset a module" },
   { 't', CmdSet, 3,     2,      "Set fake temperature" },
   { 'v', CmdSet, 3,     2,      "Set output attenuation" },
   { 'w', CmdSet, 4,     2,      "Set attenuation variable channels" },
   { 'D', CmdGet, 12,    1602,   "Get wave sample" },
   { 'M', CmdRly, 0,     0,      "Rly to KDSP board" },
   { 'Z', CmdSet, 2,     2,      "Set input signal routing" },
   { 'b', CmdSet, 1,     2,      "Set brownout detection" },
   { 'h', CmdSet, 4,     2,      "Set mains calibration high value" },
   { 'o', CmdSet, 4,     2,      "Set mains calibration low value" },
   { 'x', CmdSet, 0,     0,      "Rly to internal board" }
 };

typedef struct {
   int Value;
   char *Name;
 } EnumBinding;

#define ENUM_BINDINGS 8
typedef struct {
   int EnumValue;
   EnumBinding Bindings[ENUM_BINDINGS];
} EnumNames;

#define ENUM_NAMES 6
EnumNames ETab[ENUM_NAMES] = {

{0x89,{ {0,"Off"}          ,{1,"On (Manual)"}    ,{2,"Auto"}            ,{0,""}              ,{0,""}               ,{0,""}            ,{0,""}            ,{0,""}           } },
{0x8A,{ {1,"Powersoft"}    ,{2,"XTA"}            ,{3,"BSS Symetric"}    ,{4,"BSS Asymetric"} ,{5,"DBX Constant"}   ,{6,"DBX Adaptive"},{7,"Lake"}        ,{8,"Crown"}      } },
{0x41,{ {0,"Butterworth"}  ,{1,"Bessel"}         ,{2,"Linkwitz-Riley"}  ,{3,"FIR"}           ,{4,"FIR+"}           ,{0,""}            ,{0,""}            ,{0,""}           } },
{0x01,{ {0,"Peaking"}      ,{0x0B,"Low shelving"},{0x0C,"High shelving"},{0x0D,"Low-pass EQ"},{0x0E,"High-pass EQ"},{0x0F,"Band-pass"},{0x10,"Band-stop"},{0x11,"All-pass"}} },
{0xC3,{ {0,"Both channels"},{1,"Channel-1"}      ,{2,"Channel-2"}       ,{0,""}              ,{0,""}               ,{0,""}            ,{0,""}            ,{0,""}           } },
{0xC4,{ {0,"Mute"}         ,{1,"Auto"}           ,{2,"Analog"}          ,{0,""}              ,{0,""}               ,{0,""}            ,{0,""}            ,{0,""}           } }

};

typedef struct {
   char *Name;
   int Type;
   int Index;
   int Channel;
   int Default;
   int Max;
   int Min;
   char *Units;
 } KDspPar;

typedef struct __attribute__ ((__packed__)) {
   char Threshold[2];             // Signed short
   char Attack[2];                // Signed short
   char Decay[2];                 // Signed short
 } Limiter;

typedef struct __attribute__ ((__packed__)) {
   char Enabled;                  // Boolean
   char Align[2];                 // Signed short
   char Slope[2];                 // Signed short
   char Frequency[2];             // Signed short
 } Crossover;

typedef struct __attribute__ ((__packed__)) {
   char Enabled;                  // Boolean
   char Type[2];                  // Signed short
   char Frequency[2];             // Signed short
   char Q[2];                     // Signed short
   char Gain[2];                  // Signed short
   char Slope[2];                 // Signed short
 } Parametric;

typedef struct __attribute__ ((__packed__)) {
   char Board[2];
   char SubCommand;
   char Tag[2];                   // 0xFACE
   char Version[2];               // 25
   char PhaseReversed;            // Boolean
   char PeakLimitEnabled;         // Boolean
   char PowerLimitEnabled;        // Boolean
   char GEqBypass[2];             // Signed short
   char PresetProtection[2];      // Signed short
   char Delay[2];                 // Signed short

   Limiter Peak;

   char AuxDelay[2];              // Signed short

   Limiter Power;

   char Gain[2];                  // Signed short
   char DampingFactor[2];         // Signed short
   char DampingMode[2];           // Signed short
   char FiltersDefinition[2];     // Signed short

   Crossover CrossoverFilters[2];

   Parametric ParametricFilters[16];

 } ChannelParameters;

typedef struct __attribute__ ((__packed__)) {
   char Board[2];
   char SubCommand;
   char Tag[2];                   // 0xFADE
   char Version[2];               // 1
   char DSPBypass;                // Boolean
   char Input[2];                 // Signed short
   char MainDelay[2];             // Signed short
   char SoundSpeed[2];            // Signed short
   char AES3Gain[2];              // Signed short
   char Source[2];                // Signed short
   char NoLinkBehaviour[2];       // Signed short
 } CommonParameters;

static int ChanParsValid[2] = {0 , 0};
static ChannelParameters ChanPars[2];

static int ComParsValid = 0;
static CommonParameters ComPars;

static int StatBlkValid = 0;
static AmpStatus StatBlk;
