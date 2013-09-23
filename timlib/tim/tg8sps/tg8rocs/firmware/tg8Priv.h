/****************************************************/
/* Local declarations used by the firmware program. */
/* V. Kovaltsov for SL Timing, April, 1997	    */
/****************************************************/

/*****************************/
/* Full action's descriptor  */
/*****************************/

typedef struct {
  /* User's data */
   Tg8User User;       /* Action's configuration parameters */
   Tg8Gate Gate;       /* PLS condition */
  /* Internal data used for the counter configuration on fly */
   Word    CntControl; /* XILINX counter control word calculated from user's cw */
   Byte    CntNum;     /* XILINX counter number */
   Byte    Enabled;    /* Enabled(1)/Disabled(0) state (LSB,1bit) & Dimension */
  /* History recording data */
   Tg8Recording Rec;   /* Recording data */
} Tg8Action;

/* NOTE.
   ====
   There is the single entry {trigger, cnt/imm} in the CAM for the PPM actions
   with the same set of {trigger, machine, group number} !! It points to the
   first actions from the array of actions corresponding to the PPM action set !!
   Assume that all lines of the PPM action are sequential in the user table.
*/

/********************/
/* Telegram buffers */
/********************/

typedef struct {
  Word *aTeleg[Tg8MACHINES]; /* The array of pointers to the current
				telegrams in the DPRAM (NULL,SPS,...) */
  Word  nextLHC [Tg8GROUPS]; /* The next LHC telegram */
  Word  nextSPS [Tg8GROUPS]; /* The next SPS telegram */
  Word  nextCPS [Tg8GROUPS]; /* The next CPS telegram */
  Word  nextPSB [Tg8GROUPS]; /* The next PSB telegram */
  Word  nextLEA [Tg8GROUPS]; /* The next LEA telegram */
} InterTelegram;

/*************************/
/* Internal Action Table */
/*************************/

typedef struct {
  Tg8Action *Pointers[Tg8ACTIONS]; /* Pointers to action descriptions (next fild) */
  Tg8Action  Table[Tg8ACTIONS];	   /* Full action descriptions */
} InterActTable;

/*******************************/
/* The list of started actions */
/*******************************/

typedef struct {
  Byte	* EndMa,       		   /* The end of list of actions to start */
        * ImmEndMa,                /* --"-- for immediate actions */
          Matches [Tg8ACTIONS],    /* List of matches in the CAM for AT-proc. */
          ImmMatches [Tg8ACTIONS]; /* --"-- for immediate actions */
} StartedAct;

/*****************************/
/* The actions process queue */
/*****************************/

#define QUEUE_SIZE 32

typedef struct {
   Uint     *Bottom, /* Current running entry */
	    *Top,    /* First empty queue location */
	    *End;    /* Queue end address */
   int	    Size;    /* Number of AT-processes on queue */
   Uint     Buffer[QUEUE_SIZE]; /* Queue of AT-processes */
} Queue;

/****************************/
/* The internal clock table */
/****************************/

typedef struct {
  short     N_of_clocks,      /* Number of used entries */
            Clock_i;          /* The current entry index */
  Tg8Clock *Clock_p;          /* The pointer to the current entry */
  Tg8Clock  Clocks[Tg8CLOCKS];/* Entries */
} InterClockTable;

/************************************/
/* The internal event history table */
/************************************/

typedef struct {
  short       N_of_histories, /* Number of used entries */
              History_i;      /* The current entry index */
  Tg8History *History_p;      /* The pointer to the current entry */
  Tg8History  Histories[Tg8HISTORIES]; /* Entries */
} InterHistoryTable;

/*****************************************************/
/* The current state of the Event Processing Program */
/*****************************************************/

typedef struct {
  Word	         IntSrc;          /* Pending interrupt source mask */
  Word	         Alarms;          /* Last alarms bitmask */

  Tg8Event       Event;	          /* Event code used by the AtProcess */
  Tg8Event       ImmEvent;	  /* Event code for imm. actions */
  Word           WildCard;        /* The current wildcard for the AT-process */
  Word           ImmWildCard;     /* The current wildcard for imm. action */

  Word           ImmRun;          /* The IMM process is running */
  Word           ImmSize,         /* The imm. list size */
                 ImmNxt;          /* The next free location index in the pImmInt */
  Tg8Interrupt   ImmInt[Tg8IMM_INTS];  /* Pending Immediate interrupts queue */

  Tg8Interrupt  *iFired[Tg8COUNTERS];  /* Fired actions reserve interrupt rows */
  Tg8Recording  *rFired[Tg8COUNTERS];  /* Fired actions reserve recording rows */
} EProg;

/******************************************/
/* Constants referenced from the asm-code */
/******************************************/

#define SUBSET_MASK      0xf0000000  /* Mask on event from any subset */
#define WILDCARDS_NUMBER 8           /* Number of possible wildcards */

/**************************************************************************/
/* Time stamp format							  */
/**************************************************************************/

typedef struct {
  Byte   tsHour,     /* The current hour, */
         tsMinute,   /* minute, */
         tsSecond;   /* second */
} Tg8TimeStamp;

typedef struct {
  Uint   stScNum;    /* The Super Cycle number */
  Uint   stMs;       /* The time from the SSC event */
} Tg8ScTime;

/********************************************************************/
/* The definitions to refer to the same locations in different ways */
/********************************************************************/

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

/*************************************************************************/

typedef enum {  ANY_OTHER_EVENT_INDEX = 0,
                MILLISECOND_EVENT_INDEX = 1,
		TIME_EVENT_INDEX = 2,
		DATE_EVENT_INDEX = 3
	     } EventIndex;

/* eof */
