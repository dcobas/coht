#define MAP_LONGS 0x80
static char *MapNames[MAP_LONGS] = {

   /* 0x0000 */ "RO FPGA Version",
   /* 0x0001 */ "RW Control Reg ",
   /* 0x0002 */ "RO Test        ",
   /* 0x0003 */ "RO Temperature ",
   /* 0x0004 */ "RW Temp Warning",
   /* 0x0005 */ "RW Temp Failure",
   /* 0x0006 */ "RW SRAM Address",
   /* 0x0007 */ "RO SRAM Data   ",
   /* 0x0008 */ "RW PLL ClockCtl",
   /* 0x0009 */ "RW PLD TimerCtl",
   /* 0x000a */ "RW PLL Intrface",
   /* 0x000b */ "RW ADC ChanEnbl",
   /* 0x000c */ "RW DAC ChanEnbl",
   /* 0x000d */ "RW             ",
   /* 0x000e */ "RW ZBTSRAM Addr",
   /* 0x000f */ "RW ZBTSRAM Data",
   /* 0x0010 */ "RW AD Amp Enabl",
   /* 0x0011 */ "RW DAC Controls",
   /* 0x0012 */ "RW ADC Controls",
   /* 0x0013 */ "RW P16DIO   Reg",
   /* 0x0014 */ "RW P16DIOCtlReg",
   /* 0x0015 */ "RW FrontPanlDIO",
   /* 0x0016 */ "RW             ",
   /* 0x0017 */ "RW TrigCtl AD  ",
   /* 0x0018 */ "RW TrigCtl DAC ",
   /* 0x0019 */ "RW AD  Decimatn",
   /* 0x001a */ "RW DAC Decimatn",
   /* 0x001b */ "RW PktHeadr DAC",
   /* 0x001c */ "RW PktHeadr AD ",
   /* 0x001d */ "RW SoftAlert   ",
   /* 0x001e */ "RW Alert CtlReg",
   /* 0x001f */ "RW Alert Enabls",

   /* 0x0020 */ "WO AD  GainErCf",
   /* 0x0021 */ "WO AD  GainErCf",
   /* 0x0022 */ "WO AD  GainErCf",
   /* 0x0023 */ "WO AD  GainErCf",
   /* 0x0024 */ "WO AD  GainErCf",
   /* 0x0025 */ "WO AD  GainErCf",
   /* 0x0026 */ "WO AD  GainErCf",
   /* 0x0027 */ "WO AD  GainErCf",
   /* 0x0028 */ "WO AD  GainErCf",
   /* 0x0029 */ "WO AD  GainErCf",
   /* 0x002a */ "WO AD  GainErCf",
   /* 0x002b */ "WO AD  GainErCf",
   /* 0x002c */ "WO AD  GainErCf",
   /* 0x002d */ "WO AD  GainErCf",
   /* 0x002e */ "WO AD  GainErCf",
   /* 0x002f */ "WO AD  GainErCf",

   /* 0x0030 */ "WO AD  OffsErCf",
   /* 0x0031 */ "WO AD  OffsErCf",
   /* 0x0032 */ "WO AD  OffsErCf",
   /* 0x0033 */ "WO AD  OffsErCf",
   /* 0x0034 */ "WO AD  OffsErCf",
   /* 0x0035 */ "WO AD  OffsErCf",
   /* 0x0036 */ "WO AD  OffsErCf",
   /* 0x0037 */ "WO AD  OffsErCf",
   /* 0x0038 */ "WO AD  OffsErCf",
   /* 0x0039 */ "WO AD  OffsErCf",
   /* 0x003a */ "WO AD  OffsErCf",
   /* 0x003b */ "WO AD  OffsErCf",
   /* 0x003c */ "WO AD  OffsErCf",
   /* 0x003d */ "WO AD  OffsErCf",
   /* 0x003e */ "WO AD  OffsErCf",
   /* 0x003f */ "WO AD  OffsErCf",

   /* 0x0040 */ "WO DAC GainErCf",
   /* 0x0041 */ "WO DAC GainErCf",
   /* 0x0042 */ "WO DAC GainErCf",
   /* 0x0043 */ "WO DAC GainErCf",
   /* 0x0044 */ "WO DAC GainErCf",
   /* 0x0045 */ "WO DAC GainErCf",
   /* 0x0046 */ "WO DAC GainErCf",
   /* 0x0047 */ "WO DAC GainErCf",
   /* 0x0048 */ "WO DAC GainErCf",
   /* 0x0049 */ "WO DAC GainErCf",
   /* 0x004a */ "WO DAC GainErCf",
   /* 0x004b */ "WO DAC GainErCf",
   /* 0x004c */ "WO DAC GainErCf",
   /* 0x004d */ "WO DAC GainErCf",
   /* 0x004e */ "WO DAC GainErCf",
   /* 0x004f */ "WO DAC GainErCf",

   /* 0x0050 */ "WO DAC OffsErCf",
   /* 0x0051 */ "WO DAC OffsErCf",
   /* 0x0052 */ "WO DAC OffsErCf",
   /* 0x0053 */ "WO DAC OffsErCf",
   /* 0x0054 */ "WO DAC OffsErCf",
   /* 0x0055 */ "WO DAC OffsErCf",
   /* 0x0056 */ "WO DAC OffsErCf",
   /* 0x0057 */ "WO DAC OffsErCf",
   /* 0x0058 */ "WO DAC OffsErCf",
   /* 0x0059 */ "WO DAC OffsErCf",
   /* 0x005a */ "WO DAC OffsErCf",
   /* 0x005b */ "WO DAC OffsErCf",
   /* 0x005c */ "WO DAC OffsErCf",
   /* 0x005d */ "WO DAC OffsErCf",
   /* 0x005e */ "WO DAC OffsErCf",
   /* 0x005f */ "WO DAC OffsErCf",

   /* 0x0060 */ "WO             ",
   /* 0x0061 */ "WO             ",
   /* 0x0062 */ "WO             ",
   /* 0x0063 */ "WO             ",

   /* 0x0064 */ "RW SinFrequency",
   /* 0x0065 */ "RW SinAmplitude",
   /* 0x0066 */ "RW SinRise Time",
   /* 0x0067 */ "RW SinFlTopTime",
   /* 0x0068 */ "RW SinFall Time",

   /* 0x0069 */ "RW             ",
   /* 0x006a */ "RW             ",
   /* 0x006b */ "RW             ",
   /* 0x006c */ "RW             ",
   /* 0x006d */ "RW             ",
   /* 0x006e */ "RW             ",
   /* 0x006f */ "RW             ",

   /* 0x0070 */ "WO AD  Gain 00 ",
   /* 0x0071 */ "WO AD  Gain 01 ",
   /* 0x0072 */ "WO AD  Gain 02 ",
   /* 0x0073 */ "WO AD  Gain 03 ",
   /* 0x0074 */ "WO AD  Gain 04 ",
   /* 0x0075 */ "WO AD  Gain 05 ",
   /* 0x0076 */ "WO AD  Gain 06 ",
   /* 0x0077 */ "WO AD  Gain 07 ",
   /* 0x0078 */ "WO AD  Gain 08 ",
   /* 0x0079 */ "WO AD  Gain 09 ",
   /* 0x007a */ "WO AD  Gain 10 ",
   /* 0x007b */ "WO AD  Gain 11 ",

   /* 0x007c */ "RW             ",
   /* 0x007d */ "RW             ",
   /* 0x007e */ "RW Sys Clock Hz",
   /* 0x007f */ "RW Smp Clock Hz"
 };

