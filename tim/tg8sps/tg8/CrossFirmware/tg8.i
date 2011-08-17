# 1 "../firmware/tg8.c"
 
 
 
 
 
 
 
 

 

 
 

 
 

 

 
 

 

 
 

 
 
 

 



 

# 1 "../firmware/tg8.h" 1
 
 
 
 
 
 

 
 
 




# 1 "tg8Pub.h" 1
 
 
 

 

 
 

 
 

 

 
 

 

 
 

 
 
 

 










 
 
 



typedef unsigned char  Uchar;
typedef unsigned char  Byte;

typedef unsigned short Ushort;
typedef unsigned short Word;

typedef unsigned int   Uint;
typedef unsigned int   DWord;


 
 
 

typedef enum { Tg8NO_MACHINE=0,
	       Tg8LHC,   
	       Tg8SPS,	 
	       Tg8CPS,	 
	       Tg8PSB,	 
	       Tg8LEA,   
	       Tg8MACHINES	 
	     } Tg8Machine;


 

typedef enum {
	Tg8MILLISECOND_HEADER = 0x01,
	Tg8SECOND_HEADER = 0x02,
	Tg8MINUTE_HEADER = 0x03,
	Tg8HOUR_HEADER   = 0x04,
	Tg8DAY_HEADER    = 0x05,
	Tg8MONTH_HEADER  = 0x06,
	Tg8YEAR_HEADER   = 0x07,
	Tg8psTIME_HEADER = 0x08,
	Tg8psDATE_HEADER = 0x09,
	TgvUTC_LOW_HEADER  = 0xB5,    
	TgvUTC_HIGH_HEADER = 0xB6    
  } Tg8EventHeader;

typedef enum {
	Tg8SSC_HEADER           = 0,  
	Tg8SUPER_CYCLE_HEADER   = 2,  
	Tg8TELEGRAM_HEADER      = 3,  
	Tg8SIMPLE_EVENT_HEADER  = 4,  
	Tg8psTCU_EVENT_HEADER   = 5   
  } Tg8MachineEventHeader;  

 



 







 





 










 
 

typedef long Tg8LongEvent;  

typedef struct {
   short Event_half1;
   short Event_half2; } Tg8ShortEvent;

typedef struct {
   unsigned char Header;
   unsigned char Byte_2;
   unsigned char Byte_3;
   unsigned char Byte_4; } Tg8AnyEvent;

typedef struct {
   unsigned char Header;
   unsigned char Event_code;
   unsigned char Cycle_type;
   unsigned char Cycle_number; } Tg8SimpleEvent;

typedef struct {
   unsigned char Header;
   unsigned char Scn_low;
   unsigned char Scn_mid;
   unsigned char Scn_msb; } Tg8SscEvent;

typedef struct {
   unsigned char Header;
   unsigned char Ticks_1ms;
   unsigned char Dcare_1;
   unsigned char Dcare_2;} Tg8MillisecondEvent;  

typedef struct  {
   unsigned char  Header;
   unsigned char  Tg8_second;
   unsigned char  Byte_3;
   unsigned char  Byte_4; } Tg8SecondEvent;

typedef struct  {
   unsigned char  Header;
   unsigned char  Tg8_minute;
   unsigned char  Byte_3;
   unsigned char  Byte_4; } Tg8MinuteEvent;

typedef struct  {
   unsigned char  Header;
   unsigned char  Tg8_hour;
   unsigned char  Byte_3;
   unsigned char  Byte_4; } Tg8HourEvent;

typedef struct  {
   unsigned char  Header;
   unsigned char  Tg8_day;
   unsigned char  Byte_3;
   unsigned char  Byte_4; } Tg8DayEvent;

typedef struct  {
   unsigned char  Header;
   unsigned char  Tg8_month;
   unsigned char  Byte_3;
   unsigned char  Byte_4; } Tg8MonthEvent;

typedef struct  {
   unsigned char  Header;
   unsigned char  Tg8_year;
   unsigned char  Byte_3;
   unsigned char  Byte_4; } Tg8YearEvent;

typedef struct  {  
   unsigned char  Header;
   unsigned char  psYear;
   unsigned char  psMonth;
   unsigned char  psDay; } Tg8psDateEvent;

typedef struct  {  
   unsigned char  Header;
   unsigned char  psHour;
   unsigned char  psMinute;
   unsigned char  psSecond; } Tg8psTimeEvent;

typedef  struct {
   unsigned char  Header;
   unsigned char  Group_number;
   short          Group_value; } Tg8TelegramEvent;

typedef struct {
   unsigned char Header;
   unsigned char Junk;
   unsigned short UtcWord;
 } TgvUtc;

 

typedef union {
   Tg8AnyEvent           Any;
   Tg8LongEvent          Long;
   Tg8ShortEvent         Short;
   Tg8SimpleEvent        Simple;
   Tg8SscEvent           Ssc;
   Tg8MillisecondEvent   Millisecond;
   Tg8SecondEvent        Tg8Second;
   Tg8MinuteEvent        Tg8Minute;
   Tg8HourEvent          Tg8Hour;
   Tg8DayEvent           Tg8Day;
   Tg8MonthEvent         Tg8Month;
   Tg8YearEvent          Tg8Year;
   Tg8psDateEvent        psDate;
   Tg8psTimeEvent        psTime;
   Tg8TelegramEvent      Telegram;
   TgvUtc                Utc;
} Tg8Event;

 



 
 
 

typedef enum {	Tg8DISABLED,	  
		Tg8ENABLED,	  
		Tg8INTERRUPT,     
		Tg8NO_INTERRUPT,  
		Tg8PPMT_DISABLED, 
		Tg8PPMT_ENABLED   
} Tg8ActionState;

 
 
 

typedef enum {
  Tg8GT_NUM = 1,        
  Tg8GT_EXC = 2,        
  Tg8GT_BIT = 3         
} Tg8GroupType;

 
 
 




 
 
 

 



 






typedef enum {
  Tg8DO_NOTHING,               
  Tg8DO_OUTPUT,       	       
  Tg8DO_INTERRUPT,	       
  Tg8DO_OUTPUT_AND_INTERRUPT   
} Tg8ResultMode;



 









 






typedef enum { Tg8CM_NORMAL,	 
	       Tg8CM_CHAINED,	 
	       Tg8CM_EXTERNAL    
} Tg8CounterMode;




 





typedef enum { Tg8TM_NORMAL,
	       Tg8TM_PPM =1,
	       Tg8TM_LINE=3
} Tg8TimingType;



 







 






typedef enum { Tg8CLK_MILLISECOND,  
	       Tg8CLK_CABLE,        
	       Tg8CLK_X1,           
	       Tg8CLK_X2            
} Tg8ClockTrain;

 
 
 

typedef struct {
   Tg8Event	uEvent;    
   Word		uControl;  
   Word         uDelay;    
} Tg8User;

 
 
 

typedef struct {
   
  Byte    Machine;     
  Byte    GroupNum;    
  Word    GroupVal;    
} Tg8Gate;

typedef struct {
  Tg8User Action;      
  Tg8Gate Gate;        
} Tg8PpmLine;

typedef struct {
  Tg8Event Trigger;                  
  Byte     Machine;                  
  Byte     GroupNum;                 
  Byte     GroupType;                
  Byte     Dim;                      
  Word     LineGv[24  ];     
  Word	   LineCw[24  ];     
  Word     LineDelay[24  ];  
} Tg8PpmUser;

 
 
 

typedef struct {
  int        Id;       
  Tg8PpmUser Timing;   
} Tg8Object;

typedef enum {
  Tg8SEL_STATE= 1,     
  Tg8SEL_DELAY= 2      
} Tg8Selector;

 
 
 

typedef struct {
  Uint   rSc;         
  Uint   rOcc,        
         rOut;        
  Word   rNumOfTrig;  
  Byte   rOvwrCnt,    
         rOvwrAct;    
} Tg8Recording;

 
 
 

typedef struct {
  Byte     iRcvErr;     
  Byte     iOvwrAct;    
  Byte     iAct;        
  Byte     iSem;        
} Tg8IntAction;

typedef struct {
  Tg8Event iEvent;      
  Uint     iSc;         
  Uint     iOcc,        
           iOut;        
  Tg8IntAction iExt;    
} Tg8Interrupt;




typedef struct {
   Tg8Interrupt CntInter[8 ];  
   Tg8Interrupt ImmInter[8 ];  
} Tg8InterruptTable;

 
 
 



typedef struct {
  Tg8Event hEvent;      
  Uint     hSc;         
  Uint     hOcc;        
  Byte     hRcvErr,     
           hHour,       
           hMinute,     
           hSecond;     
} Tg8History;

 
 
 



typedef struct {
  Tg8Event cMsEvent;    
  Uint     cNumOfMs;    
  Uint     cSc;         
  Uint     cOcc;        
  Byte     cRcvErr,     
           cHour,       
           cMinute,     
           cSecond;     
} Tg8Clock;

 
 
 

typedef struct {
  Byte     aYear,       
           aMonth,
           aDay,
           aSpare1;     
  Byte     aRcvErr,     
           aHour,       
           aMinute,
           aSecond;
  Word     aMilliSecond,
           aMsDrift;    
} Tg8DateTime;

typedef struct {
  Tg8Event aEvent;      
  Tg8Event aMsEvent;    
  Uint     aScLen;      
  Uint     aScTime;     
  Uint     aSc;         
  Uint     aNumOfSc;    
  Word     aTrace;      
  Word     aIntSrc;     

  Word     aNumOfBus,   
           aNumOfSpur;  
  Word     aMbox;       
  short    aCoco;       
  Word     aFwStat;     
  Word     aSscHeader;  
  Word     aNumOfEv,    
           aPrNumOfEv;  
  Word     aNumOfAct;   
  Word     aAlarms;     
   
  Tg8DateTime aDt;      
  Byte     aFwVer[16];  
  Word     aSem;        
  Word     aQueueSize;    
  Word     aMaxQueueSize; 
  Word     aMovedFrames;  
  Uint     aMovedScTime;  
  Uint     aMovedSc;      
  Word     aProcFrames,   
           aModStatus;    
  Tg8Event aQueue[8];     
  Word     aTelegLHC [24 ];  
  Word     aTelegSPS [24 ];  
  Word     aTelegCPS [24 ];  
  Word     aTelegPSB [24 ];  
  Word     aTelegLEA [24 ];  
   
} Tg8Aux;

 
 
 

typedef enum {
  Tg8FS_RUNNING  = 0x1,    
  Tg8FS_ALARMS   = 0x2     


} Tg8FirmwareStatus;

 
 
 

typedef enum {
   Tg8ALARM_OK       = 0,	 
    
   Tg8ALARM_ISR      = 0x1,	 
   Tg8ALARM_LOST_IMM = 0x2,      
   Tg8ALARM_LOST_OUT = 0x4,      
   Tg8ALARM_MANY_PROC= 0x8,      
   Tg8ALARM_QUEUED_PROC = 0x10,  
   Tg8ALARM_DIFF_EVN = 0x20,     
   Tg8ALARM_DIFF_LEN = 0x40,     
   Tg8ALARM_MOVED_PROC=0x80,     
   Tg8ALARM_MOVED_IMM =0x100,    
   Tg8ALARM_ACT_DISBL =0x200,    
   Tg8ALARM_IMMQ_OVF  =0x400,    
   Tg8ALARM_MBX_BUSY  =0x800,    
    
   Tg8ALARM_UNCOM    = 0x1000,   

   Tg8ALARM_BAD_SWITCH=0x2000,   
   Tg8ALARM_INT_LOST  =0x4000    
} Tg8Alarm;

 
 
 

typedef enum { Tg8ST_OK = 0  
	        
} Tg8SelfTesrError;

 
 
 

typedef enum {
  Tg8ERR_OK = 0,                   
  Tg8ERR_ILLEGAL_OPERATION = -1,   
  Tg8ERR_ILLEGAL_ARG = -2,         
  Tg8ERR_TIMEOUT     = -3,         
  Tg8ERR_WRONG_DIM   = -4,	   
  Tg8ERR_WRONG_DELAY = -5,         
  Tg8ERR_WRONG_CLOCK = -6,         
  Tg8ERR_WRONG_MODE  = -7,         
  Tg8ERR_REJECTED    = -8,         
  Tg8ERR_BAD_OBJECT  = -9,         
  Tg8ERR_NO_ACK      = -10,        
  Tg8ERR_NOT_RUN     = -11,        
  Tg8ERR_NO_FILE     = -12,        
  Tg8ERR_PENDING     = -13,        
  Tg8ERR_BUS_ERR     = -14,        
  Tg8ERR_FIRMWARE    = -15,        
  Tg8ERR_TABLE_FULL  = -16,        
  Tg8ERR_BAD_VECT    = -17,        
  Tg8ERR_BAD_SWITCH  = -18,        
  Tg8ERR_BAD_TRIGGER = -19         
} Tg8Error;

 
 
 

typedef struct {
  Tg8DateTime Dt;     
  Uint ScTime;        
  Uint Epc;           
  Word Evo;           
  Word Hw;            
  Word Fw;            
  Word Cc;            
  Word Am;            
  Word SpareWS;
} Tg8StatusBlock;

 
 
 

typedef enum {
  Tg8DrvrMODULE_ENABLED            = 0x00000001,
  Tg8DrvrMODULE_FIRMWARE_LOADED    = 0x00000004,
  Tg8DrvrMODULE_USER_TABLE_VALID   = 0x00000008,
  Tg8DrvrMODULE_RUNNING            = 0x00000010,
  Tg8DrvrMODULE_FIRMWARE_ERROR     = 0x00000020,
  Tg8DrvrMODULE_HARDWARE_ERROR     = 0x00000040,
  Tg8DrvrMODULE_SWITCH_SET         = 0x00000080,
  Tg8DrvrMODULE_TIMED_OUT          = 0x00000100,
  Tg8DrvrDRIVER_DEBUG_ON           = 0x00000200,
  Tg8DrvrDRIVER_ALARMS             = 0x00000400
} Tg8DrvrSoftStatus;



 
# 15 "../firmware/tg8.h" 2

# 1 "tg8Hardw.h" 1
 
 
 
 
 

 

 
 

 
 

 

 
 

 

 
 

 
 
 

 




# 1 "mc68332.h" 1
 
 
 
 
 
 




 
 
 

 

































 
 
 
 
 
 
 
 



				   
				   
				   
				   
				   
				   











 
 
 













typedef struct {      
	char    SimSpare_bytes [0 ];
	short   SimConf;            
	short   SimTest;            
	short   SimSync;            
	short   SimReset;           
	short   SimTestE;           
	short   SimU0a;             
	short   SimU0c;             
	short   SimU0e;             
	short   SimDataE0;          
	short   SimDataE1;          
	short   SimDirE;            
	short   SimPinE;            
	short   SimDataF0;          
	short   SimDataF1;          
	short   SimDirF;            
	short   SimPinF;            
	short   SimProtect;         
	short   SimPic;             
	short   SimPit;             
	short   SimServ;            
	short   SimU28;             
	short   SimU2a;             
	short   SimU2c;             
	short   SimU2e;             
	short   SimTestMrA;         
	short   SimTestMrB;         
	char    SimTestScA,TstScB;  
	short   SimTestRep;         
	short   SimTestCtl;         
	short   SimTestDis;         
	short   SimU3c;             
	short   SimU3e;             
	short   SimDataC;           
	short   SimU42;             
	int     SimCSpin;           
	int     SimCSboot;          
	int     SimCS0;             
	int     SimCS1;             
	int     SimCS2;             
	int     SimCS3;             
	int     SimCS4;             
	int     SimCS5;             
	int     SimCS6;             
	int     SimCS7;             
	int     SimCS8;             
	int     SimCS9;             
	int     SimCS10;            
	int     SimU78;             
	int     SimU7c;             
} McpSim;                        

 
 
 

typedef struct {      
	char    SbrSpare_bytes [0 ];
	short   SbrConf;         
	short   SbrTest;         
	short   SbrBaSt;         
} McpSbr;                        

 
 
 

typedef struct {     
	char    QsmSpare_bytes [0 ];
	short   QsmConf;            
	short   QsmTest;            
	char    QsmIntLev,          
		QsmIntVec;          
	short   QsmU06;             
	short   QsmSciC0;           
	short   QsmSciC1;           
	short   QsmSciS;            
	short   QsmSciD;            
	short   QsmU10;             
	short   QsmU12;             
	char    QsmU14,             
		QsmPd;              
	char    QsmPin,             
		QsmDir;             
	short   QsmSpiC0;           
	short   QsmSpiC1;           
	short   QsmSpiC2;           
	char    QsmSpiC3,           
		QsmSpiS;            
	char    QsmU20[0xE0];       
	unsigned
	short   QsmRecv_ram[16],    
		QsmTran_ram[16];    
	unsigned
	char    QsmComd_ram[16];    
} McpQsm;                        

 
 
 
 
 

typedef struct {      
	char    TpuSpare_bytes [0 ];
	short   TpuConf;            
	short   TpuCR;              
	short   TpuDsc;             
	short   TpuDss;             
	short   TpuIc;              
	short   TpuIe;              
	short   TpuCfs0;            
	short   TpuCfs1;            
	short   TpuCfs2;            
	short   TpuCfs3;            
	short   TpuHs0;             
	short   TpuHs1;             
	short   TpuHsr0;            
	short   TpuHsr1;            
	short   TpuCp0;             
	short   TpuCp1;             
	short   TpuIs;              
	short   TpuLink;            
	short   TpuSgl;             
	short   TpuDcn;             
} McpTpu;                           

 
 
 

 




 










 




 






 





 




 


















 






 










 




 








 









 





 
# 34 "tg8Hardw.h" 2








 
 
 


			        
			        















 
 
 
 

 
 

 
 
 


typedef unsigned char Tg8Ram[0x800 ];

 
 
 









typedef struct {
   char  XlxSpare_Bytes[0 ];  
   short XlxDelay [8 ];        
   short XlxConfig[8 ];        
   short XWsscRframe1;          
   short XRframe2;              
   short WClrRerr;              

 } Tg8Xlx;

 
 
 

typedef enum {
  ConfOUTPUT      = 1,          
  ConfINTERRUPT   = 4,          
  ConfCABLE_CLOCK = 8,          
  ConfEXT_CLOCK_1 = 0x10,       
  ConfEXT_CLOCK_2 = 0x18,       
  ConfEXT_START   = 0x20,       
  ConfNORMAL      = 0x40,       
  ConfCHAINE      = 0x80,       
  ConfBURST       = 0xC0        

} Tg8CounterConfig;

 
 
 

typedef enum {
   XrDATA_OVERFLOW =  0x01,
   XrPARITY_ERROR  =  0x02,
   XrEND_SEQUENCE  =  0x04,
   XrMID_BIT       =  0x08,
   XrSTART_SEQUENCE=  0x10,
   XrDISABLED      =  0x20,
   XrMS_MISSING    =  0x40,
   XrMS_WATCH_DOG  =  0x80

 } Tg8XrHardwareStatus;

 
 
 

typedef enum {
  Tg8HS_COUNTER_1_INTERRUPT=    0x0001,  
  Tg8HS_COUNTER_2_INTERRUPT=    0x0002,
  Tg8HS_COUNTER_3_INTERRUPT=    0x0004,
  Tg8HS_COUNTER_4_INTERRUPT=    0x0008,
  Tg8HS_COUNTER_5_INTERRUPT=    0x0010,
  Tg8HS_COUNTER_6_INTERRUPT=    0x0020,
  Tg8HS_COUNTER_7_INTERRUPT=    0x0040,
  Tg8HS_COUNTER_8_INTERRUPT=    0x0080,  
  Tg8HS_DPRAM_INTERRUPT    =    0x0100,  
  Tg8HS_RECEIVER_ENABLED =      0x0200,  
  Tg8HS_PENDING_DOWNLOAD =      0x0400,  
  Tg8HS_XILINX_XR_LOADED =      0x0800,  
  Tg8HS_XILINX_X1_LOADED =      0x1000,  
  Tg8HS_XILINX_X2_LOADED =      0x2000,  
  Tg8HS_EXTERNAL_CLOCK_JUMPER = 0x4000,  
  Tg8HS_SELF_TEST_ERROR  =      0x8000   

} Tg8HardwareStatus;



                                         

 
 
 

typedef enum {
   ClrINT_CHANNEL_0 = 0xfffe,  
   ClrINT_CHANNEL_1 = 0xfffd,  
   ClrINT_CHANNEL_2 = 0xfffb,
   ClrINT_CHANNEL_3 = 0xfff7,
   ClrINT_CHANNEL_4 = 0xffef,
   ClrINT_CHANNEL_5 = 0xffdf,
   ClrINT_CHANNEL_6 = 0xffbf,
   ClrINT_CHANNEL_7 = 0xff7f,
   ClrINT_CHANNEL_8 = 0xfeff   
 } Tg8TpuClearInterrupt;

 
 
 




typedef enum {
    CamDATA_MODE	= 0x0002,  
    CamCOMMAND_MODE	= 0xFFFD,  
    CamWRITE		= 0xE000,  
    CamCLEAR_SKIP_BITS	= 0x9800,  
    CamSET_SKIP_BIT	= 0x9400,  
    CamSTATUS_MULTIPLE	= 14,      
    CamMATCH		= 0x8000,  
    CamMUL		= 0x4000,  
    CamBYTE_MASK        = 0x00FF   

  } Tg8CamControl;

 
 
 

typedef enum {
    SelfTestOk = 4,  
    OkLed      = 8   
  } OkLedEnum;

 
 
 

 

 
 
 

 

 
 

 
 

 

 
 
 


 
 
 


 
 


 
 

 

 
 


 
 
 


 
 

 

 
 


 
 

 

 
 

 

 


 




 


# 16 "../firmware/tg8.h" 2

# 1 "selftest.h" 1
 
 
 
 
 




 
 
 
 

typedef enum {
   MbxST_GO   = 1234,       
   MbxST_BUSY = 5678,       
   MbxST_DONE = 9012,       
   MbxST_BUS_ERROR = 3456   
 } MbxStatus;
 
 
 
 

typedef enum {
   MbxOP_READ  = -1,
   MbxOP_WRITE = 1,
   MbxOP_NONE  = 0
 } MbxOperation;

 
 
 

typedef enum {
   MbxCC_BUSY             = -1,
   MbxCC_DONE             =  1,
   MbxCC_BOOT             = 0x3842,
   MbxCC_332Bug           = 0x3844,
   MbxCC_DOWNLOAD         = 0x3856,
   MbxCC_DOWNLOAD_PENDING = 0x4678,
   MbxCC_FIRMWARE_RUNNING = 0x7777  
 } MbxComplCode;

 
 
 




 

typedef struct {
   Ushort Bot;           
   Ushort Top;           
 } StMemory;

 

typedef struct {
   int   Code;          
   int   Templ;         
   int   Actual;        
   Ushort At;            
   Ushort Dir;           
   Ushort BusInt;        
   Ushort N_of_bus;      
} StFault;

 
 
 

typedef struct { unsigned short Year,
				Month,
				Day,
				Hour,
				Minute,
				Second;
		 unsigned int   Ms;
} StDateTime;

 

typedef struct {
   int   N_of_bus;      
   int   N_of_frames;   
   int   EventFrame;    

   int   CycleNum;      
   short CycleDur;      
   short CTrain;        
   StDateTime Date;     

   Ushort TestProg;      
   Ushort TestPassed;    
   Ushort FaultType;     
   Ushort FaultCnt;      
   struct {  
     StFault  Err;      
     StMemory Mem;      
   } Ram;
   struct {  
     StFault  Err;      
     StMemory Mem;      
   } Dpram;
   struct {  
     StFault  Err;      
     StMemory Mem;      
     struct {
       Ushort At;         
       Ushort Cnt;        
     } Match;
   } Cam;
   struct {  
     StFault  Err;          
     Ushort    Rcv;          
     Ushort    Ssc;          
   } Xilinx;
   struct {  
     StFault  Err[8 ];  
     Ushort    Bad;          
     Ushort    Delay;        
   } Counter;
   int DscInt[1+ 8 ];  
   int MbxInt;                

 } StDpmInfo;

 

typedef struct {
   short IoDirection;     
   short DoneFlag;        
   short FirmwareStatus;  
   short VectorOffset;    
   int   ExceptionPC;     
    

 } StDpmHead;

 

typedef struct StDpm {

    
    
   StDpmHead Head;

    
   StDpmInfo Info;

    
   short Buffer [2048 /2 -2 -1 -(sizeof(StDpmHead)+sizeof(StDpmInfo))/2];

    
   short XilinxStatus;

    

   short InterruptDsc;  
   short InterruptMcp;  

    

   short HardwareStatus;         
   short Vector;                 
				 
   short CounterInterruptMask;   
                                 
 } StDpm;

 
 
 

typedef enum {
   BI_INIT = 1,       
   BI_RAM,            
   BI_RAM_TEST,       
   BI_DPRAM,          
   BI_DPRAM_TEST,     
   BI_CAM,            
   BI_CAM_TEST,       
   BI_XILINX,         
   BI_XILINX_TEST,    
   XI_READF1,         
   XI_READF2,         
   XI_READERR,        
   XI_RESETERR,       
   XI_SSC,            
   BI_COUNTER_TEST,   
   CI_DELAY,          
   CI_CONFIG,         
   N_OF_BI
} StBusInt;
 
 
 
 

typedef enum {
   D_FORWARD = 0,  
   D_BACKWARD,     
   N_OF_DIR
} StDirection;

 
 
 

