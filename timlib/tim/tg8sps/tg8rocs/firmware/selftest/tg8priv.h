/****************************************************/
/* Local declarations used by the firmware program. */
/* V. Kovaltsov for SL Timing, April, 1997	    */
/****************************************************/

/********************************************************************************/
/* Full action's descriptor (its fields are used by 'asm' part of the firmware) */
/********************************************************************************/

typedef struct {
   Tg8User      User;       /* Action's configuration parameters */
   Word         CntControl; /* XILINX counter control word calc. from user's cw */
   Byte         CntNum;     /* XILINX counter number */
   Byte         Enabled;    /* Enabled/Disabled state */
   Tg8Recording Rec;        /* Recording data */
} Tg8Action;

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

#define QUEUE_SIZE 8

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
  Uint	         IntSrc;          /* Pending interrupt source mask */
  Tg8Event       Event;	          /* Event code used by the AtProcess */
  Word           WildCard;        /* The current wildcard for the AT-process */
  Word	         Alarms;          /* Last alarms bitmask */
  Word           CamBusy;         /* CAM busy flag (0=free) */
  Tg8Event       ImmEvent;	       /* Event code for imm. actions */
  Word           ImmWildCard;          /* The current wildcard for imm. action */
  Word           ImmSize,              /* The imm. list size */
                 ImmNxt,               /* The next free location index in the pImmInt */
                 ImmBot,               /* Next index in the actions list */
                 ImmTop;               /* Actions list top */
  Tg8Event       ImmEvents[Tg8ACTIONS];/* Action's triggers */
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

typedef enum {	MILLISECOND_EVENT_INDEX = 1,
		TIME_EVENT_INDEX,
		DATE_EVENT_INDEX
	     } EventIndex;

/* eof */