/* Function ram definitions */

#define TOTAL_POINTS 1048576

#define SIN_POINTS_PER_SECOND 1000000
#define AQN_POINTS_PER_SECOND 250000
#define AQN_MICROS_PER_POINT  4
#define SIN_MICROS_PER_POINT  1

typedef struct {
   unsigned long FPGAVersion;
   unsigned long ControlReg;
   unsigned long Test;
   unsigned long Temperature;
   unsigned long TempWarning;
   unsigned long TempFailure;
   unsigned long SRAMAddr;
   unsigned long SRAMData;
   unsigned long PLLClockCtl;
   unsigned long PLDTimerCtl;
   unsigned long PLLIntrface;
   unsigned long ADCChanEnbl;
   unsigned long DACChanEnbl;
   unsigned long Spare01;
   unsigned long ZBTSRAMAddr;
   unsigned long ZBTSRAMData;
   unsigned long ADAmpEnabl;
   unsigned long DACControls;
   unsigned long ADCControls;
   unsigned long P16DIOReg;
   unsigned long P16DIOCtlReg;
   unsigned long FrontPanlDIO;
   unsigned long Spare02;
   unsigned long TrigCtlAD;
   unsigned long TrigCtlDAC;
   unsigned long ADDecimatn;
   unsigned long DACDecimatn;
   unsigned long PktHeadrDAC;
   unsigned long PktHeadrAD;
   unsigned long SoftAlert;
   unsigned long AlertCtlReg;
   unsigned long AlertEnabls;

   unsigned long ADGainErCf01;
   unsigned long ADGainErCf02;
   unsigned long ADGainErCf03;
   unsigned long ADGainErCf04;
   unsigned long ADGainErCf05;
   unsigned long ADGainErCf06;
   unsigned long ADGainErCf07;
   unsigned long ADGainErCf08;
   unsigned long ADGainErCf09;
   unsigned long ADGainErCf10;
   unsigned long ADGainErCf11;
   unsigned long ADGainErCf12;
   unsigned long ADGainErCf13;
   unsigned long ADGainErCf14;
   unsigned long ADGainErCf15;
   unsigned long ADGainErCf16;

   unsigned long ADOffsErCf01;
   unsigned long ADOffsErCf02;
   unsigned long ADOffsErCf03;
   unsigned long ADOffsErCf04;
   unsigned long ADOffsErCf05;
   unsigned long ADOffsErCf06;
   unsigned long ADOffsErCf07;
   unsigned long ADOffsErCf08;
   unsigned long ADOffsErCf09;
   unsigned long ADOffsErCf10;
   unsigned long ADOffsErCf11;
   unsigned long ADOffsErCf12;
   unsigned long ADOffsErCf13;
   unsigned long ADOffsErCf14;
   unsigned long ADOffsErCf15;
   unsigned long ADOffsErCf16;

   unsigned long DACGainErCf01;
   unsigned long DACGainErCf02;
   unsigned long DACGainErCf03;
   unsigned long DACGainErCf04;
   unsigned long DACGainErCf05;
   unsigned long DACGainErCf06;
   unsigned long DACGainErCf07;
   unsigned long DACGainErCf08;
   unsigned long DACGainErCf09;
   unsigned long DACGainErCf10;
   unsigned long DACGainErCf11;
   unsigned long DACGainErCf12;
   unsigned long DACGainErCf13;
   unsigned long DACGainErCf14;
   unsigned long DACGainErCf15;
   unsigned long DACGainErCf16;

   unsigned long DACOffsErCf01;
   unsigned long DACOffsErCf02;
   unsigned long DACOffsErCf03;
   unsigned long DACOffsErCf04;
   unsigned long DACOffsErCf05;
   unsigned long DACOffsErCf06;
   unsigned long DACOffsErCf07;
   unsigned long DACOffsErCf08;
   unsigned long DACOffsErCf09;
   unsigned long DACOffsErCf10;
   unsigned long DACOffsErCf11;
   unsigned long DACOffsErCf12;
   unsigned long DACOffsErCf13;
   unsigned long DACOffsErCf14;
   unsigned long DACOffsErCf15;
   unsigned long DACOffsErCf16;

   unsigned long Spare03;
   unsigned long Spare04;
   unsigned long Spare05;
   unsigned long Spare06;

   unsigned long SinFrequency;
   unsigned long SinAmplitude;
   unsigned long SinRiseTime;
   unsigned long SinFlTopTime;
   unsigned long SinFallTime;

   unsigned long Spare07;
   unsigned long Spare08;
   unsigned long Spare09;
   unsigned long Spare10;
   unsigned long Spare11;
   unsigned long Spare12;
   unsigned long Spare13;

   unsigned long ADGain00;
   unsigned long ADGain01;
   unsigned long ADGain02;
   unsigned long ADGain03;
   unsigned long ADGain04;
   unsigned long ADGain05;
   unsigned long ADGain06;
   unsigned long ADGain07;
   unsigned long ADGain08;
   unsigned long ADGain09;
   unsigned long ADGain10;
   unsigned long ADGain11;

   unsigned long Spare14;
   unsigned long Spare15;
   unsigned long SysClkHz;
   unsigned long SmpClkHz;
 } AcdxMap;