typedef enum {
   A_BYTE = 0,  
   A_WORD,      
   A_INT32,     
   N_OF_ACC
} StAccess;

 
 
 

typedef enum {
   TM_AA,       
   TM_55,       
   TM_00,       
   N_OF_TEMPL
} StTemplate;

 
 
 

typedef enum {
   TC_A = 1,     
   TC_B,         
   TC_C,         
   TC_D,         
   TC_E          
} StCamTemplate;

 
 
 

typedef enum {
   CO_COUNT_FAST=1,   
   CO_BLOCKED,        
   CO_NO_DSC_INT,     
   N_OF_CONT_ER
} StCounting;

 
 
 

typedef enum {
   T_RAM   = 0x0001,  
   T_DPRAM = 0x0002,  
   T_CAM   = 0x0004,  
   T_XILINX= 0x0008,  
   T_MS    = 0x0010,   
   T_COUNTERS= 0x0020  
} StTest;

 
 
 

typedef enum {
   E_CAM_RW = 1,
   E_CAM_MATCH
} StErrCode;



 
# 17 "../firmware/tg8.h" 2


 
 
 

typedef enum {
  Tg8OP_NO_COMMAND = 0,
  Tg8OP_PING_MODULE        = 1,   
  Tg8OP_GET_STATUS         = 2,   
  Tg8OP_SET_SSC_HEADER     = 3,   

  Tg8OP_SET_STATE          = 4,	  
  Tg8OP_SET_DELAY          = 5,   
  Tg8OP_CLEAR_USER_TABLE   = 6,   
  Tg8OP_SET_USER_TABLE     = 7,   

  Tg8OP_GET_USER_TABLE     = 8,   
  Tg8OP_GET_RECORDING_TABLE= 9,   
  Tg8OP_GET_HISTORY_TABLE  = 10,  
  Tg8OP_GET_CLOCK_TABLE    = 11,  

  Tg8OP_SET_PPM_TIMING     = 12,  
  Tg8OP_SET_GATE           = 13,  
  Tg8OP_SET_DIM            = 14,  
  Tg8OP_GET_PPM_LINE       = 15,  
  Tg8OP_SIMULATE_PULSE     = 16,  
  Tg8OP_SELFTEST_RESULT    = 17   
} Tg8Operation;

 
 
 

typedef enum {
  Tg8HR_INT_VME = 0x7fc,    
			    
  Tg8HR_INT_MCP = 0x7fe,    
  Tg8HR_STATUS  = 0x800,    
		    












		    
  Tg8HR_INTERRUPT= 0x802,    
		    

		    





  Tg8HR_CLEAR_INT= 0x804,    
		    

  Tg8HR_END = -1    
} Tg8HardwareRegister;

 
 
 
typedef enum {
  Tg8ENABLE_INT = 0,	   
  Tg8DISABLE_INT= 2,	   
  Tg8ENABLE_SYNC_INT = 1,  
  Tg8DISABLE_SYNC_INT= 3   
} Tg8TimingInterrupt;

 
 
 

typedef enum {
  Tg8IS_MAILBOX = 0,  
  Tg8IS_IMM     = 1,  
  Tg8IS_ERR     = 2   
} Tg8InterruptSource;





 
 
 

typedef struct {
  Word	Row;	  
  Word  Number;   
} Tg8MbxOpHdr;

 
 
 

typedef struct {
  Tg8MbxOpHdr Hdr;	   
  Tg8User     Actions[1];  
} Tg8MbxRwAction;

 
 
 

typedef struct {
  Tg8MbxOpHdr Hdr;	   
  Tg8PpmUser  Actions[1];  
} Tg8MbxRwPpmAction;

 
 
 

typedef struct {
  Tg8MbxOpHdr Hdr;	   
  Tg8PpmLine  Lines[1];    
} Tg8MbxRwPpmLine;

 
 
 

typedef struct {
  Tg8MbxOpHdr Hdr;	   
  Tg8Gate     Gates[1];    
} Tg8MbxRwGate;

 
 
 

typedef struct {
  Tg8MbxOpHdr Hdr;	   
  Byte        Dims[4];     
} Tg8MbxRwDim;

 
 
 

typedef struct {
  Tg8MbxOpHdr  Hdr;	   
  Tg8Recording Recs[1];    
} Tg8MbxRecording;

 
 
 

typedef struct {
  Tg8MbxOpHdr Hdr;	  
  Tg8History  Hist[1];    
} Tg8MbxHistory;

 
 
 

typedef struct {
  Tg8MbxOpHdr Hdr;	  
  Tg8Clock    Clks[1];    
} Tg8MbxClock;

 
 
 

typedef struct {
  Tg8MbxOpHdr	Hdr;	 
} Tg8MbxClearAction;

 
 
 

typedef struct {
  Tg8MbxOpHdr	Hdr;	 
  Word  	State,   
                Aux;     
} Tg8MbxActionState;

 
 
 

   




typedef union {
  struct {
    Word Mask;
    Word SpareMsk;
  } SimPulse;
  Tg8MbxHistory      HistoryBlock;
  Tg8MbxRwAction     RwAction;
  Tg8MbxRwPpmAction  RwPpmAction;
  Tg8MbxRwPpmLine    RwPpmLine;
  Tg8MbxRwGate       RwGate;
  Tg8MbxRwDim        RwDim;
  Tg8MbxRecording    Recording;
  Tg8MbxClock        ClockBlock;
  Tg8MbxClearAction  ClearAction;
  Tg8MbxActionState  ActionState;
  Tg8DateTime        DateTime;
  Tg8StatusBlock     StatusBlock;
  StDpmInfo          SelfTestResult;
  Byte               MbxBuff[(0x62A-5*2* 24 ) ];
} Tg8BlockData;

 
 
 











typedef struct Tg8Dpm {

 
  char DpmSpare_Bytes [0 ];  

 

  short ExceptionVector,    






	Spare_ev;           

  Uint ExceptionPC;         






 

  Tg8InterruptTable It;      

 

  Tg8Aux At;                 

 

  Tg8BlockData BlockData;    

 
  short HrIntVme;   
                    
  short HrIntMpc;   

 
 
 
 

} Tg8Dpm;



 
# 38 "../firmware/tg8.c" 2

# 1 "../firmware/tg8P.h" 1
 
 
 

 

 
 

 
 

 

 
 

 

 
 

 
 
 

 

# 1 "../firmware/tg8Priv.h" 1
 
 
 
 

 
 
 

typedef struct {
   
   Tg8User User;        
   Tg8Gate Gate;        
   
   Word    CntControl;  
   Byte    CntNum;      
   Byte    Enabled;     
   
   Tg8Recording Rec;    
} Tg8Action;

 







 
 
 

typedef struct {
  Word *aTeleg[Tg8MACHINES];  

  Word  nextLHC [24 ];  
  Word  nextSPS [24 ];  
  Word  nextCPS [24 ];  
  Word  nextPSB [24 ];  
  Word  nextLEA [24 ];  
} InterTelegram;

 
 
 

typedef struct {
  Tg8Action *Pointers[256 ];  
  Tg8Action  Table[256 ];	    
} InterActTable;

 
 
 

typedef struct {
  Byte	* EndMa,       		    
        * ImmEndMa,                 
          Matches [256 ],     
          ImmMatches [256 ];  
} StartedAct;

 
 
 



typedef struct {
   Uint     *Bottom,  
	    *Top,     
	    *End;     
   int	    Size;     
   Uint     Buffer[32 ];  
} Queue;

 
 
 

typedef struct {
  short     N_of_clocks,       
            Clock_i;           
  Tg8Clock *Clock_p;           
  Tg8Clock  Clocks[16 ]; 
} InterClockTable;

 
 
 

typedef struct {
  short       N_of_histories,  
              History_i;       
  Tg8History *History_p;       
  Tg8History  Histories[1000 ];  
} InterHistoryTable;

 
 
 

typedef struct {
  Word	         IntSrc;           
  Word	         Alarms;           

  Tg8Event       Event;	           
  Tg8Event       ImmEvent;	   
  Word           WildCard;         
  Word           ImmWildCard;      

  Word           ImmRun;           
  Word           ImmSize,          
                 ImmNxt;           
  Tg8Interrupt   ImmInt[8 ];   

  Tg8Interrupt  *iFired[8 ];   
  Tg8Recording  *rFired[8 ];   
} EProg;

 
 
 




 
 
 

typedef struct {
  Byte   tsHour,      
         tsMinute,    
         tsSecond;    
} Tg8TimeStamp;

typedef struct {
  Uint   stScNum;     
  Uint   stMs;        
} Tg8ScTime;

 
 
 

typedef struct { Uchar         Byte_1;
		 Uchar         Byte_2;
   } Tg8TwoBytes;

typedef struct { short	       Short_1;
		 short         Short_2;
   } Tg8TwoShorts;

typedef union { Tg8TwoBytes    Bytes;
		short          Short;
   } Tg8ShortOrTwoBytes;

typedef union { Tg8TwoShorts   Short;
		long           Long;
   } Tg8LongOrTwoShorts;

 

typedef enum {  ANY_OTHER_EVENT_INDEX = 0,
                MILLISECOND_EVENT_INDEX = 1,
		TIME_EVENT_INDEX = 2,
		DATE_EVENT_INDEX = 3
	     } EventIndex;

 
# 29 "../firmware/tg8P.h" 2

# 1 "../firmware/tg8User.h" 1
 
 
 
 
 

 





 





 





 





 





 









 
# 30 "../firmware/tg8P.h" 2

# 1 "../firmware/macros.h" 1
 
 
 
 






 
 
 



















 
 
 




 
 
 

 






 
 
 
































# 101 "../firmware/macros.h"

 
 
 



 
 
 



 
 
 

















 
 
 











# 157 "../firmware/macros.h"





 
 
 






 
# 31 "../firmware/tg8P.h" 2


 

extern void ImmCompletion();   
extern void AtCompletion();    
extern void AtStartProcess();  
extern void InsertToCam();     
extern void ClearCam();        
extern void SetIntSourceMask();  

extern void Abort_Isr();		 
extern void Dsc_Isr();            
extern void Xr_Isr();		 

extern void Spurious_Isr();	 
extern void BusError_Isr();       
extern void AddressError_Isr();   
extern void PrivViolation_Isr();  
extern void Default_Isr();        

extern void Tpu0_Isr();		 
extern void Tpu1_Isr();		 
extern void Tpu2_Isr();
extern void Tpu3_Isr();
extern void Tpu4_Isr();
extern void Tpu5_Isr();
extern void Tpu6_Isr();
extern void Tpu7_Isr();
extern void Tpu8_Isr();

void AtProcess();	 
void ImmProcess();	 
void StartActions();     

static void SoftInit();	   
static void Init();	   
static void MbxProcess();  
static int  GetXConfiguration(int cw);

static void memset(volatile short *m,short v,unsigned int bc);
static void memcpy16(short *d,short *s,int bc);

 
 

volatile McpSim * sim;  
volatile McpTpu * tpu;  
volatile Tg8Xlx * xlx;  
volatile short  * cam;  
volatile Tg8Dpm * dpm;  

 
 
 

short mbx_ccr,imm_ccr;	    
long  mbx_pc ,imm_pc;       
long  mbx_context[16],
      imm_context[16],
      xr_context [16];
long  context_c[3];  
long  regic[5];      

 
 
 

Tg8Event timing_frame;	 
struct {
   Byte Err;      
   Byte Hour;     
} rcv_error;     
           
 
 
 

static struct {
   Word	              Addr;       
   Tg8LongOrTwoShorts Event;	  
   Tg8ShortOrTwoBytes Last_word;  
} ins, ins_sav;

 
 
 

static struct {
  Tg8ShortOrTwoBytes mm;         
  short * tpu_parameters[16];	 
} var;

static Tg8Recording sim_rec;    
static Tg8Interrupt sim_int;

static unsigned long utc;        
static unsigned long utcl;       
unsigned char dm[12];            

 
 
 

Uint wild_c[8];  
Byte time_event_index[16];  

struct {
  Byte Simple_event[256],     
       SimpleH_wc[256],       
       Simple_wc[256],        
       Simple_wc_imm[256];    
  Byte Time_event[256],       
       TimeH_wc[16],          
       Time_wc[256],          
       Time_wc_imm[256];      
} in_use;

 

Word              camBusy;   
Word              doImm;     

EProg             eprog;     
InterActTable     act;       
InterClockTable   clk;       
StartedAct        match;     
Queue             atQueue,   
                  immQueue;  
InterHistoryTable hist;	     
InterTelegram     tel;       

 
StDpmInfo         info;

 
# 39 "../firmware/tg8.c" 2

# 1 "/tmp/tg8Sym.c" 1

 