typedef enum {
   AcdxFPGAVersion,
   AcdxControlReg,
   AcdxTest,
   AcdxTemperature,
   AcdxTempWarning,
   AcdxTempFailure,
   AcdxSRAMAddr,
   AcdxSRAMData,
   AcdxPLLClockCtl,
   AcdxPLDTimerCtl,
   AcdxPLLIntrface,
   AcdxADCChanEnbl,
   AcdxDACChanEnbl,
   AcdxSpare01,
   AcdxZBTSRAMAddr,
   AcdxZBTSRAMData,
   AcdxADAmpEnabl,
   AcdxDACControls,
   AcdxADCControls,
   AcdxP16DIOReg,
   AcdxP16DIOCtlReg,
   AcdxFrontPanlDIO,
   AcdxSpare02,
   AcdxTrigCtlAD,
   AcdxTrigCtlDAC,
   AcdxADDecimatn,
   AcdxDACDecimatn,
   AcdxPktHeadrDAC,
   AcdxPktHeadrAD,
   AcdxSoftAlert,
   AcdxAlertCtlReg,
   AcdxAlertEnabls,

   AcdxADGainErCf01,
   AcdxADGainErCf02,
   AcdxADGainErCf03,
   AcdxADGainErCf04,
   AcdxADGainErCf05,
   AcdxADGainErCf06,
   AcdxADGainErCf07,
   AcdxADGainErCf08,
   AcdxADGainErCf09,
   AcdxADGainErCf10,
   AcdxADGainErCf11,
   AcdxADGainErCf12,
   AcdxADGainErCf13,
   AcdxADGainErCf14,
   AcdxADGainErCf15,
   AcdxADGainErCf16,

   AcdxADOffsErCf01,
   AcdxADOffsErCf02,
   AcdxADOffsErCf03,
   AcdxADOffsErCf04,
   AcdxADOffsErCf05,
   AcdxADOffsErCf06,
   AcdxADOffsErCf07,
   AcdxADOffsErCf08,
   AcdxADOffsErCf09,
   AcdxADOffsErCf10,
   AcdxADOffsErCf11,
   AcdxADOffsErCf12,
   AcdxADOffsErCf13,
   AcdxADOffsErCf14,
   AcdxADOffsErCf15,
   AcdxADOffsErCf16,

   AcdxDACGainErCf01,
   AcdxDACGainErCf02,
   AcdxDACGainErCf03,
   AcdxDACGainErCf04,
   AcdxDACGainErCf05,
   AcdxDACGainErCf06,
   AcdxDACGainErCf07,
   AcdxDACGainErCf08,
   AcdxDACGainErCf09,
   AcdxDACGainErCf10,
   AcdxDACGainErCf11,
   AcdxDACGainErCf12,
   AcdxDACGainErCf13,
   AcdxDACGainErCf14,
   AcdxDACGainErCf15,
   AcdxDACGainErCf16,

   AcdxDACOffsErCf01,
   AcdxDACOffsErCf02,
   AcdxDACOffsErCf03,
   AcdxDACOffsErCf04,
   AcdxDACOffsErCf05,
   AcdxDACOffsErCf06,
   AcdxDACOffsErCf07,
   AcdxDACOffsErCf08,
   AcdxDACOffsErCf09,
   AcdxDACOffsErCf10,
   AcdxDACOffsErCf11,
   AcdxDACOffsErCf12,
   AcdxDACOffsErCf13,
   AcdxDACOffsErCf14,
   AcdxDACOffsErCf15,
   AcdxDACOffsErCf16,

   AcdxSpare03,
   AcdxSpare04,
   AcdxSpare05,
   AcdxSpare06,

   AcdxSinFrequency,
   AcdxSinAmplitude,
   AcdxSinRiseTime,
   AcdxSinFlTopTime,
   AcdxSinFallTime,

   AcdxSpare07,
   AcdxSpare08,
   AcdxSpare09,
   AcdxSpare10,
   AcdxSpare11,
   AcdxSpare12,
   AcdxSpare13,

   AcdxADGain00,
   AcdxADGain01,
   AcdxADGain02,
   AcdxADGain03,
   AcdxADGain04,
   AcdxADGain05,
   AcdxADGain06,
   AcdxADGain07,
   AcdxADGain08,
   AcdxADGain09,
   AcdxADGain10,
   AcdxADGain11,

   AcdxSpare14,
   AcdxSpare15,

   AcdxSysClkHz,
   AcdxSmpClkHz,

   AcdxADDRESSES

 } AcdxAddress;


typedef struct {
   unsigned long Address;
   unsigned long Data;
 } AcdxIoItem;


#define AcdxOUTPUT_VOLTAGE_FACTOR 40.0
#define AcdxOUTPUT_CURRENT_FACTOR 100
#define AcdxMAGNET_CURRENT_FACTOR 1000

typedef enum {
   AcdxFunctionMAGNET_CURRENT,
   AcdxFunctionRIGHT_OUTPUT_CURRENT,
   AcdxFunctionLEFT_OUTPUT_CURRENT,
   AcdxFunctionMAGNET_VOLTAGE,
   AcdxFunctionLEFT_OUTPUT_VOLTAGE,
   AcdxFunctionRIGHT_OUTPUT_VOLTAGE,
   AcdxFUNCTIONS
 } AcdxFunction;

char *FuncNames[AcdxFUNCTIONS] = {"mac","roc","loc","mav","lov","rov"};

int FuncNext[4] = {0,4,6,5};

char *Titles[AcdxFUNCTIONS] = {

   "MAGNET CURRENT",
   "AMPLIFIER-1 OUTPUT CURRENT",
   "AMPLIFIER-2 OUTPUT CURRENT",
   "MAGNET VOLTAGE",
   "AMPLIFIER-2 OUTPUT VOLTAGE",
   "AMPLIFIER-1 OUTPUT VOLTAGE"
 };

static unsigned int Funcn = 1;
static unsigned int Funcs = AcdxFUNCTIONS;;

typedef struct {
   int    Valid;         /* Set when valid data present */
   double ScaledMaxVal;  /* Can be volts or amps */
   double ScaledMinVal;  /* Can be volts or amps */
   double PosWaveLength; /* In seconds */
   double NegWaveLength; /* In seconds */
   double Tzc;           /* Time of first posative zero crossing in seconds */
 } AcdxFunctionParams;

static AcdxFunctionParams fpars[AcdxFUNCTIONS];

#define AcdxCOEFS 512