asm("
  CONVERTOR                       =1	/* 0x0001 	1	 |NUM | */
  TG8                             =0	/* 0x0000 	0	 |NUM | */
  TG8PUB_H                        =0	/* 0x0000 	0	 |NUM | */
  _MVME_167_                      =0	/* 0x0000 	0	 |NUM | */
  _PS_                            =0	/* 0x0000 	0	 |NUM | */
  DEBUG                           =1	/* 0x0001 	1	 |NUM | */
  TG8_TYPES                       =0	/* 0x0000 	0	 |NUM | */

  Uchar                           =1	/* 0x0001 	1	 |TYPE| char */

  Byte                            =1	/* 0x0001 	1	 |TYPE| char */

  Ushort                          =2	/* 0x0002 	2	 |TYPE| short */

  Word                            =2	/* 0x0002 	2	 |TYPE| short */

  Uint                            =4	/* 0x0004 	4	 |TYPE| int */

  DWord                           =4	/* 0x0004 	4	 |TYPE| int */
  Tg8NO_MACHINE                   =0	/* 0x0000 	0	 |NUM | */

  Tg8Machine                      =4	/* 0x0004 	4	 |TYPE|  */
  Tg8LHC                          =1	/* 0x0001 	1	 |NUM | */
  Tg8SPS                          =2	/* 0x0002 	2	 |NUM | */
  Tg8CPS                          =3	/* 0x0003 	3	 |NUM | */
  Tg8PSB                          =4	/* 0x0004 	4	 |NUM | */
  Tg8LEA                          =5	/* 0x0005 	5	 |NUM | */
  Tg8MACHINES                     =6	/* 0x0006 	6	 |NUM | */
  Tg8MILLISECOND_HEADER           =1	/* 0x0001 	1	 |NUM | */

  Tg8EventHeader                  =4	/* 0x0004 	4	 |TYPE|  */
  Tg8SECOND_HEADER                =2	/* 0x0002 	2	 |NUM | */
  Tg8MINUTE_HEADER                =3	/* 0x0003 	3	 |NUM | */
  Tg8HOUR_HEADER                  =4	/* 0x0004 	4	 |NUM | */
  Tg8DAY_HEADER                   =5	/* 0x0005 	5	 |NUM | */
  Tg8MONTH_HEADER                 =6	/* 0x0006 	6	 |NUM | */
  Tg8YEAR_HEADER                  =7	/* 0x0007 	7	 |NUM | */
  Tg8psTIME_HEADER                =8	/* 0x0008 	8	 |NUM | */
  Tg8psDATE_HEADER                =9	/* 0x0009 	9	 |NUM | */
  TgvUTC_LOW_HEADER               =181	/* 0x00b5 	181	 |NUM | */
  TgvUTC_HIGH_HEADER              =182	/* 0x00b6 	182	 |NUM | */
  Tg8SSC_HEADER                   =0	/* 0x0000 	0	 |NUM | */

  Tg8MachineEventHeader           =4	/* 0x0004 	4	 |TYPE|  */
  Tg8SUPER_CYCLE_HEADER           =2	/* 0x0002 	2	 |NUM | */
  Tg8TELEGRAM_HEADER              =3	/* 0x0003 	3	 |NUM | */
  Tg8SIMPLE_EVENT_HEADER          =4	/* 0x0004 	4	 |NUM | */
  Tg8psTCU_EVENT_HEADER           =5	/* 0x0005 	5	 |NUM | */
  Tg8SPS_SSC_HEADER               =32	/* 0x0020 	32	 |NUM | */
  Tg8LHC_SIMPLE_HEADER            =20	/* 0x0014 	20	 |NUM | */
  Tg8SPS_SIMPLE_HEADER            =36	/* 0x0024 	36	 |NUM | */
  Tg8CPS_SIMPLE_HEADER            =52	/* 0x0034 	52	 |NUM | */
  Tg8PSB_SIMPLE_HEADER            =68	/* 0x0044 	68	 |NUM | */
  Tg8LEA_SIMPLE_HEADER            =84	/* 0x0054 	84	 |NUM | */
  Tg8CPS_TCU_HEADER               =53	/* 0x0035 	53	 |NUM | */
  Tg8PSB_TCU_HEADER               =69	/* 0x0045 	69	 |NUM | */
  Tg8LEA_TCU_HEADER               =85	/* 0x0055 	85	 |NUM | */
  Tg8LHC_TELEGRAM_HEADER          =19	/* 0x0013 	19	 |NUM | */
  Tg8SPS_TELEGRAM_HEADER          =35	/* 0x0023 	35	 |NUM | */
  Tg8CPS_TELEGRAM_HEADER          =51	/* 0x0033 	51	 |NUM | */
  Tg8PSB_TELEGRAM_HEADER          =67	/* 0x0043 	67	 |NUM | */
  Tg8LEA_TELEGRAM_HEADER          =83	/* 0x0053 	83	 |NUM | */
  Tg8READY_TELEGRAM               =254	/* 0x00fe 	254	 |NUM | */
  Tg8GROUPS                       =24	/* 0x0018 	24	 |NUM | */

  Tg8LongEvent                    =4	/* 0x0004 	4	 |TYPE| long */

  Tg8ShortEvent                   =4	/* 0x0004 	4	 |TYPE|  */
  Event_half1                     =0	/* 0x0000 	0	 |VAR | Tg8ShortEvent.short 	=2= */
  Event_half2                     =2	/* 0x0002 	2	 |VAR | Tg8ShortEvent.short 	=2= */

  Tg8AnyEvent                     =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8AnyEvent.char 	=1= */
  Byte_2                          =1	/* 0x0001 	1	 |VAR | Tg8AnyEvent.char 	=1= */
  Byte_3                          =2	/* 0x0002 	2	 |VAR | Tg8AnyEvent.char 	=1= */
  Byte_4                          =3	/* 0x0003 	3	 |VAR | Tg8AnyEvent.char 	=1= */

  Tg8SimpleEvent                  =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8SimpleEvent.char 	=1= */
  Event_code                      =1	/* 0x0001 	1	 |VAR | Tg8SimpleEvent.char 	=1= */
  Cycle_type                      =2	/* 0x0002 	2	 |VAR | Tg8SimpleEvent.char 	=1= */
  Cycle_number                    =3	/* 0x0003 	3	 |VAR | Tg8SimpleEvent.char 	=1= */

  Tg8SscEvent                     =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8SscEvent.char 	=1= */
  Scn_low                         =1	/* 0x0001 	1	 |VAR | Tg8SscEvent.char 	=1= */
  Scn_mid                         =2	/* 0x0002 	2	 |VAR | Tg8SscEvent.char 	=1= */
  Scn_msb                         =3	/* 0x0003 	3	 |VAR | Tg8SscEvent.char 	=1= */

  Tg8MillisecondEvent             =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8MillisecondEvent.char 	=1= */
  Ticks_1ms                       =1	/* 0x0001 	1	 |VAR | Tg8MillisecondEvent.char 	=1= */
  Dcare_1                         =2	/* 0x0002 	2	 |VAR | Tg8MillisecondEvent.char 	=1= */
  Dcare_2                         =3	/* 0x0003 	3	 |VAR | Tg8MillisecondEvent.char 	=1= */

  Tg8SecondEvent                  =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8SecondEvent.char 	=1= */
  Tg8_second                      =1	/* 0x0001 	1	 |VAR | Tg8SecondEvent.char 	=1= */
  Byte_3                          =2	/* 0x0002 	2	 |VAR | Tg8SecondEvent.char 	=1= */
  Byte_4                          =3	/* 0x0003 	3	 |VAR | Tg8SecondEvent.char 	=1= */

  Tg8MinuteEvent                  =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8MinuteEvent.char 	=1= */
  Tg8_minute                      =1	/* 0x0001 	1	 |VAR | Tg8MinuteEvent.char 	=1= */
  Byte_3                          =2	/* 0x0002 	2	 |VAR | Tg8MinuteEvent.char 	=1= */
  Byte_4                          =3	/* 0x0003 	3	 |VAR | Tg8MinuteEvent.char 	=1= */

  Tg8HourEvent                    =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8HourEvent.char 	=1= */
  Tg8_hour                        =1	/* 0x0001 	1	 |VAR | Tg8HourEvent.char 	=1= */
  Byte_3                          =2	/* 0x0002 	2	 |VAR | Tg8HourEvent.char 	=1= */
  Byte_4                          =3	/* 0x0003 	3	 |VAR | Tg8HourEvent.char 	=1= */

  Tg8DayEvent                     =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8DayEvent.char 	=1= */
  Tg8_day                         =1	/* 0x0001 	1	 |VAR | Tg8DayEvent.char 	=1= */
  Byte_3                          =2	/* 0x0002 	2	 |VAR | Tg8DayEvent.char 	=1= */
  Byte_4                          =3	/* 0x0003 	3	 |VAR | Tg8DayEvent.char 	=1= */

  Tg8MonthEvent                   =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8MonthEvent.char 	=1= */
  Tg8_month                       =1	/* 0x0001 	1	 |VAR | Tg8MonthEvent.char 	=1= */
  Byte_3                          =2	/* 0x0002 	2	 |VAR | Tg8MonthEvent.char 	=1= */
  Byte_4                          =3	/* 0x0003 	3	 |VAR | Tg8MonthEvent.char 	=1= */

  Tg8YearEvent                    =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8YearEvent.char 	=1= */
  Tg8_year                        =1	/* 0x0001 	1	 |VAR | Tg8YearEvent.char 	=1= */
  Byte_3                          =2	/* 0x0002 	2	 |VAR | Tg8YearEvent.char 	=1= */
  Byte_4                          =3	/* 0x0003 	3	 |VAR | Tg8YearEvent.char 	=1= */

  Tg8psDateEvent                  =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8psDateEvent.char 	=1= */
  psYear                          =1	/* 0x0001 	1	 |VAR | Tg8psDateEvent.char 	=1= */
  psMonth                         =2	/* 0x0002 	2	 |VAR | Tg8psDateEvent.char 	=1= */
  psDay                           =3	/* 0x0003 	3	 |VAR | Tg8psDateEvent.char 	=1= */

  Tg8psTimeEvent                  =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8psTimeEvent.char 	=1= */
  psHour                          =1	/* 0x0001 	1	 |VAR | Tg8psTimeEvent.char 	=1= */
  psMinute                        =2	/* 0x0002 	2	 |VAR | Tg8psTimeEvent.char 	=1= */
  psSecond                        =3	/* 0x0003 	3	 |VAR | Tg8psTimeEvent.char 	=1= */

  Tg8TelegramEvent                =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8TelegramEvent.char 	=1= */
  Group_number                    =1	/* 0x0001 	1	 |VAR | Tg8TelegramEvent.char 	=1= */
  Group_value                     =2	/* 0x0002 	2	 |VAR | Tg8TelegramEvent.short 	=2= */

  TgvUtc                          =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | TgvUtc.char 	=1= */
  Junk                            =1	/* 0x0001 	1	 |VAR | TgvUtc.char 	=1= */
  UtcWord                         =2	/* 0x0002 	2	 |VAR | TgvUtc.short 	=2= */

  Tg8Event                        =4	/* 0x0004 	4	 |TYPE|  */
  Any                             =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8AnyEvent 	=4= */
  Long                            =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8LongEvent 	=4= */
  Short                           =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8ShortEvent 	=4= */
  Simple                          =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8SimpleEvent 	=4= */
  Ssc                             =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8SscEvent 	=4= */
  Millisecond                     =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8MillisecondEvent 	=4= */
  Tg8Second                       =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8SecondEvent 	=4= */
  Tg8Minute                       =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8MinuteEvent 	=4= */
  Tg8Hour                         =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8HourEvent 	=4= */
  Tg8Day                          =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8DayEvent 	=4= */
  Tg8Month                        =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8MonthEvent 	=4= */
  Tg8Year                         =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8YearEvent 	=4= */
  psDate                          =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8psDateEvent 	=4= */
  psTime                          =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8psTimeEvent 	=4= */
  Telegram                        =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8TelegramEvent 	=4= */
  Utc                             =0	/* 0x0000 	0	 |VAR | Tg8Event.TgvUtc 	=4= */
  Tg8DONT_CARE                    =255	/* 0x00ff 	255	 |NUM | */
  Tg8DISABLED                     =0	/* 0x0000 	0	 |NUM | */

  Tg8ActionState                  =4	/* 0x0004 	4	 |TYPE|  */
  Tg8ENABLED                      =1	/* 0x0001 	1	 |NUM | */
  Tg8INTERRUPT                    =2	/* 0x0002 	2	 |NUM | */
  Tg8NO_INTERRUPT                 =3	/* 0x0003 	3	 |NUM | */
  Tg8PPMT_DISABLED                =4	/* 0x0004 	4	 |NUM | */
  Tg8PPMT_ENABLED                 =5	/* 0x0005 	5	 |NUM | */
  Tg8GT_NUM                       =1	/* 0x0001 	1	 |NUM | */

  Tg8GroupType                    =4	/* 0x0004 	4	 |TYPE|  */
  Tg8GT_EXC                       =2	/* 0x0002 	2	 |NUM | */
  Tg8GT_BIT                       =3	/* 0x0003 	3	 |NUM | */
  Tg8ACTIONS                      =256	/* 0x0100 	256	 |NUM | */
  Tg8PPM_LINES                    =24	/* 0x0018 	24	 |NUM | */
  Tg8CW_INT_BITN                  =14	/* 0x000e 	14	 |NUM | */
  Tg8CW_INT_BITM                  =3	/* 0x0003 	3	 |NUM | */
  Tg8DO_NOTHING                   =0	/* 0x0000 	0	 |NUM | */

  Tg8ResultMode                   =4	/* 0x0004 	4	 |TYPE|  */
  Tg8DO_OUTPUT                    =1	/* 0x0001 	1	 |NUM | */
  Tg8DO_INTERRUPT                 =2	/* 0x0002 	2	 |NUM | */
  Tg8DO_OUTPUT_AND_INTERRUPT      =3	/* 0x0003 	3	 |NUM | */
  Tg8CW_CNT_BITN                  =8	/* 0x0008 	8	 |NUM | */
  Tg8CW_CNT_BITM                  =7	/* 0x0007 	7	 |NUM | */
  Tg8CW_START_BITN                =6	/* 0x0006 	6	 |NUM | */
  Tg8CW_START_BITM                =3	/* 0x0003 	3	 |NUM | */
  Tg8CM_NORMAL                    =0	/* 0x0000 	0	 |NUM | */

  Tg8CounterMode                  =4	/* 0x0004 	4	 |TYPE|  */
  Tg8CM_CHAINED                   =1	/* 0x0001 	1	 |NUM | */
  Tg8CM_EXTERNAL                  =2	/* 0x0002 	2	 |NUM | */
  Tg8CW_PPML_BITN                 =3	/* 0x0003 	3	 |NUM | */
  Tg8CW_PPML_BITM                 =3	/* 0x0003 	3	 |NUM | */
  Tg8TM_NORMAL                    =0	/* 0x0000 	0	 |NUM | */

  Tg8TimingType                   =4	/* 0x0004 	4	 |TYPE|  */
  Tg8TM_PPM                       =1	/* 0x0001 	1	 |NUM | */
  Tg8TM_LINE                      =3	/* 0x0003 	3	 |NUM | */
  Tg8CW_STATE_BITN                =2	/* 0x0002 	2	 |NUM | */
  Tg8CW_STATE_BITM                =1	/* 0x0001 	1	 |NUM | */
  Tg8CW_CLOCK_BITN                =0	/* 0x0000 	0	 |NUM | */
  Tg8CW_CLOCK_BITM                =3	/* 0x0003 	3	 |NUM | */
  Tg8CLK_MILLISECOND              =0	/* 0x0000 	0	 |NUM | */

  Tg8ClockTrain                   =4	/* 0x0004 	4	 |TYPE|  */
  Tg8CLK_CABLE                    =1	/* 0x0001 	1	 |NUM | */
  Tg8CLK_X1                       =2	/* 0x0002 	2	 |NUM | */
  Tg8CLK_X2                       =3	/* 0x0003 	3	 |NUM | */

  Tg8User                         =8	/* 0x0008 	8	 |TYPE|  */
  uEvent                          =0	/* 0x0000 	0	 |VAR | Tg8User.Tg8Event 	=4= */
  uControl                        =4	/* 0x0004 	4	 |VAR | Tg8User.Word 	=2= */
  uDelay                          =6	/* 0x0006 	6	 |VAR | Tg8User.Word 	=2= */

  Tg8Gate                         =4	/* 0x0004 	4	 |TYPE|  */
  Machine                         =0	/* 0x0000 	0	 |VAR | Tg8Gate.Byte 	=1= */
  GroupNum                        =1	/* 0x0001 	1	 |VAR | Tg8Gate.Byte 	=1= */
  GroupVal                        =2	/* 0x0002 	2	 |VAR | Tg8Gate.Word 	=2= */

  Tg8PpmLine                      =12	/* 0x000c 	12	 |TYPE|  */
  Action                          =0	/* 0x0000 	0	 |VAR | Tg8PpmLine.Tg8User 	=8= */
  Gate                            =8	/* 0x0008 	8	 |VAR | Tg8PpmLine.Tg8Gate 	=4= */

  Tg8PpmUser                      =152	/* 0x0098 	152	 |TYPE|  */
  Trigger                         =0	/* 0x0000 	0	 |VAR | Tg8PpmUser.Tg8Event 	=4= */
  Machine                         =4	/* 0x0004 	4	 |VAR | Tg8PpmUser.Byte 	=1= */
  GroupNum                        =5	/* 0x0005 	5	 |VAR | Tg8PpmUser.Byte 	=1= */
  GroupType                       =6	/* 0x0006 	6	 |VAR | Tg8PpmUser.Byte 	=1= */
  Dim                             =7	/* 0x0007 	7	 |VAR | Tg8PpmUser.Byte 	=1= */
  LineGv                          =8	/* 0x0008 	8	 |VAR | Tg8PpmUser.Word[24] 	=48= */
  LineCw                          =56	/* 0x0038 	56	 |VAR | Tg8PpmUser.Word[24] 	=48= */
  LineDelay                       =104	/* 0x0068 	104	 |VAR | Tg8PpmUser.Word[24] 	=48= */

  Tg8Object                       =156	/* 0x009c 	156	 |TYPE|  */
  Id                              =0	/* 0x0000 	0	 |VAR | Tg8Object.int 	=4= */
  Timing                          =4	/* 0x0004 	4	 |VAR | Tg8Object.Tg8PpmUser 	=152= */
  Tg8SEL_STATE                    =1	/* 0x0001 	1	 |NUM | */

  Tg8Selector                     =4	/* 0x0004 	4	 |TYPE|  */
  Tg8SEL_DELAY                    =2	/* 0x0002 	2	 |NUM | */

  Tg8Recording                    =16	/* 0x0010 	16	 |TYPE|  */
  rSc                             =0	/* 0x0000 	0	 |VAR | Tg8Recording.Uint 	=4= */
  rOcc                            =4	/* 0x0004 	4	 |VAR | Tg8Recording.Uint 	=4= */
  rOut                            =8	/* 0x0008 	8	 |VAR | Tg8Recording.Uint 	=4= */
  rNumOfTrig                      =12	/* 0x000c 	12	 |VAR | Tg8Recording.Word 	=2= */
  rOvwrCnt                        =14	/* 0x000e 	14	 |VAR | Tg8Recording.Byte 	=1= */
  rOvwrAct                        =15	/* 0x000f 	15	 |VAR | Tg8Recording.Byte 	=1= */

  Tg8IntAction                    =4	/* 0x0004 	4	 |TYPE|  */
  iRcvErr                         =0	/* 0x0000 	0	 |VAR | Tg8IntAction.Byte 	=1= */
  iOvwrAct                        =1	/* 0x0001 	1	 |VAR | Tg8IntAction.Byte 	=1= */
  iAct                            =2	/* 0x0002 	2	 |VAR | Tg8IntAction.Byte 	=1= */
  iSem                            =3	/* 0x0003 	3	 |VAR | Tg8IntAction.Byte 	=1= */

  Tg8Interrupt                    =20	/* 0x0014 	20	 |TYPE|  */
  iEvent                          =0	/* 0x0000 	0	 |VAR | Tg8Interrupt.Tg8Event 	=4= */
  iSc                             =4	/* 0x0004 	4	 |VAR | Tg8Interrupt.Uint 	=4= */
  iOcc                            =8	/* 0x0008 	8	 |VAR | Tg8Interrupt.Uint 	=4= */
  iOut                            =12	/* 0x000c 	12	 |VAR | Tg8Interrupt.Uint 	=4= */
  iExt                            =16	/* 0x0010 	16	 |VAR | Tg8Interrupt.Tg8IntAction 	=4= */
  Tg8CHANNELS                     =8	/* 0x0008 	8	 |NUM | */
  Tg8IMM_INTS                     =8	/* 0x0008 	8	 |NUM | */

  Tg8InterruptTable               =320	/* 0x0140 	320	 |TYPE|  */
  CntInter                        =0	/* 0x0000 	0	 |VAR | Tg8InterruptTable.Tg8Interrupt[8] 	=160= */
  ImmInter                        =160	/* 0x00a0 	160	 |VAR | Tg8InterruptTable.Tg8Interrupt[8] 	=160= */
  Tg8HISTORIES                    =1000	/* 0x03e8 	1000	 |NUM | */

  Tg8History                      =16	/* 0x0010 	16	 |TYPE|  */
  hEvent                          =0	/* 0x0000 	0	 |VAR | Tg8History.Tg8Event 	=4= */
  hSc                             =4	/* 0x0004 	4	 |VAR | Tg8History.Uint 	=4= */
  hOcc                            =8	/* 0x0008 	8	 |VAR | Tg8History.Uint 	=4= */
  hRcvErr                         =12	/* 0x000c 	12	 |VAR | Tg8History.Byte 	=1= */
  hHour                           =13	/* 0x000d 	13	 |VAR | Tg8History.Byte 	=1= */
  hMinute                         =14	/* 0x000e 	14	 |VAR | Tg8History.Byte 	=1= */
  hSecond                         =15	/* 0x000f 	15	 |VAR | Tg8History.Byte 	=1= */
  Tg8CLOCKS                       =16	/* 0x0010 	16	 |NUM | */

  Tg8Clock                        =20	/* 0x0014 	20	 |TYPE|  */
  cMsEvent                        =0	/* 0x0000 	0	 |VAR | Tg8Clock.Tg8Event 	=4= */
  cNumOfMs                        =4	/* 0x0004 	4	 |VAR | Tg8Clock.Uint 	=4= */
  cSc                             =8	/* 0x0008 	8	 |VAR | Tg8Clock.Uint 	=4= */
  cOcc                            =12	/* 0x000c 	12	 |VAR | Tg8Clock.Uint 	=4= */
  cRcvErr                         =16	/* 0x0010 	16	 |VAR | Tg8Clock.Byte 	=1= */
  cHour                           =17	/* 0x0011 	17	 |VAR | Tg8Clock.Byte 	=1= */
  cMinute                         =18	/* 0x0012 	18	 |VAR | Tg8Clock.Byte 	=1= */
  cSecond                         =19	/* 0x0013 	19	 |VAR | Tg8Clock.Byte 	=1= */

  Tg8DateTime                     =12	/* 0x000c 	12	 |TYPE|  */
  aYear                           =0	/* 0x0000 	0	 |VAR | Tg8DateTime.Byte 	=1= */
  aMonth                          =1	/* 0x0001 	1	 |VAR | Tg8DateTime.Byte 	=1= */
  aDay                            =2	/* 0x0002 	2	 |VAR | Tg8DateTime.Byte 	=1= */
  aSpare1                         =3	/* 0x0003 	3	 |VAR | Tg8DateTime.Byte 	=1= */
  aRcvErr                         =4	/* 0x0004 	4	 |VAR | Tg8DateTime.Byte 	=1= */
  aHour                           =5	/* 0x0005 	5	 |VAR | Tg8DateTime.Byte 	=1= */
  aMinute                         =6	/* 0x0006 	6	 |VAR | Tg8DateTime.Byte 	=1= */
  aSecond                         =7	/* 0x0007 	7	 |VAR | Tg8DateTime.Byte 	=1= */
  aMilliSecond                    =8	/* 0x0008 	8	 |VAR | Tg8DateTime.Word 	=2= */
  aMsDrift                        =10	/* 0x000a 	10	 |VAR | Tg8DateTime.Word 	=2= */

  Tg8Aux                          =368	/* 0x0170 	368	 |TYPE|  */
  aEvent                          =0	/* 0x0000 	0	 |VAR | Tg8Aux.Tg8Event 	=4= */
  aMsEvent                        =4	/* 0x0004 	4	 |VAR | Tg8Aux.Tg8Event 	=4= */
  aScLen                          =8	/* 0x0008 	8	 |VAR | Tg8Aux.Uint 	=4= */
  aScTime                         =12	/* 0x000c 	12	 |VAR | Tg8Aux.Uint 	=4= */
  aSc                             =16	/* 0x0010 	16	 |VAR | Tg8Aux.Uint 	=4= */
  aNumOfSc                        =20	/* 0x0014 	20	 |VAR | Tg8Aux.Uint 	=4= */
  aTrace                          =24	/* 0x0018 	24	 |VAR | Tg8Aux.Word 	=2= */
  aIntSrc                         =26	/* 0x001a 	26	 |VAR | Tg8Aux.Word 	=2= */
  aNumOfBus                       =28	/* 0x001c 	28	 |VAR | Tg8Aux.Word 	=2= */
  aNumOfSpur                      =30	/* 0x001e 	30	 |VAR | Tg8Aux.Word 	=2= */
  aMbox                           =32	/* 0x0020 	32	 |VAR | Tg8Aux.Word 	=2= */
  aCoco                           =34	/* 0x0022 	34	 |VAR | Tg8Aux.short 	=2= */
  aFwStat                         =36	/* 0x0024 	36	 |VAR | Tg8Aux.Word 	=2= */
  aSscHeader                      =38	/* 0x0026 	38	 |VAR | Tg8Aux.Word 	=2= */
  aNumOfEv                        =40	/* 0x0028 	40	 |VAR | Tg8Aux.Word 	=2= */
  aPrNumOfEv                      =42	/* 0x002a 	42	 |VAR | Tg8Aux.Word 	=2= */
  aNumOfAct                       =44	/* 0x002c 	44	 |VAR | Tg8Aux.Word 	=2= */
  aAlarms                         =46	/* 0x002e 	46	 |VAR | Tg8Aux.Word 	=2= */
  aDt                             =48	/* 0x0030 	48	 |VAR | Tg8Aux.Tg8DateTime 	=12= */
  aFwVer                          =60	/* 0x003c 	60	 |VAR | Tg8Aux.Byte[16] 	=16= */
  aSem                            =76	/* 0x004c 	76	 |VAR | Tg8Aux.Word 	=2= */
  aQueueSize                      =78	/* 0x004e 	78	 |VAR | Tg8Aux.Word 	=2= */
  aMaxQueueSize                   =80	/* 0x0050 	80	 |VAR | Tg8Aux.Word 	=2= */
  aMovedFrames                    =82	/* 0x0052 	82	 |VAR | Tg8Aux.Word 	=2= */
  aMovedScTime                    =84	/* 0x0054 	84	 |VAR | Tg8Aux.Uint 	=4= */
  aMovedSc                        =88	/* 0x0058 	88	 |VAR | Tg8Aux.Uint 	=4= */
  aProcFrames                     =92	/* 0x005c 	92	 |VAR | Tg8Aux.Word 	=2= */
  aModStatus                      =94	/* 0x005e 	94	 |VAR | Tg8Aux.Word 	=2= */
  aQueue                          =96	/* 0x0060 	96	 |VAR | Tg8Aux.Tg8Event[8] 	=32= */
  aTelegLHC                       =128	/* 0x0080 	128	 |VAR | Tg8Aux.Word[24] 	=48= */
  aTelegSPS                       =176	/* 0x00b0 	176	 |VAR | Tg8Aux.Word[24] 	=48= */
  aTelegCPS                       =224	/* 0x00e0 	224	 |VAR | Tg8Aux.Word[24] 	=48= */
  aTelegPSB                       =272	/* 0x0110 	272	 |VAR | Tg8Aux.Word[24] 	=48= */
  aTelegLEA                       =320	/* 0x0140 	320	 |VAR | Tg8Aux.Word[24] 	=48= */
  Tg8FS_RUNNING                   =1	/* 0x0001 	1	 |NUM | */

  Tg8FirmwareStatus               =4	/* 0x0004 	4	 |TYPE|  */
  Tg8FS_ALARMS                    =2	/* 0x0002 	2	 |NUM | */
  Tg8ALARM_OK                     =0	/* 0x0000 	0	 |NUM | */

  Tg8Alarm                        =4	/* 0x0004 	4	 |TYPE|  */
  Tg8ALARM_ISR                    =1	/* 0x0001 	1	 |NUM | */
  Tg8ALARM_LOST_IMM               =2	/* 0x0002 	2	 |NUM | */
  Tg8ALARM_LOST_OUT               =4	/* 0x0004 	4	 |NUM | */
  Tg8ALARM_MANY_PROC              =8	/* 0x0008 	8	 |NUM | */
  Tg8ALARM_QUEUED_PROC            =16	/* 0x0010 	16	 |NUM | */
  Tg8ALARM_DIFF_EVN               =32	/* 0x0020 	32	 |NUM | */
  Tg8ALARM_DIFF_LEN               =64	/* 0x0040 	64	 |NUM | */
  Tg8ALARM_MOVED_PROC             =128	/* 0x0080 	128	 |NUM | */
  Tg8ALARM_MOVED_IMM              =256	/* 0x0100 	256	 |NUM | */
  Tg8ALARM_ACT_DISBL              =512	/* 0x0200 	512	 |NUM | */
  Tg8ALARM_IMMQ_OVF               =1024	/* 0x0400 	1024	 |NUM | */
  Tg8ALARM_MBX_BUSY               =2048	/* 0x0800 	2048	 |NUM | */
  Tg8ALARM_UNCOM                  =4096	/* 0x1000 	4096	 |NUM | */
  Tg8ALARM_BAD_SWITCH             =8192	/* 0x2000 	8192	 |NUM | */
  Tg8ALARM_INT_LOST               =16384	/* 0x4000 	16384	 |NUM | */
  Tg8ST_OK                        =0	/* 0x0000 	0	 |NUM | */

  Tg8SelfTesrError                =4	/* 0x0004 	4	 |TYPE|  */
  Tg8ERR_OK                       =0	/* 0x0000 	0	 |NUM | */

  Tg8Error                        =4	/* 0x0004 	4	 |TYPE|  */
  Tg8ERR_ILLEGAL_OPERATION        =-1	/* 0xffffffff 	-1	 |NUM | */
  Tg8ERR_ILLEGAL_ARG              =-2	/* 0xfffffffe 	-2	 |NUM | */
  Tg8ERR_TIMEOUT                  =-3	/* 0xfffffffd 	-3	 |NUM | */
  Tg8ERR_WRONG_DIM                =-4	/* 0xfffffffc 	-4	 |NUM | */
  Tg8ERR_WRONG_DELAY              =-5	/* 0xfffffffb 	-5	 |NUM | */
  Tg8ERR_WRONG_CLOCK              =-6	/* 0xfffffffa 	-6	 |NUM | */
  Tg8ERR_WRONG_MODE               =-7	/* 0xfffffff9 	-7	 |NUM | */
  Tg8ERR_REJECTED                 =-8	/* 0xfffffff8 	-8	 |NUM | */
  Tg8ERR_BAD_OBJECT               =-9	/* 0xfffffff7 	-9	 |NUM | */
  Tg8ERR_NO_ACK                   =-10	/* 0xfffffff6 	-10	 |NUM | */
  Tg8ERR_NOT_RUN                  =-11	/* 0xfffffff5 	-11	 |NUM | */
  Tg8ERR_NO_FILE                  =-12	/* 0xfffffff4 	-12	 |NUM | */
  Tg8ERR_PENDING                  =-13	/* 0xfffffff3 	-13	 |NUM | */
  Tg8ERR_BUS_ERR                  =-14	/* 0xfffffff2 	-14	 |NUM | */
  Tg8ERR_FIRMWARE                 =-15	/* 0xfffffff1 	-15	 |NUM | */
  Tg8ERR_TABLE_FULL               =-16	/* 0xfffffff0 	-16	 |NUM | */
  Tg8ERR_BAD_VECT                 =-17	/* 0xffffffef 	-17	 |NUM | */
  Tg8ERR_BAD_SWITCH               =-18	/* 0xffffffee 	-18	 |NUM | */
  Tg8ERR_BAD_TRIGGER              =-19	/* 0xffffffed 	-19	 |NUM | */

  Tg8StatusBlock                  =32	/* 0x0020 	32	 |TYPE|  */
  Dt                              =0	/* 0x0000 	0	 |VAR | Tg8StatusBlock.Tg8DateTime 	=12= */
  ScTime                          =12	/* 0x000c 	12	 |VAR | Tg8StatusBlock.Uint 	=4= */
  Epc                             =16	/* 0x0010 	16	 |VAR | Tg8StatusBlock.Uint 	=4= */
  Evo                             =20	/* 0x0014 	20	 |VAR | Tg8StatusBlock.Word 	=2= */
  Hw                              =22	/* 0x0016 	22	 |VAR | Tg8StatusBlock.Word 	=2= */
  Fw                              =24	/* 0x0018 	24	 |VAR | Tg8StatusBlock.Word 	=2= */
  Cc                              =26	/* 0x001a 	26	 |VAR | Tg8StatusBlock.Word 	=2= */
  Am                              =28	/* 0x001c 	28	 |VAR | Tg8StatusBlock.Word 	=2= */
  SpareWS                         =30	/* 0x001e 	30	 |VAR | Tg8StatusBlock.Word 	=2= */
  Tg8DrvrMODULE_ENABLED           =1	/* 0x0001 	1	 |NUM | */

  Tg8DrvrSoftStatus               =4	/* 0x0004 	4	 |TYPE|  */
  Tg8DrvrMODULE_FIRMWARE_LOADED   =4	/* 0x0004 	4	 |NUM | */
  Tg8DrvrMODULE_USER_TABLE_VALID  =8	/* 0x0008 	8	 |NUM | */
  Tg8DrvrMODULE_RUNNING           =16	/* 0x0010 	16	 |NUM | */
  Tg8DrvrMODULE_FIRMWARE_ERROR    =32	/* 0x0020 	32	 |NUM | */
  Tg8DrvrMODULE_HARDWARE_ERROR    =64	/* 0x0040 	64	 |NUM | */
  Tg8DrvrMODULE_SWITCH_SET        =128	/* 0x0080 	128	 |NUM | */
  Tg8DrvrMODULE_TIMED_OUT         =256	/* 0x0100 	256	 |NUM | */
  Tg8DrvrDRIVER_DEBUG_ON          =512	/* 0x0200 	512	 |NUM | */
  Tg8DrvrDRIVER_ALARMS            =1024	/* 0x0400 	1024	 |NUM | */
  Tg8HARDWARE                     =0	/* 0x0000 	0	 |NUM | */
  MCP_I                           =0	/* 0x0000 	0	 |NUM | */
  McpERAM_START                   =0	/* 0x0000 	0	 |NUM | */
  McpBUG_END                      =12288	/* 0x3000 	12288	 |NUM | */
  McpERAM_END                     =65536	/* 0x10000 	65536	 |NUM | */
  McpPROM_START                   =393216	/* 0x60000 	393216	 |NUM | */
  McpPROM_END                     =524288	/* 0x80000 	524288	 |NUM | */
  McpSIM_BASE                     =-1536	/* 0xfffffa00 	-1536	 |NUM | */
  McpSBR_BASE                     =-1280	/* 0xfffffb00 	-1280	 |NUM | */
  McpQSM_BASE                     =-1024	/* 0xfffffc00 	-1024	 |NUM | */
  McpTPU_BASE                     =-512	/* 0xfffffe00 	-512	 |NUM | */
  McpTPU_ACT                      =-256	/* 0xffffff00 	-256	 |NUM | */
  McpSim_spare                    =-1536	/* 0xfffffa00 	-1536	 |NUM | */
  McpQsm_spare                    =-1024	/* 0xfffffc00 	-1024	 |NUM | */
  McpSbr_spare                    =-1280	/* 0xfffffb00 	-1280	 |NUM | */
  McpTpu_spare                    =-512	/* 0xfffffe00 	-512	 |NUM | */

  McpSim                          =-1408	/* 0xfffffa80 	-1408	 |TYPE|  */
  SimSpare_bytes                  =0	/* 0x0000 	0	 |VAR | McpSim.char[-1536] 	=-1536= */
  SimConf                         =-1536	/* 0xfffffa00 	-1536	 |VAR | McpSim.short 	=2= */
  SimTest                         =-1534	/* 0xfffffa02 	-1534	 |VAR | McpSim.short 	=2= */
  SimSync                         =-1532	/* 0xfffffa04 	-1532	 |VAR | McpSim.short 	=2= */
  SimReset                        =-1530	/* 0xfffffa06 	-1530	 |VAR | McpSim.short 	=2= */
  SimTestE                        =-1528	/* 0xfffffa08 	-1528	 |VAR | McpSim.short 	=2= */
  SimU0a                          =-1526	/* 0xfffffa0a 	-1526	 |VAR | McpSim.short 	=2= */
  SimU0c                          =-1524	/* 0xfffffa0c 	-1524	 |VAR | McpSim.short 	=2= */
  SimU0e                          =-1522	/* 0xfffffa0e 	-1522	 |VAR | McpSim.short 	=2= */
  SimDataE0                       =-1520	/* 0xfffffa10 	-1520	 |VAR | McpSim.short 	=2= */
  SimDataE1                       =-1518	/* 0xfffffa12 	-1518	 |VAR | McpSim.short 	=2= */
  SimDirE                         =-1516	/* 0xfffffa14 	-1516	 |VAR | McpSim.short 	=2= */
  SimPinE                         =-1514	/* 0xfffffa16 	-1514	 |VAR | McpSim.short 	=2= */
  SimDataF0                       =-1512	/* 0xfffffa18 	-1512	 |VAR | McpSim.short 	=2= */
  SimDataF1                       =-1510	/* 0xfffffa1a 	-1510	 |VAR | McpSim.short 	=2= */
  SimDirF                         =-1508	/* 0xfffffa1c 	-1508	 |VAR | McpSim.short 	=2= */
  SimPinF                         =-1506	/* 0xfffffa1e 	-1506	 |VAR | McpSim.short 	=2= */
  SimProtect                      =-1504	/* 0xfffffa20 	-1504	 |VAR | McpSim.short 	=2= */
  SimPic                          =-1502	/* 0xfffffa22 	-1502	 |VAR | McpSim.short 	=2= */
  SimPit                          =-1500	/* 0xfffffa24 	-1500	 |VAR | McpSim.short 	=2= */
  SimServ                         =-1498	/* 0xfffffa26 	-1498	 |VAR | McpSim.short 	=2= */
  SimU28                          =-1496	/* 0xfffffa28 	-1496	 |VAR | McpSim.short 	=2= */
  SimU2a                          =-1494	/* 0xfffffa2a 	-1494	 |VAR | McpSim.short 	=2= */
  SimU2c                          =-1492	/* 0xfffffa2c 	-1492	 |VAR | McpSim.short 	=2= */
  SimU2e                          =-1490	/* 0xfffffa2e 	-1490	 |VAR | McpSim.short 	=2= */
  SimTestMrA                      =-1488	/* 0xfffffa30 	-1488	 |VAR | McpSim.short 	=2= */
  SimTestMrB                      =-1486	/* 0xfffffa32 	-1486	 |VAR | McpSim.short 	=2= */
  SimTestScA                      =-1484	/* 0xfffffa34 	-1484	 |VAR | McpSim.char 	=1= */
  TstScB                          =-1483	/* 0xfffffa35 	-1483	 |VAR | McpSim.char 	=1= */
  SimTestRep                      =-1482	/* 0xfffffa36 	-1482	 |VAR | McpSim.short 	=2= */
  SimTestCtl                      =-1480	/* 0xfffffa38 	-1480	 |VAR | McpSim.short 	=2= */
  SimTestDis                      =-1478	/* 0xfffffa3a 	-1478	 |VAR | McpSim.short 	=2= */
  SimU3c                          =-1476	/* 0xfffffa3c 	-1476	 |VAR | McpSim.short 	=2= */
  SimU3e                          =-1474	/* 0xfffffa3e 	-1474	 |VAR | McpSim.short 	=2= */
  SimDataC                        =-1472	/* 0xfffffa40 	-1472	 |VAR | McpSim.short 	=2= */
  SimU42                          =-1470	/* 0xfffffa42 	-1470	 |VAR | McpSim.short 	=2= */
  SimCSpin                        =-1468	/* 0xfffffa44 	-1468	 |VAR | McpSim.int 	=4= */
  SimCSboot                       =-1464	/* 0xfffffa48 	-1464	 |VAR | McpSim.int 	=4= */
  SimCS0                          =-1460	/* 0xfffffa4c 	-1460	 |VAR | McpSim.int 	=4= */
  SimCS1                          =-1456	/* 0xfffffa50 	-1456	 |VAR | McpSim.int 	=4= */
  SimCS2                          =-1452	/* 0xfffffa54 	-1452	 |VAR | McpSim.int 	=4= */
  SimCS3                          =-1448	/* 0xfffffa58 	-1448	 |VAR | McpSim.int 	=4= */
  SimCS4                          =-1444	/* 0xfffffa5c 	-1444	 |VAR | McpSim.int 	=4= */
  SimCS5                          =-1440	/* 0xfffffa60 	-1440	 |VAR | McpSim.int 	=4= */
  SimCS6                          =-1436	/* 0xfffffa64 	-1436	 |VAR | McpSim.int 	=4= */
  SimCS7                          =-1432	/* 0xfffffa68 	-1432	 |VAR | McpSim.int 	=4= */
  SimCS8                          =-1428	/* 0xfffffa6c 	-1428	 |VAR | McpSim.int 	=4= */
  SimCS9                          =-1424	/* 0xfffffa70 	-1424	 |VAR | McpSim.int 	=4= */
  SimCS10                         =-1420	/* 0xfffffa74 	-1420	 |VAR | McpSim.int 	=4= */
  SimU78                          =-1416	/* 0xfffffa78 	-1416	 |VAR | McpSim.int 	=4= */
  SimU7c                          =-1412	/* 0xfffffa7c 	-1412	 |VAR | McpSim.int 	=4= */

  McpSbr                          =-1274	/* 0xfffffb06 	-1274	 |TYPE|  */
  SbrSpare_bytes                  =0	/* 0x0000 	0	 |VAR | McpSbr.char[-1280] 	=-1280= */
  SbrConf                         =-1280	/* 0xfffffb00 	-1280	 |VAR | McpSbr.short 	=2= */
  SbrTest                         =-1278	/* 0xfffffb02 	-1278	 |VAR | McpSbr.short 	=2= */
  SbrBaSt                         =-1276	/* 0xfffffb04 	-1276	 |VAR | McpSbr.short 	=2= */

  McpQsm                          =-688	/* 0xfffffd50 	-688	 |TYPE|  */
  QsmSpare_bytes                  =0	/* 0x0000 	0	 |VAR | McpQsm.char[-1024] 	=-1024= */
  QsmConf                         =-1024	/* 0xfffffc00 	-1024	 |VAR | McpQsm.short 	=2= */
  QsmTest                         =-1022	/* 0xfffffc02 	-1022	 |VAR | McpQsm.short 	=2= */
  QsmIntLev                       =-1020	/* 0xfffffc04 	-1020	 |VAR | McpQsm.char 	=1= */
  QsmIntVec                       =-1019	/* 0xfffffc05 	-1019	 |VAR | McpQsm.char 	=1= */
  QsmU06                          =-1018	/* 0xfffffc06 	-1018	 |VAR | McpQsm.short 	=2= */
  QsmSciC0                        =-1016	/* 0xfffffc08 	-1016	 |VAR | McpQsm.short 	=2= */
  QsmSciC1                        =-1014	/* 0xfffffc0a 	-1014	 |VAR | McpQsm.short 	=2= */
  QsmSciS                         =-1012	/* 0xfffffc0c 	-1012	 |VAR | McpQsm.short 	=2= */
  QsmSciD                         =-1010	/* 0xfffffc0e 	-1010	 |VAR | McpQsm.short 	=2= */
  QsmU10                          =-1008	/* 0xfffffc10 	-1008	 |VAR | McpQsm.short 	=2= */
  QsmU12                          =-1006	/* 0xfffffc12 	-1006	 |VAR | McpQsm.short 	=2= */
  QsmU14                          =-1004	/* 0xfffffc14 	-1004	 |VAR | McpQsm.char 	=1= */
  QsmPd                           =-1003	/* 0xfffffc15 	-1003	 |VAR | McpQsm.char 	=1= */
  QsmPin                          =-1002	/* 0xfffffc16 	-1002	 |VAR | McpQsm.char 	=1= */
  QsmDir                          =-1001	/* 0xfffffc17 	-1001	 |VAR | McpQsm.char 	=1= */
  QsmSpiC0                        =-1000	/* 0xfffffc18 	-1000	 |VAR | McpQsm.short 	=2= */
  QsmSpiC1                        =-998	/* 0xfffffc1a 	-998	 |VAR | McpQsm.short 	=2= */
  QsmSpiC2                        =-996	/* 0xfffffc1c 	-996	 |VAR | McpQsm.short 	=2= */
  QsmSpiC3                        =-994	/* 0xfffffc1e 	-994	 |VAR | McpQsm.char 	=1= */
  QsmSpiS                         =-993	/* 0xfffffc1f 	-993	 |VAR | McpQsm.char 	=1= */
  QsmU20                          =-992	/* 0xfffffc20 	-992	 |VAR | McpQsm.char[224] 	=224= */
  QsmRecv_ram                     =-768	/* 0xfffffd00 	-768	 |VAR | McpQsm.short[16] 	=32= */
  QsmTran_ram                     =-736	/* 0xfffffd20 	-736	 |VAR | McpQsm.short[16] 	=32= */
  QsmComd_ram                     =-704	/* 0xfffffd40 	-704	 |VAR | McpQsm.char[16] 	=16= */

  McpTpu                          =-472	/* 0xfffffe28 	-472	 |TYPE|  */
  TpuSpare_bytes                  =0	/* 0x0000 	0	 |VAR | McpTpu.char[-512] 	=-512= */
  TpuConf                         =-512	/* 0xfffffe00 	-512	 |VAR | McpTpu.short 	=2= */
  TpuCR                           =-510	/* 0xfffffe02 	-510	 |VAR | McpTpu.short 	=2= */
  TpuDsc                          =-508	/* 0xfffffe04 	-508	 |VAR | McpTpu.short 	=2= */
  TpuDss                          =-506	/* 0xfffffe06 	-506	 |VAR | McpTpu.short 	=2= */
  TpuIc                           =-504	/* 0xfffffe08 	-504	 |VAR | McpTpu.short 	=2= */
  TpuIe                           =-502	/* 0xfffffe0a 	-502	 |VAR | McpTpu.short 	=2= */
  TpuCfs0                         =-500	/* 0xfffffe0c 	-500	 |VAR | McpTpu.short 	=2= */
  TpuCfs1                         =-498	/* 0xfffffe0e 	-498	 |VAR | McpTpu.short 	=2= */
  TpuCfs2                         =-496	/* 0xfffffe10 	-496	 |VAR | McpTpu.short 	=2= */
  TpuCfs3                         =-494	/* 0xfffffe12 	-494	 |VAR | McpTpu.short 	=2= */
  TpuHs0                          =-492	/* 0xfffffe14 	-492	 |VAR | McpTpu.short 	=2= */
  TpuHs1                          =-490	/* 0xfffffe16 	-490	 |VAR | McpTpu.short 	=2= */
  TpuHsr0                         =-488	/* 0xfffffe18 	-488	 |VAR | McpTpu.short 	=2= */
  TpuHsr1                         =-486	/* 0xfffffe1a 	-486	 |VAR | McpTpu.short 	=2= */
  TpuCp0                          =-484	/* 0xfffffe1c 	-484	 |VAR | McpTpu.short 	=2= */
  TpuCp1                          =-482	/* 0xfffffe1e 	-482	 |VAR | McpTpu.short 	=2= */
  TpuIs                           =-480	/* 0xfffffe20 	-480	 |VAR | McpTpu.short 	=2= */
  TpuLink                         =-478	/* 0xfffffe22 	-478	 |VAR | McpTpu.short 	=2= */
  TpuSgl                          =-476	/* 0xfffffe24 	-476	 |VAR | McpTpu.short 	=2= */
  TpuDcn                          =-474	/* 0xfffffe26 	-474	 |VAR | McpTpu.short 	=2= */
  McpCS_BA_ADDRESS_MASK           =-524288	/* 0xfff80000 	-524288	 |NUM | */
  McpCS_BA_ADDRESS_SHIFT          =8	/* 0x0008 	8	 |NUM | */
  McpCS_BA_BLKSZ_2K               =0	/* 0x0000 	0	 |NUM | */
  McpCS_BA_BLKSZ_8K               =65536	/* 0x10000 	65536	 |NUM | */
  McpCS_BA_BLKSZ_16K              =131072	/* 0x20000 	131072	 |NUM | */
  McpCS_BA_BLKSZ_64K              =196608	/* 0x30000 	196608	 |NUM | */
  McpCS_BA_BLKSZ_128K             =262144	/* 0x40000 	262144	 |NUM | */
  McpCS_BA_BLKSZ_256K             =327680	/* 0x50000 	327680	 |NUM | */
  McpCS_BA_BLKSZ_512K             =393216	/* 0x60000 	393216	 |NUM | */
  McpCS_BA_BLKSZ_1M               =458752	/* 0x70000 	458752	 |NUM | */
  McpCS_OP_MODE_SYNC              =32768	/* 0x8000 	32768	 |NUM | */
  McpCS_OP_MODE_ASYNC             =0	/* 0x0000 	0	 |NUM | */
  McpCS_OP_BYTE_DISABLE           =0	/* 0x0000 	0	 |NUM | */
  McpCS_OP_BYTE_LOWER             =8192	/* 0x2000 	8192	 |NUM | */
  McpCS_OP_BYTE_UPPER             =16384	/* 0x4000 	16384	 |NUM | */
  McpCS_OP_BYTE_BOTH              =24576	/* 0x6000 	24576	 |NUM | */
  McpCS_OP_RW_RO                  =2048	/* 0x0800 	2048	 |NUM | */
  McpCS_OP_RW_WO                  =4096	/* 0x1000 	4096	 |NUM | */
  McpCS_OP_RW_RW                  =6144	/* 0x1800 	6144	 |NUM | */
  McpCS_OP_STRB_ADDRESS           =0	/* 0x0000 	0	 |NUM | */
  McpCS_OP_STRB_DATA              =1024	/* 0x0400 	1024	 |NUM | */
  McpCS_OP_DSACK_NOWAIT           =0	/* 0x0000 	0	 |NUM | */
  McpCS_OP_DSACK_WAIT_1           =64	/* 0x0040 	64	 |NUM | */
  McpCS_OP_DSACK_WAIT_2           =128	/* 0x0080 	128	 |NUM | */
  McpCS_OP_DSACK_WAIT_3           =192	/* 0x00c0 	192	 |NUM | */
  McpCS_OP_DSACK_WAIT_4           =256	/* 0x0100 	256	 |NUM | */
  McpCS_OP_DSACK_WAIT_5           =320	/* 0x0140 	320	 |NUM | */
  McpCS_OP_DSACK_WAIT_6           =384	/* 0x0180 	384	 |NUM | */
  McpCS_OP_DSACK_WAIT_7           =448	/* 0x01c0 	448	 |NUM | */
  McpCS_OP_DSACK_WAIT_8           =512	/* 0x0200 	512	 |NUM | */
  McpCS_OP_DSACK_WAIT_9           =576	/* 0x0240 	576	 |NUM | */
  McpCS_OP_DSACK_WAIT_10          =640	/* 0x0280 	640	 |NUM | */
  McpCS_OP_DSACK_WAIT_11          =704	/* 0x02c0 	704	 |NUM | */
  McpCS_OP_DSACK_WAIT_12          =768	/* 0x0300 	768	 |NUM | */
  McpCS_OP_DSACK_WAIT_13          =832	/* 0x0340 	832	 |NUM | */
  McpCS_OP_DSACK_FAST             =896	/* 0x0380 	896	 |NUM | */
  McpCS_OP_DSACK_EXTERNAL         =960	/* 0x03c0 	960	 |NUM | */
  McpCS_OP_SPACE_CPU              =0	/* 0x0000 	0	 |NUM | */
  McpCS_OP_SPACE_USER             =16	/* 0x0010 	16	 |NUM | */
  McpCS_OP_SPACE_SUPER            =32	/* 0x0020 	32	 |NUM | */
  McpCS_OP_SPACE_BOTH             =48	/* 0x0030 	48	 |NUM | */
  McpCS_OP_IPL_ANY                =0	/* 0x0000 	0	 |NUM | */
  McpCS_OP_IPL_1                  =2	/* 0x0002 	2	 |NUM | */
  McpCS_OP_IPL_2                  =4	/* 0x0004 	4	 |NUM | */
  McpCS_OP_IPL_3                  =6	/* 0x0006 	6	 |NUM | */
  McpCS_OP_IPL_4                  =8	/* 0x0008 	8	 |NUM | */
  McpCS_OP_IPL_5                  =10	/* 0x000a 	10	 |NUM | */
  McpCS_OP_IPL_6                  =12	/* 0x000c 	12	 |NUM | */
  McpCS_OP_IPL_7                  =14	/* 0x000e 	14	 |NUM | */
  McpCS_OP_AVEC_EXTERNAL          =0	/* 0x0000 	0	 |NUM | */
  McpCS_OP_AVEC_ENABLED           =1	/* 0x0001 	1	 |NUM | */
  McpVECTOR_BUS_ERROR             =8	/* 0x0008 	8	 |NUM | */
  McpVECTOR_ADDRESS_ERROR         =12	/* 0x000c 	12	 |NUM | */
  McpVECTOR_ILLEGAL_INSTRUCTION   =16	/* 0x0010 	16	 |NUM | */
  McpVECTOR_ZERO_DIVISION         =20	/* 0x0014 	20	 |NUM | */
  McpVECTOR_PRIVILEGE_VIOLATION   =32	/* 0x0020 	32	 |NUM | */
  McpVECTOR_SPURIOUS_INTERRUPT    =96	/* 0x0060 	96	 |NUM | */
  McpVECTOR_AUTO_1                =100	/* 0x0064 	100	 |NUM | */
  McpVECTOR_AUTO_2                =104	/* 0x0068 	104	 |NUM | */
  McpVECTOR_AUTO_3                =108	/* 0x006c 	108	 |NUM | */
  McpVECTOR_AUTO_4                =112	/* 0x0070 	112	 |NUM | */
  McpVECTOR_AUTO_5                =116	/* 0x0074 	116	 |NUM | */
  McpVECTOR_AUTO_6                =120	/* 0x0078 	120	 |NUM | */
  McpVECTOR_AUTO_7                =124	/* 0x007c 	124	 |NUM | */
  McpTRAP_VECTOR_BASE             =32	/* 0x0020 	32	 |NUM | */
  Tg8DISABLE_INTERRUPTS           =9984	/* 0x2700 	9984	 |NUM | */
  Tg8ENABLE_INTERRUPTS            =9216	/* 0x2400 	9216	 |NUM | */
  Tg8ENABLE_WATCHDOG              =160	/* 0x00a0 	160	 |NUM | */
  Tg8ENABLE_BUSMONITOR            =4	/* 0x0004 	4	 |NUM | */
  Tg8RAM_BASE                     =0	/* 0x0000 	0	 |NUM | */
  Tg8DPM_BASE                     =65536	/* 0x10000 	65536	 |NUM | */
  Tg8CAM_BASE                     =69632	/* 0x11000 	69632	 |NUM | */
  Tg8XLX_BASE                     =73728	/* 0x12000 	73728	 |NUM | */
  Tg8DSC_VECTOR                   =112	/* 0x0070 	112	 |NUM | */
  Tg8DPM_VECTOR                   =116	/* 0x0074 	116	 |NUM | */
  Tg8XLX_VECTOR                   =120	/* 0x0078 	120	 |NUM | */
  Tg8ABT_VECTOR                   =124	/* 0x007c 	124	 |NUM | */
  Tg8VME_INTERRUPT                =67580	/* 0x107fc 	67580	 |NUM | */
  Tg8DSC_INTERRUPT                =67582	/* 0x107fe 	67582	 |NUM | */
  Tg8TPU_BASE_VECTOR              =64	/* 0x0040 	64	 |NUM | */
  Tg8TPU_INT_LEVEL                =7	/* 0x0007 	7	 |NUM | */
  Tg8RAM_SIZE                     =2048	/* 0x0800 	2048	 |NUM | */

  Tg8Ram                          =2048	/* 0x0800 	2048	 |TYPE| char[2048] */
  Tg8COUNTERS                     =8	/* 0x0008 	8	 |NUM | */
  Tg8Xlx_spare                    =73728	/* 0x12000 	73728	 |NUM | */

  Tg8Xlx                          =73766	/* 0x12026 	73766	 |TYPE|  */
  XlxSpare_Bytes                  =0	/* 0x0000 	0	 |VAR | Tg8Xlx.char[73728] 	=73728= */
  XlxDelay                        =73728	/* 0x12000 	73728	 |VAR | Tg8Xlx.short[8] 	=16= */
  XlxConfig                       =73744	/* 0x12010 	73744	 |VAR | Tg8Xlx.short[8] 	=16= */
  XWsscRframe1                    =73760	/* 0x12020 	73760	 |VAR | Tg8Xlx.short 	=2= */
  XRframe2                        =73762	/* 0x12022 	73762	 |VAR | Tg8Xlx.short 	=2= */
  WClrRerr                        =73764	/* 0x12024 	73764	 |VAR | Tg8Xlx.short 	=2= */
  ConfOUTPUT                      =1	/* 0x0001 	1	 |NUM | */

  Tg8CounterConfig                =4	/* 0x0004 	4	 |TYPE|  */
  ConfINTERRUPT                   =4	/* 0x0004 	4	 |NUM | */
  ConfCABLE_CLOCK                 =8	/* 0x0008 	8	 |NUM | */
  ConfEXT_CLOCK_1                 =16	/* 0x0010 	16	 |NUM | */
  ConfEXT_CLOCK_2                 =24	/* 0x0018 	24	 |NUM | */
  ConfEXT_START                   =32	/* 0x0020 	32	 |NUM | */
  ConfNORMAL                      =64	/* 0x0040 	64	 |NUM | */
  ConfCHAINE                      =128	/* 0x0080 	128	 |NUM | */
  ConfBURST                       =192	/* 0x00c0 	192	 |NUM | */
  XrDATA_OVERFLOW                 =1	/* 0x0001 	1	 |NUM | */

  Tg8XrHardwareStatus             =4	/* 0x0004 	4	 |TYPE|  */
  XrPARITY_ERROR                  =2	/* 0x0002 	2	 |NUM | */
  XrEND_SEQUENCE                  =4	/* 0x0004 	4	 |NUM | */
  XrMID_BIT                       =8	/* 0x0008 	8	 |NUM | */
  XrSTART_SEQUENCE                =16	/* 0x0010 	16	 |NUM | */
  XrDISABLED                      =32	/* 0x0020 	32	 |NUM | */
  XrMS_MISSING                    =64	/* 0x0040 	64	 |NUM | */
  XrMS_WATCH_DOG                  =128	/* 0x0080 	128	 |NUM | */
  Tg8HS_COUNTER_1_INTERRUPT       =1	/* 0x0001 	1	 |NUM | */

  Tg8HardwareStatus               =4	/* 0x0004 	4	 |TYPE|  */
  Tg8HS_COUNTER_2_INTERRUPT       =2	/* 0x0002 	2	 |NUM | */
  Tg8HS_COUNTER_3_INTERRUPT       =4	/* 0x0004 	4	 |NUM | */
  Tg8HS_COUNTER_4_INTERRUPT       =8	/* 0x0008 	8	 |NUM | */
  Tg8HS_COUNTER_5_INTERRUPT       =16	/* 0x0010 	16	 |NUM | */
  Tg8HS_COUNTER_6_INTERRUPT       =32	/* 0x0020 	32	 |NUM | */
  Tg8HS_COUNTER_7_INTERRUPT       =64	/* 0x0040 	64	 |NUM | */
  Tg8HS_COUNTER_8_INTERRUPT       =128	/* 0x0080 	128	 |NUM | */
  Tg8HS_DPRAM_INTERRUPT           =256	/* 0x0100 	256	 |NUM | */
  Tg8HS_RECEIVER_ENABLED          =512	/* 0x0200 	512	 |NUM | */
  Tg8HS_PENDING_DOWNLOAD          =1024	/* 0x0400 	1024	 |NUM | */
  Tg8HS_XILINX_XR_LOADED          =2048	/* 0x0800 	2048	 |NUM | */
  Tg8HS_XILINX_X1_LOADED          =4096	/* 0x1000 	4096	 |NUM | */
  Tg8HS_XILINX_X2_LOADED          =8192	/* 0x2000 	8192	 |NUM | */
  Tg8HS_EXTERNAL_CLOCK_JUMPER     =16384	/* 0x4000 	16384	 |NUM | */
  Tg8HS_SELF_TEST_ERROR           =32768	/* 0x8000 	32768	 |NUM | */
  Tg8INTERRUPT_MASK               =511	/* 0x01ff 	511	 |NUM | */
  Tg8HW_OK_STATUS                 =14336	/* 0x3800 	14336	 |NUM | */
  ClrINT_CHANNEL_0                =65534	/* 0xfffe 	65534	 |NUM | */

  Tg8TpuClearInterrupt            =4	/* 0x0004 	4	 |TYPE|  */
  ClrINT_CHANNEL_1                =65533	/* 0xfffd 	65533	 |NUM | */
  ClrINT_CHANNEL_2                =65531	/* 0xfffb 	65531	 |NUM | */
  ClrINT_CHANNEL_3                =65527	/* 0xfff7 	65527	 |NUM | */
  ClrINT_CHANNEL_4                =65519	/* 0xffef 	65519	 |NUM | */
  ClrINT_CHANNEL_5                =65503	/* 0xffdf 	65503	 |NUM | */
  ClrINT_CHANNEL_6                =65471	/* 0xffbf 	65471	 |NUM | */
  ClrINT_CHANNEL_7                =65407	/* 0xff7f 	65407	 |NUM | */
  ClrINT_CHANNEL_8                =65279	/* 0xfeff 	65279	 |NUM | */
  Tg8CAM_SIZE                     =256	/* 0x0100 	256	 |NUM | */
  Tg8ACTIONS_NUMBER               =256	/* 0x0100 	256	 |NUM | */
  CamDATA_MODE                    =2	/* 0x0002 	2	 |NUM | */

  Tg8CamControl                   =4	/* 0x0004 	4	 |TYPE|  */
  CamCOMMAND_MODE                 =65533	/* 0xfffd 	65533	 |NUM | */
  CamWRITE                        =57344	/* 0xe000 	57344	 |NUM | */
  CamCLEAR_SKIP_BITS              =38912	/* 0x9800 	38912	 |NUM | */
  CamSET_SKIP_BIT                 =37888	/* 0x9400 	37888	 |NUM | */
  CamSTATUS_MULTIPLE              =14	/* 0x000e 	14	 |NUM | */
  CamMATCH                        =32768	/* 0x8000 	32768	 |NUM | */
  CamMUL                          =16384	/* 0x4000 	16384	 |NUM | */
  CamBYTE_MASK                    =255	/* 0x00ff 	255	 |NUM | */
  SelfTestOk                      =4	/* 0x0004 	4	 |NUM | */

  OkLedEnum                       =4	/* 0x0004 	4	 |TYPE|  */
  OkLed                           =8	/* 0x0008 	8	 |NUM | */
  Tg8CSBOOT                       =100952240	/* 0x60468b0 	100952240	 |NUM | */
  Tg8CS0                          =217150	/* 0x3503e 	217150	 |NUM | */
  Tg8CS1                          =208958	/* 0x3303e 	208958	 |NUM | */
  Tg8CS2                          =223294	/* 0x3683e 	223294	 |NUM | */
  Tg8CS3                          =18906160	/* 0x1207c30 	18906160	 |NUM | */
  Tg8CS4                          =-497649	/* 0xfff8680f 	-497649	 |NUM | */
  Tg8CS5                          =-497651	/* 0xfff8680d 	-497651	 |NUM | */
  Tg8CS6                          =17855536	/* 0x1107430 	17855536	 |NUM | */
  Tg8CS7                          =17852464	/* 0x1106830 	17852464	 |NUM | */
  Tg8CS8                          =16808944	/* 0x1007bf0 	16808944	 |NUM | */
  Tg8CS9                          =0	/* 0x0000 	0	 |NUM | */
  Tg8CS10                         =0	/* 0x0000 	0	 |NUM | */
  Tg8SELF_TEST                    =0	/* 0x0000 	0	 |NUM | */
  MbxST_GO                        =1234	/* 0x04d2 	1234	 |NUM | */

  MbxStatus                       =4	/* 0x0004 	4	 |TYPE|  */
  MbxST_BUSY                      =5678	/* 0x162e 	5678	 |NUM | */
  MbxST_DONE                      =9012	/* 0x2334 	9012	 |NUM | */
  MbxST_BUS_ERROR                 =3456	/* 0x0d80 	3456	 |NUM | */
  MbxOP_READ                      =-1	/* 0xffffffff 	-1	 |NUM | */

  MbxOperation                    =4	/* 0x0004 	4	 |TYPE|  */
  MbxOP_WRITE                     =1	/* 0x0001 	1	 |NUM | */
  MbxOP_NONE                      =0	/* 0x0000 	0	 |NUM | */
  MbxCC_BUSY                      =-1	/* 0xffffffff 	-1	 |NUM | */

  MbxComplCode                    =4	/* 0x0004 	4	 |TYPE|  */
  MbxCC_DONE                      =1	/* 0x0001 	1	 |NUM | */
  MbxCC_BOOT                      =14402	/* 0x3842 	14402	 |NUM | */
  MbxCC_332Bug                    =14404	/* 0x3844 	14404	 |NUM | */
  MbxCC_DOWNLOAD                  =14422	/* 0x3856 	14422	 |NUM | */
  MbxCC_DOWNLOAD_PENDING          =18040	/* 0x4678 	18040	 |NUM | */
  MbxCC_FIRMWARE_RUNNING          =30583	/* 0x7777 	30583	 |NUM | */
  Tg8DPRAM_SIZE                   =2048	/* 0x0800 	2048	 |NUM | */
  StCOUNTERS                      =8	/* 0x0008 	8	 |NUM | */

  StMemory                        =4	/* 0x0004 	4	 |TYPE|  */
  Bot                             =0	/* 0x0000 	0	 |VAR | StMemory.Ushort 	=2= */
  Top                             =2	/* 0x0002 	2	 |VAR | StMemory.Ushort 	=2= */

  StFault                         =20	/* 0x0014 	20	 |TYPE|  */
  Code                            =0	/* 0x0000 	0	 |VAR | StFault.int 	=4= */
  Templ                           =4	/* 0x0004 	4	 |VAR | StFault.int 	=4= */
  Actual                          =8	/* 0x0008 	8	 |VAR | StFault.int 	=4= */
  At                              =12	/* 0x000c 	12	 |VAR | StFault.Ushort 	=2= */
  Dir                             =14	/* 0x000e 	14	 |VAR | StFault.Ushort 	=2= */
  BusInt                          =16	/* 0x0010 	16	 |VAR | StFault.Ushort 	=2= */
  N_of_bus                        =18	/* 0x0012 	18	 |VAR | StFault.Ushort 	=2= */

  StDateTime                      =16	/* 0x0010 	16	 |TYPE|  */
  Year                            =0	/* 0x0000 	0	 |VAR | StDateTime.short 	=2= */
  Month                           =2	/* 0x0002 	2	 |VAR | StDateTime.short 	=2= */
  Day                             =4	/* 0x0004 	4	 |VAR | StDateTime.short 	=2= */
  Hour                            =6	/* 0x0006 	6	 |VAR | StDateTime.short 	=2= */
  Minute                          =8	/* 0x0008 	8	 |VAR | StDateTime.short 	=2= */
  Second                          =10	/* 0x000a 	10	 |VAR | StDateTime.short 	=2= */
  Ms                              =12	/* 0x000c 	12	 |VAR | StDateTime.int 	=4= */

  StDpmInfo                       =348	/* 0x015c 	348	 |TYPE|  */
  N_of_bus                        =0	/* 0x0000 	0	 |VAR | StDpmInfo.int 	=4= */
  N_of_frames                     =4	/* 0x0004 	4	 |VAR | StDpmInfo.int 	=4= */
  EventFrame                      =8	/* 0x0008 	8	 |VAR | StDpmInfo.int 	=4= */
  CycleNum                        =12	/* 0x000c 	12	 |VAR | StDpmInfo.int 	=4= */
  CycleDur                        =16	/* 0x0010 	16	 |VAR | StDpmInfo.short 	=2= */
  CTrain                          =18	/* 0x0012 	18	 |VAR | StDpmInfo.short 	=2= */
  Date                            =20	/* 0x0014 	20	 |VAR | StDpmInfo.StDateTime 	=16= */
  TestProg                        =36	/* 0x0024 	36	 |VAR | StDpmInfo.Ushort 	=2= */
  TestPassed                      =38	/* 0x0026 	38	 |VAR | StDpmInfo.Ushort 	=2= */
  FaultType                       =40	/* 0x0028 	40	 |VAR | StDpmInfo.Ushort 	=2= */
  FaultCnt                        =42	/* 0x002a 	42	 |VAR | StDpmInfo.Ushort 	=2= */
  Err                             =0	/* 0x0000 	0	 |VAR | _su_.StFault 	=20= */
  Mem                             =20	/* 0x0014 	20	 |VAR | _su_.StMemory 	=4= */
  Ram                             =44	/* 0x002c 	44	 |VAR | StDpmInfo._su_ 	=24= */
  Err                             =0	/* 0x0000 	0	 |VAR | _su_.StFault 	=20= */
  Mem                             =20	/* 0x0014 	20	 |VAR | _su_.StMemory 	=4= */
  Dpram                           =68	/* 0x0044 	68	 |VAR | StDpmInfo._su_ 	=24= */
  Err                             =0	/* 0x0000 	0	 |VAR | _su_.StFault 	=20= */
  Mem                             =20	/* 0x0014 	20	 |VAR | _su_.StMemory 	=4= */
  At                              =0	/* 0x0000 	0	 |VAR | _su_.Ushort 	=2= */
  Cnt                             =2	/* 0x0002 	2	 |VAR | _su_.Ushort 	=2= */
  Match                           =24	/* 0x0018 	24	 |VAR | _su_._su_ 	=4= */
  Cam                             =92	/* 0x005c 	92	 |VAR | StDpmInfo._su_ 	=28= */
  Err                             =0	/* 0x0000 	0	 |VAR | _su_.StFault 	=20= */
  Rcv                             =20	/* 0x0014 	20	 |VAR | _su_.Ushort 	=2= */
  Ssc                             =22	/* 0x0016 	22	 |VAR | _su_.Ushort 	=2= */
  Xilinx                          =120	/* 0x0078 	120	 |VAR | StDpmInfo._su_ 	=24= */
  Err                             =0	/* 0x0000 	0	 |VAR | _su_.StFault[8] 	=160= */
  Bad                             =160	/* 0x00a0 	160	 |VAR | _su_.Ushort 	=2= */
  Delay                           =162	/* 0x00a2 	162	 |VAR | _su_.Ushort 	=2= */
  Counter                         =144	/* 0x0090 	144	 |VAR | StDpmInfo._su_ 	=164= */
  DscInt                          =308	/* 0x0134 	308	 |VAR | StDpmInfo.int[9] 	=36= */
  MbxInt                          =344	/* 0x0158 	344	 |VAR | StDpmInfo.int 	=4= */

  StDpmHead                       =12	/* 0x000c 	12	 |TYPE|  */
  IoDirection                     =0	/* 0x0000 	0	 |VAR | StDpmHead.short 	=2= */
  DoneFlag                        =2	/* 0x0002 	2	 |VAR | StDpmHead.short 	=2= */
  FirmwareStatus                  =4	/* 0x0004 	4	 |VAR | StDpmHead.short 	=2= */
  VectorOffset                    =6	/* 0x0006 	6	 |VAR | StDpmHead.short 	=2= */
  ExceptionPC                     =8	/* 0x0008 	8	 |VAR | StDpmHead.int 	=4= */

  StDpm                           =2054	/* 0x0806 	2054	 |TYPE|  */
  Head                            =0	/* 0x0000 	0	 |VAR | StDpm.StDpmHead 	=12= */
  Info                            =12	/* 0x000c 	12	 |VAR | StDpm.StDpmInfo 	=348= */
  Buffer                          =360	/* 0x0168 	360	 |VAR | StDpm.short[841] 	=1682= */
  XilinxStatus                    =2042	/* 0x07fa 	2042	 |VAR | StDpm.short 	=2= */
  InterruptDsc                    =2044	/* 0x07fc 	2044	 |VAR | StDpm.short 	=2= */
  InterruptMcp                    =2046	/* 0x07fe 	2046	 |VAR | StDpm.short 	=2= */
  HardwareStatus                  =2048	/* 0x0800 	2048	 |VAR | StDpm.short 	=2= */
  Vector                          =2050	/* 0x0802 	2050	 |VAR | StDpm.short 	=2= */
  CounterInterruptMask            =2052	/* 0x0804 	2052	 |VAR | StDpm.short 	=2= */
  BI_INIT                         =1	/* 0x0001 	1	 |NUM | */

  StBusInt                        =4	/* 0x0004 	4	 |TYPE|  */
  BI_RAM                          =2	/* 0x0002 	2	 |NUM | */
  BI_RAM_TEST                     =3	/* 0x0003 	3	 |NUM | */
  BI_DPRAM                        =4	/* 0x0004 	4	 |NUM | */
  BI_DPRAM_TEST                   =5	/* 0x0005 	5	 |NUM | */
  BI_CAM                          =6	/* 0x0006 	6	 |NUM | */
  BI_CAM_TEST                     =7	/* 0x0007 	7	 |NUM | */
  BI_XILINX                       =8	/* 0x0008 	8	 |NUM | */
  BI_XILINX_TEST                  =9	/* 0x0009 	9	 |NUM | */
  XI_READF1                       =10	/* 0x000a 	10	 |NUM | */
  XI_READF2                       =11	/* 0x000b 	11	 |NUM | */
  XI_READERR                      =12	/* 0x000c 	12	 |NUM | */
  XI_RESETERR                     =13	/* 0x000d 	13	 |NUM | */
  XI_SSC                          =14	/* 0x000e 	14	 |NUM | */
  BI_COUNTER_TEST                 =15	/* 0x000f 	15	 |NUM | */
  CI_DELAY                        =16	/* 0x0010 	16	 |NUM | */
  CI_CONFIG                       =17	/* 0x0011 	17	 |NUM | */
  N_OF_BI                         =18	/* 0x0012 	18	 |NUM | */
  D_FORWARD                       =0	/* 0x0000 	0	 |NUM | */

  StDirection                     =4	/* 0x0004 	4	 |TYPE|  */
  D_BACKWARD                      =1	/* 0x0001 	1	 |NUM | */
  N_OF_DIR                        =2	/* 0x0002 	2	 |NUM | */
  A_BYTE                          =0	/* 0x0000 	0	 |NUM | */

  StAccess                        =4	/* 0x0004 	4	 |TYPE|  */
  A_WORD                          =1	/* 0x0001 	1	 |NUM | */
  A_INT32                         =2	/* 0x0002 	2	 |NUM | */
  N_OF_ACC                        =3	/* 0x0003 	3	 |NUM | */
  TM_AA                           =0	/* 0x0000 	0	 |NUM | */

  StTemplate                      =4	/* 0x0004 	4	 |TYPE|  */
  TM_55                           =1	/* 0x0001 	1	 |NUM | */
  TM_00                           =2	/* 0x0002 	2	 |NUM | */
  N_OF_TEMPL                      =3	/* 0x0003 	3	 |NUM | */
  TC_A                            =1	/* 0x0001 	1	 |NUM | */

  StCamTemplate                   =4	/* 0x0004 	4	 |TYPE|  */
  TC_B                            =2	/* 0x0002 	2	 |NUM | */
  TC_C                            =3	/* 0x0003 	3	 |NUM | */
  TC_D                            =4	/* 0x0004 	4	 |NUM | */
  TC_E                            =5	/* 0x0005 	5	 |NUM | */
  CO_COUNT_FAST                   =1	/* 0x0001 	1	 |NUM | */

  StCounting                      =4	/* 0x0004 	4	 |TYPE|  */
  CO_BLOCKED                      =2	/* 0x0002 	2	 |NUM | */
  CO_NO_DSC_INT                   =3	/* 0x0003 	3	 |NUM | */
  N_OF_CONT_ER                    =4	/* 0x0004 	4	 |NUM | */
  T_RAM                           =1	/* 0x0001 	1	 |NUM | */

  StTest                          =4	/* 0x0004 	4	 |TYPE|  */
  T_DPRAM                         =2	/* 0x0002 	2	 |NUM | */
  T_CAM                           =4	/* 0x0004 	4	 |NUM | */
  T_XILINX                        =8	/* 0x0008 	8	 |NUM | */
  T_MS                            =16	/* 0x0010 	16	 |NUM | */
  T_COUNTERS                      =32	/* 0x0020 	32	 |NUM | */
  E_CAM_RW                        =1	/* 0x0001 	1	 |NUM | */

  StErrCode                       =4	/* 0x0004 	4	 |TYPE|  */
  E_CAM_MATCH                     =2	/* 0x0002 	2	 |NUM | */
  Tg8OP_NO_COMMAND                =0	/* 0x0000 	0	 |NUM | */

  Tg8Operation                    =4	/* 0x0004 	4	 |TYPE|  */
  Tg8OP_PING_MODULE               =1	/* 0x0001 	1	 |NUM | */
  Tg8OP_GET_STATUS                =2	/* 0x0002 	2	 |NUM | */
  Tg8OP_SET_SSC_HEADER            =3	/* 0x0003 	3	 |NUM | */
  Tg8OP_SET_STATE                 =4	/* 0x0004 	4	 |NUM | */
  Tg8OP_SET_DELAY                 =5	/* 0x0005 	5	 |NUM | */
  Tg8OP_CLEAR_USER_TABLE          =6	/* 0x0006 	6	 |NUM | */
  Tg8OP_SET_USER_TABLE            =7	/* 0x0007 	7	 |NUM | */
  Tg8OP_GET_USER_TABLE            =8	/* 0x0008 	8	 |NUM | */
  Tg8OP_GET_RECORDING_TABLE       =9	/* 0x0009 	9	 |NUM | */
  Tg8OP_GET_HISTORY_TABLE         =10	/* 0x000a 	10	 |NUM | */
  Tg8OP_GET_CLOCK_TABLE           =11	/* 0x000b 	11	 |NUM | */
  Tg8OP_SET_PPM_TIMING            =12	/* 0x000c 	12	 |NUM | */
  Tg8OP_SET_GATE                  =13	/* 0x000d 	13	 |NUM | */
  Tg8OP_SET_DIM                   =14	/* 0x000e 	14	 |NUM | */
  Tg8OP_GET_PPM_LINE              =15	/* 0x000f 	15	 |NUM | */
  Tg8OP_SIMULATE_PULSE            =16	/* 0x0010 	16	 |NUM | */
  Tg8OP_SELFTEST_RESULT           =17	/* 0x0011 	17	 |NUM | */
  Tg8HR_INT_VME                   =2044	/* 0x07fc 	2044	 |NUM | */

  Tg8HardwareRegister             =4	/* 0x0004 	4	 |TYPE|  */
  Tg8HR_INT_MCP                   =2046	/* 0x07fe 	2046	 |NUM | */
  Tg8HR_STATUS                    =2048	/* 0x0800 	2048	 |NUM | */
  Tg8HR_INTERRUPT                 =2050	/* 0x0802 	2050	 |NUM | */
  Tg8HR_CLEAR_INT                 =2052	/* 0x0804 	2052	 |NUM | */
  Tg8HR_END                       =-1	/* 0xffffffff 	-1	 |NUM | */
  Tg8ENABLE_INT                   =0	/* 0x0000 	0	 |NUM | */

  Tg8TimingInterrupt              =4	/* 0x0004 	4	 |TYPE|  */
  Tg8DISABLE_INT                  =2	/* 0x0002 	2	 |NUM | */
  Tg8ENABLE_SYNC_INT              =1	/* 0x0001 	1	 |NUM | */
  Tg8DISABLE_SYNC_INT             =3	/* 0x0003 	3	 |NUM | */
  Tg8IS_MAILBOX                   =0	/* 0x0000 	0	 |NUM | */

  Tg8InterruptSource              =4	/* 0x0004 	4	 |TYPE|  */
  Tg8IS_IMM                       =1	/* 0x0001 	1	 |NUM | */
  Tg8IS_ERR                       =2	/* 0x0002 	2	 |NUM | */
  Tg8ISM_MAILBOX                  =1	/* 0x0001 	1	 |NUM | */
  Tg8ISM_IMM                      =2	/* 0x0002 	2	 |NUM | */
  Tg8ISM_ERR                      =4	/* 0x0004 	4	 |NUM | */

  Tg8MbxOpHdr                     =4	/* 0x0004 	4	 |TYPE|  */
  Row                             =0	/* 0x0000 	0	 |VAR | Tg8MbxOpHdr.Word 	=2= */
  Number                          =2	/* 0x0002 	2	 |VAR | Tg8MbxOpHdr.Word 	=2= */

  Tg8MbxRwAction                  =12	/* 0x000c 	12	 |TYPE|  */
  Hdr                             =0	/* 0x0000 	0	 |VAR | Tg8MbxRwAction.Tg8MbxOpHdr 	=4= */
  Actions                         =4	/* 0x0004 	4	 |VAR | Tg8MbxRwAction.Tg8User[1] 	=8= */

  Tg8MbxRwPpmAction               =156	/* 0x009c 	156	 |TYPE|  */
  Hdr                             =0	/* 0x0000 	0	 |VAR | Tg8MbxRwPpmAction.Tg8MbxOpHdr 	=4= */
  Actions                         =4	/* 0x0004 	4	 |VAR | Tg8MbxRwPpmAction.Tg8PpmUser[1] 	=152= */

  Tg8MbxRwPpmLine                 =16	/* 0x0010 	16	 |TYPE|  */
  Hdr                             =0	/* 0x0000 	0	 |VAR | Tg8MbxRwPpmLine.Tg8MbxOpHdr 	=4= */
  Lines                           =4	/* 0x0004 	4	 |VAR | Tg8MbxRwPpmLine.Tg8PpmLine[1] 	=12= */

  Tg8MbxRwGate                    =8	/* 0x0008 	8	 |TYPE|  */
  Hdr                             =0	/* 0x0000 	0	 |VAR | Tg8MbxRwGate.Tg8MbxOpHdr 	=4= */
  Gates                           =4	/* 0x0004 	4	 |VAR | Tg8MbxRwGate.Tg8Gate[1] 	=4= */

  Tg8MbxRwDim                     =8	/* 0x0008 	8	 |TYPE|  */
  Hdr                             =0	/* 0x0000 	0	 |VAR | Tg8MbxRwDim.Tg8MbxOpHdr 	=4= */
  Dims                            =4	/* 0x0004 	4	 |VAR | Tg8MbxRwDim.Byte[4] 	=4= */

  Tg8MbxRecording                 =20	/* 0x0014 	20	 |TYPE|  */
  Hdr                             =0	/* 0x0000 	0	 |VAR | Tg8MbxRecording.Tg8MbxOpHdr 	=4= */
  Recs                            =4	/* 0x0004 	4	 |VAR | Tg8MbxRecording.Tg8Recording[1] 	=16= */

  Tg8MbxHistory                   =20	/* 0x0014 	20	 |TYPE|  */
  Hdr                             =0	/* 0x0000 	0	 |VAR | Tg8MbxHistory.Tg8MbxOpHdr 	=4= */
  Hist                            =4	/* 0x0004 	4	 |VAR | Tg8MbxHistory.Tg8History[1] 	=16= */

  Tg8MbxClock                     =24	/* 0x0018 	24	 |TYPE|  */
  Hdr                             =0	/* 0x0000 	0	 |VAR | Tg8MbxClock.Tg8MbxOpHdr 	=4= */
  Clks                            =4	/* 0x0004 	4	 |VAR | Tg8MbxClock.Tg8Clock[1] 	=20= */

  Tg8MbxClearAction               =4	/* 0x0004 	4	 |TYPE|  */
  Hdr                             =0	/* 0x0000 	0	 |VAR | Tg8MbxClearAction.Tg8MbxOpHdr 	=4= */

  Tg8MbxActionState               =8	/* 0x0008 	8	 |TYPE|  */
  Hdr                             =0	/* 0x0000 	0	 |VAR | Tg8MbxActionState.Tg8MbxOpHdr 	=4= */
  State                           =4	/* 0x0004 	4	 |VAR | Tg8MbxActionState.Word 	=2= */
  Aux                             =6	/* 0x0006 	6	 |VAR | Tg8MbxActionState.Word 	=2= */
  Tg8MBX_BUFF_SIZE                =1338	/* 0x053a 	1338	 |NUM | */

  Tg8BlockData                    =1338	/* 0x053a 	1338	 |TYPE|  */
  Mask                            =0	/* 0x0000 	0	 |VAR | _su_.Word 	=2= */
  SpareMsk                        =2	/* 0x0002 	2	 |VAR | _su_.Word 	=2= */
  SimPulse                        =0	/* 0x0000 	0	 |VAR | Tg8BlockData._su_ 	=4= */
  HistoryBlock                    =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8MbxHistory 	=20= */
  RwAction                        =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8MbxRwAction 	=12= */
  RwPpmAction                     =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8MbxRwPpmAction 	=156= */
  RwPpmLine                       =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8MbxRwPpmLine 	=16= */
  RwGate                          =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8MbxRwGate 	=8= */
  RwDim                           =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8MbxRwDim 	=8= */
  Recording                       =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8MbxRecording 	=20= */
  ClockBlock                      =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8MbxClock 	=24= */
  ClearAction                     =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8MbxClearAction 	=4= */
  ActionState                     =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8MbxActionState 	=8= */
  DateTime                        =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8DateTime 	=12= */
  StatusBlock                     =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8StatusBlock 	=32= */
  SelfTestResult                  =0	/* 0x0000 	0	 |VAR | Tg8BlockData.StDpmInfo 	=348= */
  MbxBuff                         =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Byte[1338] 	=1338= */
  Tg8Dpm_spare                    =65536	/* 0x10000 	65536	 |NUM | */

  Tg8Dpm                          =67574	/* 0x107f6 	67574	 |TYPE|  */
  DpmSpare_Bytes                  =0	/* 0x0000 	0	 |VAR | Tg8Dpm.char[65536] 	=65536= */
  ExceptionVector                 =65536	/* 0x10000 	65536	 |VAR | Tg8Dpm.short 	=2= */
  Spare_ev                        =65538	/* 0x10002 	65538	 |VAR | Tg8Dpm.short 	=2= */
  ExceptionPC                     =65540	/* 0x10004 	65540	 |VAR | Tg8Dpm.Uint 	=4= */
  It                              =65544	/* 0x10008 	65544	 |VAR | Tg8Dpm.Tg8InterruptTable 	=320= */
  At                              =65864	/* 0x10148 	65864	 |VAR | Tg8Dpm.Tg8Aux 	=368= */
  BlockData                       =66232	/* 0x102b8 	66232	 |VAR | Tg8Dpm.Tg8BlockData 	=1338= */
  HrIntVme                        =67570	/* 0x107f2 	67570	 |VAR | Tg8Dpm.short 	=2= */
  HrIntMpc                        =67572	/* 0x107f4 	67572	 |VAR | Tg8Dpm.short 	=2= */

  Tg8Action                       =32	/* 0x0020 	32	 |TYPE|  */
  User                            =0	/* 0x0000 	0	 |VAR | Tg8Action.Tg8User 	=8= */
  Gate                            =8	/* 0x0008 	8	 |VAR | Tg8Action.Tg8Gate 	=4= */
  CntControl                      =12	/* 0x000c 	12	 |VAR | Tg8Action.Word 	=2= */
  CntNum                          =14	/* 0x000e 	14	 |VAR | Tg8Action.Byte 	=1= */
  Enabled                         =15	/* 0x000f 	15	 |VAR | Tg8Action.Byte 	=1= */
  Rec                             =16	/* 0x0010 	16	 |VAR | Tg8Action.Tg8Recording 	=16= */

  InterTelegram                   =264	/* 0x0108 	264	 |TYPE|  */
  aTeleg                          =0	/* 0x0000 	0	 |VAR | InterTelegram.Word[6]* 	=24= */
  nextLHC                         =24	/* 0x0018 	24	 |VAR | InterTelegram.Word[24] 	=48= */
  nextSPS                         =72	/* 0x0048 	72	 |VAR | InterTelegram.Word[24] 	=48= */
  nextCPS                         =120	/* 0x0078 	120	 |VAR | InterTelegram.Word[24] 	=48= */
  nextPSB                         =168	/* 0x00a8 	168	 |VAR | InterTelegram.Word[24] 	=48= */
  nextLEA                         =216	/* 0x00d8 	216	 |VAR | InterTelegram.Word[24] 	=48= */

  InterActTable                   =9216	/* 0x2400 	9216	 |TYPE|  */
  Pointers                        =0	/* 0x0000 	0	 |VAR | InterActTable.Tg8Action[256]* 	=1024= */
  Table                           =1024	/* 0x0400 	1024	 |VAR | InterActTable.Tg8Action[256] 	=8192= */

  StartedAct                      =520	/* 0x0208 	520	 |TYPE|  */
  EndMa                           =0	/* 0x0000 	0	 |VAR | StartedAct.Byte* 	=4= */
  ImmEndMa                        =4	/* 0x0004 	4	 |VAR | StartedAct.Byte* 	=4= */
  Matches                         =8	/* 0x0008 	8	 |VAR | StartedAct.Byte[256] 	=256= */
  ImmMatches                      =264	/* 0x0108 	264	 |VAR | StartedAct.Byte[256] 	=256= */
  QUEUE_SIZE                      =32	/* 0x0020 	32	 |NUM | */

  Queue                           =144	/* 0x0090 	144	 |TYPE|  */
  Bottom                          =0	/* 0x0000 	0	 |VAR | Queue.Uint* 	=4= */
  Top                             =4	/* 0x0004 	4	 |VAR | Queue.Uint* 	=4= */
  End                             =8	/* 0x0008 	8	 |VAR | Queue.Uint* 	=4= */
  Size                            =12	/* 0x000c 	12	 |VAR | Queue.int 	=4= */
  Buffer                          =16	/* 0x0010 	16	 |VAR | Queue.Uint[32] 	=128= */

  InterClockTable                 =328	/* 0x0148 	328	 |TYPE|  */
  N_of_clocks                     =0	/* 0x0000 	0	 |VAR | InterClockTable.short 	=2= */
  Clock_i                         =2	/* 0x0002 	2	 |VAR | InterClockTable.short 	=2= */
  Clock_p                         =4	/* 0x0004 	4	 |VAR | InterClockTable.Tg8Clock* 	=4= */
  Clocks                          =8	/* 0x0008 	8	 |VAR | InterClockTable.Tg8Clock[16] 	=320= */

  InterHistoryTable               =16008	/* 0x3e88 	16008	 |TYPE|  */
  N_of_histories                  =0	/* 0x0000 	0	 |VAR | InterHistoryTable.short 	=2= */
  History_i                       =2	/* 0x0002 	2	 |VAR | InterHistoryTable.short 	=2= */
  History_p                       =4	/* 0x0004 	4	 |VAR | InterHistoryTable.Tg8History* 	=4= */
  Histories                       =8	/* 0x0008 	8	 |VAR | InterHistoryTable.Tg8History[1000] 	=16000= */

  EProg                           =246	/* 0x00f6 	246	 |TYPE|  */
  IntSrc                          =0	/* 0x0000 	0	 |VAR | EProg.Word 	=2= */
  Alarms                          =2	/* 0x0002 	2	 |VAR | EProg.Word 	=2= */
  Event                           =4	/* 0x0004 	4	 |VAR | EProg.Tg8Event 	=4= */
  ImmEvent                        =8	/* 0x0008 	8	 |VAR | EProg.Tg8Event 	=4= */
  WildCard                        =12	/* 0x000c 	12	 |VAR | EProg.Word 	=2= */
  ImmWildCard                     =14	/* 0x000e 	14	 |VAR | EProg.Word 	=2= */
  ImmRun                          =16	/* 0x0010 	16	 |VAR | EProg.Word 	=2= */
  ImmSize                         =18	/* 0x0012 	18	 |VAR | EProg.Word 	=2= */
  ImmNxt                          =20	/* 0x0014 	20	 |VAR | EProg.Word 	=2= */
  ImmInt                          =22	/* 0x0016 	22	 |VAR | EProg.Tg8Interrupt[8] 	=160= */
  iFired                          =182	/* 0x00b6 	182	 |VAR | EProg.Tg8Interrupt[8]* 	=32= */
  rFired                          =214	/* 0x00d6 	214	 |VAR | EProg.Tg8Recording[8]* 	=32= */
  SUBSET_MASK                     =-268435456	/* 0xf0000000 	-268435456	 |NUM | */
  WILDCARDS_NUMBER                =8	/* 0x0008 	8	 |NUM | */

  Tg8TimeStamp                    =3	/* 0x0003 	3	 |TYPE|  */
  tsHour                          =0	/* 0x0000 	0	 |VAR | Tg8TimeStamp.Byte 	=1= */
  tsMinute                        =1	/* 0x0001 	1	 |VAR | Tg8TimeStamp.Byte 	=1= */
  tsSecond                        =2	/* 0x0002 	2	 |VAR | Tg8TimeStamp.Byte 	=1= */

  Tg8ScTime                       =8	/* 0x0008 	8	 |TYPE|  */
  stScNum                         =0	/* 0x0000 	0	 |VAR | Tg8ScTime.Uint 	=4= */
  stMs                            =4	/* 0x0004 	4	 |VAR | Tg8ScTime.Uint 	=4= */

  Tg8TwoBytes                     =2	/* 0x0002 	2	 |TYPE|  */
  Byte_1                          =0	/* 0x0000 	0	 |VAR | Tg8TwoBytes.Uchar 	=1= */
  Byte_2                          =1	/* 0x0001 	1	 |VAR | Tg8TwoBytes.Uchar 	=1= */

  Tg8TwoShorts                    =4	/* 0x0004 	4	 |TYPE|  */
  Short_1                         =0	/* 0x0000 	0	 |VAR | Tg8TwoShorts.short 	=2= */
  Short_2                         =2	/* 0x0002 	2	 |VAR | Tg8TwoShorts.short 	=2= */

  Tg8ShortOrTwoBytes              =2	/* 0x0002 	2	 |TYPE|  */
  Bytes                           =0	/* 0x0000 	0	 |VAR | Tg8ShortOrTwoBytes.Tg8TwoBytes 	=2= */
  Short                           =0	/* 0x0000 	0	 |VAR | Tg8ShortOrTwoBytes.short 	=2= */

  Tg8LongOrTwoShorts              =4	/* 0x0004 	4	 |TYPE|  */
  Short                           =0	/* 0x0000 	0	 |VAR | Tg8LongOrTwoShorts.Tg8TwoShorts 	=4= */
  Long                            =0	/* 0x0000 	0	 |VAR | Tg8LongOrTwoShorts.long 	=4= */
  ANY_OTHER_EVENT_INDEX           =0	/* 0x0000 	0	 |NUM | */

  EventIndex                      =4	/* 0x0004 	4	 |TYPE|  */
  MILLISECOND_EVENT_INDEX         =1	/* 0x0001 	1	 |NUM | */
  TIME_EVENT_INDEX                =2	/* 0x0002 	2	 |NUM | */
  DATE_EVENT_INDEX                =3	/* 0x0003 	3	 |NUM | */
");
# 40 "../firmware/tg8.c" 2


 
 
 
 
 
 
 
 
 
 

asm(" .text
      .globl __main
      __main: bra Main ");

main_prog () {
asm ("
 .text
 .globl Main
Main: ");

  Init();        
  MbxProcess();	 
}

 
 
 

static void Init() {

void         ( * p)();
long           * v;
register int     j, k;
register short * f;

   asm(" movew #Tg8DISABLE_INTERRUPTS,SR ") ;	 

    

   v = (long *) 4;
   for (k=255; k>=0; k--)  *v++ = (long) Default_Isr;

    
    

   v = (long*) (64 *4);
   p = Tpu0_Isr; *v++ = (long) p;	 
   p = Tpu1_Isr; *v++ = (long) p;	 
   p = Tpu2_Isr; *v++ = (long) p;
   p = Tpu3_Isr; *v++ = (long) p;
   p = Tpu4_Isr; *v++ = (long) p;
   p = Tpu5_Isr; *v++ = (long) p;
   p = Tpu6_Isr; *v++ = (long) p;
   p = Tpu7_Isr; *v++ = (long) p;
   p = Tpu8_Isr; *v   = (long) p;	 

   v = (long*) 0x07C  ; p = Abort_Isr; *v = (long) p;
   v = (long*) 0x078  ; p = Xr_Isr;    *v = (long) p;
   v = (long*) 0x070  ; p = Dsc_Isr;   *v = (long) p;

   v = (long*) 0x008 ;          p = BusError_Isr; *v = (long) p;
   v = (long*) 0x00C ;      p = AddressError_Isr; *v = (long) p;
   v = (long*) 0x020 ;p = PrivViolation_Isr; *v = (long) p;
   v = (long*) 0x060 ; p = Spurious_Isr; *v = (long) p;

    

   v = (long *) ((0  + 32 )*4);
   p = AtCompletion; *v = (long) p;
   v = (long*) ((1  + 32 )*4);
   p = InsertToCam; *v = (long) p;
   v = (long*) ((2  + 32 )*4);
   p = ClearCam; *v = (long) p;
   v = (long *) ((3  + 32 )*4);
   p = SetIntSourceMask; *v = (long) p;
   v = (long *) ((4  + 32 )*4);
   p = AtStartProcess; *v = (long) p;
   v = (long *) ((5  + 32 )*4);
   p = ImmCompletion; *v = (long) p;

    
    

   sim = (McpSim *) 0xfffffa00 ;         
   tpu = (McpTpu *) 0xfffffe00 ;         
   xlx = (Tg8Xlx *) 0x00012000 ;         
   cam = (short  *) 0x00011000 ;         
   dpm = (Tg8Dpm *) 0x00010000 ;         

    
    

   sim->SimCS3 = (0x00012000 << 8 )  | 0x00000000  | 0x6000  | 0x1800    | 0x0400 | 0x0030  | 0x0000  ;  
   sim->SimCS4 = 0xFFF80000  | 0x00000000  | 0x6000  | 0x0800  | 0x0000  | 0x0000  | 0x000E  | 0x0001  ;  
   sim->SimCS5 = 0xFFF80000  | 0x00000000   | 0x6000  | 0x0800  | 0x0000  | 0x0000  | 0x000C  | 0x0001  ;  
   sim->SimCS6 = (0x00011000 << 8 ) | 0x00000000  | 0x6000  | 0x1000    | 0x0400  | 0x0030  | 0x0000  ;  
   sim->SimCS7 = (0x00011000 << 8 ) | 0x00000000    | 0x6000  | 0x0800  | 0x0000  | 0x0030 | 0x0000  ;  
   sim->SimCS8 = (0x00010000 << 8 ) | 0x00000000    | 0x6000  | 0x1800  | 0x0000  | 0x03C0  | 0x0030  | 0x0000  ;  

    
    

   tpu->TpuConf = 7;  

    

   tpu->TpuIc = 64  | (7 <<8);

    
    
    

   tpu->TpuCfs1 = 0xa;      
   tpu->TpuCfs2 = 0xaaaa;   
   tpu->TpuCfs3 = 0xaaaa;   

    
    
    

   tpu->TpuHs0 = 1;
   tpu->TpuHs1 = 0x5555;
   tpu->TpuIe  = 0;  

    
    
    
    

   asm(" movew #0,TpuIs");

    
    

   k = 0xffffff00 ;     
   for(j=0; j<16; j++) {
      var.tpu_parameters[j] = (short*) k;
      k += 16;
   };

   for(j=0; j<9; j++) {
      f = var.tpu_parameters[j];
      *f++ = 0x7;  
      *f++ = 0xe;  
      *f++ = 0x1;  
      *f++ = 0x0;  
   }

    
    

   tpu->TpuHsr0 = 1;
   tpu->TpuHsr1 = 0x5555;

    

   tpu->TpuCp0 = 2;          
   tpu->TpuCp1 = 0xaaab;     
   tpu->TpuIe  = 0x1fe;      

    
    

   sim->SimPinF   = 0xfff1;
   sim->SimDirF   = 0xe;
   sim->SimDataF1 |= (short)  SelfTestOk | OkLed;
   sim->SimDataF1  = (short)  0;
   sim->SimDataF1 |= (short)  SelfTestOk | OkLed;

    
    

   xlx->XWsscRframe1 = ((Tg8SPS << 4) | Tg8SSC_HEADER) ;  

    
    
    
   
   SoftInit();

}  

 
 
 

static void CamInitialize() {

   asm(" andiw #CamCOMMAND_MODE,SimDataF1 ") ;
   asm(" movel _cam,a0
		       movew #0,a0@") ;                             
# 233 "../firmware/tg8.c"

    

   asm(" moveal _cam,a0
	 movew  #CamWRITE,d0
	 movel  #255,d1
      0: oriw   #CamDATA_MODE,SimDataF1  /* Set data mode */
	 movew  #0,a0@                   /* Clear the comparand register */
	 movew  #0,a0@                   /* words 0 1 2. */
	 movew  #0,a0@
	 andiw  #CamCOMMAND_MODE,SimDataF1 /* Set command mode */
	 movew  d0,a0@                     /* Do the COPY command */
	 addiw  #1,d0                      /* Next CAM array address */
	 dbf    d1,0b
	 movew  #0,a0@ ");                
}

 
 
 
 

static void SoftInit() {
Tg8Action *a; int j; short err;
StDpm *std;

   asm(" movew #Tg8DISABLE_INTERRUPTS,SR ") ;		 
   sim->SimProtect = 0;  

    

   CamInitialize();

   memset((volatile short *) &info,0,sizeof(info));

    

   dpm->ExceptionVector = 0;
   dpm->ExceptionPC = 0;
   memset((volatile short *) &dpm->It,0,sizeof(Tg8InterruptTable));
   memset((volatile short *) &dpm->At,0,sizeof(Tg8Aux));
   memset((volatile short *) &dpm->BlockData,0,sizeof(Tg8BlockData));

    

   camBusy = 0;
   memset((volatile short *) &eprog,0,sizeof(EProg));
   memset((volatile short *) &act,0,sizeof(InterActTable));
   memset((volatile short *) &clk,0,sizeof(InterClockTable));
   memset((volatile short *) &hist,0,sizeof(hist));
   memset((volatile short *) &match,0,sizeof(StartedAct));
   memset((volatile short *) &tel,0,sizeof(InterTelegram));

   memset((volatile short *) &atQueue,0,sizeof(Queue));
   memset((volatile short *) &immQueue,0,sizeof(Queue));

   memset((volatile short *) &ins,0,sizeof(ins));
   memset((volatile short *) &var,0,sizeof(var));
   memset((volatile short *) &in_use,0,sizeof(in_use));

    

   wild_c[0] = 0x000000L;
   wild_c[1] = 0x0000ffL;
   wild_c[2] = 0x00ff00L;
   wild_c[3] = 0x00ffffL;
   wild_c[4] = 0xff0000L;
   wild_c[5] = 0xff00ffL;
   wild_c[6] = 0xffff00L;
   wild_c[7] = 0xffffffL;

    

   memset((volatile short *) time_event_index,0,sizeof(time_event_index));
   time_event_index[Tg8MILLISECOND_HEADER]= MILLISECOND_EVENT_INDEX;
   time_event_index[Tg8psTIME_HEADER]=
   time_event_index[Tg8SECOND_HEADER]=
   time_event_index[Tg8MINUTE_HEADER]=
   time_event_index[Tg8HOUR_HEADER]  = TIME_EVENT_INDEX;
   time_event_index[Tg8psDATE_HEADER]=
   time_event_index[Tg8DAY_HEADER]   =
   time_event_index[Tg8MONTH_HEADER] =
   time_event_index[Tg8YEAR_HEADER]  = DATE_EVENT_INDEX;

    

   tel.aTeleg[Tg8LHC] = (Word *) dpm->At.aTelegLHC;
   tel.aTeleg[Tg8SPS] = (Word *) dpm->At.aTelegSPS;
   tel.aTeleg[Tg8CPS] = (Word *) dpm->At.aTelegCPS;
   tel.aTeleg[Tg8PSB] = (Word *) dpm->At.aTelegPSB;
   tel.aTeleg[Tg8LEA] = (Word *) dpm->At.aTelegLEA;

    
   for (j=0,a=act.Table; j< 256 ; j++,a++) act.Pointers[j] = a;

    
   clk.Clock_p = clk.Clocks;

    
   hist.History_p = hist.Histories;

    

   atQueue.Top  = atQueue.Bottom = atQueue.Buffer;
   atQueue.End  = atQueue.Buffer + 32 ;  
   immQueue.Top = immQueue.Bottom = immQueue.Buffer;
   immQueue.End = immQueue.Buffer + 32 ;  

    
   memcpy16((short*)&dpm->At.aFwVer,(short*)&("Dec 21 2006"),12);

    

   err = xlx->WClrRerr;  
   xlx->WClrRerr = 0;    
   rcv_error.Err = err;
   rcv_error.Hour= 0;
   *(Word*)&dpm->At.aDt.aRcvErr = *(Word*)&rcv_error;  

    
   dpm->At.aMbox = Tg8OP_NO_COMMAND;
   dpm->At.aCoco = Tg8ERR_OK;

    
   dpm->At.aSscHeader = ((Tg8SPS << 4) | Tg8SSC_HEADER) ;  

    
   dpm->At.aFwStat = Tg8FS_RUNNING;

   sim->SimProtect = 0xa0 ;  

   dm[0]=dm[2]=dm[4]=dm[6]=dm[7]=dm[9]=dm[11]=31;
   dm[1]=28;
   dm[3]=dm[5]=dm[8]=dm[10]=30;

   asm(" movew #Tg8ENABLE_INTERRUPTS,SR ") ;        

}  

 
 
 
 
 

static int InsertAction(action,ppmfl)
Tg8User *action;  
int ppmfl;         {

Tg8PpmUser *ppm;
Tg8Action  *a,*aa;
Tg8Event trigger;
int st; short j,k,m,wm, config, cw,lcw, ln,dd;

   a = act.Pointers[ins.Addr];
   memset((volatile short *) a,0,sizeof(*a));   

   if (ppmfl) {  
     ppm = (Tg8PpmUser*) action;
     ln = ppm->Dim;  
     if (ins.Addr+ln > 256 ) return Tg8ERR_WRONG_DIM;
     a->User.uEvent   = ppm->Trigger;
     a->User.uControl = ppm->LineCw[0];
     a->User.uDelay   = ppm->LineDelay[0];
     a->Gate.Machine  = (ppm->Machine<<4) | ppm->GroupType;
     a->Gate.GroupNum = ppm->GroupNum;
     a->Gate.GroupVal = ppm->LineGv[0];
   } else {
     a->User = *action;
     ln = 1;  
   };

   a->Enabled = ln<<1;  
   trigger = a->User.uEvent;

    

   cw = a->User.uControl;    
   k = ((( cw )>> 8 ) & 0x7 ) ;    
   k = (k==0? 7: k-1);       
   a->CntNum = k;            

    
    

   config = GetXConfiguration(cw);
   if (config<0) return config;  
   a->CntControl = config;       

    

   if (config &ConfOUTPUT) {
     if (a->User.uDelay == 0xFFFF) return Tg8ERR_WRONG_DELAY;
     a->User.uDelay++;
     dd = 1;
   } else {
      
     a->User.uDelay = 0;
     dd = 0;
   };

   if (! ((( cw )>> 14 ) & 0x3 ) ) {  
     (( cw ) |= (( 1 )<< 2 )) ;
     a->User.uControl = cw;
   };

   if (((( cw )>> 3 ) & 0x3 )  != Tg8TM_LINE) {  

    
    
    

   j = 0;
   if (trigger.Any.Byte_2 == (unsigned char) 0xFF ) j |= 4;
   if (trigger.Any.Byte_3 == (unsigned char) 0xFF ) j |= 2;
   if (trigger.Any.Byte_4 == (unsigned char) 0xFF ) j |= 1;
   wm = 1<<j;

    
    

   if (trigger.Any.Header != 0x01) {  

      in_use.SimpleH_wc[trigger.Any.Header] |= wm;
      m = trigger.Any.Header>>4;  
      m = 1<<m;                   
      if ((j&4) == 0) {  
	                 
	in_use.Simple_event[trigger.Simple.Event_code] |= m;
	if (config &ConfOUTPUT)
	  in_use.Simple_wc[trigger.Simple.Event_code] |= wm;
	else
	  in_use.Simple_wc_imm[trigger.Simple.Event_code] |= wm;
      } else             
	for (k=255; k>=0; k--) {
	  in_use.Simple_event[k] |= m;
	  if (config &ConfOUTPUT)
	    in_use.Simple_wc[k] |= wm;
	  else
	    in_use.Simple_wc_imm[k] |= wm;
	};

   } else {  

     in_use.TimeH_wc[trigger.Any.Header] |= wm;
     m = time_event_index[trigger.Any.Header];
     m = 1<<m;
     if ((j&4) == 0) {  
       in_use.Time_event[trigger.Simple.Event_code] |= m;
       if (config &ConfOUTPUT)
	 in_use.Time_wc[trigger.Simple.Event_code] |= wm;
       else
	 in_use.Time_wc_imm[trigger.Simple.Event_code] |= wm;
     } else             
       for (k=255; k>=0; k--) {
	 in_use.Time_event[k] |= m;
	 if (config &ConfOUTPUT)
	   in_use.Time_wc[k] |= wm;
	 else
	   in_use.Time_wc_imm[k] |= wm;
       };
   };

    
    
    
    

   if (((( cw )>> 2 ) & 0x1 )  == 0) {  
     ins.Event.Long = trigger.Long;
      
     ins.Last_word.Short = (config &ConfOUTPUT)? 0xFF : 0x00 ;
   } else {
     ins.Event.Long = 0;
     ins.Last_word.Short = 0;
   };

    

   if (ppmfl) {
     (( a->User.uControl ) |= (( Tg8TM_PPM )<< 3 )) ;
     for (k=1,aa=a; k<ln; k++) {
       *++aa = *a;
       aa->Gate.GroupVal = ppm->LineGv[k];
       aa->User.uControl = lcw = ppm->LineCw[k];
       aa->User.uDelay   = dd+ppm->LineDelay[k];

        

       j = ((( lcw )>> 8 ) & 0x7 ) ;   
       j = (j==0? 7: j-1);       
       if (j != a->CntNum) return Tg8ERR_ILLEGAL_ARG;

        
        

       j = GetXConfiguration(lcw);
       if (j<0) return j;   
       if ((j&ConfOUTPUT) != (config&ConfOUTPUT)) return Tg8ERR_ILLEGAL_ARG;

       aa->CntControl = j;  

        

       if (j &ConfOUTPUT) {
	 if (!aa->User.uDelay) return Tg8ERR_WRONG_DELAY;
       } else
	  
	 aa->User.uDelay = 0;

       if (! ((( lcw )>> 14 ) & 0x3 ) )  
	 (( lcw ) |= (( 1 )<< 2 )) ;

       (( lcw ) |= (( Tg8TM_LINE )<< 3 )) ;  
       aa->User.uControl = lcw;
       if (((( lcw )>> 2 ) & 0x1 )  == 0) aa->Enabled |= Tg8ENABLED;
     };
   };

    
    
    

   asm("trap #1" );

   }; 

    
   if (((( cw )>> 2 ) & 0x1 )  == 0) a->Enabled |= Tg8ENABLED;

    
   ins.Addr += ln;
   if (dpm->At.aNumOfAct < ins.Addr) dpm->At.aNumOfAct = ins.Addr;

   return(Tg8ERR_OK);

} 

 
 
 

static int GetXConfiguration(cw)
int cw; {
int config = 0;   

   switch(((( cw )>> 14 ) & 0x3 ) ) {
   case Tg8DO_OUTPUT:
     config |= ConfOUTPUT;
   break;
   case Tg8DO_OUTPUT_AND_INTERRUPT:
     config |= ConfOUTPUT;
      
   case Tg8DO_INTERRUPT:
     config |= ConfINTERRUPT;
   break;
   };

   switch(((( cw )>> 0 ) & 0x3 ) ) {
   case Tg8CLK_MILLISECOND:
   break;
   case Tg8CLK_X1:
     config |= ConfEXT_CLOCK_1;
   break;
   case Tg8CLK_X2:
     config |= ConfEXT_CLOCK_2;
   break;
   default:
     return Tg8ERR_WRONG_CLOCK;
   };

   switch(((( cw )>> 6 ) & 0x3 ) ) {
   case Tg8CM_NORMAL:
     config |= ConfNORMAL;
   break;
   case Tg8CM_CHAINED:
     config |= ConfCHAINE;
   break;
   case Tg8CM_EXTERNAL:
     config |= ConfEXT_START;
   break;
   default:
      return Tg8ERR_WRONG_MODE;
   };
   return config;
}

 
 
 

static void memset(volatile short *m,short v,unsigned int bc) {
volatile short *p = (volatile short *) m;
  bc >>= 1;	 
  while (bc--) *p++ = v;
}

 
 
 

static void memcpy16(short *d,short *s,int bc) {
register int n;
  for (n=(bc>>1); n>0; n--) *d++ = *s++;
  if (bc&1) *(char*)d = *(char*)s;
}

static void bcopy(short *s,short *d,int bc) {
register int n;
  for (n=(bc>>1); n>0; n--) *d++ = *s++;
  if (bc&1) *(char*)d = *(char*)s;
}

 
 
 

static Debug(n) short n; { while (dpm->At.aTrace); dpm->At.aTrace = n; }

 
 
 

# 1 "../firmware/UtcToTime.c" 1
 
 
 
 
 

 
 




 








unsigned char DecToBcd(unsigned char dec) {
   return ((dec/10) << 4) | ((dec%10) & 0xF);
}

 
 
 
 

void UtcToTime() {

unsigned long x;  
char lp;          
int i;
Tg8DateTime ldt;

   utc -= 1041379200 ;
   i=0;
   do {
      x = 31536000 ; lp = 0;
      if (!((i+ 3 ) % 4)) { x += 86400 ; lp = 1; }   
      if (utc >= x) utc -= x;
      else break;
      i++;
   } while (1);
   ldt.aYear = DecToBcd(3 +i);

   i = 0;
   do {
      x = ((unsigned long) dm[i]) * 86400 ;
      if (lp && (i==1)) x += 86400 ;  
      if (utc >= x) utc -= x;
      else break;
      i++;
   } while (1);
   ldt.aMonth  = DecToBcd(i+1);

   ldt.aDay    = DecToBcd((utc/ 86400 ) +1); utc = utc% 86400 ;
   ldt.aHour   = DecToBcd(utc/ 3600 );      utc = utc% 3600 ;
   ldt.aMinute = DecToBcd(utc/ 60 );      utc = utc% 60 ;
   ldt.aSecond = DecToBcd(utc);

   memcpy16((short *) &(dpm->At.aDt),(short *) &ldt,sizeof(Tg8DateTime));
   return;
}
# 656 "../firmware/tg8.c" 2

# 1 "../firmware/mbx.c" 1
 
 
 
 
 
 

 

 
 

 
 

 

 
 

 

 
 

 
 
 

 

 
 
 

void MbxProcess() {

Tg8Action *a; Tg8User *ad;
Tg8PpmUser *ppm; Tg8PpmLine *ppml; Tg8Gate *g;
int j,cnt,pos; short cw,err;

   
   
   

  for (;;) {

# 74 "../firmware/mbx.c"

    
    
    

    

   asm volatile ("movew #0x55,SimServ
		  movew #0xAA,SimServ");

    
    
    

    

   asm volatile ("oriw  #OkLed,SimDataF1
		  andiw #~OkLed,SimDataF1
		  oriw  #OkLed,SimDataF1");

    
    
    

   if (err = xlx->WClrRerr) {  

      rcv_error.Err = err;

       

      *(Word *) &dpm->At.aDt.aRcvErr = *(Word *) &rcv_error;
   }



    
    
    

   if (eprog.IntSrc && !dpm->At.aIntSrc) {
      
      
      
     asm volatile ("trap  #3" );
   };

    

   switch (dpm->At.aMbox) {

      
      
      

   case Tg8OP_NO_COMMAND:
     continue;          

      
      
      

   case Tg8OP_PING_MODULE:
     break;

      
      
      

   case Tg8OP_SIMULATE_PULSE: {
     Tg8IntAction  uu;
     Tg8Interrupt *ip;
     j = dpm->BlockData.SimPulse.Mask;
     for (cnt=0; j; cnt++,j>>=1) {
       if (j&1) {  
	 xlx->XlxDelay [cnt] = 1;
	 xlx->XlxConfig[cnt] = ConfOUTPUT|ConfNORMAL;

	  
	 ip = (Tg8Interrupt *) &dpm->It.CntInter[cnt];
	 eprog.iFired[cnt] = ip;         
	 eprog.rFired[cnt] = &sim_rec;   

	  
	  

	 ip->iOut = 0xFFFFFFFF;      
	 ip->iEvent.Long = 0;        
	 ip->iSc = dpm->At.aSc;      
	 ip->iOcc= dpm->At.aScTime;  
	 uu.iRcvErr = rcv_error.Err; 
	 uu.iOvwrAct = 0;            
	 uu.iAct = 0;                
	 uu.iSem = 1;                
	 ip->iExt= uu;               
       };
     };
     break;
   };

      
      
      

   case Tg8OP_GET_STATUS: {    
     Tg8StatusBlock *s = (Tg8StatusBlock *) &dpm->BlockData.StatusBlock;
     s->Fw = dpm->At.aFwStat;
     s->Am = dpm->At.aAlarms; { dpm->At.aAlarms = 0; dpm->At.aFwStat &= ~Tg8FS_ALARMS;} ;
     s->Cc = dpm->At.aCoco;
     s->Evo= dpm->ExceptionVector;
     s->Epc= dpm->ExceptionPC;
     s->ScTime = dpm->At.aScTime;
     s->Dt = dpm->At.aDt;
     dpm->At.aMaxQueueSize = 0;
     break;
   };

      
      
      

   case Tg8OP_SELFTEST_RESULT:
     dpm->BlockData.SelfTestResult = info;
     break;

      
      
      

   case Tg8OP_GET_USER_TABLE: {
     Tg8MbxRwAction *tab = (Tg8MbxRwAction *) &dpm->BlockData.RwAction;
     ins.Addr = tab->Hdr.Row &0xFF;
     cnt = tab->Hdr.Number;
     if (ins.Addr+cnt > 256 ) cnt = 256  - ins.Addr;
     for (ad=tab->Actions,a=act.Pointers[ins.Addr]; cnt>0;
	  cnt--,ins.Addr++,ad++,a++) {
       *ad = a->User;
       if (ad->uDelay) --(ad->uDelay);
     };
     tab->Hdr.Row = ins.Addr;
     break;
   };

      
      
      

   case Tg8OP_GET_RECORDING_TABLE: {
     Tg8MbxRecording *tab = (Tg8MbxRecording *) &dpm->BlockData.Recording;
     Tg8Recording *r;
     ins.Addr = tab->Hdr.Row &0xFF;
     cnt = tab->Hdr.Number;
     if (ins.Addr+cnt > 256 ) cnt = 256  - ins.Addr;
     for (r=tab->Recs,a=act.Pointers[ins.Addr]; cnt>0;
	  cnt--,ins.Addr++,r++,a++) *r = a->Rec;
     tab->Hdr.Row = ins.Addr;
     break;
   };

      
      
      

   case Tg8OP_GET_HISTORY_TABLE: {
     Tg8MbxHistory *tab = (Tg8MbxHistory *) &dpm->BlockData.HistoryBlock;
     Tg8History *ah,*d;
     cnt = tab->Hdr.Number;
     pos = tab->Hdr.Row;    
     if (!pos) pos = hist.History_i;  
     if (pos >= 1000 ) pos = 1000 -1;
     ah  = &hist.Histories[pos];
     for (j=0,d=tab->Hist; j<cnt; j++,d++) {
       if (--pos <0) {
	 pos = 1000 -1;
	 ah  = &hist.Histories[pos];
       } else
	 --ah;
       *d = *ah;
     };
     tab->Hdr.Row = pos;
     break;
   };
   
      
      
      

   case Tg8OP_GET_CLOCK_TABLE: {
     Tg8MbxClock *tab = (Tg8MbxClock *) &dpm->BlockData.ClockBlock;
     Tg8Clock *cc,*d;
     cnt = tab->Hdr.Number;
     pos = tab->Hdr.Row;    
     if (!pos) pos = clk.Clock_i;  
     if (pos >= 16 ) pos = 16 -1;
     cc  = &clk.Clocks[pos];
     for (j=0,d=tab->Clks; j<cnt; j++,d++) {
       if (--pos <0) {
	 pos = 16 -1;
	 cc  = &clk.Clocks[pos];
       } else
	 --cc;
       *d = *cc;
     };
     tab->Hdr.Row = pos;
     break;
   };

      
      
      

   case Tg8OP_CLEAR_USER_TABLE: {
     Tg8MbxClearAction *tab = (Tg8MbxClearAction *) &dpm->BlockData.ClearAction;
     ins.Addr = pos = tab->Hdr.Row &0xFF;
     cnt = tab->Hdr.Number;
     if (ins.Addr+cnt > 256 ) cnt = 256  - ins.Addr;
     ins.Event.Long = 0;
     ins.Last_word.Short = 0;
     for (a=act.Pointers[ins.Addr]; cnt>0 && ins.Addr<dpm->At.aNumOfAct;
	  cnt--,a++,ins.Addr++) {
        
       memset((volatile short *) a,0,sizeof(*a));
       asm("trap #1" );
     };
     if (ins.Addr == dpm->At.aNumOfAct) {
       dpm->At.aNumOfAct = pos;
       if (pos == 0) { 
	 memset((volatile short *) &dpm->It,0,sizeof(Tg8InterruptTable));  
	 memset((volatile short *) &in_use,0,sizeof(in_use));              
       };
     };
     break;
   };

    
    
    

   case Tg8OP_SET_USER_TABLE: {
     Tg8MbxRwAction *tab = (Tg8MbxRwAction *) &dpm->BlockData.RwAction;
     ins.Addr = tab->Hdr.Row &0xFF;
     cnt = tab->Hdr.Number;
     if (ins.Addr+cnt > 256 ) cnt = 256  - ins.Addr;
     for (ad=tab->Actions; cnt>0; cnt--,ad++)
       if ((j = InsertAction(ad,0 ))<0) {
	 dpm->At.aTrace= ins.Addr;
	 dpm->At.aCoco = j;
	 goto done;
       };
     tab->Hdr.Row = ins.Addr;
     break;
   };

      
      
      

   case Tg8OP_SET_PPM_TIMING: {
     Tg8MbxRwPpmAction *tab = (Tg8MbxRwPpmAction *) &dpm->BlockData.RwPpmAction;
     ins.Addr = tab->Hdr.Row &0xFF;
     cnt = tab->Hdr.Number;
     if (ins.Addr+cnt > 256 ) cnt = 256  - ins.Addr;
     for (ppm=tab->Actions; cnt>0; cnt--,ppm++)
       if ((j = InsertAction((Tg8User*)ppm,1 ))<0) {
	 dpm->At.aTrace= ins.Addr;
	 dpm->At.aCoco = j;
	 goto done;
       };
     tab->Hdr.Row = ins.Addr;
     break;
   };

    
    
    

   case Tg8OP_GET_PPM_LINE: {
     Tg8MbxRwPpmLine *tab = (Tg8MbxRwPpmLine *) &dpm->BlockData.RwPpmLine;
     ins.Addr = tab->Hdr.Row &0xFF;
     cnt = tab->Hdr.Number;
     if (ins.Addr+cnt > 256 ) cnt = 256  - ins.Addr;
     for (ppml=tab->Lines,a=act.Pointers[ins.Addr]; cnt>0;
	  cnt--,ins.Addr++,ppml++,a++) {
       ppml->Action = a->User;
       if (ppml->Action.uDelay) --(ppml->Action.uDelay);
       ppml->Gate = a->Gate;
     };
     tab->Hdr.Row = ins.Addr;
     break;
   };

      
      
      

   case Tg8OP_SET_GATE: {
     Tg8MbxRwGate *tab = (Tg8MbxRwGate *) &dpm->BlockData.RwGate;
     ins.Addr = tab->Hdr.Row &0xFF;
     cnt = tab->Hdr.Number;
     if (ins.Addr+cnt > 256 ) cnt = 256  - ins.Addr;
     for (g=tab->Gates,a=act.Pointers[ins.Addr]; cnt>0;
	  cnt--,g++,a++,ins.Addr++) {
       a->Gate = *g;
     };
     tab->Hdr.Row = ins.Addr;
     break;
   };

      
      
      

   case Tg8OP_SET_DIM: {
     Tg8MbxRwDim *tab = (Tg8MbxRwDim *) &dpm->BlockData.RwDim; Byte *p;
     ins.Addr = tab->Hdr.Row &0xFF;
     cnt = tab->Hdr.Number;
     if (ins.Addr+cnt > 256 ) cnt = 256  - ins.Addr;
     for (p=tab->Dims,a=act.Pointers[ins.Addr]; cnt>0;
	  cnt--,p++,a++,ins.Addr++) {
       a->Enabled &= 1;  
       a->Enabled |= (*p<<1);
     };
     tab->Hdr.Row = ins.Addr;
     break;
   };

    
    
    

   case Tg8OP_SET_DELAY: {
     Tg8MbxActionState *tab = (Tg8MbxActionState *) &dpm->BlockData.ActionState;
     ins.Addr = tab->Hdr.Row &0xFF;
     cnt = tab->Hdr.Number;
     if (ins.Addr+cnt > 256 ) cnt = 256  - ins.Addr;
     for (a=act.Pointers[ins.Addr]; cnt>0; cnt--,a++,ins.Addr++) {
       if (a->CntControl &ConfOUTPUT) {
	 if (tab->State == 0xFFFF) {
	   dpm->At.aTrace= ins.Addr;
	   dpm->At.aCoco = Tg8ERR_WRONG_DELAY;
	   goto done;
	 };
	 if (tab->Aux) {  
	   cw = a->CntControl & ~(ConfEXT_CLOCK_1|ConfEXT_CLOCK_2|ConfCABLE_CLOCK);
	   switch(tab->Aux &= 0x3) {
	   case Tg8CLK_MILLISECOND:
	     break;
	   case Tg8CLK_X1:
	     cw |= ConfEXT_CLOCK_1;
	     break;
	   case Tg8CLK_X2:
	     cw |= ConfEXT_CLOCK_2;
	     break;
	   default:
	     dpm->At.aTrace= ins.Addr;
	     dpm->At.aCoco = Tg8ERR_WRONG_CLOCK;
	     goto done;
	   };
	   (( a->User.uControl ) &= (~(0x3 << 0 ))) ;
	   (( a->User.uControl ) |= (( tab->Aux )<< 0 )) ;
	   a->CntControl = cw;
	 };
	 a->User.uDelay = tab->State+1;
       };
     };
     tab->Hdr.Row = ins.Addr;
     break;
   };

    
    
    

   case Tg8OP_SET_STATE: {
     Tg8MbxActionState *tab = (Tg8MbxActionState *) &dpm->BlockData.ActionState;
     ins.Addr = tab->Hdr.Row &0xFF;
     cnt = tab->Hdr.Number;
     if (ins.Addr+cnt > 256 ) cnt = 256  - ins.Addr;
     for (a=act.Pointers[ins.Addr]; cnt>0; cnt--,a++,ins.Addr++) {
       switch (tab->State) {
       case Tg8ENABLED:
	 if (((( a->User.uControl )>> 3 ) & 0x3 ) ) goto st_en;  
       case Tg8PPMT_ENABLED:
	 ins.Event.Long = a->User.uEvent.Long;
	 ins.Last_word.Short = (a->CntControl &ConfOUTPUT)? 0xFF : 0x00 ;
	 asm("trap #1" );
st_en:
	 a->Enabled |= Tg8ENABLED;
	 (( a->User.uControl ) &= (~(0x1 << 2 ))) ;
	 break;
       case Tg8DISABLED:
	 if (((( a->User.uControl )>> 3 ) & 0x3 ) ) goto st_dis;  
       case Tg8PPMT_DISABLED:
	 ins.Event.Long = 0;
	 ins.Last_word.Short = 0;
	 asm("trap #1" );
st_dis:
	 a->Enabled &= ~Tg8ENABLED;
	 (( a->User.uControl ) |= (( 1 )<< 2 )) ;
	 break;
       case Tg8INTERRUPT:  
	 a->CntControl |= ConfINTERRUPT;  
	 (( a->User.uControl ) |= (( Tg8DO_INTERRUPT )<< 14 )) ;
	 break;
       case Tg8NO_INTERRUPT:  
	 a->CntControl &= ~ConfINTERRUPT;  
	 a->User.uControl &= ~(Tg8DO_INTERRUPT<< 14 );
	 break;
       };
     };
     break;
   };

    
    
    

   case Tg8OP_SET_SSC_HEADER:
     j = xlx->WClrRerr;         
     xlx->WClrRerr = 0;         
     xlx->XWsscRframe1 = dpm->At.aSscHeader;  
     break;

      
      
      

   default:
     dpm->At.aCoco = Tg8ERR_ILLEGAL_OPERATION;
     goto done;
   };
   
   dpm->At.aCoco = Tg8ERR_OK;

done:  

   dpm->At.aMbox = Tg8OP_NO_COMMAND;    

    
   if (dpm->At.aIntSrc & (1<<Tg8IS_MAILBOX) ) { eprog.Alarms |= ( Tg8ALARM_MBX_BUSY ); dpm->At.aAlarms |= eprog.Alarms; dpm->At.aFwStat |= Tg8FS_ALARMS; } ;

   eprog.IntSrc |= 1<<( Tg8IS_MAILBOX ); ;  

 };  

}  

 
# 657 "../firmware/tg8.c" 2

# 1 "../firmware/isr.c" 1
 
 
 
 
 
 

 

 
 

 
 

 

 
 

 

 
 

 
 
 

 

 

# 1 "macros.h" 1
 
 
 
 






 
 
 



















 
 
 




 
 
 

 






 
 
 
































# 101 "macros.h"

 
 
 



 
 
 



 
 
 

















 
 
 











# 157 "macros.h"





 
 
 






 
# 34 "../firmware/isr.c" 2


 
 
 
 


# 56 "../firmware/isr.c"

 
 
 






 
 
 

void dummyIsrEtc() {
  register int err;

   asm(" .text");

 
 
 

   asm(" .globl _SetIntSourceMask
_SetIntSourceMask:
	 movew  #Tg8DISABLE_INTERRUPTS,SR  /* Mask all interrupts */
	 moveml a2/a3,_regic
       ");
   if (eprog.IntSrc && !dpm->At.aIntSrc) {
      
     dpm->At.aIntSrc = eprog.IntSrc; eprog.IntSrc = 0;
      
     (*(short*)(Tg8HR_INT_VME+(char*)dpm)) = 1;
   };
   asm ("  moveml  _regic,a2/a3
	   rte ");

 
 
 

   asm(" .globl _InsertToCam
_InsertToCam:
	 movew  #Tg8DISABLE_INTERRUPTS,SR
	 moveml d4/a2/a3,_regic ");
   asm(" oriw  #CamDATA_MODE,SimDataF1 ") ; *cam = ins.Event.Short.Short_1; *cam = ins.Event.Short.Short_2; *cam = ins.Last_word.Short; asm(" andiw #CamCOMMAND_MODE,SimDataF1 ") ; *cam = ins.Addr | CamWRITE; ;
   asm(" moveml _regic,d4/a2/a3
	 rte ");

 
 
 

   asm(" .globl _ClearCam
_ClearCam:
	 movew  #Tg8DISABLE_INTERRUPTS,SR
	 movel a0,_regic ");
   asm(" andiw #CamCOMMAND_MODE,SimDataF1 ") ;
   asm(" movel _cam,a0
		       movew #0,a0@") ;
# 115 "../firmware/isr.c"
   asm(" movel _regic,a0
         rte ");

 
 
 

asm(" .globl _Tpu0_Isr
_Tpu0_Isr:
       movew  #Tg8DISABLE_INTERRUPTS,SR
       andw   #-1-Tg8HS_DPRAM_INTERRUPT,TpuIs /* Clear the TPU ch.'0' inter.*/
       rte
");

 
 
 
 

   asm("
       .globl _Tpu""1""_Isr
_Tpu""1""_Isr:
       movew   #Tg8DISABLE_INTERRUPTS,SR
       moveml  a0,_context_c              /* Save registers */
       moveal _eprog+iFired+((""1""-1)*4),a0  /* Points to the interr. table */
       movew   At+aScTime,a0@(iOut)
       movew   At+aScTime+2,a0@(iOut+2)
       moveal _eprog+rFired+((""1""-1)*4),a0  /* Points to the recording table */
       movew   At+aScTime,a0@(rOut)
       movew   At+aScTime+2,a0@(rOut+2)
       moveml _context_c,a0	             /* Restore registers */
       andw   #ClrINT_CHANNEL_""1"",TpuIs     /* Clear the TPU channel 'c' interrupt */
       rte
") ;
# 135 "../firmware/isr.c"
   asm("
       .globl _Tpu""2""_Isr
_Tpu""2""_Isr:
       movew   #Tg8DISABLE_INTERRUPTS,SR
       moveml  a0,_context_c              /* Save registers */
       moveal _eprog+iFired+((""2""-1)*4),a0  /* Points to the interr. table */
       movew   At+aScTime,a0@(iOut)
       movew   At+aScTime+2,a0@(iOut+2)
       moveal _eprog+rFired+((""2""-1)*4),a0  /* Points to the recording table */
       movew   At+aScTime,a0@(rOut)
       movew   At+aScTime+2,a0@(rOut+2)
       moveml _context_c,a0	             /* Restore registers */
       andw   #ClrINT_CHANNEL_""2"",TpuIs     /* Clear the TPU channel 'c' interrupt */
       rte
") ;
# 136 "../firmware/isr.c"
   asm("
       .globl _Tpu""3""_Isr
_Tpu""3""_Isr:
       movew   #Tg8DISABLE_INTERRUPTS,SR
       moveml  a0,_context_c              /* Save registers */
       moveal _eprog+iFired+((""3""-1)*4),a0  /* Points to the interr. table */
       movew   At+aScTime,a0@(iOut)
       movew   At+aScTime+2,a0@(iOut+2)
       moveal _eprog+rFired+((""3""-1)*4),a0  /* Points to the recording table */
       movew   At+aScTime,a0@(rOut)
       movew   At+aScTime+2,a0@(rOut+2)
       moveml _context_c,a0	             /* Restore registers */
       andw   #ClrINT_CHANNEL_""3"",TpuIs     /* Clear the TPU channel 'c' interrupt */
       rte
") ;
# 137 "../firmware/isr.c"
   asm("
       .globl _Tpu""4""_Isr
_Tpu""4""_Isr:
       movew   #Tg8DISABLE_INTERRUPTS,SR
       moveml  a0,_context_c              /* Save registers */
       moveal _eprog+iFired+((""4""-1)*4),a0  /* Points to the interr. table */
       movew   At+aScTime,a0@(iOut)
       movew   At+aScTime+2,a0@(iOut+2)
       moveal _eprog+rFired+((""4""-1)*4),a0  /* Points to the recording table */
       movew   At+aScTime,a0@(rOut)
       movew   At+aScTime+2,a0@(rOut+2)
       moveml _context_c,a0	             /* Restore registers */
       andw   #ClrINT_CHANNEL_""4"",TpuIs     /* Clear the TPU channel 'c' interrupt */
       rte
") ;
# 138 "../firmware/isr.c"
   asm("
       .globl _Tpu""5""_Isr
_Tpu""5""_Isr:
       movew   #Tg8DISABLE_INTERRUPTS,SR
       moveml  a0,_context_c              /* Save registers */
       moveal _eprog+iFired+((""5""-1)*4),a0  /* Points to the interr. table */
       movew   At+aScTime,a0@(iOut)
       movew   At+aScTime+2,a0@(iOut+2)
       moveal _eprog+rFired+((""5""-1)*4),a0  /* Points to the recording table */
       movew   At+aScTime,a0@(rOut)
       movew   At+aScTime+2,a0@(rOut+2)
       moveml _context_c,a0	             /* Restore registers */
       andw   #ClrINT_CHANNEL_""5"",TpuIs     /* Clear the TPU channel 'c' interrupt */
       rte
") ;
# 139 "../firmware/isr.c"
   asm("
       .globl _Tpu""6""_Isr
_Tpu""6""_Isr:
       movew   #Tg8DISABLE_INTERRUPTS,SR
       moveml  a0,_context_c              /* Save registers */
       moveal _eprog+iFired+((""6""-1)*4),a0  /* Points to the interr. table */
       movew   At+aScTime,a0@(iOut)
       movew   At+aScTime+2,a0@(iOut+2)
       moveal _eprog+rFired+((""6""-1)*4),a0  /* Points to the recording table */
       movew   At+aScTime,a0@(rOut)
       movew   At+aScTime+2,a0@(rOut+2)
       moveml _context_c,a0	             /* Restore registers */
       andw   #ClrINT_CHANNEL_""6"",TpuIs     /* Clear the TPU channel 'c' interrupt */
       rte
") ;
# 140 "../firmware/isr.c"
   asm("
       .globl _Tpu""7""_Isr
_Tpu""7""_Isr:
       movew   #Tg8DISABLE_INTERRUPTS,SR
       moveml  a0,_context_c              /* Save registers */
       moveal _eprog+iFired+((""7""-1)*4),a0  /* Points to the interr. table */
       movew   At+aScTime,a0@(iOut)
       movew   At+aScTime+2,a0@(iOut+2)
       moveal _eprog+rFired+((""7""-1)*4),a0  /* Points to the recording table */
       movew   At+aScTime,a0@(rOut)
       movew   At+aScTime+2,a0@(rOut+2)
       moveml _context_c,a0	             /* Restore registers */
       andw   #ClrINT_CHANNEL_""7"",TpuIs     /* Clear the TPU channel 'c' interrupt */
       rte
") ;
# 141 "../firmware/isr.c"
   asm("
       .globl _Tpu""8""_Isr
_Tpu""8""_Isr:
       movew   #Tg8DISABLE_INTERRUPTS,SR
       moveml  a0,_context_c              /* Save registers */
       moveal _eprog+iFired+((""8""-1)*4),a0  /* Points to the interr. table */
       movew   At+aScTime,a0@(iOut)
       movew   At+aScTime+2,a0@(iOut+2)
       moveal _eprog+rFired+((""8""-1)*4),a0  /* Points to the recording table */
       movew   At+aScTime,a0@(rOut)
       movew   At+aScTime+2,a0@(rOut+2)
       moveml _context_c,a0	             /* Restore registers */
       andw   #ClrINT_CHANNEL_""8"",TpuIs     /* Clear the TPU channel 'c' interrupt */
       rte
") ;
# 142 "../firmware/isr.c"

 
 
 

   asm(" .globl _Abort_Isr
_Abort_Isr:
	 movew #Tg8DISABLE_INTERRUPTS,SR
         movel #JmpToMonitor,sp@(2)
         rte
         .globl JmpToMonitor
JmpToMonitor: ");
         asm(" movew #Tg8DISABLE_INTERRUPTS,SR ") ;             
         tpu->TpuIe = 0x0;   
	 sim->SimProtect = 0;  
    asm volatile ("
         movel #0x6e12a,a0  /* and start the 332 BUG */
         jmp   a0@");

 
 
 

   asm volatile("
	 .globl _Spurious_Isr
_Spurious_Isr:
	 movel d0,sp@-
	 movel Tg8DSC_INTERRUPT,d0              /* Clear DSC Interrupt */
	 movel sp@+,d0
	 addql #1,At+aNumOfSpur
	 rte
   ");

 
 
 

   asm(" .globl _BusError_Isr
_BusError_Isr:
	 .globl _AddressError_Isr
_AddressError_Isr:
	 .globl _PrivViolation_Isr
_PrivViolation_Isr:
	 addql #1,At+aNumOfBus
	 rte ");

 
 
 

   asm("
	.text
	.globl  _""Dsc""_Isr
_""Dsc""_Isr:  rte") ;
# 193 "../firmware/isr.c"

 
 
 

asm("
 .globl _Default_Isr
 _Default_Isr:
  movew #Tg8DISABLE_INTERRUPTS,SR
  movew sp@(6),ExceptionVector
  movel sp@(2),ExceptionPC
  stop  #Tg8DISABLE_INTERRUPTS
  rte" );

}

void dummyIsr() {
  register Word err;

   asm(" .text");

 
 
 
 
 
 
 
 

   asm (" .globl _Xr_Isr
_Xr_Isr:
	  movew   #Tg8DISABLE_INTERRUPTS,SR
	  moveml  d0-d7/a0-a6,_xr_context" );

   if (!camBusy && !atQueue.Size) {

      

     if (eprog.ImmRun) {  
	 asm (" movew   sp@,_imm_ccr    /* Save CCR */
                movel   sp@(2),_imm_pc  /* Save PC */
	        moveml  d0-d7/a0-a6,_imm_context /* Save registers */
	      ");
       } else {  
	 asm (" movew   sp@,_mbx_ccr    /* Save CCR */
                movel   sp@(2),_mbx_pc  /* Save PC */
	        moveml  d0-d7/a0-a6,_mbx_context /* Save registers */
	      ");
       };
   };

    

   asm(" movel  XWsscRframe1,d0       /* Read frame's word1 and word2 */
	 rorw   #8,d0                 /* Unscramble incomming frame byte order */
	 movew  d0,_timing_frame+2
	 swap   d0
	 rorw   #8,d0
	 movew  d0,_timing_frame
       ");

   if (timing_frame.Any.Header == 0x24) {

      

     dpm->At.aEvent = timing_frame;
     dpm->At.aNumOfEv++;

      

     hist.History_p->hEvent = timing_frame;    
     hist.History_p->hSc = dpm->At.aSc;        
     hist.History_p->hOcc= dpm->At.aScTime;    
      
     *(int*)&hist.History_p->hRcvErr = *(int*)&dpm->At.aDt.aRcvErr;
     if (++hist.History_i >= 1000 ) {
       hist.History_i = 0;
       hist.History_p = hist.Histories;
     } else
       hist.History_p++;  
   };

    

   switch(timing_frame.Any.Header) {

    

   case 0x21:
     goto ret_from_int;

   case Tg8MILLISECOND_HEADER:
      
      

     dpm->At.aMsEvent = timing_frame;
     dpm->At.aScTime++;
     if (++dpm->At.aDt.aMilliSecond > 999) dpm->At.aDt.aMilliSecond = 0;

     err = xlx->WClrRerr;   
     rcv_error.Err = err;
     err = *(Word*)&rcv_error;
     *(Word*)&(dpm->At.aDt.aRcvErr) = err;  

      

     if (atQueue.Size) {
       dpm->At.aAlarms |= Tg8ALARM_MOVED_PROC;
       dpm->At.aMovedFrames = atQueue.Size;
       dpm->At.aMovedScTime = dpm->At.aScTime;
       dpm->At.aMovedSc = dpm->At.aSc;
     } else
       dpm->At.aProcFrames = 0;

     clk.Clock_p->cNumOfMs++;  

     if (rcv_error.Err) {
        
       xlx->WClrRerr = 0;  
       clk.Clock_p->cMsEvent = dpm->At.aMsEvent;    
       clk.Clock_p->cSc = dpm->At.aSc;              
       clk.Clock_p->cOcc= dpm->At.aScTime;          
        
       *(int*)&clk.Clock_p->cRcvErr = *(int*)&dpm->At.aDt.aRcvErr;
        
       if (++clk.Clock_i >= 16 ) {
	 clk.Clock_i = 0;
	 clk.Clock_p = clk.Clocks;
       } else
	 clk.Clock_p++;  
       clk.Clock_p->cNumOfMs = 0;
     };

     if (eprog.ImmSize) {
       doImm = 1;
       break;  
     };

      

     if (in_use.Time_event[timing_frame.Simple.Event_code] &
	 (1<<MILLISECOND_EVENT_INDEX)) break;
     goto ret_from_int;

    
    
    

   case TgvUTC_LOW_HEADER:
     utc = timing_frame.Utc.UtcWord;
     goto ret_from_int;

   case TgvUTC_HIGH_HEADER:
     utc |= ((unsigned long) timing_frame.Utc.UtcWord) << 16;
     utcl = utc;
     UtcToTime(); utc = 0;
     rcv_error.Hour = dpm->At.aDt.aHour;
     dpm->At.aDt.aMsDrift = dpm->At.aDt.aMilliSecond;
     dpm->At.aDt.aMilliSecond = 998;
     goto ret_from_int;

   case ((Tg8SPS << 4) | Tg8SSC_HEADER) :
     if (dpm->At.aSscHeader == timing_frame.Any.Header) {

        

       dpm->At.aScLen = dpm->At.aScTime;
       dpm->At.aScTime = 0;
       dpm->At.aPrNumOfEv = dpm->At.aNumOfEv;
       dpm->At.aNumOfEv = 0;
       dpm->At.aSc = (timing_frame.Ssc.Scn_low)      +
		     (timing_frame.Ssc.Scn_mid << 8) +
		     (timing_frame.Ssc.Scn_msb << 16);
       dpm->At.aNumOfSc++;
     }
   break;

   case ((Tg8SPS << 4) | Tg8SIMPLE_EVENT_HEADER) :
     { if (timing_frame.Simple.Event_code == 0xFE ) { asm(" moveal #_tel+next""SPS"",a0
		     moveal #At+aTeleg""SPS"",a1
		     movel  #Tg8GROUPS/2-1,d0 /* telegram size in longwords */
		  0: movel  a0@+,a1@+
		     dbf    d0,0b"); }; } ;
# 373 "../firmware/isr.c"
   break;

   case ((Tg8CPS << 4) | Tg8SIMPLE_EVENT_HEADER) :
   case ((Tg8PSB << 4) | Tg8SIMPLE_EVENT_HEADER) :
   case ((Tg8LEA << 4) | Tg8SIMPLE_EVENT_HEADER) :
   case ((Tg8LHC << 4) | Tg8SIMPLE_EVENT_HEADER) :
     goto ret_from_int;

    
    
    

   case ((Tg8SPS << 4) | Tg8TELEGRAM_HEADER) :
      if (timing_frame.Telegram.Group_number <= 24 ) tel.nextSPS[timing_frame.Telegram.Group_number-1] = timing_frame.Telegram.Group_value; ;
      goto ret_from_int;

   case ((Tg8LHC << 4) | Tg8TELEGRAM_HEADER) :
   case ((Tg8CPS << 4) | Tg8TELEGRAM_HEADER) :
   case ((Tg8PSB << 4) | Tg8TELEGRAM_HEADER) :
   case ((Tg8LEA << 4) | Tg8TELEGRAM_HEADER) :
      goto ret_from_int;

   default:  

     switch (timing_frame.Any.Header >> 4) {  

     case Tg8SPS:
       if (!(in_use.Simple_event[timing_frame.Simple.Event_code] & (1<<Tg8SPS))) goto ret_from_int; ;
       break;

     case Tg8NO_MACHINE:
       if (in_use.Time_event[timing_frame.Simple.Event_code] &
	   (1<<ANY_OTHER_EVENT_INDEX)) break;

     default:
       break;  
     };
     break;  

   }   

    
    
    
    

   if (atQueue.Size>=4) {
     dpm->At.aAlarms |= Tg8ALARM_QUEUED_PROC;
     if (atQueue.Size >= 32 ) {
       dpm->At.aAlarms |= Tg8ALARM_MANY_PROC;
       goto ret_from_int;
     };
   };

   dpm->At.aQueue[atQueue.Size] = timing_frame;

   atQueue.Size++;
   *atQueue.Top++ = timing_frame.Long;  

   if (immQueue.Size >= 32 )
     dpm->At.aAlarms |= Tg8ALARM_IMMQ_OVF;
   else {
     immQueue.Size++;
     *immQueue.Top++ = timing_frame.Long;  
   };

   dpm->At.aQueueSize = atQueue.Size;
   if (atQueue.Size > dpm->At.aMaxQueueSize) dpm->At.aMaxQueueSize = atQueue.Size;

   if (atQueue.Size == 1 && !camBusy) {
       
      asm volatile ("
      movel  #_AtProcess,sp@(2)
      rte " );
   };

ret_from_int:

    

   asm volatile ("
   moveml  _xr_context,d0-d7/a0-a6
   rte ");

 
 
 
 

   asm(" .globl _AtStartProcess
_AtStartProcess:
	 movew  #Tg8DISABLE_INTERRUPTS,SR    /* Mask all interrupts  */
         movew   sp@,_imm_ccr   /* Save CCR */
         movel   sp@(2),_imm_pc /* Save PC */
	 moveml  d0-d7/a0-a6,_imm_context /* Save registers */
         movel  #_AtProcess,sp@(2) /* Serve the next entry in the AT queue */
	 rte
   ");

 
 
 
 
 
 
 

   asm(" .globl _AtCompletion
_AtCompletion:
	 movew  #Tg8DISABLE_INTERRUPTS,SR  ");   

   if (--atQueue.Size == 0) {   
      atQueue.Top = atQueue.Bottom = atQueue.Buffer;

      if (eprog.ImmRun) {  
	                   
	asm volatile (" movew  _imm_ccr,sp@
		        movel  _imm_pc,sp@(2)
		        moveml _imm_context,d0-d7/a0-a6
		        rte");
      };

       

      asm volatile ("
         movew  #1,_eprog+ImmRun
         movel  #_ImmProcess,sp@(2) /* Serve the next entry in the IMM queue */
	 rte");
   };

    

   atQueue.Bottom++;  
   asm volatile (" movel  #_AtProcess,sp@(2)
		   rte");

 
 
 

   asm volatile (".globl _AtProcess
_AtProcess: ");

at_proc:
   eprog.Event.Long = *atQueue.Bottom;

    

   if (eprog.Event.Any.Header != 0x01) {  
     eprog.WildCard = in_use.Simple_wc[eprog.Event.Simple.Event_code] &
                      in_use.SimpleH_wc[eprog.Event.Any.Header];
   } else {  
     eprog.WildCard = in_use.Time_wc[eprog.Event.Simple.Event_code] &
                      in_use.TimeH_wc[eprog.Event.Any.Header];
   };

   asm ("
   /* For each wild card combination present scan the CAM */

   movew   _eprog+WildCard,d4    /* d4 is the wildcards combination mask */
   beq     5f                    /* An event is not in use - escape */

   movel   _eprog+Event,d1	 /* d1 is the queued timing frame */
   lea     _wild_c,a0            /* a0 points to the wildcards */
   moveal  _cam,a1               /* a1 points to the CAM register */
   moveal  #_match+Matches,a2	 /* a2 points to the resulting list      */

0: btst    #0,d4                 /* Is next wildcard presents? */
   beq     3f                    /* No, try next */

   /* Look for an event */

   movel   d1,d2                  /* We will OR the timing frame with    */
   orl     a0@,d2                 /* the wildcard bit mask               */
   movel   d2,_eprog+Event        /* to the comparand register in 3 steps*/

   /* Load the template for the CAM */

   oriw    #CamDATA_MODE,SimDataF1  /* Set Data mode for CAM             */
   movew   _eprog+Event,a1@         /* Load the first 16 bits            */
   movew   _eprog+Event+2,a1@       /* Load the second 16 bits           */
   movew   #0xFF,a1@		    /* Load 3rd word (0xFF for chan. output) */

   /* Start the CAM looking for the template */
   andw    #CamCOMMAND_MODE,SimDataF1 /* Set Command mode for CAM */

   /* Wait for the match to be completed while setting command mode */
1: movel   d0,d0                   /* The matching is in progress: wait   */
   movew   a1@,d0                  /* Read the CAM status                 */
   blt     2f                      /* If no more matches, escape          */

   /* For each match place the action number in resulting buffer (Matches)*/

   moveb   d0,a2@+                 /* Put it to the resulting list        */
   btst    #CamSTATUS_MULTIPLE,d0  /* Multiple matches ?                  */
   bne     2f                      /* No -> break                         */
   andiw   #CamBYTE_MASK,d0        /* Which is the action number          */
   oriw    #CamSET_SKIP_BIT,d0     /* Set the Skip bit for the match      */
   movew   d0,a1@                  /* And look for the next match         */
   bra     1b

   /* No more matches for that wildcard - stop the CAM work */

2: movew   #CamCLEAR_SKIP_BITS,a1@  /* Clear all skip bits */

  /* Try with the next wildcard type */

3: lsrw    #1,d4                /* Try the next one */
   tstw    d4
   beq     4f                   /* No more wildcards at all, escape */

   addql   #4,a0                /* The next wildcard combination */
   bra     0b                   /* Loop ... */

   /* Start all the actions we have prepared in the matching list */
4:
   cmpal   #_match+Matches,a2	/* a2 points to the resulting list */
   beq     5f

   movel   a2,_match+EndMa     	/* Set the End of list (match.End)	*/
   bsr     _StartActions	/* Post-processing of the list of started actions */

5: ");

    

   dpm->At.aProcFrames++;
   dpm->At.aQueueSize--;

   if (atQueue.Size > 1) {
     atQueue.Size--;
     atQueue.Bottom++;
     goto at_proc;    
   };

asm(" trap #0 ");  

}  

 
 
 

void StartActions() {
Tg8Action    *a ,*aa;
Tg8Interrupt *ip;
Tg8IntAction  u ,uu;
int j; Byte *alist; short actn, cn, mach,gt,gv;

  for (alist = match.Matches; alist != match.EndMa; alist++) {

    actn = *alist;
    a = act.Pointers[actn];    

# 654 "../firmware/isr.c"

      if (!(a->Enabled &Tg8ENABLED)) continue;  

     

    cn = a->CntNum;
    xlx->XlxDelay [cn] = a->User.uDelay;
    xlx->XlxConfig[cn] = a->CntControl;

     

    ip = (Tg8Interrupt *) &dpm->It.CntInter[cn];
    eprog.iFired[cn] = ip;        
    eprog.rFired[cn] = &a->Rec;   

     
     

    a->Rec.rSc = dpm->At.aSc;      
    a->Rec.rOcc= dpm->At.aScTime;  
    a->Rec.rOut= 0xFFFFFFFF;
    a->Rec.rNumOfTrig++;           

    {((short*)&( u ))[0]=((short*)&( ip->iExt ))[0];((short*)&( u ))[1]=((short*)&( ip->iExt ))[1];} ;                
    if (ip->iOut == 0xFFFFFFFF)    
      { eprog.Alarms |= ( Tg8ALARM_LOST_OUT ); dpm->At.aAlarms |= eprog.Alarms; dpm->At.aFwStat |= Tg8FS_ALARMS; } ;

    if (u.iSem && (a->CntControl & ConfINTERRUPT)) {
       
      eprog.iFired[cn] = &sim_int;  
      a->Rec.rOvwrAct = u.iAct;     
      a->Rec.rOvwrCnt++;            
      { eprog.Alarms |= ( Tg8ALARM_INT_LOST ); dpm->At.aAlarms |= eprog.Alarms; dpm->At.aFwStat |= Tg8FS_ALARMS; } ;
      continue;
    };

     
     

    ip->iEvent.Long = *atQueue.Bottom;  
    ip->iSc = dpm->At.aSc;              
    ip->iOcc= dpm->At.aScTime;          
    ip->iOut= 0xFFFFFFFF;               
    uu.iRcvErr = rcv_error.Err;         
    uu.iOvwrAct = 0;                    

    uu.iAct = actn+1;                   
    uu.iSem = (a->CntControl & ConfINTERRUPT)? 1: 0;  
    {((short*)&( ip->iExt ))[0]=((short*)&( uu ))[0];((short*)&( ip->iExt ))[1]=((short*)&( uu ))[1];} ;                    
  };

}  

 
# 658 "../firmware/tg8.c" 2

# 1 "../firmware/immproc.c" 1
 
 
 
 

void dummyImmEtc() {

 
 
 
 
 
 
 

   asm(" .globl _ImmCompletion
_ImmCompletion:
	 movew  #Tg8DISABLE_INTERRUPTS,SR  ");   

   if (--immQueue.Size == 0) {   
      immQueue.Top = immQueue.Bottom = immQueue.Buffer;
      eprog.ImmRun = 0;
      asm volatile (" movew  _mbx_ccr,sp@
		      movel  _mbx_pc,sp@(2)
		      moveml _mbx_context,d0-d7/a0-a6
		      rte");
   };

    

   immQueue.Bottom++;  
   asm volatile (" movel  #_ImmProcess,sp@(2)
		   rte");

 
 
 
 

   asm volatile (".globl _ImmProcess
_ImmProcess: ");

imm_proc:

   eprog.ImmEvent.Long = *immQueue.Bottom;

    
    
    

   if (doImm  ) {
     dpm->At.aTrace = eprog.ImmSize;
     if (eprog.IntSrc & (1<<Tg8IS_IMM) ) {
       dpm->At.aAlarms |= Tg8ALARM_MOVED_IMM;
        
     } else {
        
       memcpy16((short*)dpm->It.ImmInter,(short*)eprog.ImmInt,
		eprog.ImmSize * sizeof(Tg8Interrupt));
       eprog.IntSrc |= 1<<( Tg8IS_IMM ); ;
       if (!dpm->At.aIntSrc) {  
	  
	 asm volatile ("trap  #3" );
       };
       eprog.ImmSize = eprog.ImmNxt = 0;
     };
     doImm = 0;
   };

    

   if (eprog.ImmEvent.Any.Header != 0x01) {  
     eprog.ImmWildCard = in_use.Simple_wc_imm[eprog.ImmEvent.Simple.Event_code] &
                         in_use.SimpleH_wc[eprog.Event.Any.Header];
   } else {
     eprog.ImmWildCard = in_use.Time_wc_imm[eprog.ImmEvent.Simple.Event_code] &
                         in_use.TimeH_wc[eprog.Event.Any.Header];
   };

   asm ("
   /* For each wild card combination present scan the CAM */

   moveal  #_match+ImmMatches,a2 /* a2 points to the resulting list      */
   movew   _eprog+ImmWildCard,d4 /* d4 is the wildcards combination mask */
   beq     5f                    /* An event is not in use - escape */

   movel   _eprog+ImmEvent,d1	 /* d1 is the queued timing frame */
   lea     _wild_c,a0            /* a0 points to the wildcards */
   moveal  _cam,a1               /* a1 points to the CAM register */

0: btst    #0,d4                 /* Is a wildcard present? */
   beq     3f                    /* No, try next */

   /* Look for an event */

   movel   d1,d2                  /* We will OR the timing frame with    */
   orl     a0@,d2                 /* the wildcard bit mask               */
   movel   d2,_eprog+ImmEvent     /* to the comparand register in 3 steps*/

   /* Load the template for the CAM. Interrupts OFF */

   movew   #1,_camBusy              /* Protect the CAM access for the AT-process */

   oriw    #CamDATA_MODE,SimDataF1  /* Set Data mode for CAM             */
   movew   _eprog+ImmEvent,a1@      /* Load the first 16 bits            */
   movew   _eprog+ImmEvent+2,a1@    /* Load the second 16 bits           */
   movew   #0x00,a1@		    /* Load 3rd word (ZERO for imm.action) */

   /* Start the CAM looking for the template */
   andw    #CamCOMMAND_MODE,SimDataF1 /* Set Command mode for CAM */

   /* Wait for the match to be completed while setting command mode */

1: movel   d0,d0                   /* The matching is in progress: wait   */
   movew   a1@,d0                  /* Read the CAM status                 */
   blt     2f                      /* If no more matches, escape          */

   /* For each match place the action number in resulting buffer (Matches)*/

   moveb   d0,a2@+                 /* Put it to the resulting list        */
   btst    #CamSTATUS_MULTIPLE,d0  /* Multiple matches ?                  */
   bne     2f                      /* No -> break                         */
   andiw   #CamBYTE_MASK,d0        /* Which is the action number          */
   oriw    #CamSET_SKIP_BIT,d0     /* Set the Skip bit for the match      */
   movew   d0,a1@                  /* And look for the next match         */
   bra     1b

   /* No more matches for that wildcard - stop the CAM work. Interrupts ON */

2: movew   #CamCLEAR_SKIP_BITS,a1@  /* Clear all skip bits */

   /* Start the AT-process if the frames queue is not empty now */

   movew   #Tg8DISABLE_INTERRUPTS,SR 
   tstl    _atQueue+Size
   beq     5f                   /* No pending frames, continue */

   movew   #0,_camBusy          /* Allow the CAM access for the AT-process */
   trap    #4                   /* and start the AT-process. */

5: movew   #0,_camBusy          /* Allow the CAM access for the AT-process */
   movew  #Tg8ENABLE_INTERRUPTS,SR 

  /* Try with the next wildcard type */

3: lsrw    #1,d4                /* Try the next one */
   tstw    d4
   beq     4f                   /* No more wildcards at all, escape */

   addql   #4,a0                /* The next wildcard type */
   bra     0b                   /* Loop ... */
4:
   cmpal   #_match+ImmMatches,a2 /* a2 points to the resulting list */
   beq     5f

   movel   a2,_match+ImmEndMa    /* Set the End of list (match.End) */
   bsr     _StartImmActions	 /* Post-processing of the list of started actions */

5: ");

    
    
    

   if (immQueue.Size > 1) {
     immQueue.Size--;
     immQueue.Bottom++;
     goto imm_proc;    
   };

   

   dpm->At.aSem++;

   asm(" trap #5 ");  
}

 
 
 
 

  static Tg8Action    *a,*aa;
  static Tg8Interrupt *ip;
  static Tg8IntAction  u,uu; static Word actn, dim,mach,gt,gv;
  static Byte *alist;

void StartImmActions() {

   for (alist = match.ImmMatches; alist < match.ImmEndMa; alist++) {

     actn = *alist;
     a = act.Pointers[actn];              

# 222 "../firmware/immproc.c"

      if (!(a->Enabled &Tg8ENABLED)) continue;  

     ip = &eprog.ImmInt[eprog.ImmNxt++];  
     if (eprog.ImmSize < 8 ) {
       eprog.ImmSize++;
       ip->iExt.iSem = 0;     
     } else {
        
       { eprog.Alarms |= ( Tg8ALARM_LOST_IMM ); dpm->At.aAlarms |= eprog.Alarms; dpm->At.aFwStat |= Tg8FS_ALARMS; } ;
       if (eprog.ImmNxt > 8 ) {
	 eprog.ImmNxt = 0;    
	 ip = eprog.ImmInt;
       };
     };

      

     a->Rec.rSc = dpm->At.aSc;       
     a->Rec.rOcc= dpm->At.aScTime;   
     a->Rec.rOut= a->Rec.rOcc;       
     a->Rec.rNumOfTrig++;            

     {((short*)&( u ))[0]=((short*)&( ip->iExt ))[0];((short*)&( u ))[1]=((short*)&( ip->iExt ))[1];} ;                
     uu.iRcvErr = rcv_error.Err;    
     if (u.iSem) {
        
       a->Rec.rOvwrAct = u.iAct;   
       a->Rec.rOvwrCnt++;          
       { eprog.Alarms |= ( Tg8ALARM_INT_LOST ); dpm->At.aAlarms |= eprog.Alarms; dpm->At.aFwStat |= Tg8FS_ALARMS; } ;
       continue;
     };
# 262 "../firmware/immproc.c"

       uu.iOvwrAct = 0;            

     ip->iEvent.Long = *immQueue.Bottom;  
     ip->iSc = dpm->At.aSc;        
     ip->iOcc= dpm->At.aScTime;    
     ip->iOut= ip->iOcc;           
     uu.iAct = actn+1;             
     uu.iSem = 1;                  
     {((short*)&( ip->iExt ))[0]=((short*)&( uu ))[0];((short*)&( ip->iExt ))[1]=((short*)&( uu ))[1];} ;              

   }; 
}

 
# 659 "../firmware/tg8.c" 2



 
